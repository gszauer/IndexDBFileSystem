#include "../FileSystem.h"
#include <Windows.h>

extern "C" int _fltused = 0;

void printf(const char* pszFormat, ...) {
    char buf[1024];
    va_list argList;
    va_start(argList, pszFormat);
    wvsprintfA(buf, pszFormat, argList);
    va_end(argList);
    DWORD done;
    unsigned int len = 0;
    for (char* c = buf; *c != '\0'; ++c, ++len);
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, (DWORD)len, &done, NULL);
}

int run() {
    printf("Hello, world\n");

    return 0;
}