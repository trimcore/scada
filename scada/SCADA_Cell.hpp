#ifndef SCADA_CELL_HPP
#define SCADA_CELL_HPP

#include "TRIMCORE.h"
#include "SCADA_Atom.hpp"
#include "SCADA_AtomPath.hpp"
#include "SCADA_RwSpinLock.hpp"
#include "SCADA_Manifold_Address.hpp"
#include "..\Libraries\double_integer.hpp"

#pragma pack(push,2)
#pragma warning(3:4062)

namespace Scada {

    // Cell
    //  - 
    //
    struct Cell {

        // Cell::ID
        //  - unique identifier of a runtime information within the system
        //  - ID 0:0 usually means NULL, no such object, nothing referenced, etc
        //
        struct ID {
            std::uint32_t index; // unique number of the Cell within the segment (data area)
            std::uint16_t segment; // 0 - system information/status and settings, 1 - main data segment (TBD), and so on...

        public:
            inline constexpr ID () noexcept;
            inline constexpr ID (std::uint16_t segment, std::uint32_t index) noexcept;
            inline constexpr explicit operator bool () const noexcept;

            ID (const ID &) = default;
            ID & operator = (const ID &) = default;
        };

        // Expression
        //  - TBD
        //  - TODO: more than 4 opcodes when some do nothing?
        //  - target sets 'base' to expression definition cell (more cells can use the same expression)
        //     - our 'base' is???
        //  - app writes: 'raw/data' and 'raw/type' 
        //     - list of expression cells is checked?
        //     - bitmap: Segment::dependencies [cellid.index] (512 MB?) ??? one global map of cells wanted for cross-process
        //        - pointers to vector of IDs of dependent cells
        //
        struct Expression {
            static constexpr std::size_t max_inputs = 4;

            enum Operation : std::uint8_t {
                Nop = 0x00,

                // TODO:
                // Inc, // R++ if 'x' changes where x is a, b, c, or d, depending on trigger above
                // Dec,
                // For Shl,Shr,etc...
                //  - trigger may be 'a', but 'x' can be fixed value stored in 'b' (i.e. 1)
                //    then 'b_op' needs to be set to 'Shl' or whatever
                // Shl, // R <<= 'x'
                // Shr, // R >>= 'x'
                // ShlC, // circular SHL
                // ShrC, // circular SHR
                Add, // R += 'x'
                Subtract, // R -= 'x' // Negative operation instead?
                // Set, // R = 'x' ???
                Multiply, // R *= 'x'
                Divide, // R /= 'x'
                // Xor, // R ^= 'x'
                Equals,  // R = (R == 'x')
                EqualsAnyOf,
                NotEquals, // R = (R != 'x')

                // Less, // R = (R < 'x')
                BitwiseAnd, // R = R & 'x'
                // And, Or, Nor, Not, Clamp? Concat?
                // SignExtension, VectorExtension
                // SwapBytes

                ExtractIndex = 0x42,
                ExtractIndexSignExtended = 0x43,
                ExtractBits = 0x44,
                ExtractBitsSignExtended = 0x45,
                ExtractFull = 0x46,
                ExtractFullSignExtended = 0x47,

                // TODO: ExtractIndex[SignExtended], ExtractBits[SignExtended], ExtractFull[SignExtended]

                Restrict = 0x80,
            };
            struct Input {
                ID        cell;
                Operation operation;
                struct Flags {
                    bool absolute : 1; // do not copy the input cell when instantiating
                    bool dependent : 1;
                    bool convert : 1; // at this point (before applying operation) convert data to target cell type (useful only once)
                    bool byteswap : 1;

                } flags;
            } input [max_inputs];
        };

        // Type
        //  - data type
        //  - bitmasking for special behavior:
        //     - 0x80 - not synchronizing listen/notifications
        //
        enum class Type : std::uint8_t {
            Null = 0, // cell contains no value, may be signalled as event
            TimeStamp = 1, // UTC FILETIME in Cell::Information::TimeStamp
            Atom = 2, // Atom (or Atom Path), 0...4 (count) Atoms long path, optionally prefixed with 0...255 '../' atoms
            Float = 3, // floating point real number, supported widths are 4 and 8, float and double
            Integer = 4, // signed integer(s) of fixed width of: 0 (single bit), 1, 2, 4, 8 (int64_t), 16, 32 (int256_t)
            Unsigned = 5, // unsigned integer(s) of fixed width of: 0 (single bit), 1, 2, 4, 8 (uint64_t), 16, 32 (uint256_t)

