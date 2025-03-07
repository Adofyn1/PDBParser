#include "SymbolLoader.h"

#include <filesystem>
#include <sstream>
#include <thread>
#include <wininet.h>

#include "../File/File.h"
#include "../skCrypter.h"

#ifdef DEBUG
#define LOG std::printf
#else
#define LOG()
#endif

std::string SymbolLoader::CleanerGuid(std::wstring guid)
{
    guid.erase( std::ranges::remove( guid, '{' ).begin(), guid.end() );
    guid.erase( std::ranges::remove( guid, '}' ).begin(), guid.end() );
    guid.erase( std::ranges::remove( guid, '-' ).begin(), guid.end() );

    return std::string( guid.begin(), guid.end() );
}

int SymbolLoader::Initialize(const std::string& modulePath, const std::string& toPath, std::string& pdbPath, bool waitForConnection)
{
    File moduleFile( modulePath, std::ios::binary );

    auto rawData = moduleFile.GetRawData();
    if (rawData.empty())
    {
        LOG( "[ERROR] Failed to get file data: %s\n", modulePath.c_str() );
        return 1;
    }

    const auto* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(rawData.data());
    auto* ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(rawData.data() + dosHeader->e_lfanew);
    const auto* fileHeader = &ntHeader->FileHeader;

    IMAGE_OPTIONAL_HEADER32* optHeader32 = nullptr;
    IMAGE_OPTIONAL_HEADER64* optHeader64 = nullptr;
    bool isX64 = false;

    if (fileHeader->Machine == IMAGE_FILE_MACHINE_AMD64)
    {
        optHeader64 = &ntHeader->OptionalHeader;
        isX64 = true;
    }
    else if (fileHeader->Machine == IMAGE_FILE_MACHINE_I386)
        optHeader32 = reinterpret_cast<IMAGE_OPTIONAL_HEADER32*>(&ntHeader->OptionalHeader);
    else
    {
        LOG( "[ERROR] Invalid file architecture\n" );
        return 2;
    }

    const IMAGE_DATA_DIRECTORY dataDir = isX64 ? optHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG] : optHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    const auto* debugDir = reinterpret_cast<IMAGE_DEBUG_DIRECTORY*>(rawData.data() + dataDir.VirtualAddress);

    if (!dataDir.Size || debugDir->Type != IMAGE_DEBUG_TYPE_CODEVIEW)
    {
        LOG( "[ERROR] No PDB debug data\n" );
        return 3;
    }

    const auto* pdbInfo = reinterpret_cast<PdbInfo*>(rawData.data() + debugDir->AddressOfRawData);
    if (pdbInfo->signature != 0x53445352)
    {
        LOG( "[ERROR] Invalid PDB signature\n" );
        return 4;
    }

    pdbPath = toPath;
    pdbPath.append( "\\" ).append( isX64 ? "x64\\" : "x86\\" );

    if (!std::filesystem::is_directory( pdbPath ))
        std::filesystem::create_directory( pdbPath );

    const auto pdbFileName = std::string( pdbInfo->pdbFileName, pdbInfo->pdbFileName + strlen( pdbInfo->pdbFileName ) );
    pdbPath.append( pdbFileName );

    if (std::filesystem::exists( pdbPath ))
        return 0;

    wchar_t wGuid[100]{ };
    if (!StringFromGUID2( pdbInfo->guid, wGuid, sizeof(wGuid) ))
    {
        LOG( "[ERROR] Failed convert GUID to string\n" );
        return 5;
    }

    const auto cleanGuid = CleanerGuid( wGuid );
    std::stringstream url;
    url << $( "https://msdl.microsoft.com/download/symbols/" ) << pdbFileName << '/' << cleanGuid << pdbInfo->age << '/' << pdbFileName;
    LOG( "URL for download PDB: %s\n", url.str().c_str() );

    if (waitForConnection)
    {
        while (InternetCheckConnectionA( $( "https://msdl.microsoft.com" ), FLAG_ICC_FORCE_CONNECTION, NULL ) == FALSE)
        {
            if (GetLastError() == ERROR_INTERNET_CANNOT_CONNECT)
            {
                LOG( "[ERROR] Cannot connect to Microsoft Symbol Server\n" );
                return 6;
            }

            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        }
    }

    const auto hr = URLDownloadToFileA( nullptr, url.str().c_str(), pdbPath.c_str(), 0, nullptr );
    if (FAILED( hr ))
    {
        LOG( "[ERROR] Failed download PDB. Code: %X\n", hr );
        return 7;
    }

    return 0;
}
