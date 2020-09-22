#ifndef SCADA_SCADAABI_H
#define SCADA_SCADAABI_H

// ScadaABI.h
//  - provides declarations of exports from Manifold.exe (imports to .dll modules)
//  - while contractural and stable, prefer abstractions in Scada.h

#include "SCADA_Atom.hpp"
#include "SCADA_Cell.hpp"

#ifndef SCADA_DLL_IMPORT
#define SCADA_DLL_IMPORT extern __declspec(dllimport)
#define SCADA_DLL_IMPORT_DEFINED
#endif

namespace Scada::ABI {

    // ApiVersion
    //  - retrieves highest major API level supported, i.e. when 1 then only Api1 functions are available
    //
    SCADA_DLL_IMPORT unsigned int ApiVersion () noexcept;
    SCADA_DLL_IMPORT void ApiDeclareSupport (Scada::Atom) noexcept;
    SCADA_DLL_IMPORT bool ApiRequireSupport (Scada::Atom) noexcept;

    // ManifoldName
    //  - 'Atom' name of the current manifold process
    //
    SCADA_DLL_IMPORT std::uint64_t ManifoldName;
}

#include "ScadaABI1Dir.h"
#include "ScadaABI1Cells.h"
#include "ScadaABI1Notify.h"
#include "ScadaABI1Helpers.h"
#include "ScadaABI1Database.h"

#ifdef SCADA_DLL_IMPORT_DEFINED
#undef SCADA_DLL_IMPORT_DEFINED
#undef SCADA_DLL_IMPORT
#endif

#endif

