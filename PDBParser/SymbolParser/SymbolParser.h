// Author: Adofyn
// Created: 07.03.2025 13:03

#pragma once
#include <string>

class SymbolParser
{
private:
    HANDLE process_;
    uint64_t symbolTable_;

public:
    explicit SymbolParser(const std::string& pdbPath);
    ~SymbolParser();

    uint32_t GetSymbolRva(const std::string& symbolName) const;
};
