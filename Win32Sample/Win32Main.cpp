#include "../FileSystem.h"
#include <Windows.h>

#undef CreateFile

extern "C" int _fltused = 0;
char buf[1024];

void printf(const char* pszFormat, ...) {
    va_list argList;
    va_start(argList, pszFormat);
    wvsprintfA(buf, pszFormat, argList);
    va_end(argList);
    DWORD done;
    unsigned int len = 0;
    for (char* c = buf; *c != '\0'; ++c, ++len) {
        if (len >= 1024) {
            len = 1024;
            break;
        }
    }
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, (DWORD)len, &done, NULL);
}

int run() {
    printf("Working directory:\n\t");
    {
        GetCurrentDirectoryA(1024, buf);
        unsigned int len = 0;
        for (char* c = buf; *c != '\0'; ++c, ++len) {
            if (len >= 1024) {
                len = 1024;
                break;
            }
        }
        DWORD done;
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, (DWORD)len, &done, NULL);
    }
    printf("\n\n");

    FileSystem::Exists("test.txt", [](const char* path, bool isDir, bool isFile) {
        if (!isDir && !isFile) {
            printf("test.txt does not exist\n");
        }
        else if (isDir) {
            printf("test.txt is a folder\n");
        }
        else {
            printf("test.txt is a file\n");
        }
    });

    FileSystem::CreateFile("test.txt",
        [](const char* path) {
            printf("test.txt created\n");
        }, [](const char* error) {
            printf("test.txt NOT created\n");
            printf("%s\n", error);
        }
        );

    FileSystem::CreateFile("test.txt",
        [](const char* path) {
            printf("test.txt NOT created\n");
        }, [](const char* error) {
            printf("test.txt NOT created\n");
            printf("%s\n", error);
        }
        );

    FileSystem::Delete("test.txt",
        [](const char* path) {
            printf("test.txt deleted\n");
        },
        [](const char* error) {
            printf("test.txt NOT deleted\n");
            printf("%s\n", error);
        }
        );

    FileSystem::Delete("test.txt",
        [](const char* path) {
            printf("test.txt deleted\n");
        },
        [](const char* error) {
            printf("test.txt NOT deleted\n");
            printf("%s\n", error);
        }
        );

    FileSystem::CreateFolder("test",
        [](const char* path) {
            printf("test created\n");
        }, [](const char* error) {
            printf("test NOT created\n");
            printf("%s\n", error);
        }
        );

    FileSystem::CreateFolder("test",
        [](const char* path) {
            printf("test NOT created\n");
        }, [](const char* error) {
            printf("test NOT created\n");
            printf("%s\n", error);
        }
        );

    FileSystem::Delete("test",
        [](const char* path) {
            printf("test deleted\n");
        },
        [](const char* error) {
            printf("test NOT deleted\n");
            printf("%s\n", error);
        }
        );

    FileSystem::Delete("test",
        [](const char* path) {
            printf("test deleted\n");
        },
        [](const char* error) {
            printf("test NOT deleted\n");
            printf("%s\n", error);
        }
        );


    printf("\n\n");
    FileSystem::CreateFolder("Testing", 0, 0);
    FileSystem::CreateFile("Testing/test.txt", 0, 0);
    FileSystem::CreateFile("Testing1/test.txt", 0,
        [](const char* error) {
            printf("Can't create Testing1/test.txt\n");
            printf("%s\n", error);
        });
    FileSystem::Delete("Testing", [](const char* path) {
        printf("Deleted Testing and Testing/test.txt\n");
        }, [](const char* error) {
            printf("Couldn't delete Testing (and Testing/test.txt)\n");
            printf("%s\n", error);
        });

    printf("\n\n");
    FileSystem::CreateFolder("A", 0, 0);
    FileSystem::CreateFolder("A/B", 0, 0);
    FileSystem::CreateFolder("A/B/C", 0, 0);
    FileSystem::CreateFolder("A/B/D", 0, 0);
    FileSystem::CreateFolder("A/B/C/E", 0, 0);
    FileSystem::CreateFolder("A/B/C/F", 0, 0);
    FileSystem::CreateFolder("A/B/C/G", 0, 0);
    FileSystem::CreateFolder("A/B/D/H", 0, 0);
    FileSystem::CreateFolder("A/I", 0, 0);
    FileSystem::CreateFile("A/I/J.txt", 0, 0);
    FileSystem::CreateFolder("A/I/K", 0, 0);
    FileSystem::CreateFolder("A/I/K/L", 0, 0);
    FileSystem::CreateFile("A/I/K/M.txt", 0, 0);

    FileSystem::PreOrderDepthFirstTraversal("A", [](const char* path, u32 depth, bool isDirectory, bool isFile) {
        for (u32 i = 0; i < depth; ++i) {
            printf("\t");
        }
        printf(path);
        printf("\n");
    }, []() {
        printf("Done iterating\n");
    });

    printf("\n\n\n");
    FileSystem::PostOrderDepthFirstTraversal("A", [](const char* path, u32 depth, bool isDirectory, bool isFile) {
        for (u32 i = 0; i < depth; ++i) {
            printf("\t");
        }
        printf(path);
        printf("\n");
        }, []() {
            printf("Done iterating\n");
        });


    return 0;
}