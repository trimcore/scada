#ifndef SCADA_SCADAABI1HELPERS_H
#define SCADA_SCADAABI1HELPERS_H

// Manifold ABI, version 1, Helpers API

namespace Scada::ABI {

    // DirPathBreak
    //  - 

    SCADA_DLL_IMPORT std::size_t Api1DirPathBreakAz (const char * path, Atom * atoms, std::size_t max) noexcept;
    SCADA_DLL_IMPORT std::size_t Api1DirPathBreakWz (const wchar_t * path, Atom * atoms, std::size_t max) noexcept;
    SCADA_DLL_IMPORT std::size_t Api1DirPathBreakAsv (const char * path, std::size_t length, Atom * atoms, std::size_t max) noexcept;
    SCADA_DLL_IMPORT std::size_t Api1DirPathBreakWsv (const wchar_t * path, std::size_t length, Atom * atoms, std::size_t max) noexcept;
}

#endif