            // Address
            //  - device address as supported by Scada Manifolds
            //  - af_family == cell.width
            //
            Address,

            Data, // raw data block

            // Text
            //  - short string, supported 'width' values are: 0 (UTF-8), 1 (ASCII), 2 (UTF-16)
            //  - if 'count'×'width' <= 32 the local storage is used (for UTF-8 the limit is bytes)
            //
            Text,

            // Expression
            //  - cell describes an expression and links dependencies
            //  - Cell::Information::Expression
            //
            Expression = 0x10,

            // Stream
            //  - unsynchronized representation of in-process data exchange
            //  - 'value' is not used, data are passed asynchronously only to callbacks
            //  - bit 0x80 is used for not synchronizing listen/notifications
            //     - types with this bit are virtual and cannot be retrieved by users using ScadaGet or ScadaGetValue
            //
            Stream = 0x81,

            // External Function (module, index, pocet a typy parametru?) - 
            // Expression, ExpressionVariableSet
            // SIMD
            // Semaphore,
            // Queue,
        };

        // Flags
        //  - data/cell state flags
        //
        struct Flags {
            union {
                std::uint8_t raw = 0;

                struct {
                    std::uint8_t invalid : 1;   // the information should be disregarded, failure to checksum also sets this flag
                    std::uint8_t stale : 1;     // the information in this cell is known to be not current
                    std::uint8_t reserved1 : 1;
                    std::uint8_t external : 1;  // Text/Data point to external (large data) storage, cannot convert cell directly, query database

                    std::uint8_t base_cell_dependent : 1; // TBD: for expressions
                    std::uint8_t reserved2 : 1; // TBD: precise timestamp??? GetPreciseSystemTime instead of 
                    std::uint8_t fixed_type : 1; // ScadaSetValue operation cannot change type/width (and count for vector types)
                    std::uint8_t constant : 1;  // TBD: if set, no locking is required, and changes to cell are not allowed
                    // TODO: source error, expression type mismatch, overflow, underflow, expression logic error
                };
            };
        };

        // Conversion
        //  - result code of Cell::Information to data type conversion
        //
        enum class Conversion : std::uint8_t {
            Success = 0,
            NotConversionError,         // conversion functions do not return this; try GetLastError
            External,                   // external flag, cannot convert, ask database
            InvalidFlagSet,             // invalid flag is set; fix error, unset, and call again
            Corrupted,                  // data in cell are corrupted
            UnsupportedData,            // data contains possibly valid, but not yet implemented, value
            UnsupportedType,            // the information type cannot be converted as requested
            UnsupportedWidth,           // misconfigured or not implemented value of 'width'
            IndexOutOfTypeBounds,       // requested element of vector that does not exist, i.e. 'uint64 [1]' when count == 1
            IndexOutOfSupportedBounds,  // requested element of vector that can not exist, i.e. 'uint64 [5]'
            ValueOutOfRangeBelow,       // stored value is too small to be retrieved into type T
            ValueOutOfRangeAbove,       // stored value is too large to be retrieved into type T
            FailedConversion,           // attempt to convert (e.g. string to integer) failed, or charset/UTF conversion failed
            InsufficientBuffer,
        };

        // ConversionException
        //  - thrown on error by helper member function: cell->to <T> ();
        //
        class ConversionException
            : public std::runtime_error {

        public:
            Conversion error;

        public:
            inline explicit ConversionException (Conversion error);
            
            ConversionException (const ConversionException &) = default;
            ConversionException (ConversionException && from) noexcept = default;
            ConversionException & operator = (ConversionException && from) noexcept = default;
        };

        // Specs
        //  -
        //
        struct Specs {
            union {
                std::uint32_t specs_raw = 0;
                std::uint16_t specs_typewidth; // 'type' and 'width'

                struct {
                    Type         type;
                    std::uint8_t width; // meaning depends on 'type', typically number of bytes
                    std::uint8_t count; // meaning depends on 'type', for Float,Integer,Unsigned types is 1-based (i.e. value 0 means 1 item and 255 means 256 items)
                    Flags        flags;
                };
            };
        };

