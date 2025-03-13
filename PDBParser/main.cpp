#include <iostream>

#include "SymbolLoader/SymbolLoader.h"
#include "SymbolParser/SymbolParser.h"

int main()
{
    const std::string ntdllPath = R"(C:\Windows\System32\ntdll.dll)";

    std::string pdbPath;
    const auto result = SymbolLoader::Initialize( ntdllPath, std::getenv( "TEMP" ), pdbPath, true );

    if (result == 0)
    {
        const SymbolParser symbolParser{ pdbPath };
        const auto ldrpHandleTlsDataOffset = symbolParser.GetSymbolRva( "LdrpHandleTlsData" );

        std::printf( "LdrpHandleTlsData offset: %X\n", ldrpHandleTlsDataOffset );
    }
    else
        std::printf( "Failed download PDB.\n" );

    std::cin.get();
    return 0;
}
