#include "FileSystem.h"
#include <Windows.h>

#undef CreateFile
#define CUSTOM_PATH_LEN 512

namespace FileSystem {
	namespace Internal {
		bool Exists(const char* path) {
			DWORD ftyp = GetFileAttributesA(path);
			if (ftyp == INVALID_FILE_ATTRIBUTES) {
				return false;
			}
			return true;
		}

		bool Isfile(const char* path) {
			DWORD ftyp = GetFileAttributesA(path);
			if (ftyp == INVALID_FILE_ATTRIBUTES) {
				return false;
			}
			else if (ftyp & FILE_ATTRIBUTE_DIRECTORY) {
				return false;
			}
			return true;
		}

		bool IsFolder(const char* path) {
			DWORD ftyp = GetFileAttributesA(path);
			if (ftyp == INVALID_FILE_ATTRIBUTES) {
				return false;
			}
			else if (ftyp & FILE_ATTRIBUTE_DIRECTORY) {
				return true;
			}
			return false;
		}

		void GetInfo(const char* path, bool* isDir, bool* isFile) {
			DWORD ftyp = GetFileAttributesA(path);
			if (ftyp == INVALID_FILE_ATTRIBUTES) {
				*isDir = false;
				*isFile = false;
			}
			else if (ftyp & FILE_ATTRIBUTE_DIRECTORY) {
				*isDir = true;
				*isFile = false;
			}
			else {
				*isDir = false;
				*isFile = true;
			}
		}
	}
}

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
	if (Internal::Exists(filePath)) {
		if (onError) {
			onError("FileSystem::CreateFile tryig to create a file that already exists");
		}
		return;
	}

	HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		if (onError) {
			onError("FileSystem::CreateFile could not create file");
		}
	}
	else {
		if (onSuccess) {
			onSuccess(filePath);
		}
		CloseHandle(hFile);
	}
}

void FileSystem::CreateFolder(const char* folderPath, fpPathCallback onSuccess, fpErrorCallback onError) {
	if (Internal::Exists(folderPath)) {
		if (onError) {
			onError("FileSystem::CreateFolder tryig to create a folder that already exists");
		}
		return;
	}
	
	if (!CreateDirectoryA(folderPath, 0)) {
		if (onError) {
			onError("FileSystem::CreateFolder could not create folder");
		}
	}
	else {
		if (onSuccess) {
			onSuccess(folderPath);
		}
	}
}

void FileSystem::Exists(const char* path, fpExistsCallback onSuccess) {
	DWORD ftyp = GetFileAttributesA(path);
	if (ftyp == INVALID_FILE_ATTRIBUTES) {
		if (onSuccess) {
			onSuccess(path, false, false);
		}
	}
	else if (ftyp & FILE_ATTRIBUTE_DIRECTORY) {
		if (onSuccess) {
			onSuccess(path, true, false);
		}
	}
	else {
		if (onSuccess) {
			onSuccess(path, false, true);
		}
	}
}

void FileSystem::Delete(const char* path, fpPathCallback onSuccess, fpErrorCallback onError) {
	DWORD ftyp = GetFileAttributesA(path);
	if (ftyp == INVALID_FILE_ATTRIBUTES) {
		if (onError) {
			onError("FileSystem::Delete Can't delete invalid path");
		}
	}
	else if (ftyp & FILE_ATTRIBUTE_DIRECTORY) {
		if (RemoveDirectoryA(path)) {
			if (onSuccess) {
				onSuccess(path);
			}
		}
		else {
			if (onError) {
				onError("FileSystem::Delete Can't delete folder");
			}
		}
	}
	else {
		if (DeleteFileA(path)) {
			if (onSuccess) {
				onSuccess(path);
			}
		}
		else {
			if (onError) {
				onError("FileSystem::Delete Can't delete file");
			}
		}
	}
}