        // Information
        //  - union of various data types, instanced as 'cell.value'
        //  - 'cell.type' determines member used; layout enables some to alias
        //
        struct Information : Specs {
            struct TimeStampObject : TRIMCORE::Timestamp {
                //struct {
                    // bool substituted; // not from device's clock
                    // DST?
                //} flags;
            };

            struct Data {
                union {
                    /*struct { // when not local
                        std::uint64_t id;   // to locate the data storage (index to file (small/medium), or part of filename (large))
                        std::uint64_t size; // cached size; size for local data is embedded in type code
                    };*/
                    std::uint8_t local [32]; // when !flags.external
                };
            };
            struct Text {
                union {
                    /*struct {
                        std::uint64_t id;   // to locate the data storage (index to file (small/medium), or part of filename (large))
                        std::uint64_t size; // cached size; size for local data is embedded in type code
                    } external; // */
                    char local [32]; // when !flags.external
                };
            };

            // Stream - nexus data
            //  - unsynchronized, in parallel delivered, data, only withing single manifold process
            //  - not written into the cell, nor is it locked
            //  - TODO: expressions connected to Nexus are re-evaluated only on new data
            //
            struct Stream {
                const void * data;
                std::size_t  size;

                // remote/local
                //  - 

                const Manifold::Address * remote;
                const Manifold::Address * local;
            };

        public:
            typedef double_integer <std::uint64_t, std::int64_t>  int128_t;
            typedef double_integer <std::uint64_t, std::uint64_t> uint128_t;
            typedef double_integer <uint128_t, int128_t>          int256_t;
            typedef double_integer <uint128_t, uint128_t>         uint256_t;

            // union
            //  - this is aligned to 32B boundary within Segments

            union {
                TimeStampObject timestamp;
                Expression      expression;
                Stream          stream;
                Data            data;
                Text            text;
                Atom            atom [4];

                std::int32_t    int1 : 1;
                std::int8_t     int8 [32];
                std::int16_t    int16 [16];
                std::int32_t    int32 [8];
                std::int64_t    int64 [4];
                int128_t        int128 [2];
                int256_t        int256;

                std::uint32_t   uint1 : 1;
                std::uint8_t    uint8 [32];
                std::uint16_t   uint16 [16];
                std::uint32_t   uint32 [8];
                std::uint64_t   uint64 [4];
                uint128_t       uint128 [2];
                uint256_t       uint256;

                // short           ieee754s [16];
                // short           ieee754b [16];
                float           ieee754f [8];
                double          ieee754d [4];
                // std::uint8_t    ieee754l [10];
            };

            inline Information () noexcept;
            inline Information (const Information & other) noexcept;
            inline Information & operator = (const Information & other) noexcept;

            bool operator == (const Information & other) const noexcept;
            bool operator != (const Information & other) const noexcept { return !this->operator == (other); }

            // ElementCount
            //  - 
            //
            std::size_t ElementCount () const noexcept;

        public:

            // ToInteger, ToBoolean, ToString
            //  - performs lossless conversion (except rounding floats), or fails with error code
            //  - fails on failure to convert, out of bounds, if the result would wrap, etc.
            //  - TODO: export what is possible from SCADA.DLL for anyone to use

            Conversion ToBoolean (bool * value, std::size_t index = 0) const noexcept;

            template <typename T> inline
            Conversion ToInteger (T * value, std::size_t index = 0) const;

            template <typename T> inline
            Conversion ToFloat (T * value, std::size_t index = 0) const;

            Conversion ToString (std::string *, std::size_t index = 0, TRIMCORE::DescriptionFormatting * format = nullptr) const;
            Conversion ToString (std::wstring *, std::size_t index = 0, TRIMCORE::DescriptionFormatting * format = nullptr) const;
            Conversion ToString (std::u8string *, std::size_t index = 0, TRIMCORE::DescriptionFormatting * format = nullptr) const;

            inline Conversion ToString (std::string * result, std::size_t index, std::wstring_view fmt) const;
            inline Conversion ToString (std::wstring * result, std::size_t index, std::wstring_view fmt) const;
            inline Conversion ToString (std::u8string * result, std::size_t index, std::wstring_view fmt) const;

            Conversion ToFileTime (FILETIME *, std::size_t index = 0) const;
            Conversion ToAddress (Manifold::Address *, std::size_t index = 0) const;
            Conversion ToAtom (Atom *, std::size_t index = 0) const;

