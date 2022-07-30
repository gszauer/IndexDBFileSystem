#include "IndexDBFileSystem.h"

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

export void IndexDBFileSystem_wasmInitializeFileSystem() {
    FileSystem::Initialize();
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

void FileSystem::Exists(const char* path, FileSystem::fpExistsCallback onSuccess, FileSystem::fpErrorCallback onError) {

}

void FileSystem::DepthFirstTraversal(const char* path, FileSystem::fpDepthFirstIterateCallback onIterate, FileSystem::fpDepthFirstFinishedCallback onFinished) {

}

void FileSystem::Unwatch(FileSystem::WatchToken& token) {

}

void FileSystem::UnwatchAll(const char* path) {

}

bool FileSystem::IsWatching(const char* path) {
    return false; // TODO
}

FileSystem::WatchToken FileSystem::Watch(const char* path, FileSystem::fpWatchChanged onChange) {
    FileSystem::WatchToken token;
    return token;
}

void FileSystem::Initialize() {
    // Nothing to do....
}

void FileSystem::Shutdown() {
    // Nothing to do
}

export void IndexDBFileSystem_wasmTestFileSystem() {
    const char* content = "File created in C++";
    FileSystem::Write("Cpp.text", content, IndexDBFileSystem_strlen(content), 
        [](const char* fileName, u32 bytesWritten) {
            FileSystem::Log("Successfully wrote Cpp.text");

            static char tmp[512];//= {0};
            for (int i = 0; i < 512; ++i) {
                tmp[i] = '\0';
            }

            FileSystem::Read("test.txt", tmp, 512,
                [](const char* fileName, void* targetBuffer, u32 bytesRead) {
                    FileSystem::Log("Successfully read test.txt, content:");

                    char* strTarget = (char*)targetBuffer;
                    strTarget[bytesRead] = '\0';
                    int len = IndexDBFileSystem_strlen(strTarget);
                    IndexDBFileSystem_wasmLog(strTarget, len);

                    FileSystem::CreateFile("FileFromCpp.dummy", 
                        [](const char* path) {
                            FileSystem::Log("Successfully created FileFromCpp.dummy");

                            FileSystem::CreateFolder("FolderFromCpp",
                                [](const char* path) {
                                    FileSystem::Log("Successfully created FolderFromCpp");

                                    FileSystem::CreateFile("FolderFromCpp/FileFromCpp2.dummy",
                                        [](const char* path) {
                                            FileSystem::Delete("FolderFromCpp",
                                                [](const char* path) {
                                                    FileSystem::Delete("test.txt", 
                                                        [](const char* path) {
                                                            FileSystem::Log("Deleted test.txt");

                                                        },
                                                        [](const char* error) {
                                                            FileSystem::Log("Could not delete test.txt");
                                                            FileSystem::Log(error);
                                                        }
                                                    );
                                                },
                                                [](const char* error) {
                                                    FileSystem::Log("Could not delete FolderFromCpp");
                                                    FileSystem::Log(error);
                                                }
                                            );
                                        },
                                        [](const char* error) {
                                            FileSystem::Log("Could not create FileFromCpp2.dummy");
                                            FileSystem::Log(error);
                                        }
                                    );
                                },
                                [](const char* error) {
                                    FileSystem::Log("Could not create FolderFromCpp");
                                    FileSystem::Log(error);
                                }
                            );
                        },
                        [](const char* error) {
                            FileSystem::Log("Could not create FileFromCpp.dummy");
                            FileSystem::Log(error);
                        }
                    );
                },
                [](const char* erro) {
                    FileSystem::Log("Could not read test.txt");
                    FileSystem::Log(erro);
                }
            );
        },
        [](const char* erro) {
            FileSystem::Log("Could not write Cpp.text");
            FileSystem::Log(erro);
        }
    );
}