namespace FileSystem {
	namespace Internal { // TODO: Move to top
		void Traverse(const char* path, bool expectedToBefile, char* dirBuff, u32 dirLen, bool pre, u32 depth, fpDepthFirstIterateCallback onIterate) {
			u32 pathLen = 0;
			u32 local_i = 0;
			{ // Copy path to tmpPath
				for (local_i = 0; local_i < MAX_PATH; ++local_i) {
					dirBuff[local_i + dirLen] = path[local_i];
					if (path[local_i] == '\0') {
						break;
					}
				}

				if (!expectedToBefile) {
					if (dirBuff[(local_i - 1) + dirLen] != '\\') {
						dirBuff[dirLen + local_i++] = '\\';
					}
				}

				pathLen = local_i;
				dirBuff[dirLen + pathLen] = '\0';
			}

			bool isDir = false;
			bool isFile = false;
			Internal::GetInfo(dirBuff, &isDir, &isFile);

			if (!isDir && !isFile) {
				Log("Error traversing directory, non existant object?\n");
				return;
			}
			else if (isDir && isFile) {
				Log("Error traversing directory, both a directory and file?\n");
				return;
			}

			if (expectedToBefile && !isFile) {
				Log("Error expecting file to be object, but it wasn't\n");
				return;
			}
			else if (!expectedToBefile && !isDir) {
				Log("Error expecting folder to be object, but it wasn't\n");
				return;
			}

			if (isDir) {
				if (pre) {
					if (onIterate) {
						onIterate(dirBuff, depth, isDir, isFile);
					}
				}

				{ // Add \*.*
					if (dirBuff[dirLen + (local_i - 1)] != '*') {
						dirBuff[dirLen + local_i++] = '*';
					}

					if (dirBuff[dirLen + (local_i - 1)] != '.') {
						dirBuff[dirLen + local_i++] = '.';
					}

					if (dirBuff[dirLen + (local_i - 1)] != '*') {
						dirBuff[dirLen + local_i++] = '*';
					}

					dirBuff[dirLen + local_i] = '\0';
				}

				WIN32_FIND_DATAA ffd;
				HANDLE currentFile = FindFirstFileA(dirBuff, &ffd);
				dirBuff[dirLen + pathLen] = '\0'; // Remove \*.* from end of path
				BOOL nextfileFound = FindNextFileA(currentFile, &ffd); // .
				nextfileFound = FindNextFileA(currentFile, &ffd); // ..
				while (nextfileFound) {
					bool isDirectory = ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
					Traverse(ffd.cFileName, !isDirectory, dirBuff, dirLen + pathLen, pre, depth + 1, onIterate); // TODO: Maybe wrong
					nextfileFound = FindNextFileA(currentFile, &ffd);
				}
				FindClose(currentFile);

				if (!pre) {
					if (onIterate) {
						onIterate(dirBuff, depth, isDir, isFile);
					}
				}
				dirBuff[dirLen] = '\0';
			}
			else { // Is file
				if (onIterate) {
					onIterate(dirBuff, depth, isDir, isFile);
				}
				dirBuff[dirLen] = '\0';
			}
		}
	}
}

void FileSystem::PreOrderDepthFirstTraversal(const char* path, fpDepthFirstIterateCallback onIterate, fpDepthFirstFinishedCallback onFinished) {
	bool isDir = false;
	bool isFile = false;
	Internal::GetInfo(path, &isDir, &isFile);

	// Have not recursed yet, calling on invalid path
	if (!isDir && !isFile) {
		if (onFinished) {
			onFinished();
		}
		return;
	}
	else if (isDir && isFile) {
		if (onFinished) {
			onFinished();
		}
		return;
	}

	char tmpName[CUSTOM_PATH_LEN]; // TODO: Could be a dynamic buffer
	GetCurrentDirectoryA(CUSTOM_PATH_LEN, tmpName);
	u32 len = 0;
	for (char* t = tmpName; *t != '\0'; t++, len++);
	if (tmpName[len - 1] != '\\') {
		tmpName[len++] = '\\';
		tmpName[len] = '\0';
	}

	Internal::Traverse(path, false, tmpName, len, true, 0, onIterate);

	if (onFinished) {
		onFinished();
	}
}

void FileSystem::PostOrderDepthFirstTraversal(const char* path, fpDepthFirstIterateCallback onIterate, fpDepthFirstFinishedCallback onFinished) {
	bool isDir = false;
	bool isFile = false;
	Internal::GetInfo(path, &isDir, &isFile);

	// Have not recursed yet, calling on invalid path
	if (!isDir && !isFile) {
		if (onFinished) {
			onFinished();
		}
		return;
	}
	else if (isDir && isFile) {
		if (onFinished) {
			onFinished();
		}
		return;
	}

	char tmpName[CUSTOM_PATH_LEN]; // TODO: Could be a dynamic buffer
	GetCurrentDirectoryA(CUSTOM_PATH_LEN, tmpName);
	u32 len = 0;
	for (char* t = tmpName; *t != '\0'; t++, len++);
	if (tmpName[len - 1] != '\\') {
		tmpName[len++] = '\\';
		tmpName[len] = '\0';
	}

	Internal::Traverse(path, false, tmpName, len, false, 0, onIterate);

	if (onFinished) {
		onFinished();
	}
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