            // ToAtomPath, ToRawBuffer
            //  - these convert a whole cell, even vector ones, not just single element 

            Conversion ToAtomPath (AtomPath *) const;
            Conversion ToRawData (std::vector <std::byte> &) const;
            Conversion ToRawData (void * buffer, std::size_t * length) const;
            Conversion ToRawDataBufferSize (std::size_t * length, bool * direct = nullptr) const;


            template <typename T>
            inline Conversion to (T             * value, std::size_t index = 0) const { return this->ToInteger (value, index); }
            inline Conversion to (bool          * value, std::size_t index = 0) const { return this->ToBoolean (value, index); }
            inline Conversion to (float         * value, std::size_t index = 0) const { return this->ToFloat (value, index); }
            inline Conversion to (double        * value, std::size_t index = 0) const { return this->ToFloat (value, index); }
            inline Conversion to (long double   * value, std::size_t index = 0) const { return this->ToFloat (value, index); }
            inline Conversion to (std::string   * value, std::size_t index = 0) const { return this->ToString (value, index); }
            inline Conversion to (std::wstring  * value, std::size_t index = 0) const { return this->ToString (value, index); }
            inline Conversion to (std::u8string * value, std::size_t index = 0) const { return this->ToString (value, index); }
            inline Conversion to (Atom          * value, std::size_t index = 0) const { return this->ToAtom (value, index); }
            inline Conversion to (FILETIME      * value, std::size_t index = 0) const { return this->ToFileTime (value, index); }
            inline Conversion to (Manifold::Address * value, std::size_t index = 0) const { return this->ToAddress (value, index); }
            inline Conversion to (AtomPath      * value, std::size_t index = 0) const { return this->ToAtomPath (value); }

            inline bool initialize (bool value);
            
            inline bool initialize (float              value) { return this->CommonNumberInitialization (Cell::Type::Float, value); }
            inline bool initialize (double             value) { return this->CommonNumberInitialization (Cell::Type::Float, value); }
            inline bool initialize (signed        char value) { return this->CommonNumberInitialization (Cell::Type::Integer, value); }
            inline bool initialize (signed       short value) { return this->CommonNumberInitialization (Cell::Type::Integer, value); }
            inline bool initialize (signed         int value) { return this->CommonNumberInitialization (Cell::Type::Integer, value); }
            inline bool initialize (signed        long value) { return this->CommonNumberInitialization (Cell::Type::Integer, value); }
            inline bool initialize (signed   long long value) { return this->CommonNumberInitialization (Cell::Type::Integer, value); }
            inline bool initialize (unsigned      char value) { return this->CommonNumberInitialization (Cell::Type::Unsigned, value); }
            inline bool initialize (unsigned     short value) { return this->CommonNumberInitialization (Cell::Type::Unsigned, value); }
            inline bool initialize (unsigned       int value) { return this->CommonNumberInitialization (Cell::Type::Unsigned, value); }
            inline bool initialize (unsigned      long value) { return this->CommonNumberInitialization (Cell::Type::Unsigned, value); }
            inline bool initialize (unsigned long long value) { return this->CommonNumberInitialization (Cell::Type::Unsigned, value); }

            inline bool initialize (FILETIME value);

            inline bool initialize (char8_t value) { return this->CommonCharInitialization (0, value); }
            inline bool initialize (char value) { return this->CommonCharInitialization (sizeof value, value); }
            inline bool initialize (wchar_t value) { return this->CommonCharInitialization (sizeof value, value); }

            bool initialize (std::string_view value);
            bool initialize (std::wstring_view value);
            bool initialize (std::u8string_view value);

            bool initialize (const AtomPathView & value);
            bool initialize (const Manifold::Address & value);

            //inline bool initialize (const std::vector <bool> &);

            // ConvertFrom
            //  - and then used by 'set' member?


        private:
            template <typename T> inline bool CommonNumberInitialization (Cell::Type type, T value);
            template <typename T> inline bool CommonCharInitialization (std::uint8_t width, T value);

            template <typename T>                            static Conversion SafeBitAssignment (T * tgt, const std::uint32_t (&bits) [8], std::size_t index);
            template <typename T, typename S>                static Conversion SafeIntExtraction (T * tgt, const S & src, std::size_t index);
            template <typename T, typename S, std::size_t N> static Conversion SafeIntExtraction (T * tgt, const S (&src) [N], std::size_t index);
            template <typename T, typename S, std::size_t N> static Conversion SafeFltExtraction (T * tgt, const S (&src) [N], std::size_t index);

