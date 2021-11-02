#ifndef SCADA_SCADAABI1CELLS_H
#define SCADA_SCADAABI1CELLS_H

// Manifold ABI, version 1, Cells API

namespace Scada::ABI {

    // Api1CellCreate
    //  - creates a simple new cell to store 1 value of provided type/size
    //
    SCADA_DLL_IMPORT Cell::Handle Api1CellCreate (const Atom * path, std::size_t depth,
                                                  Cell::Type type, std::uint8_t size) noexcept;

    // Api1CellCreateTemplate
    //  - creates new cell using 'template' to initialize the data
    //  - 'template' is optional, NULL cell is created if missing
    //
    SCADA_DLL_IMPORT Cell::Handle Api1CellCreateTemplate (const Atom * path, std::size_t depth, Cell * template_) noexcept;

    // Api1CellAccess[Path]
    //  - retrieves handle to a cell, either by ID or name
    //  - access level is honored only in isolated processes

    enum class Api1AccessLevel : unsigned int {
        Same = ~0u, // Api1CellDuplicateHandle only
        Test = 0u,  // no access, only verify existence
        Wait = 1u,
        Read = 2u,
        Set  = 3u,
        Change = 4u // change type
    };
    SCADA_DLL_IMPORT Cell::Handle Api1CellAccess (Cell::ID id, Api1AccessLevel level) noexcept;
    SCADA_DLL_IMPORT Cell::Handle Api1CellAccessPath (const Atom * path, std::size_t depth, Api1AccessLevel level) noexcept;
    SCADA_DLL_IMPORT Cell::Handle Api1CellDuplicateHandle (Cell::Handle handle, Api1AccessLevel level) noexcept;
    
    // Api1CellAccessOrCreate
    // SCADA_DLL_IMPORT Cell::Handle Api1CellAccessOrCreate (const Atom * path, std::size_t depth, Api1AccessLevel level) noexcept;

    // Api1CellRead
    //  - returns copy of a Cell prepared for network transmission (with ID)
    //  - verifies CRC and sets 'invalid' flag on failure
    //  - can crash on invalid 'handle' or return garbage
    //
    SCADA_DLL_IMPORT Cell Api1CellRead (Cell::Handle handle) noexcept;

    // Api1CellGetID
    //  - returns Cell ID for given Cell handle
    //  - returns {0,0} on invalid handle
    //
    SCADA_DLL_IMPORT Cell::ID Api1CellGetID (Cell::Handle handle) noexcept;

    // Api1CellGetDataType
    //  - retrieves current data type of the cell
    //  - data type is supposed to be stable/immutable, but e.g. data type of cell loaded from settings might not be known
    //
    SCADA_DLL_IMPORT Cell::Specs Api1CellGetDataType (Cell::Handle handle) noexcept;

    // Api1CellGetUpdateTime
    //  - 
    //
    SCADA_DLL_IMPORT FILETIME Api1CellGetUpdateTime (Cell::Handle handle) noexcept;

    // Api1Cell[Res|S]etInvalidFlag
    //  - 
    //
    SCADA_DLL_IMPORT bool Api1CellSetInvalidFlag (Cell::Handle handle) noexcept;
    SCADA_DLL_IMPORT bool Api1CellResetInvalidFlag (Cell::Handle handle) noexcept;


    // Api1CellSetBoolean
    //  - 
    //  - function will fail if ...
    //
    SCADA_DLL_IMPORT bool Api1CellSetBoolean (Cell::Handle handle, bool value) noexcept;
    SCADA_DLL_IMPORT bool Api1CellUpdateBoolean (Cell::Handle handle, bool value) noexcept;

    // Api1CellGetBoolean
    //  - 
    //  - function will fail if the value type is not single integer or unsigned
    //     - call Api1CellConvertToBoolean
    //
    SCADA_DLL_IMPORT bool Api1CellGetBoolean (Cell::Handle handle, bool * value) noexcept;

    // TODO: Timestamp
    // TODO: Address
    // TODO: mask?
    // TODO: timer, sigballed by threadpool

