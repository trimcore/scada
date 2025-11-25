#ifndef SCADA_SCADAABI_H
#define SCADA_SCADAABI_H

// ScadaABI.h
//  - provides declarations of exports from Manifold.exe (imports to .dll modules)
//  - while contractural and stable, prefer abstractions in Scada.h

#include "..\SCADA\SCADA_Atom.hpp"
#include "..\SCADA\SCADA_Cell.hpp"

#ifndef SCADA_DLL_IMPORT
#define SCADA_DLL_IMPORT extern __declspec(dllimport)
#define SCADA_DLL_IMPORT_DEFINED
#endif

namespace Scada::ABI {

    // ScadaApiVersion
    //  - retrieves highest major API level supported, i.e. when 1 then only v1 API "Scada" functions are available
    //  - TODO: rename ScadaApiSupportDecare
    //
    SCADA_DLL_IMPORT unsigned int TRIMCORE_APIENTRY ScadaApiVersion () noexcept;
    SCADA_DLL_IMPORT void TRIMCORE_APIENTRY ScadaApiSupportDeclare (Scada::Atom) noexcept;
    SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaApiSupportRequire (Scada::Atom) noexcept;
    SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaApiCheckCompatibility (std::uint16_t major, std::uint16_t minor) noexcept;

    // ScadaManifoldName
    //  - 'Atom' name of the current manifold process
    //
    SCADA_DLL_IMPORT std::uint64_t ScadaManifoldName;

    // ScadaDllMscVer
    //  - value of _MSC_FULL_VER
    //
    SCADA_DLL_IMPORT const unsigned int ScadaDllMscVer;

    // ScadaDllTimestamp
    //  - BUILD_TIMESTAMP by TRIMCORE.h
    //
    SCADA_DLL_IMPORT const char * const ScadaDllTimestamp;


#pragma pack(push,2)
    struct CellEntry {
        Atom          name;
        Cell::ID      id;
        std::uint16_t flags = 0; // reserved
    };
#pragma pack(pop)

    // TODO: rename 'ListenCallback' to 'NotificationRoutine' or something???

    typedef void (TRIMCORE_APIENTRY * ListenCallback) (void * context, Scada::Cell::Handle handle, FILETIME t, Scada::Cell::Information value);
    typedef void (TRIMCORE_APIENTRY * NewCellCallback) (void * context, const Atom * path, std::size_t depth, const CellEntry * info);
    typedef void (TRIMCORE_APIENTRY * NewSubDirCallback) (void * context, const Atom * path, std::size_t depth, Atom name);
    typedef void (TRIMCORE_APIENTRY * FinalizeCallback) (void * context);
}

#include "ScadaABI_Msvc.h"
#include "ScadaABI_Connection.h"
#include "ScadaABI_Directory.h"
#include "ScadaABI_Data.h"

// #include "ScadaABI1Dir.h"
// #include "ScadaABI1Cells.h"
// #include "ScadaABI1Notify.h"
// #include "ScadaABI1Database.h"

#ifdef SCADA_DLL_IMPORT_DEFINED
#undef SCADA_DLL_IMPORT_DEFINED
#undef SCADA_DLL_IMPORT
#endif

#endif