            template <typename T, typename S>               static Conversion SafeFltConversion (T * tgt, S src);
            template <typename T>                           static Conversion SafeIntConversion (T * tgt, T src);
            template <typename T, typename S>               static Conversion SafeIntConversion (T * tgt, const S & src);
            template <typename T, typename LO, typename HI> static Conversion SafeIntConversion (T * tgt, const double_integer <LO, HI> & src);

            template <typename T>                           static Conversion SafeStrConversion (T * tgt, std::string_view);
            template <typename T>                           static Conversion SafeStrConversion (T * tgt, std::wstring_view);
            template <typename T>                           static Conversion SafeStrConversion (T * tgt, std::u8string_view);

            template <typename S>                static Conversion SafeBoolConversion (bool * tgt, const S & src, std::size_t index);
            template <typename S, std::size_t N> static Conversion SafeBoolConversion (bool * tgt, const S (&src) [N], std::size_t index);
                                                 static Conversion SafeBoolConversion (bool * tgt, std::string_view);
                                                 static Conversion SafeBoolConversion (bool * tgt, std::wstring_view);
                                                 static Conversion SafeBoolConversion (bool * tgt, std::u8string_view);
            
            template <typename T, std::size_t N> static Conversion SafeBoolConversion (bool * tgt, const char (&src) [N], std::size_t count);
            
                                                 static Conversion SafeBoolStringConversion (std::wstring * tgt, const std::uint32_t (&bits) [8], std::size_t index, TRIMCORE::DescriptionFormatting * fmt);

            template <typename S>                static Conversion SafeStringConversion (std::wstring * tgt, const S & src, std::size_t index, TRIMCORE::DescriptionFormatting * fmt);
            template <typename S, std::size_t N> static Conversion SafeStringConversion (std::wstring * tgt, const S (&src) [N], std::size_t index, TRIMCORE::DescriptionFormatting * fmt);
            template <typename LO, typename HI>  static Conversion SafeStringConversion (std::wstring * tgt, const double_integer <LO, HI> & src, std::size_t index, TRIMCORE::DescriptionFormatting * fmt);

            template <typename T>                       Conversion SafeVectorExtract (T * result, std::size_t index) const;

            Conversion ToRawDataSynthesize (void * buffer, std::size_t length) const;
        };

    public: // data
        union {
            struct {
                mutable RwSpinLock <short>  lock;
                // TODO: reuse remaining for full 32-bit checksum? 
                // TODO: write last PID of writer, to forcefully unlock on crash (and set invalid on checksum failure)
            };
            ID id; // only valid in data stream
        };

        std::uint16_t       checksum;
        TRIMCORE::Timestamp t;      // timestamp of last update
        ID                  base; // dependency source for the cell; if 'expression' is set, this cell contains source of the initial expression value
        ID                  expression; // expression; reference to 'Expression' type cell
        Information         value; // the actual data; 'value.type' determines which member of subsequent 'value' union is active

    private: // member functions are not directly available to manifold modules
        friend class Area;
    public:

        // get
        //  - retrieves cell value (thread-safe, locks the cell for reading) and attempts to convert it to T
        //  - returns: true on success
        //             false on invalid conversion, *error receives the conversion failure type
        //
        template <typename T>
        bool get (T * value, std::size_t index = 0, Conversion * error = nullptr);

        // get
        //  - retrieves cell value (thread-safe, locks the cell for reading) and attempts to convert it to string
        //  - returns: true on success
        //             false on invalid conversion, *error receives the conversion failure type
        //
        template <typename C>
        bool get (std::basic_string <C> * value, std::size_t index = 0, Conversion * error = nullptr);

        // get
        //  - retrieves cell value (thread-safe, locks the cell for reading) and attempts to convert it to boolean
        //  - returns: true on success
        //             false on invalid conversion, *error receives the conversion failure type
        //
        bool get (bool * value, std::size_t index = 0, Conversion * error = nullptr);

        // get
        //  - converts value to T and returns the value
        //  - on conversion failure throws 'ConversionException'
        //
        template <typename T>
        inline T get (std::size_t index = 0);

