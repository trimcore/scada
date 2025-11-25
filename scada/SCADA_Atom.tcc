#ifndef SCADA_ATOM_TCC
#define SCADA_ATOM_TCC

#include "SCADA_Atom.hpp"

#if !(defined(__GNUC__) && (__GNUC__ < 10))
#include <charconv>
#endif

struct Scada::Atom::String {
    union {
        std::uint64_t raw = 0;
        struct {
            std::uint64_t value     : 60;// 12 x 5-bit code units
            std::uint64_t last_wide : 1; // last code point is wide, as-if 12th code unit was preceeded with 00001
            std::uint64_t append_s  : 1; // 13th code unit is 's'
            std::uint64_t reserved  : 1; // 0
            std::uint64_t numeric   : 1; // true if the atom is numeric, then the above does not apply
        };
    };

    // error
    //  - parsing errors
    //
    Scada::Atom::InvalidInputReason error = Scada::Atom::InvalidInputReason::None;

    static constexpr std::size_t max_code_units = 8 * sizeof (raw) / 5; // 12 code units
    static constexpr std::size_t max_length = max_code_units + 2; // + 2 extra bits
    static constexpr std::size_t max_numeric = 19; // 63-bit unsigned
    static constexpr std::size_t max_string = std::max (max_length, max_numeric);

    // set
    //  - sets non-bit code-unit into empty 'index' in 'value' part
    //
    constexpr inline bool set (unsigned int index, std::uint8_t codeunit) noexcept {
        if (index < max_code_units) {
            this->raw |= (std::uint64_t) (codeunit & 0x1F) << (index * 5);
            return true;
        } else {
            this->error = Scada::Atom::InvalidInputReason::TextTooLong;
            return false;
        }
    }

    constexpr inline std::uint8_t get (unsigned int index) const noexcept {
        if (index < max_code_units - 1) { // 1..11th code unit
            return (this->raw >> (index * 5) & 0x1F);

        } else {
            if (this->last_wide) {
                --index;
            }
            switch (index) {
                case max_code_units - 2: return 0x01;
                case max_code_units - 1: return (this->raw >> (index * 5) & 0x1F);
                case max_code_units + 0: if (this->append_s) return 0x14; else return 0x00;
                default:
                    return 0x00;
            }
        }
    }

    // append
    //  - 
    //  - returns number of code units used to represent the character
    //  - 'i' must be iterated from 0 to last, characters sequentially appended to empty atom
    //
    constexpr inline std::uint8_t append (unsigned int i, std::uint8_t character) noexcept {
        if (character >= 'a' && character <= 'z') {
            if (this->set (i, character - 'a' + 2)) {
                return 1;
            } else {
                if (character == 's' &&
                      ((!this->last_wide && i == max_code_units + 0)
                     || (this->last_wide && i == max_code_units + 1))) {

                    this->append_s = true;
                    return 1;
                }
            }
        } else
        if (character >= '0' && character <= '3') {
            if (this->set (i, character - '0' + 2 + 26)) {
                return 1;
            }
        } else
        if ((character >= 'A' && character <= 'Z') || (character >= '4' && character <= '9')) {
            if (character >= 'A' && character <= 'Z') {
                character -= 'A';
            } else {
                character -= '4';
                character += 26;
            }

            if (i == max_code_units - 1) {
                if (this->set (i, character)) {
                    this->last_wide = true;
                    return 2;
                }
            } else {
                if (this->set (i, 1) && this->set (i + 1, character)) {
                    return 2;
                }
            }
        } else
        if (character == '_' || character == '-' || character == ' ') {
            if (this->set (i, 0)) {
                return 1;
            }
        }

        if (this->error == Scada::Atom::InvalidInputReason::None) {
            this->error = Scada::Atom::InvalidInputReason::InvalidCharacter;
        }
        return 0;
    }
};

constexpr inline bool Scada::Atom::parse (std::string_view sv, InvalidInputReason * reason) noexcept {

    // attempt to parse as number
    //  - allow prefix '#' (then it's only number)
    //  - base 16 when '#x' prefixed
    //  - string enclosed in ' or " is always parsed as string

    int base = 10;
    bool number = false;
    bool string = false;

    if (sv.starts_with ('#')) {
        sv.remove_prefix (1);
        number = true;

        if (sv.starts_with ('x') || sv.starts_with ('X')) {
            sv.remove_prefix (1);
            base = 16;
        }
    } else
        if ((sv.starts_with ('\'') && sv.ends_with ('\'')) || (sv.starts_with ('"') && sv.ends_with ('"'))) {
            sv.remove_prefix (1);
            sv.remove_suffix (1);
            string = true;
        }

    if (!string) {
        const auto svb = sv.data ();
        const auto sve = sv.data () + sv.size ();
        std::uint64_t value = 0;

        // clean string only, no trailing characters allowed

        if (auto [end, error] = std::from_chars (svb, sve, value, base); (error == std::errc ()) && (end == sve)) {
            if (value & 0x8000'0000'0000'0000uLL) {
                if (reason) {
                    *reason = InvalidInputReason::NumberTooLarge;
                }
                return false;
            } else {
                this->value = value;
                this->numeric = true;
                return true;
            }
        }

        // requested number by '#' but reached non-digit character

        if (number) {
            if (reason) {
                *reason = InvalidInputReason::InvalidCharacter;
            }
            return false;
        }
    }

    // decode as string

    String s;

    auto i = 0u;
    for (auto c : sv) {
        if (auto n = s.append (i, c)) {
            i += n;
        } else {
            if (reason) {
                *reason = s.error;
            }
            return false;
        }
    }

    if (reason) {
        *reason = InvalidInputReason::None;
    }

    this->value = s.raw;
    this->numeric = false;
    return true;
}

