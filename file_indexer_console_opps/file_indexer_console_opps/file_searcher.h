#pragma once

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cwctype>

class FileSearcher {
public:
    void SetIndex(const std::vector<std::wstring>& index) {
        fileIndex = &index;
    }
   
    void Search(const std::wstring& query) const {
		std::cout << "There are " << fileIndex->max_size() << std::endl;
        std::wcout << L"Searching for: " << query << std::endl;
        int found = 0;

        for (const auto& path : *fileIndex) {
            if (CaseInsensitiveMatch(path, query)) {
                std::wcout << path << std::endl;
                if (++found >= 100) {
                    std::wcout << L"...(limited to 100 results)" << std::endl;
                    break;
                }
            }
        }

        if (found == 0)
            std::wcout << L"No matching files found." << std::endl;
    }

private:
    const std::vector<std::wstring>* fileIndex = nullptr;

    static bool CaseInsensitiveMatch(const std::wstring& haystack, const std::wstring& needle) {

        auto it = std::search(
            haystack.begin(), haystack.end(),
            needle.begin(), needle.end(),
            [](wchar_t ch1, wchar_t ch2) {
                return std::towlower(ch1) == std::towlower(ch2);
            });
        return (it != haystack.end());
    }
};
