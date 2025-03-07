#include <Windows.h>

#include "SymbolParser.h"

#include <DbgHelp.h>

#include "../File/File.h"

#ifdef DEBUG
#define LOG std::printf
#else
#define LOG()
#endif

SymbolParser::SymbolParser(const std::string& pdbPath)
{
    process_ = OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetCurrentProcessId() );
    if (!process_)
    {
        LOG( "[SymbolParser] Can`t open current process: 0x%08lX\n", GetLastError() );
        return;
    }

    SymSetOptions( SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_AUTO_PUBLICS );

    if (!SymInitialize( process_, nullptr, FALSE ))
    {
        CloseHandle( process_ );
        LOG( "[SymbolParser] SymInitialize failed: 0x%08lX\n", GetLastError() );
        return;
    }

    File file( pdbPath );

    symbolTable_ = SymLoadModuleEx( process_, nullptr, pdbPath.c_str(), nullptr, 0x10000000, static_cast<DWORD>(file.GetSize()), nullptr, NULL );
    if (!symbolTable_)
    {
        SymCleanup( process_ );
        CloseHandle( process_ );

        LOG( "[SymbolParser] SymLoadModuleEx failed: 0x%08lx\n", GetLastError() );
        return;
    }
}

SymbolParser::~SymbolParser()
{
    SymCleanup( process_ );
    CloseHandle( process_ );
}

uint32_t SymbolParser::GetSymbolRva(const std::string& symbolName) const
{
    if (!process_ || !symbolTable_)
    {
        LOG( "[SymbolParser::GetSymbolAddress] Failed process or symbolTable\n" );
        return 0;
    }

    SYMBOL_INFO symbolInfo{ };
    symbolInfo.SizeOfStruct = sizeof( SYMBOL_INFO );

    if (!SymFromName( process_, symbolName.c_str(), &symbolInfo ))
    {
        LOG( "[SymbolParser::GetSymbolAddress] Search failed: 0x%08lX\n", GetLastError() );
        return 0;
    }

    return static_cast<uint32_t>(symbolInfo.Address - symbolInfo.ModBase);
}
