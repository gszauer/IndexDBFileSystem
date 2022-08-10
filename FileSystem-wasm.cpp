#include "FileSystem.h"

// Helper functions
int IndexDBFileSystem_strlen(const char* str) {
    if (str == 0) {
        return 0;
    }

    const char *s = str;
    while (*s) {
        ++s;
    }
    return (s - str);
}

// Wasm imports
extern "C" void IndexDBFileSystem_wasmLog(const char* msg, int msgLen);
extern "C" void IndexDBFileSystem_wasmWrite(const char* str_ptr, int str_len, const void* blob_ptr, int blob_size, FileSystem::fpWriteCallback success_callback, FileSystem::fpErrorCallback error_callback);
extern "C" void IndexDBFileSystem_wasmRead(const char* str_ptr, int str_len, const void* target_ptr, int target_size, FileSystem::fpReadCallback success_callback, FileSystem::fpErrorCallback error_callback);
extern "C" void IndexDBFileSystem_wasmCreateFile(const char* name_ptr, int name_len, FileSystem::fpPathCallback success_callback, FileSystem::fpErrorCallback error_callback);
extern "C" void IndexDBFileSystem_wasmCreateFolder(const char* name_ptr, int name_len, FileSystem::fpPathCallback success_callback, FileSystem::fpErrorCallback error_callback);
extern "C" void IndexDBFileSystem_wasmDelete(const char* name_ptr, int name_len, FileSystem::fpPathCallback success_callback, FileSystem::fpErrorCallback error_callback);
extern "C" void IndexDBFileSystem_wasmExists(const char* name_ptr, int name_len, FileSystem::fpExistsCallback success_callback);
extern "C" void IndexDBFileSystem_wasmPreDepthFirstTraversal(const char* name_ptr, int name_len, FileSystem::fpDepthFirstIterateCallback iterate_callback, FileSystem::fpDepthFirstFinishedCallback done_callback);
extern "C" void IndexDBFileSystem_wasmPostDepthFirstTraversal(const char* name_ptr, int name_len, FileSystem::fpDepthFirstIterateCallback iterate_callback, FileSystem::fpDepthFirstFinishedCallback done_callback);
extern "C" int  IndexDBFileSystem_wasmWatch(const char* name_ptr, int name_len, FileSystem::fpWatchChangedCallback onchanged);
extern "C" void IndexDBFileSystem_wasmUnwatch(int unwatchTokenId);
extern "C" void IndexDBFileSystem_wasmUnwatchAll(const char* name_ptr, int name_len);
extern "C" int IndexDBFileSystem_wasmIsWatching(const char* name_ptr, int name_len);

extern unsigned char __heap_base;

// Wasm exports
#define export __attribute__ (( visibility( "default" ) )) extern "C"

export void IndexDBFileSystem_wasmTriggerErrorCallback(FileSystem::fpErrorCallback error_callback, const char* err_str) {
    if (error_callback != 0) {
        error_callback(err_str);
    }
}

export void IndexDBFileSystem_wasmTriggerWriteCallback(FileSystem::fpWriteCallback write_callback, const char* file_name, int bytes_written) {
    if (write_callback != 0) {
        write_callback(file_name, bytes_written);
    }
}

export void IndexDBFileSystem_wasmTriggerReadCallback(FileSystem::fpReadCallback read_callback, const char* file_name, void* fileData, int sizeInBytes) {
    if (read_callback != 0) {
        read_callback(file_name, fileData, (u32)sizeInBytes);
    }
}

export void IndexDBFileSystem_wasmTriggerPathCallback(FileSystem::fpPathCallback callback, const char* file_name) {
    if (callback != 0) {
        callback(file_name);
    }
}

export void IndexDBFileSystem_wasmTriggerExistsCallback(FileSystem::fpExistsCallback callback, const char* file_name, bool isDir, bool isFile) {
    if (callback) {
        callback(file_name, isDir, isFile);
    }
}

export void IndexDBFileSystem_wasmTriggerDepthFirstIterateCallback(FileSystem::fpDepthFirstIterateCallback callback, const char* path, u32 depth, bool isDirectory, bool isFile) {
    if (callback) {
        callback(path, depth, isDirectory, isFile);
    }
}

export void IndexDBFileSystem_wasmTriggerDepthFirstFinishedCallback(FileSystem::fpDepthFirstFinishedCallback callback) {
    if (callback) {
        callback();
    }
}

