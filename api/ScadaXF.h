#ifndef SCADA_SCADAXF_H
#define SCADA_SCADAXF_H

#include <Windows.h>
#include <cstddef>
#include <cstdint>

// SCADAxf.DLL
//  - minimal eXternal Functionality adapter DLL
//  - interface between full SCADA.DLL and legacy C++ software (C++11 or older, GCC 6, etc.)

#ifndef SCADA_DLL_IMPORT
#ifdef _MSC_VER
#define SCADA_DLL_IMPORT extern "C" __declspec(dllimport)
#else
#define SCADA_DLL_IMPORT extern "C" __attribute__((dllimport))
#endif
#define SCADA_DLL_IMPORT_DEFINED
#endif

#ifndef TRIMCORE_APIENTRY 
#ifdef _WIN64
#define TRIMCORE_APIENTRY APIENTRY
#else
#define TRIMCORE_APIENTRY __cdecl
#endif
#endif

// Minimal API/ABI adapters

namespace Scada {
    using Atom = std::uint64_t;

    namespace Cell {
        struct Handle__;
        typedef struct Handle__ * Handle;
    }
}

// ScadaApiVersion
//  - retrieves highest major API level supported, i.e. when 1 then only v1 API "Scada" functions are available
//
SCADA_DLL_IMPORT unsigned int TRIMCORE_APIENTRY ScadaApiVersion ();

SCADA_DLL_IMPORT const unsigned int ScadaDllMscVer;
SCADA_DLL_IMPORT const char * const ScadaDllTimestamp;

// Facilities that can be forwarded directly
//  - ScadaConnectName
//  - ScadaConnectAddr
//  - ScadaConnectionClose
//  - ScadaConnectionPump

#include "ScadaABI_Connection.h"

// 

SCADA_DLL_IMPORT Scada::Cell::Handle TRIMCORE_APIENTRY ScadaXfAccess (const char * path);
SCADA_DLL_IMPORT void TRIMCORE_APIENTRY ScadaXfRelease (Scada::Cell::Handle handle);

SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaXfSetI32X1 (Scada::Cell::Handle handle, std::int32_t value);
SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaXfSetU32X1 (Scada::Cell::Handle handle, std::uint32_t value);
SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaXfSetF32X1 (Scada::Cell::Handle handle, float value);
SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaXfSetF64X1 (Scada::Cell::Handle handle, double value);
SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaXfSetStrAz (Scada::Cell::Handle handle, const char * string);
SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaXfSetStrWz (Scada::Cell::Handle handle, const wchar_t * string);

#ifdef SCADA_DLL_IMPORT_DEFINED
#undef SCADA_DLL_IMPORT_DEFINED
#undef SCADA_DLL_IMPORT
#endif

namespace Scada {
    // Set (bool, int, string) // time?
    // Get ? from stimulus?
}

#endif
