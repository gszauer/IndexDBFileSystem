#include "IndexDBFileSystem.h"

// Wasm imports
extern "C" void IndexDBFileSystem_wasmWrite(const char* str_ptr, int str_len, const void* blob_ptr, int blob_size, IndexDBFileSystem::fpWriteCallback success_callback, IndexDBFileSystem::fpErrorCallback error_callback);
extern "C" void IndexDBFileSystem_wasmLog(const char msg, int msgLen);

// Wasm exports
#define export __attribute__ (( visibility( "default" ) )) extern "C"

export void IndexDBFileSystem_wasmTriggerErrorCallback(IndexDBFileSystem::fpErrorCallback error_callback, const char* err_str) {
    if (error_callback != 0) {
        error_callback(err_str);
    }
}

export void IndexDBFileSystem_wasmTriggerWriteCallback(IndexDBFileSystem::fpWriteCallback write_callback, const char* file_name, int bytes_written) {
    if (write_callback != 0) {
        write_callback(file_name, bytes_written);
    }
}

export void IndexDBFileSystem_wasmInitializeFileSystem(IndexDBFileSystem* fs, char* error, u32 errorSize) {
    InitializeFileSystem(fs, error, errorSize);
}

// Helper functions
int IndexDBFileSystem_strlen(const char* str) {
    const char *s = str;
    while (*s) {
        ++s;
    }
    return (s - str);
}

// File System API
void IndexDBFileSystem_Write(const char* fileName, const void* fileContent, u32 contentSize, IndexDBFileSystem::fpWriteCallback onSuccess, IndexDBFileSystem::fpErrorCallback onError) {
    int nameLen = IndexDBFileSystem_strlen(fileName);

    IndexDBFileSystem_wasmWrite(fileName, nameLen, fileContent, (int)contentSize, onSuccess, onError);
}

void IndexDBFileSystem_Read(const char* fileName, void* targetBuffer, u32 targetSize, IndexDBFileSystem::fpWriteCallback onSuccess, IndexDBFileSystem::fpErrorCallback onError) {

}

void IndexDBFileSystem_CreateFile(const char* filePath, IndexDBFileSystem::fpPathCallback onSuccess, IndexDBFileSystem::fpErrorCallback onError) {

}

void IndexDBFileSystem_CreateFolder(const char* folderPath, IndexDBFileSystem::fpPathCallback onSuccess, IndexDBFileSystem::fpErrorCallback onError) {

}

void IndexDBFileSystem_Delete(const char* path, IndexDBFileSystem::fpPathCallback onSuccess, IndexDBFileSystem::fpErrorCallback onError) {

}

void IndexDBFileSystem_IsDirectory(const char* path, IndexDBFileSystem::fpIsDirectoryCallback onSuccess, IndexDBFileSystem::fpErrorCallback onError) {

}

void IndexDBFileSystem_IsFile(const char* path, IndexDBFileSystem::fpIsFileCallback onSuccess, IndexDBFileSystem::fpErrorCallback onError) {

}

void IndexDBFileSystem_Exists(const char* path, IndexDBFileSystem::fpIsDirectoryCallback onSuccess, IndexDBFileSystem::fpErrorCallback onError) {

}

void IndexDBFileSystem_DepthFirstTraversal(const char* path, IndexDBFileSystem::fpDepthFirstIterateCallback onIterate, IndexDBFileSystem::fpDepthFirstFinishedCallback onFinished) {

}

void IndexDBFileSystem_Unwatch(IndexDBFileSystem::WatchToken& token) {

}

void IndexDBFileSystem_UnwatchAll(const char* path) {

}

bool IndexDBFileSystem_IsWatching(const char* path) {
    return false; // TODO
}

IndexDBFileSystem::WatchToken IndexDBFileSystem_Watch(const char* path, IndexDBFileSystem::fpWatchChanged onChange) {
    IndexDBFileSystem::WatchToken token;
    return token;
}

void InitializeFileSystem(IndexDBFileSystem* fs) {
    fs->Write = IndexDBFileSystem_Write;
    fs->Read = IndexDBFileSystem_Read;
    fs->CreateFile = IndexDBFileSystem_CreateFile;
    fs->CreateFolder = IndexDBFileSystem_CreateFolder;
    fs->Delete = IndexDBFileSystem_Delete;
    fs->IsDirectory = IndexDBFileSystem_IsDirectory;
    fs->IsFile = IndexDBFileSystem_IsFile;
    fs->Exists = IndexDBFileSystem_Exists;
    fs->DepthFirstTraversal = IndexDBFileSystem_DepthFirstTraversal;
    fs->Watch = IndexDBFileSystem_Watch;
    fs->Unwatch = IndexDBFileSystem_Unwatch;
    fs->UnwatchAll = IndexDBFileSystem_UnwatchAll;
    fs->IsWatching = IndexDBFileSystem_IsWatching;
}

void ShutdownFileSystem(IndexDBFileSystem* fs) {
    fs->Write = 0;
    fs->Read = 0;
    fs->CreateFile = 0;
    fs->CreateFolder = 0;
    fs->Delete = 0;
    fs->IsDirectory = 0;
    fs->IsFile = 0;
    fs->Exists = 0;
    fs->DepthFirstTraversal = 0;
    fs->Watch = 0;
    fs->Unwatch = 0;
    fs->UnwatchAll = 0;
    fs->IsWatching = 0;
}

// TODO: Disable. Using this to test if interop is working
#if 1
export void IndexDBFileSystem_wasmTestFileSystem() {
    const char* msg = "C++ IndexDBFileSystem_wasmTestFileSystem calling js IndexDBFileSystem_wasmLog";
    int len = IndexDBFileSystem_strlen(msg);
    IndexDBFileSystem_wasmLog(msg, len)
}
#endif