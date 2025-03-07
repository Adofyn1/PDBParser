#include "File.h"

File::File(const std::string& filePath, const std::ios_base::openmode mode)
{
    file_.open( filePath, mode );
}

File::~File()
{
    if (file_.is_open())
        file_.close();
}

std::vector<uint8_t> File::GetRawData()
{
    if (!file_.is_open())
        return { };

    const auto size = GetSize();
    if (!size)
        return { };

    std::vector<uint8_t> rawData( size );
    file_.read( reinterpret_cast<char*>(rawData.data()), size );

    return rawData;
}

size_t File::GetSize()
{
    if (!file_.is_open())
        return 0;

    file_.seekg( 0, std::ios::end );
    const auto size = file_.tellg();
    file_.seekg( 0, std::ios::beg );

    return size;
}
