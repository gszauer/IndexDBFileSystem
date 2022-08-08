#include "FileSystem.h"
#include <Windows.h>

#undef CreateFile

void FileSystem::Log(const char* buf) {
    unsigned int len = 0;
    for (const char* c = buf; *c != '\0'; ++c, ++len);
    DWORD done;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, (DWORD)len, &done, NULL);
}

void FileSystem::Write(const char* fileName, const void* fileContent, u32 contentSize, fpWriteCallback onSuccess, fpErrorCallback onError) {
	HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(hFile, 0, NULL, FILE_END);
	DWORD bytesWritten = 0; // Default
	if (hFile == INVALID_HANDLE_VALUE) {
		if (onError) {
			onError("FileSystem::Write failed to open file handle");
		}
	}
	else {
		if (WriteFile(hFile, fileContent, contentSize, &bytesWritten, 0) == 0) {
			if (onError) {
				onError("FileSystem::Write could not write data to file");
			}
		}
		else {
			if (onSuccess) {
				onSuccess(fileName, (u32)bytesWritten);
			}
		}
		CloseHandle(hFile);
	}
}

void FileSystem::Read(const char* fileName, void* targetBuffer, u32 targetSize, fpReadCallback onSuccess, fpErrorCallback onError) {
	HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD bytesRead = 0;

	if (hFile == INVALID_HANDLE_VALUE) {
		if (onError) {
			onError("FileSystem::Read failed to open file handle");
		}
	}
	else {
		if (ReadFile(hFile, targetBuffer, targetSize, &bytesRead, NULL) == 0) {
			if (onError) {
				onError("FileSystem::Read could not read data from file");
			}
		}
		else {
			if (onSuccess) {
				onSuccess(fileName, targetBuffer, (u32)bytesRead);
			}
		}
		CloseHandle(hFile);
	}
}

void FileSystem::CreateFile(const char* filePath, fpPathCallback onSuccess, fpErrorCallback onError) {

}

void FileSystem::CreateFolder(const char* folderPath, fpPathCallback onSuccess, fpErrorCallback onError) {
}

void FileSystem::Delete(const char* path, fpPathCallback onSuccess, fpErrorCallback onError) {

}

void FileSystem::Exists(const char* path, fpExistsCallback onSuccess, fpErrorCallback onError) {

}

void FileSystem::DepthFirstTraversal(const char* path, fpDepthFirstIterateCallback onIterate, fpDepthFirstFinishedCallback onFinished) {

}

FileSystem::WatchToken FileSystem::Watch(const char* path, fpWatchChangedCallback onChange) {
	FileSystem::WatchToken result;
	return result;
}

void FileSystem::Unwatch(WatchToken& token) {

}

void FileSystem::UnwatchAll(const char* path) {

}

bool FileSystem::IsWatching(const char* path) {
	return false;
}

void FileSystem::Initialize() {
	// TODO: Set current directory to work out of?
}

void FileSystem::Shutdown() {

}