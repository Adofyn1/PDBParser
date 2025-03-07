// Author: Adofyn
// Created: 07.03.2025 10:03

#pragma once
#include <fstream>
#include <string>
#include <vector>

class File
{
private:
    std::ifstream file_;

public:
    explicit File(const std::string& filePath, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
    ~File();

    std::vector<uint8_t> GetRawData();
    size_t GetSize();
};