    // Api1CellSetUnsigned[n]
    //  - 
    //  - returns true on success, false on ...???
    //     - 'values' is unaffected on failure
    //
    SCADA_DLL_IMPORT bool Api1CellSetUnsigned64 (Cell::Handle handle, const std::uint64_t * values, std::size_t n) noexcept;
    SCADA_DLL_IMPORT bool Api1CellUpdateUnsigned64 (Cell::Handle handle, const std::uint64_t * values, std::size_t n) noexcept;

    SCADA_DLL_IMPORT bool Api1CellGetUnsigned64 (Cell::Handle handle, std::uint64_t * values, std::size_t n) noexcept;

    //
    // Api1CellSetString[A/U/W][z/sv]
    //  - 
    //  - returns false on error, sets following GetLastError codes:
    //     - ERROR_FILE_TOO_LARGE - string is too long to fit into Cell (32 bytes)
    //     - ERROR_DATATYPE_MISMATCH - cell type is not 'Text'

    SCADA_DLL_IMPORT bool Api1CellSetStringAz (Cell::Handle handle, const char *) noexcept;
    SCADA_DLL_IMPORT bool Api1CellSetStringUz (Cell::Handle handle, const char8_t *) noexcept;
    SCADA_DLL_IMPORT bool Api1CellSetStringWz (Cell::Handle handle, const wchar_t *) noexcept;
    SCADA_DLL_IMPORT bool Api1CellSetStringAsv (Cell::Handle handle, const char *, std::size_t) noexcept;
    SCADA_DLL_IMPORT bool Api1CellSetStringUsv (Cell::Handle handle, const char8_t *, std::size_t) noexcept;
    SCADA_DLL_IMPORT bool Api1CellSetStringWsv (Cell::Handle handle, const wchar_t *, std::size_t) noexcept;

    // Api1CellGetString[A/U/W][z]
    //  - copies string from cell to buffer
    //  - returns string length in code-points
    //  - 'z' variant terminates the string with a NUL character
    //  - returns 0 on error, sets following GetLastError codes:
    //     - ERROR_SUCCESS - it's actually an empty string
    //     - ERROR_DATATYPE_MISMATCH - string contains characters that cannot be represented in target charset
    //                               - TODO: call Api1CellConvertToString to get conversion
    //     - 

    SCADA_DLL_IMPORT std::size_t Api1CellGetStringA (Cell::Handle handle, char * buffer, std::size_t length) noexcept;
    SCADA_DLL_IMPORT std::size_t Api1CellGetStringU (Cell::Handle handle, char8_t * buffer, std::size_t length) noexcept;
    SCADA_DLL_IMPORT std::size_t Api1CellGetStringW (Cell::Handle handle, wchar_t * buffer, std::size_t length) noexcept;
    SCADA_DLL_IMPORT std::size_t Api1CellGetStringAz (Cell::Handle handle, char * buffer, std::size_t length) noexcept;
    SCADA_DLL_IMPORT std::size_t Api1CellGetStringUz (Cell::Handle handle, char8_t * buffer, std::size_t length) noexcept;
    SCADA_DLL_IMPORT std::size_t Api1CellGetStringWz (Cell::Handle handle, wchar_t * buffer, std::size_t length) noexcept;


    // Api1CellGetData
    //  - copies raw data from Data cell to buffer
    //  - returns number of bytes copied
    //  - returns 0 on error, sets following GetLastError codes:
    //     - ERROR_SUCCESS - it's actually an 0 bytes long data variable
    //     - ERROR_DATATYPE_MISMATCH - not a Data cell
    //     - ERROR_INSUFFICIENT_BUFFER - buffer length supplied is not enough to fit all data
    //
    SCADA_DLL_IMPORT std::size_t Api1CellGetData (Cell::Handle handle, void * buffer, std::size_t length) noexcept;

    // Api1CellSetData
    //  - copies raw 'data' to Data cell
    //  - returns false on error, sets following GetLastError codes:
    //     - ERROR_DATATYPE_MISMATCH - not a Data cell
    //     - ERROR_FILE_TOO_LARGE - 'size' exceeds supported storage space (currently 32 bytes)
    //
    SCADA_DLL_IMPORT bool Api1CellSetData (Cell::Handle handle, const void * data, std::size_t size) noexcept;


