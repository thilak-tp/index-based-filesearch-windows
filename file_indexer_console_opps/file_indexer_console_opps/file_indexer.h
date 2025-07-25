#pragma once

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cwctype>
#include <windows.h>
#include <winioctl.h>

#include "benchmark.h"
namespace fs = std::filesystem;
class FileIndexer {
public:
    void Index(const std::wstring& root) {
        fileIndex.clear();
        fileCount = 0;
        //IndexRecursiveWin32(root);
        //IndexRecursiveFS(root);
        IndexRecursiveMFT(root);
    }

    const std::vector<std::wstring>& GetIndex() const {
        return fileIndex;
    }

private:
    LatencyBenchmark bench;
    long long fileCount;
    std::vector<std::wstring> fileIndex;

    void IndexRecursiveWin32(const std::wstring& folder, bool isRoot = true) {
        
        //std::cout << "Using Win32 APIs" << std::endl;
		if(isRoot)
            bench.start();
        WIN32_FIND_DATAW findData;
        HANDLE hFind;
        std::wstring searchPath = folder + L"\\*";

        hFind = FindFirstFileW(searchPath.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) return;

        do {
            std::wstring name = findData.cFileName;
            if (name == L"." || name == L"..") continue;

            std::wstring fullPath = folder + L"\\" + name;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                IndexRecursiveWin32(fullPath, false);
            }
            else {
                fileIndex.push_back(fullPath);
            }
        } while (FindNextFileW(hFind, &findData) != 0);
        if (isRoot) {
            bench.stop();
            bench.report();
        }
        FindClose(hFind);
        
    }



    void IndexRecursiveFS(const std::wstring& folder) {
        

        std::cout << "Using std::filesystem from C++ 17" << std::endl;
        bench.start();
        try {
            for (const auto& entry : fs::recursive_directory_iterator(folder, fs::directory_options::skip_permission_denied)) {
                if (entry.is_regular_file()) {
                    fileIndex.push_back(entry.path().wstring());
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            std::wcerr << L"Filesystem error: " << e.what() << std::endl;
        }
        catch (const std::exception& e) {
            std::wcerr << L"Error: " << e.what() << std::endl;
        }
        bench.stop();
        bench.report();
    }

    void IndexRecursiveMFT(const std::wstring& folder) {
       
        std::wcout << L"Using NTFS MFT" << std::endl;

		bench.start();
        std::wstring volumeRoot = folder.substr(0, 3); // e.g., L"C:\\"
        std::wstring volumePath = L"\\\\.\\" + std::wstring(1, towupper(volumeRoot[0])) + L":";

        HANDLE hVolume = CreateFileW(
            volumePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr
        );

        if (hVolume == INVALID_HANDLE_VALUE) {
            std::wcerr << L"Failed to open volume: " << volumePath << std::endl;
            return;
        }

        USN_JOURNAL_DATA journalData = { 0 };
        DWORD bytesReturned = 0;

        if (!DeviceIoControl(hVolume,
            FSCTL_QUERY_USN_JOURNAL,
            nullptr,
            0,
            &journalData,
            sizeof(journalData),
            &bytesReturned,
            nullptr)) {
            std::wcerr << L"FSCTL_QUERY_USN_JOURNAL failed." << std::endl;
            CloseHandle(hVolume);
            return;
        }

        MFT_ENUM_DATA_V0 mftEnum = { 0 };
        mftEnum.StartFileReferenceNumber = 0;
        mftEnum.LowUsn = 0;
        mftEnum.HighUsn = journalData.NextUsn;

        std::vector<BYTE> buffer(1024 * 1024); // 1 MB buffer

        while (DeviceIoControl(hVolume,
            FSCTL_ENUM_USN_DATA,
            &mftEnum,
            sizeof(MFT_ENUM_DATA_V0),
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            &bytesReturned,
            nullptr)) {

            BYTE* p = buffer.data();
            USN usn = *(USN*)p;
            p += sizeof(USN);

            while (p < buffer.data() + bytesReturned) {
                USN_RECORD* record = (USN_RECORD*)p;

                if (record->FileNameLength > 0 && (record->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    std::wstring fileName(record->FileName, record->FileNameLength / sizeof(WCHAR));
                    fileIndex.push_back(fileName);
                    //fileCount += 1;
                }

                p += record->RecordLength;
            }

            mftEnum.StartFileReferenceNumber = *(ULONGLONG*)buffer.data(); // or usn;
        }

        DWORD err = GetLastError();
        if (err != ERROR_HANDLE_EOF) {
            std::wcerr << L"FSCTL_ENUM_USN_DATA failed. Error: " << err << std::endl;
        }
       
        CloseHandle(hVolume);
        bench.stop();
        bench.report();
		
    }




};