        // finalize
        //  - updates timestamp and checksum
        //
        void finalize () noexcept;
        void finalize (TRIMCORE::Timestamp ts) noexcept;
        void finalize (TRIMCORE::Timestamp ts, std::uint16_t crc) noexcept;

        // acquire
        //  - acquires the cell lock for exclusive/write access
        //  - for parity with 'commit'
        //
        void acquire () noexcept;

        // commit
        //  - updates timestamp and checksum, and unlocks the cell lock
        //
        void commit () noexcept;

        // precompute_checksum
        //  - returns what checksum a cell will have after assigning 't' and 'value'
        //  - the purpose is to shorten time the cell is locked
        // TODO: export from scada.dll
        //
        std::uint16_t precompute_checksum (TRIMCORE::Timestamp t, const Information * value) const noexcept;

        void update_checksum () noexcept;
        bool verify_checksum () const noexcept;

        inline Cell () noexcept;
        inline Cell (const Cell & other) noexcept;
        inline Cell & operator = (const Cell & other) noexcept;

        // void set (std::uint16_t owner, std::uint8_t type, Information data);

    // API datatypes
    private:
        struct Handle__;
        //struct Handle__ { int unused; }; typedef struct Handle__ * Handle;
    public:

        // Handle
        //  - value of nullptr means invalid handle
        //  - internally directly Cell pointer, or TBD
        //
        typedef struct Handle__ * Handle;
    };

    // verify invariants

    static_assert (sizeof (Cell) == 64u);
    static_assert (sizeof (Cell::Specs) == 4u);
    static_assert (sizeof (Cell::Information) == 32u + 4u);
    static_assert (offsetof (Cell, t) == 8u);
    static_assert (offsetof (Cell, value) == 32u - 4u);

    // Cell::ID comparison operators

    inline constexpr bool operator == (const Cell::ID & a, const Cell::ID & b) noexcept;
    inline constexpr bool operator != (const Cell::ID & a, const Cell::ID & b) noexcept;
    inline constexpr bool operator < (const Cell::ID & a, const Cell::ID & b) noexcept;
    inline bool operator > (const Cell::ID & a, const Cell::ID & b) { return  (b < a); }
    inline bool operator <= (const Cell::ID & a, const Cell::ID & b) { return !(b < a); }
    inline bool operator >= (const Cell::ID & a, const Cell::ID & b) { return !(a < b); }
}

// enable TRIMCORE::Log to log Cell IDs

namespace TRIMCORE {
    inline std::wstring Describe (Scada::Cell::ID id, DescriptionFormatting * format = nullptr) {
        return Describe (Describe (id.segment, format) + L":" + Describe (id.index, format), format);
    }

    inline std::wstring Describe (Scada::Cell::Type type, DescriptionFormatting * format = nullptr) {
        switch (type) {
            case Scada::Cell::Type::Null: return Describe (L"Null", format);
            case Scada::Cell::Type::TimeStamp: return Describe (L"TimeStamp", format);
            case Scada::Cell::Type::Atom: return Describe (L"Atom", format);
            case Scada::Cell::Type::Float: return Describe (L"Float", format);
            case Scada::Cell::Type::Integer: return Describe (L"Integer", format);
            case Scada::Cell::Type::Unsigned: return Describe (L"Unsigned", format);
            case Scada::Cell::Type::Address: return Describe (L"Address", format);
            case Scada::Cell::Type::Data: return Describe (L"Data", format);
            case Scada::Cell::Type::Text: return Describe (L"Text", format);
            case Scada::Cell::Type::Stream: return Describe (L"Stream", format);
            case Scada::Cell::Type::Expression: return Describe (L"Expression", format);
        }
        return Describe ((std::uint8_t) type, format);
    }

