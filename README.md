# Index DB File System

IndexDBFileSystem.js emulates a case insensitive file system using IndexDB. All files are stored as blob objects, which should make them actually be stored as files by the browser engine. This file system is intended to be used with Web Assembly to emulate a file layer. There isn't a maximum disk space to querry for, and some browser engines automatically clear their IndexDB Databases automatically (every 14 days). It's best to think of this library as semi-permanent storage, like a cache. WPA apps can request that their databses be permanent.

# API

IndexDBFileSystem has a mostly async API. First, you need to create an IndexDBFileSystem object. Most functions on this object don't return anything directly, instead they rely on callbacks. Function names that start with a capital letter are a part of IndexDBFileSystem's public API. Functions that start with a lower case letter are considered private API. All variables are assumed to be private. IndexDBFileSystem has C++ bindings that allow it to be used with web-assembly. The C++ API closeley resembles the javascript API.

## Constructor

```
constructor(databaseName: string, onSuccess: function, onError: function)
```

The constructor takes a database name, a success callback and an error callback. The success callback takes one argument the IndexDBFileSystem object that was just constructed. If you want to use the constructed object without waiting for the callback, it has a ```IsReady``` member, which is a boolean. The file system can be used inside a web assembly module, it includes seperate injection and initialization code for web assembly.

### Constructor callbacks

```
onSuccess(self: IndexDBFileSystem): void
onError(errorMessage: string): void
```

The error callback takes a single argument, the error string and has no return. The success callback is a reference to the ```IndexDBFileSystem``` object that was just created.

## Write

```
Write(fileName: string, fileContent: blob, OnSuccess: function, OnError: function): void
```

Writes some bytes of data to disk. The write function takes a file name, a blob for the file content, and callbacks for success and failure. If the file being written does not exist, it will be created. If the file being written already exists, it's content will be replaced. There is no append function, but it can be implemented with read and write. If the file path is in a directory which does not exist, that directory will be created.

### Write callbacks

```
OnSuccess(fileName: string, bytesWritten: int): void
OnError(errorMessage: string): void
```

The error callback takes a single argument, the error string and has no return. The success callback takes the path of the file that was written, and the number of bytes that where written. 

## Read

```
Read(fileName: string, OnSuccess: function, OnError: function): void
```

### Read callbacks
```
OnSuccess(filePath: string, fileContent: blob): void
OnError(errorMessage: string) : void
```

The error callback takes a single argument, the error string and has no return.

## Create File

```
CreateFile(path: string, OnSuccess: function, OnError: function): void
```

### Create File callbacks
```
OnSuccess(fileName: string): void
OnError(errorMessage: string): void
```

The error callback takes a single argument, the error string and has no return.

## Create Folder
```
CreateFolder(path: string, OnSuccess: function, OnError: function): void
```

### Create File callbacks
```
OnSuccess(path: string): void
OnError(error: string): void
```

The error callback takes a single argument, the error string and has no return.

## Delete File or Folder
```
Delete(path: string, OnSuccess: function, OnError: function): void
```

### Delete File or Folder callback
```
OnSuccess(path: string): void
OnError(error: string): void
```

The error callback takes a single argument, the error string and has no return.

## Directory check
```
IsDirectory(path: string, callback: function): void
```

### Directory check callback
```
callback(path: string, isDirectory: bool): void
```

## File check
```
IsFile(path: string, callback: function): void
```

### File check callback
```
callback(path: string, isFile: bool): void
```

## File or folder check
```
Exists(path: string, callback: function): void
```

### File or folder callback
```
callback(path: string, isDirectory: bool, isFile: bool): void
```

## Directory traversal
```
DepthFirstTraversal(path: string, callback: function, finished: function): void
```

### Directory traversal callback
```
callback(path: string, depth: int, isDirectory: bool, isFile: bool): void
finished(path: string): void
```

## File Watcher
```
Watch(path: string, OnChanged: function): WatchToken
```

```
Unwatch(path: string): void
Unwatch(token: Watchtoken): void
```

```
IsWatching(path: string): bool
```

### File Watcher callback
```
OnChanged(path: string, isFolder: bool, isFile: bool): void
```

## C++ API

TODO