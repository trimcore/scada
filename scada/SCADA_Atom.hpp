#ifndef SCADA_ATOM_HPP
#define SCADA_ATOM_HPP

#if (defined(__GNUC__) && (__GNUC__ < 8))
#include <experimental/string_view>
namespace std {
    using string_view = ::std::experimental::string_view;
    using wstring_view = ::std::experimental::wstring_view;
}
#define SCADA_ATOM_IMPORT_PARSER
#else
#include <string_view>
#endif
#include <stdexcept>
#include <cstdint>

#ifndef NO_TRIMCORE_DLL
#include "TRIMCORE.h"
#endif

namespace Scada {

    // Atom
    //  - 64-bit value representing unsigned 63-bit number or a short limited string
    //  - numbers can range from 0 to 9'223'372'036'854'775'807 (2**63-1)
    //     - parser recognizes #x/#X prefixes for base 16
    //        - # must be followed by digits only
    //        - separators are not allowed
    //  - string encodes typically to 12 code units:
    //     - lowercase characters take 1 code unit, uppercase characters take 2 code units (wide code-point)
    //     - digits 0..3 take 1 code unit, 4..9 take 2 code units (wide code-point)
    //     - underscores and dashes are converted to spaces, trailing spaces are ignored
    //     - the string can have up to 13 or 14 code points (encoded as single bits) in following cases:
    //        - if the last code-point is wide
    //        - if the last letter is 's', this information is encoded in addition single bit
    //     - any other character results in parse failure
    //
    class Atom {
    public:

        // InvalidInputReason
        //  - possible failures of string/input parsing function
        //
        enum class InvalidInputReason : int {
            None,
            NumberTooLarge,
            TextTooLong,
            InvalidCharacter,
            PathTooLong, // Scada::AtomPath
            EmptyAtomInPath, // Scada::AtomPath
        };

#ifdef TRIMCORE
        using InvalidInputExceptionBase = TRIMCORE::Log::Exception;
#else
        using InvalidInputExceptionBase = std::runtime_error;
#endif

        // InvalidInputException
        //  - Atom constructor throws this 
        //
        struct InvalidInputException : public InvalidInputExceptionBase {
            InvalidInputReason reason;
        public:
            template <typename T>
            explicit InvalidInputException (InvalidInputReason reason, T && source)
#ifdef TRIMCORE
                : InvalidInputExceptionBase (nullptr, "INTERNAL ERROR, Scada::Atom error {1} on input {2}", reason, source) // TBD: Text from rsrc (SCADA_Atom.rc?)
#else
                : InvalidInputExceptionBase ("Scada::Atom failed")
#endif
                , reason (reason) {};
        };

    public:
        constexpr Atom () = default;
        constexpr Atom (const Atom &) = default;
        constexpr Atom & operator = (const Atom &) = default;

        constexpr explicit inline Atom (std::uint64_t value) {
            if (!this->set (value))
                throw InvalidInputException (InvalidInputReason::NumberTooLarge, value);
        }
        constexpr explicit inline Atom (unsigned long value) : value (value), numeric (true) {}
        constexpr explicit inline Atom (unsigned int value) : value (value), numeric (true) {}
        constexpr explicit inline Atom (unsigned short value) : value (value), numeric (true) {}
        constexpr explicit inline Atom (unsigned char value) : value (value), numeric (true) {}

        constexpr explicit inline Atom (std::string_view sv) : raw (0) {
            InvalidInputReason r;
            if (!this->parse (sv, &r))
                throw InvalidInputException (r, sv);
        }
        constexpr explicit inline Atom (std::wstring_view sv) : raw (0) {
            InvalidInputReason r;
            if (!this->parse (sv, &r))
                throw InvalidInputException (r, sv);
        }

        constexpr explicit inline Atom (const char * s) : Atom (std::string_view (s)) {};
        constexpr explicit inline Atom (const wchar_t * s) : Atom (std::wstring_view (s)) {};

        // parse
        //  - parses short string (per specs) into the Atom
        //  - string parameter must be clean string, spaces in prefix will be encoded

        constexpr bool parse (std::string_view sv, InvalidInputReason * result = nullptr) noexcept;
        constexpr bool parse (std::wstring_view sv, InvalidInputReason * result = nullptr) noexcept;

