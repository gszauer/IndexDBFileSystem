/* IndexDBFileSystem https://www.youtube.com/watch?v=8UFOSQXT_pg
constructor(databaseName: string, onSuccess: function, onError: function)
-> onSuccess(self: IndexDBFileSystem): void
-> onError(errorMessage: string): void

Write(fileName: string, fileContent: blob, OnSuccess: function, OnError: function): void
-> OnSuccess(fileName: string, bytesWritten: int): void
-> OnError(errorMessage: string): void

Read(fileName: string, OnSuccess: function, OnError: function): void
-> OnSuccess(filePath: string, fileContent: blob): void
-> OnError(errorMessage: string) : void

CreateFile(path: string, OnSuccess: function, OnError: function): void
-> OnSuccess(fileName: string): void
-> OnError(errorMessage: string): void

CreateFolder(path: string, OnSuccess: function, OnError: function): void
-> OnSuccess(path: string): void
-> OnError(error: string): void

Delete(path: string, OnSuccess: function, OnError: function): void
-> OnSuccess(path: string): void
-> OnError(error: string): void

IsDirectory(path: string, callback: function): void
-> callback(path: string, isDirectory: bool): void

IsFile(path: string, callback: function): void
-> callback(path: string, isFile: bool): void

Exists(path: string, callback: function): void
-> callback(path: string, isDirectory: bool, isFile: bool): void

DepthFirstTraversal(path: string, callback: function, finished: function): void
-> callback(path: string, depth: int, isDirectory: bool, isFile: bool): void
-> finished(path: string): void

Watch(path: string, OnChanged: function): WatchToken
-> OnChanged(path: string, isFile: bool, isFolder: bool): void

Unwatch(path: string): void
Unwatch(token: Watchtoken): void

IsWatching(path: string): bool
*/


class IndexDBFileSystem {
    // OnError(errorMsg: string): void
    // OnSuccess(self : IndexDBFileSystem) : void
    constructor(databaseName, _OnSuccess, _OnError) {
        let self = this;
        this.DatabaseName = databaseName;
        this.IsReady = false;
        this.Database = null;
        this.Index = null;
        this.Silent = false;
        this.FileWatch = {};

        this.WasmFsPointer = null;
        this.WasmFsExports = null;
        this.WasmFsMemory = null;

        let callbackIssued = false;
        const OnSuccess = function(arg) {
            self.IsReady = true;
            if (callbackIssued) {
                self.logError("Triggering callback multiple times");
            }
            callbackIssued  = true;
            if (_OnSuccess) {
                _OnSuccess(arg);
            }
        }
        const OnError = function(arg) {
            self.IsReady = false;
            if (callbackIssued) {
                self.logError("Triggering callback multiple times");
            }
            callbackIssued  = true;
            if (_OnError) {
                _OnError(arg);
            }
        }
        

        const indexedDB = window.indexedDB || window.webkitIndexedDB || window.mozIndexedDB || window.OIndexedDB || window.msIndexedDB;
        let request = indexedDB.open(databaseName, 2);

        const CreateIndexObjectIfNeeded = function(OnSuccess, OnError) {
            self.Database.onerror = function (event) {
                self.logError("Error accessing IndexedDB (" + databaseName + ")");
                if (OnError) {
                    OnError("Error accessing IndexedDB (" + databaseName + ")");
                }
            };

            let readTransaction = self.createIndexReadTransaction();
            let get = readTransaction.get("index");
            get.onsuccess = function(event) {
                if (event.target.result) {
                    self.Index = event.target.result;
                    if (OnSuccess) {
                        OnSuccess(self);
                    }
                }
                else {
                    let writeTransaction = self.createIndexWriteTransaction();
                    let root = self.createMetaDataObject();
                    root.isDirectory = true;
                    root.children = [];
                    root.path = "/";
                    root.prettyPath = "/";
                    let put = writeTransaction.put(root, "index",
                    function(path) {
                        self.Index = root;
                        if (OnSuccess) {
                            OnSuccess(self);
                        }
                    },
                    function(error) {
                        self.logError("Error creating file index meta data");
                        if (OnError) {
                            OnError("Error creating file index meta data");
                        }
                    });
                    
                    writeTransaction.commit();
                }
            }
            get.onerror = function(event) {
                self.logError("Error accessing file index meta data");
                if (OnError) {
                    OnError("Error accessing file index meta data");
                }
            }
            readTransaction.commit();
        }
       
        request.onerror = function(event) {
            self.logError("Failed to open database (" + databaseName + ")");
            if (OnError) {
                OnError("Failed to open database (" + databaseName + ")");
            }
        }
        request.onsuccess = function (event) {
            self.logConsole("Created IndexedDB (" + databaseName + ")");
            self.Database = request.result;
            
            // This will eventually call OnSuccess
            CreateIndexObjectIfNeeded(OnSuccess, OnError); 
        }
        request.onupgradeneeded = function (event) {
            self.logConsole("Updated IndexedDB (" + databaseName + ")");

            let db = event.target.result;

            if (!db.objectStoreNames.contains("Files")) {
                db.createObjectStore("Files");
            }
            if (!db.objectStoreNames.contains("MetaData")) {
                db.createObjectStore("MetaData");
            }

            self.Database = event.target.result;
        }
    }

