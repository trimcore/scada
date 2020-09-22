#ifndef SCADA_SCADAABI1DIR_H
#define SCADA_SCADAABI1DIR_H

// Manifold ABI, version 1, Directory API
//  - 

namespace Scada::ABI {

    // Api1DirConstruct
    //  - ensures that directory tree contains all 'depth' elements of 'path'
    //  - returns nonzero if successfully created or all already exists
    //     - value is 1 + number of creates path levels
    //
    SCADA_DLL_IMPORT std::size_t Api1DirConstruct (const Atom * path, std::size_t depth) noexcept;

    // Api1DirExists
    //  - returns true if branch exists
    //
    SCADA_DLL_IMPORT bool Api1DirExists (const Atom * path, std::size_t depth) noexcept;

    // Api1DirCountCells/Api1DirCountSubs
    //  - returns number of (cell entries / sub branches) in particular directory branch
    //  - use as allocation hint, the environment is multithreaded and the count may be immediately inaccurate
    
    SCADA_DLL_IMPORT std::size_t Api1DirCountCells (const Atom * path, std::size_t depth) noexcept;
    SCADA_DLL_IMPORT std::size_t Api1DirCountSubs (const Atom * path, std::size_t depth) noexcept;

    // Api1DirListCellInfo
    //  - 
    //  - NOTE: same as Scada::Directory::Item structure
    //
#pragma pack(push,2)
    struct Api1DirListCellInfo {
        Atom          name;
        Cell::ID      cell;
        std::uint16_t flags; // reserved
    };
#pragma pack(pop)

    // Api1DirListCells 
    //  - enumerates named cells from directory 'path' into 'buffer' of 'length' elements
    //  - returns actual number of cells in directory branch (which might be larger than 'length')
    //
    SCADA_DLL_IMPORT std::size_t Api1DirListCells (const Atom * path, std::size_t depth, Api1DirListCellInfo * buffer, std::size_t length) noexcept;

    // Api1DirListSubs 
    //  - enumerates subdirectories (branches) from directory 'path' into 'buffer' of 'length' elements
    //  - returns actual number of susb in the directory branch (which might be larger than 'length')
    //
    SCADA_DLL_IMPORT std::size_t Api1DirListSubs (const Atom * path, std::size_t depth, Atom * buffer, std::size_t length) noexcept;
}

#endif
