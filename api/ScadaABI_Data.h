#ifndef SCADA_SCADAABI_DATA_H
#define SCADA_SCADAABI_DATA_H

// Manifold ABI, Cell Data API
//  - 

namespace Scada::ABI {

    // ScadaGet
    //  - 'cell' is assigned full serializable copy of the cell
    //     - this means 'cell->id' is valid ID, and 'cell->lock' is INVALID and direct member functions (all do locking) WILL FAIL or HANG
    //  - returns sizeof(Cell) on success
    //            0 on failure; call GetLastError () for details
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaGet (Scada::Cell::Handle handle, Scada::Cell * cell) noexcept;

    // ScadaGetID
    //  - returns ID of the cell identified by open 'handle'
    //            or {0,0} on error; call GetLastError () for details
    //
    SCADA_DLL_IMPORT Cell::ID TRIMCORE_APIENTRY ScadaGetID (Scada::Cell::Handle handle) noexcept;

    // ScadaGetValue
    //  - faster way to get cell value only
    //  - 'value' is assigned copy of cell 'value' union member
    //  - returns sizeof(Cell::Information) on success
    //            0 on failure; call GetLastError () for details
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaGetValue (Scada::Cell::Handle handle, Scada::Cell::Information * value) noexcept;

    // ScadaGetCellSpecs
    //  - 
    //
    SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaGetCellSpecs (Scada::Cell::Handle handle, Scada::Cell::Specs * specs) noexcept;

    // SetValueFlags
    //  - 
    //
    enum SetValueFlags : std::size_t {
        DefaultSetMode      = 0x0000'0000,
        UpdateOnChangeOnly  = 0x0000'0001, // updates data, timestamp, dependendencies and calls listeners only if 'value' differs from what's in the cell (returns false if unchanged)
        IgnoreInfoSpecs     = 0x0000'0002, // 
    };

    // ScadaSetValue
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaSetValue (Scada::Cell::Handle handle, const Scada::Cell::Information * value, SetValueFlags flags) noexcept;

    // ScadaSetFlags
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaSetFlags (Scada::Cell::Handle handle, Scada::Cell::Flags mask, Scada::Cell::Flags value) noexcept;

    // ScadaStream
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaStream (Scada::Cell::Handle handle, const void * data, std::size_t size,
                                                                const Manifold::Address * remote, const Manifold::Address * local) noexcept;




    // ScadaListen
    //  - 
    //
    SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY ScadaListen (Scada::Cell::Handle handle, ListenCallback, void * context) noexcept;

    // ScadaSignal
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaSignal (Scada::Cell::Handle handle) noexcept;
}

#endif
