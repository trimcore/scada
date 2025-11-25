#ifndef SCADA_SCADAABI_DIRECTORY_H
#define SCADA_SCADAABI_DIRECTORY_H

// Manifold ABI, version 1, Cell Directory API
//  - 

namespace Scada::ABI {

    // ScadaCellCreate
    //  - 
    //
    SCADA_DLL_IMPORT Scada::Cell::Handle TRIMCORE_APIENTRY ScadaCellCreate (const Scada::Atom * path, std::size_t length, Scada::Atom name,
                                                                            const Scada::Cell::Information * value) noexcept;

    // ScadaAccess
    //  -
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaAccess (const Scada::Atom * path, std::size_t length,
                                                                const Scada::Atom * names, std::size_t count,
                                                                Scada::Cell::Handle * handles) noexcept;

    // ScadaAccessID
    //  - 
    //
    SCADA_DLL_IMPORT Scada::Cell::Handle TRIMCORE_APIENTRY ScadaAccessID (Scada::Cell::ID id) noexcept;

    // ScadaDuplicateHandle
    //  - 
    //
    SCADA_DLL_IMPORT Scada::Cell::Handle TRIMCORE_APIENTRY ScadaDuplicateHandle (Scada::Cell::Handle handle) noexcept;

    // ScadaRelease
    //  - 
    //  - all successfully released handles in the 'handles' array are set to NULL
    //  - returns number of successfully released handles
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaRelease (std::size_t count, Scada::Cell::Handle * handles) noexcept;

    // ScadaDirConstruct
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaDirConstruct (const Scada::Atom * path, std::size_t depth) noexcept;

    // ScadaDirInstantiate
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaDirInstantiate (const Atom * source, std::size_t nsrc,
                                                                        const Atom * target, std::size_t ntgt,
                                                                        const Atom * differences, std::size_t ndiff) noexcept;
    
    // ScadaDirConstruct
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaDirListenSub (const Scada::Atom * path, std::size_t depth,
                                                                      NewSubDirCallback, FinalizeCallback, void * context) noexcept;

    // ScadaDirConstruct
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaDirListenCell (const Scada::Atom * path, std::size_t depth,
                                                                       NewCellCallback, FinalizeCallback, void * context) noexcept;

    // ScadaDirEnumCells
    //  - enumerates named cells from directory 'path' into 'buffer' of 'length' elements
    //  - returns actual number of cells in directory branch (which might be larger than 'length')
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaDirEnumCells (const Scada::Atom * path, std::size_t depth,
                                                                      CellEntry * buffer, std::size_t length) noexcept;

    // ScadaDirEnumSubs
    //  - enumerates subdirectories (branches) from directory 'path' into 'buffer' of 'length' elements
    //  - returns actual number of susb in the directory branch (which might be larger than 'length')
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaDirEnumSubs (const Scada::Atom * path, std::size_t depth,
                                                                     Scada::Atom * buffer, std::size_t length) noexcept;
}

#endif
