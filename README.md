# Index DB File System
TODO

# API
TODO

## Constructor
```
constructor(databaseName: string, onSuccess: function, onError: function)
```

### Constructor callbacks
```
onSuccess(self: IndexDBFileSystem): void
onError(errorMessage: string): void
```

## Write
```
Write(fileName: string, fileContent: blob, OnSuccess: function, OnError: function): void
```

### Write callbacks
```
OnSuccess(fileName: string): void
OnError(errorMessage: string): void
```

## Read
```
Read(fileName: string, OnSuccess: function, OnError: function): void
```

### Read callbacks
```
OnSuccess(filePath: string, fileContent: blob): void
OnError(errorMessage: string) : void
```

## Create File
```
CreateFile(path: string, OnSuccess: function, OnError: function): void
```

### Create File callbacks
```
OnSuccess(fileName: string): void
OnError(errorMessage: string): void
```

## Create Folder
```
CreateFolder(path: string, OnSuccess: function, OnError: function): void
```

### Create File callbacks
```
OnSuccess(path: string): void
OnError(error: string): void
```

## Delete File or Folder
```
Delete(path: string, OnSuccess: function, OnError: function): void
```

### Delete File or Folder callback
```
OnSuccess(path: string): void
OnError(error: string): void
```

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