        constexpr inline bool set (std::uint64_t number) noexcept {
            if (!(number & 0x8000'0000'0000'0000uLL)) {
                this->value = number;
                this->numeric = true;
                return true;
            } else
                return false;
        }
        constexpr inline bool is_numeric (std::uint64_t * number = nullptr) const noexcept {
            if (this->numeric) {
                if (number) {
                    *number = this->value;
                }
                return true;
            } else
                return false;
        }

        // formatting_parameters
        //  - options for converting Atom to strings through 'format' function
        //
        struct formatting_parameters {
            std::uint64_t    hexadecimal_threshold = 10'000'000; // 0 = always decimal 
            std::string_view hexadecimal_prefix = "#x";
            char             space_character = '_'; // use ' ', '-' or '_' to create round-trip convertible strings
            char             string_enclosure = false; // use '\'' or '"' to enclose string-type atoms
            char             string_enclosure_end = false; // optionally different from 'string_enclosure'
        };

        // format
        //  - converts the atom to string, honoring requested parameters
        //
        std::string format (formatting_parameters = {}) const;
        std::wstring wformat (formatting_parameters = {}) const;

        // format
        //  - converts the atom to string, honoring requested parameters
        //  - will NUL-terminate IF there's space
        //  - returns string length, 0 if buffer is too small (20 chars is safe)
        //
        std::size_t format (char * buffer, std::size_t length, formatting_parameters = {}) const;

        // formatted_length
        //  - 
        //
        constexpr std::size_t formatted_length () const;
    
        // decomposition
        //  - result of 'decompose' function, breakdown array of code points
        //
        struct decomposition {
            std::uint8_t length; // number of valid characters, not including trailing spaces/dashes
            struct character {
                std::uint8_t wide : 1; // the character spans two code-points (first is fixed 5-bit value of 1)
                std::uint8_t trail : 1; // the character is encoded as trailing bit, not as code-point
                std::uint8_t reserved : 1;
                std::uint8_t value : 5;
                char         character;
            } characters [14];
        };
        
        // decompose
        //  - for String type Atoms returns breakdown array of characters
        //
        bool decompose (decomposition *, formatting_parameters = {}) const;

        // clear
        //  - sets Atom to empty string, raw value 0
        //
        inline void clear () noexcept { this->raw = 0; }

        // raw access

        explicit operator bool () const { return !!this->value; }
        explicit operator std::uint64_t & () { return this->raw; }
        explicit operator std::uint64_t () const { return this->raw; }

    private:
        struct String;

        union {
            std::uint64_t raw = 0;
            struct {
                std::uint64_t value : 63;
                std::uint64_t numeric : 1;
            };
        };

        friend bool operator < (const Atom & a, const Atom & b);
        friend bool operator == (const Atom & a, const Atom & b);
    };

    static_assert (sizeof (Atom) == sizeof (std::uint64_t));

    inline bool operator < (const Atom & a, const Atom & b) { return a.raw < b.raw; }
    inline bool operator > (const Atom & a, const Atom & b) { return  (b < a); }
    inline bool operator == (const Atom & a, const Atom & b) { return a.raw == b.raw; }
    inline bool operator != (const Atom & a, const Atom & b) { return !(a == b); }
    inline bool operator <= (const Atom & a, const Atom & b) { return !(b < a); }
    inline bool operator >= (const Atom & a, const Atom & b) { return !(a < b); }
}

constexpr auto operator""_atom (std::uint64_t x) {
    return Scada::Atom (x);
}
constexpr auto operator""_atom (const char * s, std::size_t n) {
    return Scada::Atom (std::string_view (s, n));
}

// enable TRIMCORE::Log to log Atoms

#ifdef TRIMCORE
namespace TRIMCORE {
    
    inline std::wstring Describe (Scada::Atom atom, DescriptionFormatting * format = nullptr) {
        // TODO: add 'format' options to change 'formatting_parameters'
        return Describe (atom.format (), format);
    }

    inline std::wstring Describe (Scada::Atom::InvalidInputReason reason, DescriptionFormatting * format = nullptr) {
        // TODO: SCADA_Atom.rc
        switch (reason) {
            case Scada::Atom::InvalidInputReason::None:
                return Describe ("None", format);
            case Scada::Atom::InvalidInputReason::NumberTooLarge:
                return Describe ("Number Too Large", format);
            case Scada::Atom::InvalidInputReason::TextTooLong:
                return Describe ("Text Tool Long", format);
            case Scada::Atom::InvalidInputReason::InvalidCharacter:
                return Describe ("Invalid Character", format);
            case Scada::Atom::InvalidInputReason::PathTooLong:
                return Describe ("Path Too Long To Fit Buffer", format);
            case Scada::Atom::InvalidInputReason::EmptyAtomInPath:
                return Describe ("Empty Atom Not Allowed In Paths", format); // empty Atoms sometimes indicate parent path element (relative path style)

            default:
                return Describe ((int) reason, format);
        }
    }

    inline Serialized Serialize (const Scada::Atom & atom, Temporary64kB <std::uint8_t> &) { return SerializeTrivially ('Atom', atom); }
    inline Serialized Serialize (const Scada::Atom::InvalidInputReason & reason, Temporary64kB <std::uint8_t> &) { return SerializeTrivially ('Aerr', reason); }

}
#endif

#include "SCADA_Atom.tcc"
#endif
