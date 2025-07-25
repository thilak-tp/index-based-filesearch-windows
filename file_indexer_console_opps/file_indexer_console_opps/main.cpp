#include "file_indexer.h"
#include "file_searcher.h"

BOOL IsNTFS(const std::wstring& rootPath) {
    wchar_t fileSystemName[MAX_PATH];
    if (GetVolumeInformationW(
        rootPath.c_str(), nullptr, 0, nullptr, nullptr, nullptr,
        fileSystemName, MAX_PATH)) {
        return _wcsicmp(fileSystemName, L"NTFS") == 0;
    }
    return FALSE;
}


int main(int argc, wchar_t* argv[]) {

    std::wstring drive = L"C:\\";
    if (!IsNTFS(drive)) {
        std::wcerr << L"Drive is not NTFS. MFT indexing is not supported." << std::endl;
        return -1;
    }

    FileIndexer indexer;
    FileSearcher searcher;

    std::wcout << L"Indexing drive: " << drive << std::endl;

    indexer.Index(drive);

    searcher.SetIndex(indexer.GetIndex());

    std::wstring query;
    while (true) {
        std::wcout << L"\nEnter filename to search (or type 'exit'): ";
        std::getline(std::wcin, query);

        if (query == L"exit") break;
        searcher.Search(query);
    }

    return 0;
}