    // Api1CellGetAddress
    //  - 
    //
    SCADA_DLL_IMPORT bool Api1CellGetAddress (Cell::Handle handle, Manifold::Address * address) noexcept;

    // Api1CellSetAddress
    //  - 
    //  - returns false on error, sets following GetLastError codes:
    //     - ERROR_UNSUPPORTED_TYPE - invalid address family
    //     - ERROR_FILE_TOO_LARGE - AF_UNIX path exceeds supported storage, 32 bytes
    //
    SCADA_DLL_IMPORT bool Api1CellSetAddress (Cell::Handle handle, const Manifold::Address * address) noexcept;
    SCADA_DLL_IMPORT bool Api1CellUpdateAddress (Cell::Handle handle, const Manifold::Address * address) noexcept;

    
    SCADA_DLL_IMPORT bool Api1CellGetTimeStamp (Cell::Handle handle, _Out_ FILETIME *) noexcept;
    SCADA_DLL_IMPORT bool Api1CellSetTimeStamp (Cell::Handle handle, const FILETIME *) noexcept;
    SCADA_DLL_IMPORT bool Api1CellUpdateTimeStamp (Cell::Handle handle, const FILETIME *) noexcept;

    // Api1CellGetFloat[32|64]
    //  - 
    //  - 
    //  - returns false on error, sets following GetLastError codes:
    //     - ERROR_DATATYPE_MISMATCH - not a Float/Integer/Unsigned cell
    //     - ERROR_INVALID_DATA - 'invalid' flag was set
    //     - ERROR_DATA_CHECKSUM_ERROR - 
    //     - ERROR_INSUFFICIENT_BUFFER - buffer length supplied is not enough to fit all data
    //     - ERROR_FILE_CORRUPT
    //
    SCADA_DLL_IMPORT bool Api1CellGetFloat32 (Cell::Handle handle, float *, std::size_t) noexcept;
    SCADA_DLL_IMPORT bool Api1CellGetFloat64 (Cell::Handle handle, double *, std::size_t) noexcept;

    // Api1CellSetFloat[32|64]
    //  - 
    //  - cell must be of Float type, value is converted to actual width of the cell
    //  - 
    //
    SCADA_DLL_IMPORT bool Api1CellSetFloat32 (Cell::Handle handle, const float *, std::size_t) noexcept;
    SCADA_DLL_IMPORT bool Api1CellSetFloat64 (Cell::Handle handle, const double *, std::size_t) noexcept;

    SCADA_DLL_IMPORT bool Api1CellUpdateFloat32 (Cell::Handle handle, const float *, std::size_t) noexcept;
    SCADA_DLL_IMPORT bool Api1CellUpdateFloat64 (Cell::Handle handle, const double *, std::size_t) noexcept;

    struct Api1CellSetInfo {
        Cell::Information::Type type = Cell::Information::Type::Null;
        std::uint64_t       timeout = 0;
        Cell::Information   information;
    };

    // Api1CellConvertToAtom 
    //  - 
    //  - value must contain single valid atom value, otherwise this function fails
    //
    SCADA_DLL_IMPORT bool Api1CellConvertToAtom  (Cell::Handle handle, Atom * result, Atom::InvalidInputReason * error) noexcept;

    // Api1CellConvertToAtoms
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t Api1CellConvertToAtoms (Cell::Handle handle, Atom * result, std::size_t max, Atom::InvalidInputReason * error) noexcept;


    // Api1CellStreamData
    //  -
    //
    SCADA_DLL_IMPORT bool Api1CellStreamData (Cell::Handle handle,
                                              const void * data, std::size_t size,
                                              const Manifold::Address * remote, const Manifold::Address * local) noexcept;

    // Api1CellSignal
    //  - synchronously invokes all manifold-local callbacks listening on the cell
    //  - TODO: signal other manifolds
    //
    SCADA_DLL_IMPORT bool Api1CellSignal (Cell::Handle handle) noexcept;

    // Api1CellRelease
    //  - 
    //
    SCADA_DLL_IMPORT void Api1CellRelease (Cell::Handle handle) noexcept;
}

#endif

