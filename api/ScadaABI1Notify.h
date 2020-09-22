#ifndef SCADA_SCADAABI1NOTIFY_H
#define SCADA_SCADAABI1NOTIFY_H

// Manifold ABI, version 1, Listen/Notification API
//  - 

namespace Scada::ABI {

    // Api1CellListen
    //  - 
    //  - cell is share-locked
    //  - state of other cells is unspecified, dependent cells may be updated before, after or in parallel to the callback
    //
    SCADA_DLL_IMPORT bool Api1CellListen (Cell::Handle cell,
                                          void (*callback) (void * context, Cell::Handle handle, FILETIME t, Cell::Flags flags, Scada::Cell::DataType, Cell::Information value),
                                          void * context) noexcept;

    // Api1DirListenSub
    //  - requests that 'callback' be invoked for every new subdirectory added into 'path'
    //  - 
    //
    SCADA_DLL_IMPORT bool Api1DirListenSub (const Atom * path, std::size_t depth,
                                            void (*callback) (void * context, const Atom * path, std::size_t depth, Atom sub),
                                            void * context) noexcept;
    
    // Api1DirListenCell
    //  - requests that 'callback' be invoked for every new Cell added into 'path'
    //  - TBD: what is locked
    //
    SCADA_DLL_IMPORT bool Api1DirListenCell (const Atom * path, std::size_t depth,
                                             void (*callback) (void * context, const Atom * path, std::size_t depth, Atom name, Cell::ID id),
                                             void * context) noexcept;
}

#endif
