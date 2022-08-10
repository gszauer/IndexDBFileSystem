#pragma once

#ifndef ATLAS_U32
	#define ATLAS_U32
	typedef unsigned int u32;
	static_assert (sizeof(u32) == 4, "u32 should be defined as a 4 byte type");
#endif 

#undef CreateFile

namespace FileSystem {
    typedef void (*fpErrorCallback)(const char* error);
    typedef void (*fpPathCallback)(const char* path);

    void Log(const char* msg);

    typedef void (*fpWriteCallback)(const char* fileName, u32 bytesWritten);
    void Write(const char* fileName, const void* fileContent, u32 contentSize, fpWriteCallback onSuccess, fpErrorCallback onError);

    typedef void (*fpReadCallback)(const char* fileName, void* targetBuffer, u32 bytesRead);
    void Read(const char* fileName, void* targetBuffer, u32 targetSize, fpReadCallback onSuccess, fpErrorCallback onError);

    void CreateFile(const char* filePath, fpPathCallback onSuccess, fpErrorCallback onError);

    void CreateFolder(const char* folderPath, fpPathCallback onSuccess, fpErrorCallback onError);

    void Delete(const char* path, fpPathCallback onSuccess, fpErrorCallback onError);

    typedef void (*fpExistsCallback)(const char* path, bool isDirectory, bool isFile);
    void Exists(const char* path, fpExistsCallback onSuccess);

    typedef void (*fpDepthFirstIterateCallback)(const char* path, u32 depth, bool isDirectory, bool isFile);
    typedef void (*fpDepthFirstFinishedCallback)(); // TODO: Rename to empty callback
    void PreOrderDepthFirstTraversal(const char* path, fpDepthFirstIterateCallback onIterate, fpDepthFirstFinishedCallback onFinished);
    void PostOrderDepthFirstTraversal(const char* path, fpDepthFirstIterateCallback onIterate, fpDepthFirstFinishedCallback onFinished);

    // TODO: Win32 style GetFirstChild function for iterating a directory

    void GetWorkingDir(char* outBuffer, u32 buffLen);

    void Initialize();
    void Shutdown();
}