    inline std::wstring Describe (Scada::Cell::Conversion result, DescriptionFormatting * format = nullptr) {
        switch (result) {
            case Scada::Cell::Conversion::Success: return Describe (L"Success", format);
            case Scada::Cell::Conversion::NotConversionError: return Describe (L"Not a Conversion Error", format);
            case Scada::Cell::Conversion::External: return Describe (L"Externally Stored Data", format);
            case Scada::Cell::Conversion::Corrupted: return Describe (L"Data Corrupt", format);
            case Scada::Cell::Conversion::InvalidFlagSet: return Describe (L"Invalid Flag Set", format);
            case Scada::Cell::Conversion::UnsupportedData: return Describe (L"Unsupported Data", format);
            case Scada::Cell::Conversion::UnsupportedType: return Describe (L"Unsupported Type", format);
            case Scada::Cell::Conversion::UnsupportedWidth: return Describe (L"Unsupported Width", format);
            case Scada::Cell::Conversion::IndexOutOfTypeBounds: return Describe (L"Index Out Of Type Bounds", format);
            case Scada::Cell::Conversion::IndexOutOfSupportedBounds: return Describe (L"Index Out Of Supported Bounds", format);
            case Scada::Cell::Conversion::ValueOutOfRangeBelow: return Describe (L"Value Out Of Range (underflow)", format);
            case Scada::Cell::Conversion::ValueOutOfRangeAbove: return Describe (L"Value Out Of Range (overflow)", format);
            case Scada::Cell::Conversion::InsufficientBuffer: return Describe (L"Insufficient Output Buffer", format);
            case Scada::Cell::Conversion::FailedConversion: return Describe (L"Cannot Convert", format);
        }
        return Describe ((std::uint8_t) result, format);
    }

    inline std::wstring Describe (Scada::Cell::Expression::Operation op, DescriptionFormatting * format = nullptr) {
        switch (op) {
            case Scada::Cell::Expression::Operation::Nop: return Describe (L"NOP", format);
            case Scada::Cell::Expression::Operation::Add: return Describe (L"+", format);
            case Scada::Cell::Expression::Operation::Subtract: return Describe (L"-", format);
            case Scada::Cell::Expression::Operation::Multiply: return Describe (L"\x00D7", format);
            case Scada::Cell::Expression::Operation::Divide: return Describe (L"\x00F7", format);
            case Scada::Cell::Expression::Operation::BitwiseAnd: return Describe (L"&", format);
            case Scada::Cell::Expression::Operation::Equals: return Describe (L"==", format);
            case Scada::Cell::Expression::Operation::EqualsAnyOf: return Describe (L"==:", format);
            case Scada::Cell::Expression::Operation::NotEquals: return Describe (L"\x2260", format);
            case Scada::Cell::Expression::Operation::ExtractIndex: return Describe (L"extract(index)", format);
            case Scada::Cell::Expression::Operation::ExtractBits: return Describe (L"extract(bits)", format);
            case Scada::Cell::Expression::Operation::ExtractFull: return Describe (L"extract", format);
            case Scada::Cell::Expression::Operation::ExtractIndexSignExtended: return Describe (L"extract(index) signed", format);
            case Scada::Cell::Expression::Operation::ExtractBitsSignExtended: return Describe (L"extract(bits) signed", format);
            case Scada::Cell::Expression::Operation::ExtractFullSignExtended: return Describe (L"extract signed", format);
            case Scada::Cell::Expression::Operation::Restrict: return Describe (L"restrict", format);
        }
        return Describe ((std::uint8_t) op, format);
    }

    // TODO: export following as C API from DLL, to be usable in modules
    std::wstring Describe (const Scada::Cell::Information & value, DescriptionFormatting * format = nullptr);
    std::wstring Describe (const Scada::Cell & cell, DescriptionFormatting * format = nullptr);
   
    inline Serialized Serialize (const Scada::Cell::ID & id, Temporary64kB <std::uint8_t> &) { return SerializeTrivially ('SCid', id); }
    inline Serialized Serialize (const Scada::Cell::Type & type, Temporary64kB <std::uint8_t> &) { return SerializeTrivially ('SCt', type); }
    inline Serialized Serialize (const Scada::Cell::Conversion & result, Temporary64kB <std::uint8_t> &) { return SerializeTrivially ('SCcr', result); }
    inline Serialized Serialize (const Scada::Cell::Expression::Operation & op, Temporary64kB <std::uint8_t> &) { return SerializeTrivially ('SCop', op); }
    inline Serialized Serialize (const Scada::Cell::Information & information, Temporary64kB <std::uint8_t> &) { return SerializeTrivially ('SCi', information); }
    inline Serialized Serialize (const Scada::Cell & cell, Temporary64kB <std::uint8_t> &) { return SerializeTrivially ('SC', cell); }
}

#pragma pack(pop)
#include "SCADA_Cell.tcc"
#endif
