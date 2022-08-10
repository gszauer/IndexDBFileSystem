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

Reads some bytes of data from the disk. If the the file does not exist, the error callback is called. If the file does exist, but is empty, the success callback will have null for it's blob data.

### Read callbacks

```
OnSuccess(filePath: string, fileContent: blob): void
OnError(errorMessage: string) : void
```

The success callback takes the path of the file that was read and a blob of file data. If the file is empty, the blob will be null. The error callback takes a single argument, the error string and has no return.

## Create File

```
CreateFile(path: string, OnSuccess: function, OnError: function): void
```

Created a new file, under the hood this function writes 0 bytes to a file using the Write function. 

### Create File callbacks

```
OnSuccess(fileName: string): void
OnError(errorMessage: string): void
```

The error callback takes a single argument, the error string and has no return. The only argument of the success callback is the path of the file that was created.

## Create Folder

```
CreateFolder(path: string, OnSuccess: function, OnError: function): void
```

Creates a new folder. The file system isn't touched, folders exist only as meta-data.

### Create Folder callbacks

```
OnSuccess(path: string): void
OnError(error: string): void
```

The error callback takes a single argument, the error string and has no return. The only argument of the success callback is the path of the folder that was created.

## Delete File or Folder

```
Delete(path: string, OnSuccess: function, OnError: function): void
```

The ```Delete``` function is used to remove both files and folders.

### Delete File or Folder callback
```
OnSuccess(path: string): void
OnError(error: string): void
```

The error callback takes a single argument, the error string and has no return. The only argument of the success callback is the path of the file or folder that was removed.

## Directory check

```
IsDirectory(path: string, callback: function): void
```

Checks if the provided path is a directory. Under the hood, this function calls ```Exists```

### Directory check callback

```
callback(path: string, isDirectory: bool): void
```

The directory check callback returns the path of the folder being checked, and a boolen indicating weather of not it's a folder.

## File check

```
IsFile(path: string, callback: function): void
```

Checks if the provided path is a file. Under the hood, this function calls ```Exists```

### File check callback

```
callback(path: string, isFile: bool): void
```

The file check callback returns the path of the file being checked, and a boolen indicating weather of not it's a file.

## File or folder check

```
Exists(path: string, callback: function): void
```

The ```Exists``` function checks if a given path exists. We can use this function to check if a path is a file, folder, or if it doesn't exist.

### File or folder callback

```
callback(path: string, isDirectory: bool, isFile: bool): void
```

The callback to ```Exists``` takes the path of the file being checked, and two booleans to tell if the path is a directory or folder. If both booleans are false, the path does not exist.

## Directory traversal

```
DepthFirstTraversal(path: string, preOrder: bool, callback: function, finished: function): void
```

Used to traverse a given directory, the second argument specifies if the traversal is pre or post order. There are also ```PreOrderDepthFirstTraversal``` and ```PostOrderDepthFirstTraversal``` functions that don't take this additional paramater.

### Directory traversal callback

```
callback(path: string, depth: int, isDirectory: bool, isFile: bool): void
finished(path: string): void
```

The callback function is called for every directory being traversed. It includes the full path of the directory or file, it's depth and booleans to tell if it's a file or folder. When iteration is finished, the finished callback is called with the original path.