constexpr inline bool Scada::Atom::parse (std::wstring_view sv, InvalidInputReason * reason) noexcept {
    char copy [String::max_string + 4 + 1];
    const auto b = &copy [0];
    const auto e = &copy [sizeof copy];

    auto p = b;
    for (auto w : sv) {
        if (w < 32 || w > 'z') {
            if (reason) {
                *reason = InvalidInputReason::InvalidCharacter;
            }
            return false;
        }
        if (p != e) {
            *p++ = (char) w;

        } else {
            if (reason) {
                *reason = InvalidInputReason::TextTooLong;
            }
            return false;
        }
    }
    return this->parse (std::string_view (b, p - b), reason);
}

inline std::size_t Scada::Atom::format (char * buffer, std::size_t length, formatting_parameters fmt) const {
    if (!length)
        return 0;

    const auto start = buffer;
    const auto limit = buffer + length;

    if (this->numeric) {
        auto hex = (this->value > (fmt.hexadecimal_threshold - 1));
        if (hex) {
            if (length > fmt.hexadecimal_prefix.size ()) {
                std::strncpy (buffer, fmt.hexadecimal_prefix.data (), fmt.hexadecimal_prefix.size ());
                buffer += fmt.hexadecimal_prefix.size ();
            } else
                hex = false;
        }

        auto r = std::to_chars (buffer, limit, this->value, hex ? 16 : 10);
        if (r.ec == std::errc ()) {
            if (r.ptr != limit) {
                *r.ptr = '\0'; // NUL-terminate if there's space
            }
            return r.ptr - start;
        } else
            return 0;

    } else {
        String decoded = { this->value };

        if (fmt.string_enclosure) {
            *buffer++ = fmt.string_enclosure;
        }
        for (auto i = 0u; (i < decoded.max_length) && (buffer != limit - (fmt.string_enclosure ? 1 : 0)); ++i) {
            switch (auto c = char (decoded.get (i))) {
                case 0x00:
                    *buffer++ = fmt.space_character;
                    break;
                case 0x01:
                    c = char (decoded.get (++i));
                    if (c < 26) {
                        *buffer++ = 'A' + c;
                    } else {
                        *buffer++ = '4' + c - 26;
                    }
                    break;
                default:
                    if (c < 26 + 2) {
                        *buffer++ = 'a' + c - 2;
                    } else {
                        *buffer++ = '0' + c - 2 - 26;
                    }
                    break;
            }
        }

        // trim
        buffer = start + std::string_view (start, buffer - start).find_last_not_of (fmt.space_character) + 1;

        if (fmt.string_enclosure) {
            if (fmt.string_enclosure_end) {
                *buffer++ = fmt.string_enclosure_end;
            } else {
                *buffer++ = fmt.string_enclosure;
            }
        }
        if (buffer != limit) {
            *buffer = '\0'; // NUL-terminate if there's space
        }
        return buffer - start;
    }
}

constexpr inline std::size_t Scada::Atom::formatted_length () const {
    std::size_t n = 0;
    std::size_t z = 0;

    String decoded = { this->value };
    for (auto i = 0u; (i < decoded.max_length); ++i) {
        switch (auto c = char (decoded.get (i))) {
            case 0x00:
                ++z;
                ++n;
                break;
            case 0x01:
                ++i;
            [[ fallthrough ]];
            default: 
                ++n;
                z = 0;
                break;
        }
    }

    return n - z;
}

inline std::string Scada::Atom::format (formatting_parameters fmt) const {
    char buffer [String::max_string + 1];
    auto length = this->format (buffer, sizeof buffer, fmt);
    return std::string (buffer, length);
}

inline std::wstring Scada::Atom::wformat (formatting_parameters fmt) const {
    std::wstring ws;
    auto s = this->format (fmt);
    if (auto n = s.length ()) {
        ws.resize (n);
        for (auto i = 0u; i != n; ++i) {
            ws [i] = (wchar_t) s [i];
        }
    }
    return ws;
}

inline bool Scada::Atom::decompose (decomposition * result, formatting_parameters fmt) const {
    if (!this->numeric) {
        String string = { this->value };

        result->length = 0;

        auto o = 0u;
        auto i = 0u;
        while (i < string.max_length) {
            result->characters [o].trail = false;

            switch (auto c = char (string.get (i))) {
                case 0x01:
                    c = char (string.get (++i));

                    result->characters [o].wide = true;
                    result->characters [o].value = c;
                    if (c < 26) {
                        result->characters [o].character = 'A' + c;
                    } else {
                        result->characters [o].character = '4' + c - 26;
                    }
                    result->length = o + 1;
                    break;

                default:
                    result->characters [o].wide = false;
                    result->characters [o].value = c;

                    if (c == 0) {
                        result->characters [o].character = fmt.space_character;
                    } else {
                        if (c < 26 + 2) {
                            result->characters [o].character = 'a' + c - 2;
                        } else {
                            result->characters [o].character = '0' + c - 2 - 26;
                        }
                        result->length = o + 1;
                    }
                    break;
            }
            ++i;
            ++o;
        }

        for (auto t = 0u; t != (string.last_wide + string.append_s); ++t) {
            result->characters [result->length - t - 1].trail = true;
        }
        return true;
    } else
        return false;
}

#endif