    createFileReadTransaction(OnSuccess, OnError) {
        return this.createTransaction("Files", "readonly", OnSuccess, OnError);
    }

    createFileWriteTransaction(OnSuccess, OnError) {
        return this.createTransaction("Files", "readwrite", OnSuccess, OnError);
    }

    createIndexReadTransaction(OnSuccess, OnError) {
        return this.createTransaction("MetaData", "readonly", OnSuccess, OnError);
    }

    createIndexWriteTransaction(OnSuccess, OnError) {
        return this.createTransaction("MetaData", "readwrite", OnSuccess, OnError);
    }

    triggerFolderWatchCallbacks(path, isFolder) { 
        path = path.toLowerCase();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }

        let self = this;

        // Callback for folder
        if (self.FileWatch.hasOwnProperty(path)) {
            let watchCallbacks = self.FileWatch[path];
            let numCallbacks = watchCallbacks.length;
            for (let i = 0; i < numCallbacks; ++i) {
                watchCallbacks[i](path, false, isFolder);
            }
        }

        if (!isFolder) { // Folder was deleted, clear all callbacks!
            if (self.FileWatch.hasOwnProperty(path)) {
                self.FileWatch[path] = [];
            }
        }
    }

    triggerFileWatchCallbacks(path, isFile) { 
        path = path.toLowerCase();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }

        let self = this;

        // Callback for file
        if (self.FileWatch.hasOwnProperty(path)) {
            let watchCallbacks = self.FileWatch[path];
            let numCallbacks = watchCallbacks.length;
            for (let i = 0; i < numCallbacks; ++i) {
                watchCallbacks[i](path, isFile, false);
            }
        }

        const TriggerParentDirCallback = function() { // Never called
            for (let dIter = self.getParentDirectoryFromPathString(path);
                true; dIter = self.getParentDirectoryFromPathString(dIter)) {

                if (self.FileWatch.hasOwnProperty(dIter)) {
                    let watchCallbacks = self.FileWatch[dIter];
                    let numCallbacks = watchCallbacks.length;
                    for (let i = 0; i < numCallbacks; ++i) {
                        watchCallbacks[i](dIter, false, true);
                    }
                }

                if (dIter == "") {
                    break;
                }

                // I decided that it only makes sense to trigger the callback for parent, not recursivley.
                // Keeping the loop in tact in case i change my mind.
                break;
            }
        }

        if (!isFile) { // File was deleted, clear all callbacks!
            if (self.FileWatch.hasOwnProperty(path)) {
                self.FileWatch[path] = [];
            }
        }
    }

    // OnSuccess(indexDbTransaction) : void
    // OnError(error: string) : void
    createTransaction(table, readwritemode, OnSuccess, OnError) {
        let self = this;
        let transaction = self.Database.transaction(table, readwritemode);
        transaction.oncomplete = function(event) {
            if (OnSuccess) {
                OnSuccess(transaction);
            }
        }
        transaction.onerror = function(event) {
            self.logError("Failed to create database transaction");
            if (OnError) {
                OnError("Failed to create database transaction");
            }
        }
        
        let store = transaction.objectStore(table);
        let result = {};
        result.transaction = transaction;
        result.store = store;
        result.commit = function() {
            transaction.commit();
        }
        result.get = function(key) { // TODO: Add success and error callbacks (like put)
            return store.get(key);
        }
        result.put = function(data, key, OnSuccess, OnError) {
            let req = store.put(data, key);
            req.onsuccess = function(event) {
                if (OnSuccess) {
                    OnSuccess(key);
                }
            }
            req.onerror = function(event) {
                if (OnError) {
                    OnError("Failed put: " + key);
                }
            }
            return req;
        }
        result.delete = function(key) { // TODO: Add success and error callbacks (like put)
            return store.delete(key);
        }

        return result;
    }

    logConsole(str) {
        if (!this.Silent) {
            console.log(str);
        }
    }

    logError(str) {
        if (!this.Silent) {
            console.error(str);
        }
    }

    createMetaDataObject() {
        let result = {};

        result.name = "";
        result.path = "";
        result.prettyPath = "";
        result.isFile = false;
        result.isDirectory = false;
        result.children = null;
        result.parent = null;

        return result;
    }

    getParentDirectoryFromPathString(path) {
        path = path.toLowerCase();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }

        let path_arr = path.split('/').filter(function(n) { return n !== '';});
        let parent = "";

        let size =  path_arr.length - 1;
        for (let i = 0; i < size; ++i) {
            parent += "/" + path_arr[i];
        }

        if (!parent.startsWith("/")) {
            parent = "/" + parent;
        }
        return parent;
    }

    getIndexObjectFromPathString(path) {
        path = path.toLowerCase();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }

        let path_arr = path.split('/').filter(function(n) { return n !== '';});;
        let iter = this.Index;

        let i_len = path_arr.length;
        for (let i = 0; i < i_len; ++i) {
            let found = false;
            let j_len = iter.children.length;
            for (let j = 0; j < j_len; ++j) {
                if (iter.children[j].name == path_arr[i]) {
                    found = true;
                    iter = iter.children[j];
                    break;
                }
            }
            if (!found) {
                return null;
            }
        }

        return iter;
    }

    unlinkIndexObjectFromParent(obj) {
        if (obj.parent != null) {
            let i_len = obj.parent.children.length;
            for (let i = 0; i < i_len; ++i) {
                if (obj.parent.children[i].path == obj.path) {
                    obj.parent.children.splice(i, 1);
                    break;
                }
            }
            obj.parent = null;
        }
    }

    // No args to callbacks
    updateIndexObject(OnSuccess, OnError) {
        let self = this;
        let writeTransaction = self.createIndexWriteTransaction();
        writeTransaction.put(self.Index, "index", 
            function(path) {
                if (OnSuccess) {
                    OnSuccess();
                }
            },
            function(error) {
                if (callback) {
                    OnError();
                }
            }
        );
        writeTransaction.commit();
    }

    createFileMetaData(path, callback) {
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        let original_case = path;
        path = path.toLowerCase();

        let self = this;

        let path_arr = path.split('/').filter(function(n) { return n !== '';});
        let original_arr = original_case.split('/').filter(function(n) { return n !== '';});
        let iter = self.Index;

        let i_len = path_arr.length - 1;
        for (let i = 0; i < i_len; ++i) {
            let j_len = iter.children.length;
            let found = false;

            for (let j = 0; j < j_len; ++j) {
                if (iter.children[j].name == path_arr[i]) {
                    found = true;
                    iter = iter.children[j];
                    break;
                }
            }

            if (!found) { // Need to create directory
                let this_path = "";
                let pretty_path = "";
                for (let k = 0; k < i; ++k) {
                    this_path += "/" + path_arr[k];
                    pretty_path += "/" + original_arr[k];
                }

                let new_dir = self.createMetaDataObject();
                new_dir.parent = iter;
                iter.children.push(new_dir);
                new_dir.children = [];
                new_dir.isDirectory = true;
                new_dir.path = this_path;
                new_dir.prettyPath = pretty_path;
                new_dir.name = path_arr[i];

                iter = new_dir;
            }
        }

        // Iter at this point should hold the parent directory.
        // Only create new file if it doesn't exist yet
        let found = false;
        let foundFile = false;
        for (let i = 0; i < iter.children.length; ++i) {
            if (iter.children[i].path == path) {
                foundFile = iter.children[i].isFile;
                found = true;
                break;
            }
        }

        if (!found) {
            let new_file = self.createMetaDataObject();
            new_file.isFile = true;
            new_file.path = path;
            new_file.prettyPath = original_case;
            new_file.name = path_arr[i_len];
            new_file.parent = iter;
            iter.children.push(new_file);
        }

        // Update file index
        self.updateIndexObject(
            function() { // Success
                if (callback) {
                    let success = true;
                    if (found) {
                        if (!foundFile) {
                            success = false;
                        }
                    }
                    callback(path, success);
                }
            },
            function() { // Failure
                if (callback) {
                    callback(path, false);
                }
            }
        );
    }

    Write(fileName, fileBlob, OnSuccess, OnError) {
        if (!fileName.startsWith("/")) {
            fileName = "/" + fileName;
        }
        let originalCase = fileName;
        fileName = fileName.toLowerCase();

        let self = this;
        let metaDataExists = !(self.getIndexObjectFromPathString(fileName) == null);

        let _canceled = false;
        let writeTransaction = self.createFileWriteTransaction(
            function(transaction) {
                if (metaDataExists) { // File existed, file watcher
                    // This will trigger callbacks on parent directories too
                    self.triggerFileWatchCallbacks(fileName, true);
                }
                else { // File just created, only update parent dir
                    let parentName = self.getParentDirectoryFromPathString(fileName);
                    self.triggerFolderWatchCallbacks(parentName, true);
                }
            },
            function(error) {
                _canceled = true;
                if (OnError) {
                    OnError("Failed to write " + fileName);
                }
            }
        ); // createFileWriteTransaction

        let request = writeTransaction.put(fileBlob, fileName,
            function(path) {
                self.createFileMetaData(originalCase, function(path, metaDataExists) {
                    if (metaDataExists) {
                        if (OnSuccess && !_canceled) {
                            OnSuccess(originalCase, fileBlob.size);
                        }
                    }
                    else {
                        _canceled = true;
                        if (OnError) {
                            OnError("Failed to store " + fileName);
                        }
                    }
                });
            },
            function(error) {
                _canceled = true;
                if (OnError) {
                    OnError("Failed to store " + fileName);
                }
            }
        );

        writeTransaction.commit();
    }

    Read(fileName, OnSuccess, OnError) {
        if (!fileName.startsWith("/")) {
            fileName = "/" + fileName;
        }
        let originalCase = fileName;
        fileName = fileName.toLowerCase();

        let _canceled = false;
        let readTransaction = this.createFileReadTransaction(
            function(transaction) { },
            function(error) {
                _canceled = true;
                if (OnError) {
                    OnError("Failed to read " + fileName);
                }
            }
        ); // createFileReadTransaction

        let request = readTransaction.get(fileName);
        request.onsuccess = function (event) {
            let file = event.target.result;
            if (OnSuccess && !_canceled) {
                OnSuccess(originalCase, file);
            }
        };
        request.onerror = function(event) {
                _canceled = true;
                if (OnError) {
                OnError("Failed to get " + fileName);
            }
        }

        readTransaction.commit();
    }

    CreateFile(path, OnSuccess, OnError) {
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        let originalCase = path;
        path = path.toLowerCase();

        let self = this;
        let parentDir = self.getParentDirectoryFromPathString(path);

        self.IsDirectory(parentDir, function(dir, isDirectory) {
            if (isDirectory) {
                const emptyBlob = new Blob([""], {type: 'text/plain'});
                self.Write(originalCase, emptyBlob, OnSuccess, OnError);
            }
            else {
                self.logError("Can't touch file (" + path + "), parent directory (" + dir + ") doesn't exit");
                if (OnError) {
                    OnError("Can't touch file (" + path + "), parent directory (" + dir + ") doesn't exit");
                }
            }
        })
    }

    // OnSuccess(path: string): void
    // OnError(error: string): void
    CreateFolder(path, OnSuccess, OnError) {
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        let originalCase = path;
        path = path.toLowerCase();


        let self = this;
        let parentDir = self.getParentDirectoryFromPathString(path);
        let path_arr = path.split('/').filter(function(n) { return n !== '';});;

        self.IsDirectory(parentDir, function(dir, isDirectory) {
            if (isDirectory) {
                let iter = self.Index;
                let i_len = path_arr.length - 1;

                for (let i = 0; i < i_len; ++i) {
                    let j_len = iter.children.length;
                    let found = false;
                    for (let j = 0; j < j_len; ++j) {
                        if (iter.children[j].name == path_arr[i]) {
                            iter = iter.children[j];
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        self.logError("Couldn't advance iterator");
                    }
                }

                // Iter is now the parent directory. Check to make sure no file with our name exists
                let j_len = iter.children.length;
                let found = false;
                let foundDirectory = false;
                for (let j = 0; j < j_len; ++j) {
                    if (iter.children[j].path == path) {
                        found = true;
                        foundDirectory = iter.children[j].isDirectory;
                        break;
                    }
                }
                if (found) {
                    if (!foundDirectory) {
                        if (OnError) {
                            OnError("Can't create directoyr, file with same name already exists")
                        }
                    }
                    else {
                        if (OnSuccess) {
                            OnSuccess(originalCase);
                        }
                    }
                    return;
                }

                let new_folder = self.createMetaDataObject();
                new_folder.isDirectory = true;
                new_folder.children = [];
                new_folder.path = path;
                new_folder.prettyPath = originalCase;
                new_folder.name = path_arr[i_len];
                new_folder.parent = iter;
                iter.children.push(new_folder);

                self.updateIndexObject(
                    function() {
                        // Trigger watch callback
                        let parentPath = parentDir;
                        self.triggerFolderWatchCallbacks(path, true);
                        self.triggerFolderWatchCallbacks(parentPath, true);

                        if (OnSuccess) {
                            OnSuccess(path);
                        }
                    },
                    function() {
                        if (OnError) {
                            OnError("Can't create folder meta data, couldn't write index");
                        }
                    }
                );
            }
            else {
                self.logError("Can't create folder (" + path + "), parent directory (" + dir + ") doesn't exit");
                if (OnError) {
                    OnError("Can't create folder (" + path + "), parent directory (" + dir + ") doesn't exit");
                }
            }
        })
    }

    // Only called from inside of Delete. path is a file.
    deleteFiles(paths, OnSuccess, OnError) {
        let self = this;

        if (paths.length == 0) {
            OnSuccess(paths);
            return;
        }

        // Create transaction
        let writeTransaction = self.createFileWriteTransaction(
            function(transaction) { 
                // Files have ben deleted. Remove them from the index
                let i_len = paths.length;
                for (let i = 0; i < paths.length; ++i) {
                    let indexObject = self.getIndexObjectFromPathString(paths[i]);
                    self.unlinkIndexObjectFromParent(indexObject);
                }

                self.updateIndexObject(
                    function() { // Success
                        // Trigger watch callback
                        let paths_len = paths.length;
                        for (let j = 0; j < paths_len; ++j) {
                            self.triggerFileWatchCallbacks(paths[j], false);
                            let parentName = self.getParentDirectoryFromPathString(paths[j]);
                            self.triggerFolderWatchCallbacks(parentName, true);
                        }

                        if (OnSuccess) {
                            OnSuccess(paths);
                        }
                    },
                    function() { // Failure
                        if (OnError) {
                            OnError("Deleted files, but couldn't update index");
                        }
                    }
                );
                // Files removed from index. Update index in database
            },
            OnError
        ); // createFileWriteTransaction

        let i_len = paths.length;
        for (let i = 0; i < paths.length; ++i) {
            let path = paths[i];
            if (!path.startsWith("/")) {
                path = "/" + path;
            }

            writeTransaction.delete(path);
        }
        writeTransaction.commit();
    }

    // Only called from inside of Delete. path is a folder.
    deleteFolder(path, OnSuccess, OnError) {
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        let originalCase = path;
        path = path.toLowerCase();

        let self = this;

        let indexObject = self.getIndexObjectFromPathString(path);
        let remove_files = [];
        let remove_folders = [];

        // Mark All recursive files to delete
        const collectAllFiles = function(indexObject, remove_files, remove_folders) {
            if (indexObject.isDirectory) {
                let i_len = indexObject.children.length;
                for (let i = 0; i < i_len; ++i) {
                    collectAllFiles(indexObject.children[i], remove_files, remove_folders);
                }
                remove_folders.push(indexObject.path);
            }
            else {
                remove_files.push(indexObject.path);
            }
        }
        collectAllFiles(indexObject, remove_files, remove_folders);

        remove_folders.sort(function(a, b) {
            let x_arr = a.split('/').filter(function(n) { return n !== '';});;
            let y_arr = b.split('/').filter(function(n) { return n !== '';});;

            let x = x_arr.length;
            let y = y_arr.length;

            // We want to order in reverse
            if (x < y) {
              return 1; // Normally -1
            }
            if (x > y) {
              return -1; // Normally 1
            }
            return 0;
          });

        const commitIndexChange = function() {
            self.updateIndexObject(
                function() {
                    let num_folders = remove_folders.length;
                    for (let i = 0; i < num_folders; ++i) {
                        let folder_path = remove_folders[i];
                        let parent_path = self.getParentDirectoryFromPathString(folder_path);
                        self.triggerFolderWatchCallbacks(folder_path, false);
                        self.triggerFolderWatchCallbacks(parent_path, true);
                    }
                    
                    if (OnSuccess) {
                        OnSuccess(originalCase);
                    }
                },
                function() {
                    if (OnError) {
                        OnError("Deleted files, but can't update index");
                    }
                }
            );
        }

        const removeIndexObject = function() {
            if (indexObject.parent == null) {
                let root = self.createMetaDataObject();
                root.isDirectory = true;
                root.children = [];
                self.Index = root;
            }
            else {
                let to_remove = -1;
                let i_len = indexObject.parent.children.length;
                for (let i = 0; i < i_len; ++i) {
                    if (indexObject.parent.children[i].path == indexObject.path) {
                        to_remove = i;
                        break;
                    }
                }

                if (to_remove == -1) {
                    self.logError("Could not find self reference in parent");
                }
                else {
                    indexObject.parent.children.splice(to_remove, 1);
                }
            }
            indexObject.parent = null;
        }

        if (remove_files.length == 0) {
            removeIndexObject();
            commitIndexChange();
        }
        else {
            self.deleteFiles(remove_files, function(paths_deletes) {
                removeIndexObject();
                commitIndexChange();
            }, OnError);
        }
    }

    Delete(path, OnSuccess, OnError) {
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        let originalCase = path;
        path = path.toLowerCase();
        
        let self = this;

        self.Exists(path, function(path, isDirectory, isFile) {
            if (!isDirectory && !isFile) {
                self.logError("Can't delete " + path + " it doesn't exist");
                if (OnError) {
                    OnError("Can't delete " + path + " it doesn't exist");
                }
            }
            else if (isFile) {
                self.deleteFiles([originalCase], function(paths) {
                    if (OnSuccess) {
                        OnSuccess(paths[0]);
                    }
                }, OnError);
            }
            else {
                self.deleteFolder(originalCase, OnSuccess, OnError);
            }
        });
    }

    // callback(path: string, isDirectory: bool): void
    IsDirectory(path, callback) {
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        let originalCase = path;
        path = path.toLowerCase();

        if (path == "" || path == "/") {
            if (callback) {
                callback(path, true);
            }
        }
        else {
            this.Exists(path, function(_path, isDirectory, isFile) {
                if (callback) {
                    callback(originalCase, isDirectory);
                }
            });
        }
    }

    // callback(path: string, isFile: bool): void
    IsFile(path, callback) {
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        let originalCase = path;
        path = path.toLowerCase();

        this.Exists(path, function(_path, isDirectory, isFile) {
            if (callback) {
                callback(originalCase, isFile);
            }
        });
    }

    // callback(path: string, isDirectory: bool, isFile: bool): void
    Exists(path, callback) {
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        let originalCase = path;
        path = path.toLowerCase();

        let path_arr = path.split('/').filter(function(n) { return n !== '';});;
        let iter = this.Index;

        let i_len = path_arr.length;
        for (let i = 0; i < i_len; ++i) {
            let found = false;
            let j_len = iter.children.length;
            for (let j = 0; j < j_len; ++j) {
                if (iter.children[j].name == path_arr[i]) {
                    found = true;
                    iter = iter.children[j];
                    break;
                }
            }
            if (!found) {
                if (callback) {
                    callback(originalCase, false, false);
                    return;
                }
            }
        }

        if (callback) {
            callback(originalCase, iter.isDirectory, iter.isFile);
        }
    }

    DepthFirstTraversal(path, callback, finished) {
        path = path.toLowerCase();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }

        let self = this;
        const recursiveTraversal = function(indexObject, depth, isDirectory, isFile) {
            if (callback) {
                callback(indexObject.prettyPath, depth, isDirectory, isFile);
            }

            if (indexObject.isDirectory) {
                let i_len = indexObject.children.length;
                for (let i = 0; i < i_len; ++i) {
                    recursiveTraversal(indexObject.children[i], depth + 1,
                        indexObject.children[i].isDirectory,
                        indexObject.children[i].isFile);
                }
            }
        }

        let indexObject = self.getIndexObjectFromPathString(path);
        recursiveTraversal(indexObject, 0, indexObject.isDirectory, indexObject.isFile);

        if (finished) {
            finished(path);
        }
    }

    // OnChanged(path: string, isFile: bool, isFolder: bool): watchToken
    Watch(path, OnChanged) {
        path = path.toLowerCase();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        // Make sure list exists
        if (!this.FileWatch.hasOwnProperty(path)) {
            this.FileWatch[path] = [];
        }

        let i_size = this.FileWatch[path].length;
        for (let i = 0; i < i_size; ++i) {
            if (this.FileWatch[path][i] == OnChanged) {
                this.logError("Adding duplicate on change handler to: " + path);
            }
        }

        this.FileWatch[path].push(OnChanged);

        let watchToken = {}
        watchToken.path = path;
        watchToken.callback = OnChanged;
        return watchToken;
    }

    Unwatch(watchReturnToken) {
        if (typeof watchReturnToken === "string") {
            watchReturnToken = watchReturnToken.toLowerCase();
            if (!watchReturnToken.startsWith("/")) {
                watchReturnToken = "/" + watchReturnToken;
            }
            if (this.FileWatch.hasOwnProperty(watchReturnToken)) {
                this.FileWatch[watchReturnToken] = [];
            }
        }
        else {
            let path = watchReturnToken.path;
            let OnChanged = watchReturnToken.callback;

            if (this.FileWatch.hasOwnProperty(path)) {
                let i_size = this.FileWatch[path].length;
                let to_remove = -1;
                for (let i = 0; i < i_size; ++i) {
                    if (this.FileWatch[path][i] == OnChanged) {
                        to_remove = i;
                        break;
                    }
                }

                if (to_remove == -1) {
                    this.logError("Removing non existant watcher from: " + path);
                }
                this.FileWatch[path].splice(to_remove, 1);
            }
        }
    }

    IsWatching(path) {
        path = path.toLowerCase();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        if (this.FileWatch.hasOwnProperty(path)) {
            return this.FileWatch[path].length > 0;
        }
        return false;
    }

    InjectWebAssemblyImportObject(wasmImportObject) {
        let self = this;

        wasmImportObject["IndexDBFileSystem_wasmWrite"] = function(name_ptr, name_len, file_ptr, file_size, success_callback, error_callback) {
            let name_arr = self.WasmFsMemory.subarray(name_ptr, name_ptr + name_len);
            let name_str = utf8decoder.decode(name_arr);

            let blob_arr = self.WasmFsMemory.subarray(file_ptr, file_ptr + file_size);
            let blob = new Blob([blob_arr], {type: 'application/octet-stream'});
            
            self.Write(name_str, blob, 
                function(fileName, bytesWritten) {
                    self.WasmFsExports.IndexDBFileSystem_wasmTriggerWriteCallback(success_callback, name_ptr, bytesWritten);
                },
                function(errorMessage) {
                    let error_ptr = self.WasmFsExports.IndexDBFileSystem_wasmGetTempStrPtr(self.WasmFsPointer);

                    if (errorMessage && errorMessage != 0) {
                        let memory = new Uint8Array(self.WasmFsMemory.buffer);
                        for (let i = 0; i < errorMessage.length; ++i) {
                            memory[error_ptr + i] = errorMessage[i];
                        }
                    }

                    self.WasmFsExports.IndexDBFileSystem_wasmTriggerErrorCallback(error_callback, error_ptr);
                }
            );
        }
    }

    InitializeWebAssembly(wasmMemory, wasmExports, wasmFsPointer) {
        this.WasmFsPointer = wasmFsPointer;
        this.WasmFsExports = wasmExports;
        this.WasmFsMemory = wasmMemory;
    }
}