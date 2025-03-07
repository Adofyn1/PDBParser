// Author: Adofyn
// Created: 07.03.2025 10:03

#pragma once
#include <Windows.h>
#include <string>

class SymbolLoader
{
private:
    struct PdbInfo
    {
        DWORD signature;
        GUID guid;
        DWORD age;
        char pdbFileName[1];
    };

    static std::string CleanerGuid(std::wstring guid);

public:
    static int Initialize(IN const std::string& modulePath, IN const std::string& toPath, OUT std::string& pdbPath, IN bool waitForConnection = false);
};
