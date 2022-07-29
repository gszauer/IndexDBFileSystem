#pragma once

#ifndef ATLAS_U32
	#define ATLAS_U32
	typedef unsigned int u32;
	static_assert (sizeof(u32) == 4, "u32 should be defined as a 4 byte type");
#endif 

struct IndexDBFileSystem {
    struct WatchToken {
        int TODO;
    };

    typedef void (*fpErrorCallback)(const char* error);
    typedef void (*fpPathCallback)(const char* path);

    typedef void (*fpWriteCallback)(const char* fileName, u32 bytesWritten);
    typedef void (*fpWrite)(const char* fileName, const void* fileContent, u32 contentSize, fpWriteCallback onSuccess, fpErrorCallback onError);
    fpWrite Write;

    typedef void (*fpReadCallback)(const char* fileName, void* targetBuffer, u32 bytesRead);
    typedef void (*fpRead)(const char* fileName, void* targetBuffer, u32 targetSize, fpWriteCallback onSuccess, fpErrorCallback onError);
    fpRead Read;

    typedef void(*fpCreateFile)(const char* filePath, fpPathCallback onSuccess, fpErrorCallback onError);
    fpCreateFile CreateFile;

    typedef void (*fpCreateFolder)(const char* folderPath, fpPathCallback onSuccess, fpErrorCallback onError);
    fpCreateFolder CreateFolder;

    typedef void (*fpDelete)(const char* path, fpPathCallback onSuccess, fpErrorCallback onError);
    fpDelete Delete;

    typedef void (*fpIsDirectoryCallback)(const char* path, bool isDirectory);
    typedef void (*fpIsDirectory)(const char* path, fpIsDirectoryCallback onSuccess, fpErrorCallback onError);
    fpIsDirectory IsDirectory;

    typedef void (*fpIsFileCallback)(const char* path, bool isFile);
    typedef void (*fpIsFile)(const char* path, fpIsFileCallback onSuccess, fpErrorCallback onError);
    fpIsFile IsFile;

    typedef void (*fpExistsCallback)(const char* path, bool isDirectory, bool isFile);
    typedef void (*fpExists)(const char* path, fpIsDirectoryCallback onSuccess, fpErrorCallback onError);
    fpExists Exists;

    typedef void (*fpDepthFirstIterateCallback)(const char* path, u32 depth, bool isDirectory, bool isFile);
    typedef void (*fpDepthFirstFinishedCallback)();
    typedef void (*fpDepthFirstTraversal)(const char* path, fpDepthFirstIterateCallback onIterate, fpDepthFirstFinishedCallback onFinished);
    fpDepthFirstTraversal DepthFirstTraversal;

    typedef void (*fpWatchChanged)(const char* path, bool isDirectory, bool isFile);
    typedef WatchToken (*fpWatch)(const char* path, fpWatchChanged onChange);
    fpWatch Watch;

    typedef void (*fpUnwatch)(WatchToken& token);
    fpUnwatch Unwatch;

    typedef void (*fpUnwatchAll)(const char* path);
    fpUnwatchAll UnwatchAll;

    typedef bool (*fpIsWatching)(const char* path);
    fpIsWatching IsWatching;
};

void InitializeFileSystem(IndexDBFileSystem* fs);
void ShutdownFileSystem(IndexDBFileSystem* fs);