export void IndexDBFileSystem_wasmTriggerWatchChangedCallback(FileSystem::fpWatchChangedCallback callback, const char* path, bool isDirectory, bool isFile) {
    if (callback) {
        callback(path, isDirectory, isFile);
    }
 }

export void* IndexDBFileSystem_wasmGetHeapPtr() {
    void* memory = &__heap_base;
    return memory;
}

// File System API
void FileSystem::Log(const char* msg) {
    int len = IndexDBFileSystem_strlen(msg);
    IndexDBFileSystem_wasmLog(msg, len);
}

void FileSystem::Write(const char* fileName, const void* fileContent, u32 contentSize, FileSystem::fpWriteCallback onSuccess, FileSystem::fpErrorCallback onError) {
    int nameLen = IndexDBFileSystem_strlen(fileName);
    IndexDBFileSystem_wasmWrite(fileName, nameLen, fileContent, (int)contentSize, onSuccess, onError);
}

void FileSystem::Read(const char* fileName, void* targetBuffer, u32 targetSize, FileSystem::fpReadCallback onSuccess, FileSystem::fpErrorCallback onError) {
    int nameLen = IndexDBFileSystem_strlen(fileName);
    IndexDBFileSystem_wasmRead(fileName,nameLen, targetBuffer, (int)targetSize, onSuccess, onError);
}

void FileSystem::CreateFile(const char* filePath, FileSystem::fpPathCallback onSuccess, FileSystem::fpErrorCallback onError) {
    int nameLen = IndexDBFileSystem_strlen(filePath);
    IndexDBFileSystem_wasmCreateFile(filePath, nameLen, onSuccess, onError);
}

void FileSystem::CreateFolder(const char* folderPath, FileSystem::fpPathCallback onSuccess, FileSystem::fpErrorCallback onError) {
    int nameLen = IndexDBFileSystem_strlen(folderPath);
    IndexDBFileSystem_wasmCreateFolder(folderPath, nameLen, onSuccess, onError);
}

void FileSystem::Delete(const char* path, FileSystem::fpPathCallback onSuccess, FileSystem::fpErrorCallback onError) {
    int nameLen = IndexDBFileSystem_strlen(path);
    IndexDBFileSystem_wasmDelete(path, nameLen, onSuccess, onError);
}

void FileSystem::Exists(const char* path, FileSystem::fpExistsCallback onSuccess) {
    int nameLen = IndexDBFileSystem_strlen(path);
    IndexDBFileSystem_wasmExists(path, nameLen, onSuccess);
}

void FileSystem::PreOrderDepthFirstTraversal(const char* path, FileSystem::fpDepthFirstIterateCallback onIterate, FileSystem::fpDepthFirstFinishedCallback onFinished) {
    int nameLen = IndexDBFileSystem_strlen(path);
    IndexDBFileSystem_wasmPreDepthFirstTraversal(path, nameLen, onIterate, onFinished); // TODO: Fix
}

void FileSystem::PostOrderDepthFirstTraversal(const char* path, FileSystem::fpDepthFirstIterateCallback onIterate, FileSystem::fpDepthFirstFinishedCallback onFinished) {
    int nameLen = IndexDBFileSystem_strlen(path);
    IndexDBFileSystem_wasmPostDepthFirstTraversal(path, nameLen, onIterate, onFinished); // TODO: Fix
}

FileSystem::WatchToken FileSystem::Watch(const char* path, FileSystem::fpWatchChangedCallback onChange) {
    int nameLen = IndexDBFileSystem_strlen(path);
    FileSystem::WatchToken token;
    token.wasmId = IndexDBFileSystem_wasmWatch(path, nameLen, onChange);
    return token;
}

void FileSystem::Unwatch(FileSystem::WatchToken& token) {
    IndexDBFileSystem_wasmUnwatch(token.wasmId);
}

void FileSystem::UnwatchAll(const char* path) {
    int nameLen = IndexDBFileSystem_strlen(path);
    IndexDBFileSystem_wasmUnwatchAll(path, nameLen);
}

bool FileSystem::IsWatching(const char* path) {
    int nameLen = IndexDBFileSystem_strlen(path);
    return (bool)IndexDBFileSystem_wasmIsWatching(path, nameLen);
}

void FileSystem::Initialize() {
    // Nothing to do....
}

void FileSystem::Shutdown() {
    // Nothing to do....
}