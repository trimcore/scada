#include "TRIMCORE.h"
#include "SCADA_Cell.hpp"
#include "SCADA_Manifold_Address.hpp"
#include <string>

#ifdef _M_ARM64
#include <arm64_neon.h>
#else
#include <intrin.h>
#endif

using namespace std::literals;

namespace {
#if defined (_M_ARM64)
    typedef std::uint64_t crc_type;
    inline std::uint64_t crc_round (std::uint64_t crc, std::uint64_t value) noexcept {
        return __crc32cd ((std::uint32_t) crc, value);
    }
#elif defined (_M_AMD64)
    typedef std::uint64_t crc_type;
    inline std::uint64_t crc_round (std::uint64_t crc, std::uint64_t value) noexcept {
        return _mm_crc32_u64 (crc, value);
    }
#else
    typedef std::uint32_t crc_type;
    inline std::uint32_t crc_round (std::uint64_t crc, std::uint64_t value) noexcept {
        crc = _mm_crc32_u32 ((std::uint32_t) crc, reinterpret_cast <const std::uint32_t *> (&value) [0]);
        return _mm_crc32_u32 ((std::uint32_t) crc, reinterpret_cast <const std::uint32_t *> (&value) [1]);
    }
#endif

    inline std::uint64_t crc_round (std::uint64_t crc, const void * data, std::size_t offset) noexcept {
        return crc_round (crc, static_cast <const std::uint64_t *> (data) [offset]);
    }

    // checksum
    //  - requires SSE4.2 or ARMv8 NEON
    //  - note that the implementation isn't perfect, there's ~3 cycle latency between CRC instructions
    //     - manual reordering is reverted by MSVC optimizer :-/
    //  - skipping first 8 bytes means skipping lock/id and old checksum
    //
    inline std::uint16_t checksum (const Scada::Cell * cell) noexcept {
        auto a = crc_round (0, cell, 1);
        auto b = crc_round (0, cell, 2);
        auto c = crc_round (0, cell, 3);
        a = crc_round (a, cell, 4);
        b = crc_round (b, cell, 5);
        c = crc_round (c, cell, 6);
        a = crc_round (a, cell, 7);

        b = crc_round (b, c);
        c = crc_round (a, b);

        return std::uint16_t (c) ^ std::uint16_t (std::uint32_t (c) >> 16);
    }
}

std::uint16_t Scada::Cell::precompute_checksum (TRIMCORE::Timestamp t, const Information * value) const noexcept {
    // TODO: directly compute on data, without copying?
    Cell copy = *this; 
    copy.t = t;
    copy.value = *value;
    return ::checksum (&copy);
}

void Scada::Cell::update_checksum () noexcept {
    this->checksum = ::checksum (this);
}
bool Scada::Cell::verify_checksum () const noexcept {
    return this->checksum == ::checksum (this);
}

bool Scada::Cell::Information::operator == (const Information & other) const noexcept {
    if (this->specs_raw != other.specs_raw)
        return false;

    std::uint8_t n = sizeof this->data.local;

    switch (this->type) {
        case Cell::Type::Null:
        case Cell::Type::Stream:
            return true;
        case Cell::Type::Expression:
            break;
        case Cell::Type::TimeStamp:
            n = sizeof this->timestamp;
            break;

        case Cell::Type::Address:
            switch (this->width) {
                default:
                case AF_UNIX:
                case AF_HYPERV:
                    break;

                case AF_BTH: n = sizeof (SOCKADDR_BTH); break;
                case AF_IRDA: n = sizeof (SOCKADDR_IRDA); break;
                case AF_INET: n = sizeof (SOCKADDR_IN); break;
                case AF_INET6: n = sizeof (SOCKADDR_IN6); break;
            }
            break;

        case Cell::Type::Integer:
        case Cell::Type::Unsigned:
            if (this->width == 0) {
                auto n = (std::size_t) this->count + 1;
                auto full_byte = n / 8u;
                auto rest_bits = n % 8u;

                if (full_byte) {
                    if (std::memcmp (this->data.local, other.data.local, full_byte) != 0)
                        return false;
                }
                if (rest_bits) {
                    auto mask = (1u << rest_bits) - 1;
                    return (this->data.local [full_byte] & mask) == (other.data.local [full_byte] & mask);
                } else
                    return true;

            } else {
                [[ fallthrough ]];
        case Cell::Type::Float:
                n = ((std::size_t) this->count + 1u) * this->width;
            }
            break;

        case Cell::Type::Atom:
            n = this->count * sizeof (Atom);
            break;

        case Cell::Type::Text:
            if (this->width) {
                n = this->count * this->width;
            } else {
                n = this->count;
            }
            break;
        case Cell::Type::Data:
            n = this->count;
            break;
    }

    if (n) {
        if (n <= sizeof this->data.local) {
            return std::memcmp (this->data.local, other.data.local, n) == 0;
        } else
            return false;
    } else
        return true;
}

bool Scada::Cell::Information::initialize (const AtomPathView & value) {
    std::size_t prefix = 0;
    for (; prefix != value.size (); ++prefix) {
        if (((std::uint64_t) value [prefix]) != 0)
            break;
    }

    auto length = value.size () - prefix;
    if ((prefix < 256) && (length <= 4)) {
        this->type = Cell::Type::Atom;
        this->width = (std::uint8_t) prefix;
        this->count = (std::uint8_t) length;
        for (auto i = 0u; i != length; ++i) {
            this->atom [i] = value [prefix + i];
        }
        return true;
    } else
        return false;
}

bool Scada::Cell::Information::initialize (const Manifold::Address & address) {
    this->type = Cell::Type::Address;
    this->width = (std::uint8_t) address.family;
    this->count = 0;
    this->flags.raw &= 0xF0;

    switch (address.family) {
        case AF_HYPERV:
            std::memcpy (this->data.local, &address.hv.VmId, 2 * sizeof (GUID));
            break;
        case AF_UNIX:
            std::memcpy (this->data.local, &address.unix.sun_path, sizeof (Cell::value.data.local));
            break;

        // following are assigned AS-IS

        case AF_BTH: std::memcpy (this->data.local, &address.bth, sizeof (SOCKADDR_BTH)); break;
        case AF_IRDA: std::memcpy (this->data.local, &address.irda, sizeof (SOCKADDR_IRDA)); break;
        case AF_INET: std::memcpy (this->data.local, &address.ipv4, sizeof (SOCKADDR_IN)); break;
        case AF_INET6: std::memcpy (this->data.local, &address.ipv6, sizeof (SOCKADDR_IN6)); break;
    }
    return true;
}

bool Scada::Cell::Information::initialize (std::string_view string) {
    if (string.size () <= sizeof this->text.local) {
        this->type = Cell::Type::Text;
        this->width = 1;
        this->count = (std::uint8_t) string.size ();
        this->flags.raw &= 0xF0;

        std::memcpy (this->text.local, string.data (), string.size ());
        return true;
    } else
        return false;
}

bool Scada::Cell::Information::initialize (std::wstring_view string) {
    if (string.size () <= sizeof this->text.local / sizeof (wchar_t)) {
        this->type = Cell::Type::Text;
        this->width = sizeof (wchar_t);
        this->count = (std::uint8_t) (string.size () / sizeof (wchar_t));
        this->flags.raw &= 0xF0;

        std::memcpy (this->text.local, string.data (), string.size () / sizeof (wchar_t));
        return true;
    } else
        return false;
}

bool Scada::Cell::Information::initialize (std::u8string_view string) {
    if (string.size () <= sizeof this->text.local) {
        this->type = Cell::Type::Text;
        this->width = 0;
        this->count = (std::uint8_t) string.size ();
        this->flags.raw &= 0xF0;

        std::memcpy (this->text.local, string.data (), string.size ());
        return true;
    } else
        return false;
}

Scada::Cell::Conversion
Scada::Cell::Information::SafeBoolConversion (bool * tgt, std::wstring_view string) {
    char buffer [32];
    auto length = TRIMCORE::s2ascii (string, buffer, sizeof buffer);
    return SafeBoolConversion (tgt, std::string_view (buffer, length));
}

Scada::Cell::Conversion
Scada::Cell::Information::SafeBoolConversion (bool * tgt, std::u8string_view string) {
    char buffer [32];
    auto length = TRIMCORE::s2ascii (string, buffer, sizeof buffer);
    return SafeBoolConversion (tgt, std::string_view (buffer, length));
}

namespace {
    inline bool CompareAsciiCaseInsensitive (char a, char b) {
        if (a >= 'a' && a <= 'z') a -= 'a' - 'A';
        if (b >= 'a' && b <= 'z') b -= 'a' - 'A';
        return a == b;
    }

    inline bool CompareAsciiCaseInsensitive (const std::string_view a, const std::string_view b) {
        if (a.length () == b.length ()) {
            for (std::size_t i = 0; i != a.length (); ++i) {
                if (!CompareAsciiCaseInsensitive (a [i], b [i]))
                    return false;
            }
            return true;
        } else
            return false;
    }
}

Scada::Cell::Conversion
Scada::Cell::Information::SafeBoolConversion (bool * tgt, std::string_view string) {
    if (CompareAsciiCaseInsensitive (string, "true"sv)
        || CompareAsciiCaseInsensitive (string, "yes"sv)
        || CompareAsciiCaseInsensitive (string, "y"sv)
        || CompareAsciiCaseInsensitive (string, "t"sv)
        || CompareAsciiCaseInsensitive (string, "1"sv)) {

        *tgt = true;
        return Conversion::Success;
    }

    if (CompareAsciiCaseInsensitive (string, "false"sv)
        || CompareAsciiCaseInsensitive (string, "no"sv)
        || CompareAsciiCaseInsensitive (string, "n"sv)
        || CompareAsciiCaseInsensitive (string, "f"sv)
        || CompareAsciiCaseInsensitive (string, "0"sv)) {

        *tgt = false;
        return Conversion::Success;
    }

    // TODO: nonzero integer

    return Scada::Cell::Conversion::FailedConversion;
}

template <typename T, std::size_t N> 
Scada::Cell::Conversion Scada::Cell::Information::SafeBoolConversion (bool * tgt, const char (&src) [N], std::size_t count) {
    if (count <= (N / sizeof (T))) {
        return SafeBoolConversion (tgt, std::basic_string_view <T> ((const T *) &src, count));
    } else
        return Scada::Cell::Conversion::Corrupted;
}

Scada::Cell::Conversion
Scada::Cell::Information::SafeBoolStringConversion (std::wstring * tgt, const std::uint32_t (&bits) [8], std::size_t index, TRIMCORE::DescriptionFormatting * fmt) {
    bool value = 0;
    auto result = Scada::Cell::Information::SafeBitAssignment (&value, bits, index);
    if (result == Scada::Cell::Conversion::Success) {
        tgt->assign (TRIMCORE::Describe (value, fmt));
    }
    return result;
}
Scada::Cell::Conversion Scada::Cell::Information::ToBoolean (bool * result, std::size_t index) const noexcept {
    if (this->flags.invalid)
        return Conversion::InvalidFlagSet;

    switch (this->type) {
        case Cell::Type::Null:
        case Cell::Type::Address:
        case Cell::Type::Stream:
        case Cell::Type::Expression:
            break;

        case Cell::Type::TimeStamp:
            if (index > this->count)
                return Conversion::IndexOutOfTypeBounds;

            return SafeBoolConversion (result, this->timestamp.ull, index);

        case Cell::Type::Atom:
            if (index < this->width) {
                *result = false;
                return Conversion::Success;
            }

            index -= this->width;

            if (index >= this->count)
                return Conversion::IndexOutOfTypeBounds;

            if (index < sizeof this->atom / sizeof this->atom [0]) {

                std::uint64_t numeric;
                if (this->atom [index].is_numeric (&numeric)) {
                    return SafeBoolConversion (result, numeric, 0);
                } else {
                    char buffer [20];
                    auto length = this->atom [index].format (buffer, sizeof buffer);
                    return SafeBoolConversion (result, std::string_view (buffer, length));
                }
            } else
                return Conversion::IndexOutOfSupportedBounds;

        case Cell::Type::Float:
            if (index > this->count)
                return Conversion::IndexOutOfTypeBounds;

            switch (this->width) {
                case 4: return SafeBoolConversion (result, this->ieee754f, index); // sizeof (float)
                case 8: return SafeBoolConversion (result, this->ieee754d, index); // sizeof (double)
            }
            return Conversion::UnsupportedWidth;

        case Cell::Type::Integer:
            if (index > this->count)
                return Conversion::IndexOutOfTypeBounds;

            switch (this->width) {
                case 0: return SafeBitAssignment (result, this->uint32, index);
                case 1: return SafeBoolConversion (result, this->int8, index);
                case 2: return SafeBoolConversion (result, this->int16, index);
                case 4: return SafeBoolConversion (result, this->int32, index);
                case 8: return SafeBoolConversion (result, this->int64, index);
                case 16: return SafeBoolConversion (result, this->int128, index);
                case 32: return SafeBoolConversion (result, this->int256, index);
            }
            return Conversion::UnsupportedWidth;

        case Cell::Type::Unsigned:
            if (index > this->count)
                return Conversion::IndexOutOfTypeBounds;

            switch (this->width) {
                case 0: return SafeBitAssignment (result, this->uint32, index);
                case 1: return SafeBoolConversion (result, this->uint8, index);
                case 2: return SafeBoolConversion (result, this->uint16, index);
                case 4: return SafeBoolConversion (result, this->uint32, index);
                case 8: return SafeBoolConversion (result, this->uint64, index);
                case 16: return SafeBoolConversion (result, this->uint128, index);
                case 32: return SafeBoolConversion (result, this->uint256, index);
            }
            return Conversion::UnsupportedWidth;

        case Cell::Type::Text:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;

            if (this->flags.external) {
                return Conversion::External;

            } else {
                switch (this->width) {
                    case 0: return SafeBoolConversion <char8_t> (result, this->text.local, this->count); // UTF-8
                    case 1: return SafeBoolConversion <char>    (result, this->text.local, this->count); // ASCII
                    case 2: return SafeBoolConversion <wchar_t> (result, this->text.local, this->count); // UTF-16
                }
                return Conversion::UnsupportedWidth;
            }

        case Cell::Type::Data:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;

            if (this->flags.external) {
                return Conversion::External;

            } else {
                uint256_t value;
                std::memset (&value, 0, sizeof value);
                std::memcpy (&value, this->data.local, this->count); // TODO: verify valid size
                return SafeBoolConversion (result, value, index);
            }
    }

    // invalid type
    return Conversion::UnsupportedType;
}

Scada::Cell::Conversion Scada::Cell::Information::ToString (std::u8string * result, std::size_t index, TRIMCORE::DescriptionFormatting * format) const {
    if (this->flags.invalid)
        return Conversion::InvalidFlagSet;

    switch (this->type) {
        case Cell::Type::Text:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;

            if (this->flags.external)
                return Conversion::External;

            if (!format || !format->length) {

                switch (this->width) {
                    case 0: // direct UTF-8 read
                    case 1: // ASCII is UTF-8
                        if (this->count > sizeof this->text.local)
                            return Conversion::Corrupted;

                        result->assign ((const char8_t *) this->text.local, this->count);
                        return Conversion::Success;

                    case 2: // UTF-16 -> UTF-8
                        if (this->count == 0) { // empty string
                            result->clear ();
                            return Conversion::Success;
                        }
                        if (TRIMCORE::w2u (*result, (const wchar_t *) this->text.local, this->count))
                            return Conversion::Success;
                        else
                            return Conversion::FailedConversion;

                    default:
                        return Conversion::Corrupted;
                }
            }

            [[ fallthrough ]];

        default:
            std::wstring s;
            auto r = this->ToString (&s, index, format);
            if (r == Conversion::Success) {
                if (!TRIMCORE::w2u (*result, s)) {
                    r = Conversion::FailedConversion;
                }
            }
            return r;
    }
}

namespace {
    template <typename C>
    bool is_string_clean_ascii (std::basic_string_view <C> s) {
        for (auto c : s) {
            if (c < 0 || c > 127) {
                return false;
            }
        }
        return true;
    }
}

Scada::Cell::Conversion Scada::Cell::Information::ToString (std::string * result, std::size_t index, TRIMCORE::DescriptionFormatting * format) const {
    if (this->flags.invalid)
        return Conversion::InvalidFlagSet;

    // if no formatting was requested, we may have optimized/faster paths

    if (!format || !format->length) {
        switch (this->type) {

            case Cell::Type::Null:
                if (index == 0) {
                    result->clear ();
                    return Conversion::Success;
                } else
                    return Conversion::IndexOutOfTypeBounds;

            case Cell::Type::TimeStamp:
                if (index == 0) {
                    SYSTEMTIME t;
                    if (FileTimeToSystemTime (&this->timestamp.ft, &t)) {

                        result->resize (23);
                        std::snprintf (result->data (), result->size () + 1, "%04u-%02u-%02u %02u:%02u:%02u.%03u",
                                       t.wYear, t.wMonth, t.wDay,
                                       t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

                        return Conversion::Success;
                    } else
                        return Conversion::Corrupted;
                } else
                    return Conversion::IndexOutOfTypeBounds;

            case Cell::Type::Atom:
                if (index < this->width) { // this->width is number of ".." prefixes
                    result->clear ();
                    return Conversion::Success;
                }

                index -= this->width;

                if (index >= this->count)
                    return Conversion::IndexOutOfTypeBounds;

                if (index < sizeof this->atom / sizeof this->atom [0]) {
                    result->assign (this->atom [index].format ());
                    return Conversion::Success;
                } else
                    return Conversion::IndexOutOfSupportedBounds;

            case Cell::Type::Text:
                if (index != 0)
                    return Conversion::IndexOutOfTypeBounds;

                if (this->flags.external)
                    return Conversion::External;

                if (this->count == 0) {
                    result->clear ();
                    return Conversion::Success;
                }

                switch (this->width) {
                    case 0:
                        if (this->count > sizeof this->text.local)
                            return Conversion::Corrupted;

                        if (is_string_clean_ascii (std::u8string_view ((const char8_t *) this->text.local, this->count))) {
                            result->assign (this->text.local, this->count);
                            return Conversion::Success;
                        } else
                            return Conversion::FailedConversion;

                    case 1:
                        if (this->count > sizeof this->text.local)
                            return Conversion::Corrupted;
                        
                        result->assign (this->text.local, this->count);
                        return Conversion::Success;

                    case 2:
                        if (this->count > sizeof this->text.local / sizeof (wchar_t))
                            return Conversion::Corrupted;

                        if (is_string_clean_ascii (std::wstring_view ((const wchar_t *) this->text.local, this->count))) {
                            result->resize (this->count);
                            for (auto i = 0u; i != this->count; ++i) {
                                (*result) [i] = (char) ((const wchar_t *) this->text.local) [i];
                            }
                            return Conversion::Success;
                        } else
                            return Conversion::FailedConversion;
                }
                break;

            default:
                break;
        }
    }

    // default
    //  - call UTF-16 version
    //  - then do fast conversion to ASCII

    std::wstring s;
    auto r = this->ToString (&s, index, format);
    if (r == Conversion::Success) {

        if (auto length = TRIMCORE::s2ascii (std::wstring_view (s), nullptr, 0)) {
            result->resize (length);
            if (length != TRIMCORE::s2ascii (std::wstring_view (s), result->data (), length)) {
                r = Conversion::FailedConversion;
            }
        } else {
            result->clear ();
        }
    }
    return r;
}

Scada::Cell::Conversion Scada::Cell::Information::ToString (std::wstring * result, std::size_t index, TRIMCORE::DescriptionFormatting * format) const {
    if (this->flags.invalid)
        return Conversion::InvalidFlagSet;

    switch (this->type) {
        case Cell::Type::Null:
            if (index == 0) {
                if (format && format->length) {
                    result->assign (TRIMCORE::Describe (L""s, format));
                } else {
                    result->clear ();
                }
                return Conversion::Success;
            } else
                return Conversion::IndexOutOfTypeBounds;

        case Cell::Type::TimeStamp:
            if (index == 0) {
                SYSTEMTIME t;
                if (FileTimeToSystemTime (&this->timestamp.ft, &t)) {

                    if (format && format->length) {
                        result->assign (TRIMCORE::Describe (t, format));
                    } else {
                        result->resize (23);
                        std::swprintf (result->data (), result->size () + 1, L"%04u-%02u-%02u %02u:%02u:%02u.%03u",
                                       t.wYear, t.wMonth, t.wDay,
                                       t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
                    }
                    return Conversion::Success;
                } else
                    return Conversion::Corrupted;
            } else
                return Conversion::IndexOutOfTypeBounds;

        case Cell::Type::Atom:
            if (index < this->width) { // this->width is number of ".." prefixes
                result->clear ();
                return Conversion::Success;
            }

            index -= this->width;

            if (index >= this->count)
                return Conversion::IndexOutOfTypeBounds;

            if (index < sizeof this->atom / sizeof this->atom [0]) {
                result->assign (TRIMCORE::Describe (this->atom [index], format));
                return Conversion::Success;
            } else
                return Conversion::IndexOutOfSupportedBounds;

        case Cell::Type::Float:
            if (index > this->count)
                return Conversion::IndexOutOfTypeBounds;

            switch (this->width) {
                case 4: return SafeStringConversion (result, this->ieee754f, index, format); // sizeof (float)
                case 8: return SafeStringConversion (result, this->ieee754d, index, format); // sizeof (double)
            }
            return Conversion::UnsupportedWidth;

        case Cell::Type::Integer:
            if (index > this->count)
                return Conversion::IndexOutOfTypeBounds;

            switch (this->width) {
                case 0: return SafeBoolStringConversion (result, this->uint32, index, format);
                case 1: return SafeStringConversion (result, this->int8, index, format);
                case 2: return SafeStringConversion (result, this->int16, index, format);
                case 4: return SafeStringConversion (result, this->int32, index, format);
                case 8: return SafeStringConversion (result, this->int64, index, format);
                case 16: return SafeStringConversion (result, this->int128, index, format);
                case 32: return SafeStringConversion (result, this->int256, index, format);
            }
            return Conversion::UnsupportedWidth;

        case Cell::Type::Unsigned:
            if (index > this->count)
                return Conversion::IndexOutOfTypeBounds;

            switch (this->width) {
                case 0: return SafeBoolStringConversion (result, this->uint32, index, format);
                case 1: return SafeStringConversion (result, this->uint8, index, format);
                case 2: return SafeStringConversion (result, this->uint16, index, format);
                case 4: return SafeStringConversion (result, this->uint32, index, format);
                case 8: return SafeStringConversion (result, this->uint64, index, format);
                case 16: return SafeStringConversion (result, this->uint128, index, format);
                case 32: return SafeStringConversion (result, this->uint256, index, format);
            }
            return Conversion::UnsupportedWidth;

        case Cell::Type::Address:
            if (index == 0) {
                Scada::Manifold::Address address;
                auto error = this->ToAddress (&address, index);
                if (error == Conversion::Success) {
                    result->assign (TRIMCORE::Describe (address, format));
                }
                return error;
            } else
                return Conversion::IndexOutOfTypeBounds;

        case Cell::Type::Data:
            if (this->count > sizeof this->data.local)
                return Conversion::Corrupted;

            result->assign (TRIMCORE::Describe (std::span (this->data.local, this->count), format));
            return Conversion::Success;

        case Cell::Type::Text:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;
            
            if (this->flags.external)
                return Conversion::External;

            switch (this->width) {
                case 0: // UTF-8
                    if (this->count > sizeof this->text.local)
                        return Conversion::Corrupted;

                    if (TRIMCORE::u2w (*result, (const char8_t *) this->text.local, this->count)) {
                        if (format && format->length) {
                            result->assign (TRIMCORE::Describe (*result, format));
                        }
                        return Conversion::Success;
                    } else
                        return Conversion::FailedConversion;

                case 1: // ASCII
                    if (this->count > sizeof this->text.local)
                        return Conversion::Corrupted;

                    result->resize (this->count);
                    for (auto i = 0u; i != this->count; ++i) {
                        (*result) [i] = (std::uint8_t) this->text.local [i]; // just assign ASCII first
                    }

                    if (format && format->length) {
                        result->assign (TRIMCORE::Describe (*result, format));
                    }
                    return Conversion::Success;

                case 2:
                    if (this->count > sizeof this->text.local / sizeof (wchar_t)) 
                        return Conversion::Corrupted;

                    if (format && format->length) {
                        // pass through Describe formatting
                        result->assign (TRIMCORE::Describe (std::wstring_view ((const wchar_t *) this->text.local, this->count), format));
                    } else {
                        // direct copy UTF-16 to UTF-16
                        result->assign ((const wchar_t *) this->text.local, this->count);
                    }
                    return Conversion::Success;
            }
            return Conversion::Corrupted;

        case Cell::Type::Stream:
        case Cell::Type::Expression:
            break;
    }

    // invalid type
    return Conversion::UnsupportedType;
}

Scada::Cell::Conversion Scada::Cell::Information::ToFileTime (FILETIME * timestamp, std::size_t index) const {
    if (this->flags.invalid)
        return Conversion::InvalidFlagSet;

    switch (this->type) {
        case Cell::Type::Null:
        case Cell::Type::Atom:
        case Cell::Type::Float:
        case Cell::Type::Integer:
        case Cell::Type::Unsigned: // 32-bit is unix, 64-bit is FILETIME
        case Cell::Type::Address:
        case Cell::Type::Data: // SYSTEMTIME?
        case Cell::Type::Stream:
        case Cell::Type::Expression:
            break;

        case Cell::Type::Text:
            // TODO: convert!
            break;

        case Cell::Type::TimeStamp:
            if (index > this->count)
                return Scada::Cell::Conversion::IndexOutOfTypeBounds;

            if (index == 0) {
                *timestamp = this->timestamp.ft;
                return Conversion::Success;
            } else
                return Conversion::IndexOutOfTypeBounds;
    }

    // invalid type
    return Conversion::UnsupportedType;
}

Scada::Cell::Conversion Scada::Cell::Information::ToAddress (Manifold::Address * address, std::size_t index) const {
    if (this->flags.invalid)
        return Conversion::InvalidFlagSet;

    switch (this->type) {
        case Cell::Type::Null:
        case Cell::Type::TimeStamp:
        case Cell::Type::Atom:
        case Cell::Type::Float:
        case Cell::Type::Integer:
        case Cell::Type::Unsigned:
        case Cell::Type::Data:
        case Cell::Type::Stream:
        case Cell::Type::Expression:
            break;

        case Cell::Type::Address:
            if (index == 0) {
                switch (this->width) {
                    default:
                        return Conversion::UnsupportedData;

                    case AF_UNIX:
                        address->unix.sun_family = AF_UNIX;
                        std::memcpy (&address->unix.sun_path, this->data.local, sizeof (Scada::Cell::value.data.local));
                        address->unix.sun_path [sizeof (Scada::Cell::value.data.local)] = '\0';
                        break;

                    case AF_HYPERV:
                        address->hv.Family = AF_HYPERV;
                        address->hv.Reserved = 0;
                        std::memcpy (&address->hv.VmId, this->data.local, 2 * sizeof (GUID));
                        break;

                    case AF_BTH: std::memcpy (&address->bth, this->data.local, sizeof (SOCKADDR_BTH)); break;
                    case AF_IRDA: std::memcpy (&address->irda, this->data.local, sizeof (SOCKADDR_IRDA)); break;
                    case AF_INET: std::memcpy (&address->ipv4, this->data.local, sizeof (SOCKADDR_IN)); break;
                    case AF_INET6: std::memcpy (&address->ipv6, this->data.local, sizeof (SOCKADDR_IN6)); break;
                }
                return Conversion::Success;
            } else
                return Conversion::IndexOutOfTypeBounds;

        case Cell::Type::Text:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;

            if (this->flags.external) {
                return Conversion::External;

            } else {
                switch (this->width) {
                    case 0:
                    case 1:
                        // TODO: make nothrow
                        *address = Manifold::Address (std::string_view ((const char *) this->text.local, this->count));
                        break;
                    case 2:
                        *address = Manifold::Address (std::wstring_view ((const wchar_t *) this->text.local, this->count));
                        break;
                }
                return Conversion::UnsupportedWidth;
            }
    }

    // invalid type
    return Conversion::UnsupportedType;
}

Scada::Cell::Conversion Scada::Cell::Information::ToAtom (Atom * result, std::size_t index) const {
    if (this->flags.invalid)
        return Conversion::InvalidFlagSet;

    switch (this->type) {
        case Cell::Type::Null:
            result->clear ();
            return Conversion::Success;

        case Cell::Type::TimeStamp:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;

            result->set (this->timestamp.ull);
            return Conversion::Success;

        case Cell::Type::Atom:
            if (index < this->width) { // this->width is number of ".." prefixes
                result->clear ();
                return Conversion::Success;
            }

            index -= this->width;

            if (index >= this->count)
                return Conversion::IndexOutOfTypeBounds;

            if (index < sizeof this->atom / sizeof this->atom [0]) {
                *result = this->atom [index];
                return Conversion::Success;
            } else
                return Conversion::IndexOutOfSupportedBounds;

        case Cell::Type::Float:
            break;

        case Cell::Type::Integer:
        case Cell::Type::Unsigned:
            return this->SafeVectorExtract (result, index);

        case Cell::Type::Data:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;

            if (this->flags.external)
                return Conversion::External;

            if (this->count <= sizeof (Atom)) {
                *result = Atom ();
                std::memcpy (result, this->data.local, this->count);
                return Conversion::Success;
            } else
                return Conversion::ValueOutOfRangeAbove;

        case Cell::Type::Stream:
        case Cell::Type::Expression:
        case Cell::Type::Address:
            break;

        case Cell::Type::Text:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;

            if (this->flags.external) {
                return Conversion::External;

            } else {
                bool converted = false;

                switch (this->width) {
                    case 0:
                    case 1:
                        converted = result->parse (std::string_view ((const char *) this->text.local, this->count));
                        break;
                    case 2:
                        converted = result->parse (std::wstring_view ((const wchar_t *) this->text.local, this->count));
                        break;
                    default:
                        return Conversion::UnsupportedWidth;
                }

                if (converted) {
                    return Conversion::Success;
                } else {
                    return Conversion::FailedConversion;
                }
            }
    }

    // invalid type
    return Conversion::UnsupportedType;
}

Scada::Cell::Conversion Scada::Cell::Information::ToAtomPath (Scada::AtomPath * result) const {
    if (this->flags.invalid)
        return Conversion::InvalidFlagSet;

    switch (this->type) {
        case Cell::Type::Null:
            result->clear ();
            return Conversion::Success;

        case Cell::Type::TimeStamp:
            result->resize (1);
            if (result->front ().set (this->timestamp.ull)) {
                return Conversion::Success;
            } else
                return Conversion::ValueOutOfRangeAbove;

        case Cell::Type::Atom:
            result->clear ();
            result->resize ((std::size_t) this->width + (std::size_t) this->count);

            if (this->count) {
                std::memcpy (&(*result) [this->width], &this->atom [0], this->count * sizeof (Atom));
            }
            return Conversion::Success;

        case Cell::Type::Float:
            break;
            
        case Cell::Type::Integer:
        case Cell::Type::Unsigned:
            if (auto n = (unsigned int) this->count + 1u) {
                result->resize (n);

                for (auto i = 0u; i != n; ++i) {
                    auto r = this->SafeVectorExtract (&(*result) [i], i);
                    if (r != Scada::Cell::Conversion::Success)
                        return r;
                }
                return Scada::Cell::Conversion::Success;
            } else
                return Scada::Cell::Conversion::UnsupportedWidth;

        case Cell::Type::Data:
            break;

        case Cell::Type::Stream:
        case Cell::Type::Expression:
        case Cell::Type::Address:
            break;

        case Cell::Type::Text:
            if (this->flags.external) {
                return Conversion::External;

            } else {
                bool converted = false;

                switch (this->width) {
                    case 0:
                    case 1:
                        converted = result->parse (std::string_view ((const char *) this->text.local, this->count));
                        break;
                    case 2:
                        converted = result->parse (std::wstring_view ((const wchar_t *) this->text.local, this->count));
                        break;
                    default:
                        return Conversion::UnsupportedWidth;
                }

                if (converted) {
                    return Conversion::Success;
                } else {
                    return Conversion::FailedConversion;
                }
            }
    }

    // invalid type
    return Conversion::UnsupportedType;
}

Scada::Cell::Conversion Scada::Cell::Information::ToRawDataBufferSize (std::size_t * length, bool * direct) const {
    if (this->flags.invalid)
        return Conversion::InvalidFlagSet;

    switch (this->type) {
        case Cell::Type::Null:
            *length = 0;
            return Conversion::Success;

        case Cell::Type::TimeStamp:
            *length = sizeof this->timestamp;
            if (direct) {
                *direct = true;
            }
            return Conversion::Success;

        case Cell::Type::Atom:
            *length = this->count * sizeof (Atom); // TODO: count this->width of empty atoms?
            if (direct) {
                *direct = true;
            }
            return Conversion::Success;

        case Cell::Type::Float:
            switch (this->width) {
                case 4:
                case 8:
                    *length = std::size_t (this->width) * (std::size_t (this->count) + 1);
                    if (direct) {
                        *direct = true;
                    }
                    return Conversion::Success;
            }
            return Conversion::UnsupportedWidth;

        case Cell::Type::Integer:
        case Cell::Type::Unsigned:
            switch (this->width) {
                case 0:
                    *length = 1u + this->count / 8;
                    if (direct) {
                        *direct = false;
                    }
                    return Conversion::Success;

                case 1:
                case 2:
                case 4:
                case 8:
                case 16:
                    *length = std::size_t (this->width) * (std::size_t (this->count) + 1);
                    if (direct) {
                        *direct = true;
                    }
                    return Conversion::Success;

                case 32:
                    *length = this->width;
                    if (direct) {
                        *direct = true;
                    }
                    return Conversion::Success;
            }
            return Conversion::UnsupportedWidth;

        case Cell::Type::Data:
            *length = this->count;
            if (direct) {
                *direct = true;
            }
            return Conversion::Success;

        case Cell::Type::Stream:
        case Cell::Type::Expression:
            break;

        case Cell::Type::Address:
            switch (this->width) {
                default:
                    return Conversion::UnsupportedData;

                case AF_BTH: *length = sizeof (SOCKADDR_BTH); if (direct) *direct = true; return Conversion::Success;
                case AF_IRDA: *length = sizeof (SOCKADDR_IRDA); if (direct) *direct = true; return Conversion::Success;
                case AF_INET: *length = sizeof (SOCKADDR_IN); if (direct) *direct = true; return Conversion::Success;
                case AF_INET6: *length = sizeof (SOCKADDR_IN6); if (direct) *direct = true; return Conversion::Success;

                // NOTE: following are synthesized, not direct copy from cell data

                case AF_HYPERV:
                    *length = sizeof (SOCKADDR_HV);
                    if (direct) {
                        *direct = false;
                    }
                    return Conversion::Success;

                case AF_UNIX:
                    *length = sizeof (ADDRESS_FAMILY) + sizeof (Scada::Cell::value.data.local) + 1;
                    if (direct) {
                        *direct = false;
                    }
                    return Conversion::Success;
            }
            return Conversion::Success;

        case Cell::Type::Text:
            if (this->flags.external) {
                return Conversion::External;

            } else {
                switch (this->width) {
                    case 0:
                    case 1:
                        *length = this->count;
                        if (direct) {
                            *direct = true;
                        }
                        return Conversion::Success;
                    case 2:
                        *length = this->count * sizeof (wchar_t);
                        if (direct) {
                            *direct = true;
                        }
                        return Conversion::Success;
                    default:
                        return Conversion::UnsupportedWidth;
                }
            }
    }

    // invalid type
    return Conversion::UnsupportedType;
}

Scada::Cell::Conversion Scada::Cell::Information::ToRawDataSynthesize (void * buffer, std::size_t length) const {
    switch (this->type) {
        case Cell::Type::Null:
        case Cell::Type::TimeStamp:
        case Cell::Type::Atom:
        case Cell::Type::Float:
            break;

        case Cell::Type::Integer:
        case Cell::Type::Unsigned:
            switch (this->width) {
                case 0:
                    std::memcpy (buffer, this->data.local, length);

                    if (auto bytes = (std::uint8_t *) buffer) {
                        //bytes [this->count / 8] &= 0xFF; // mask incomplete highest byte
                    }
                    return Conversion::Success;
            }
            return Conversion::UnsupportedWidth;

        case Cell::Type::Address:
            switch (this->width) {
                default:
                    return Conversion::UnsupportedData;

                case AF_UNIX:
                    if (auto address = (SOCKADDR_UN *) buffer) {
                        address->sun_family = AF_UNIX;
                        std::memcpy (&address->sun_path, this->data.local, sizeof (Scada::Cell::value.data.local));
                        address->sun_path [sizeof (Scada::Cell::value.data.local)] = '\0';
                    }
                    return Conversion::Success;

                case AF_HYPERV:
                    if (auto address = (SOCKADDR_HV *) buffer) {
                        address->Family = AF_HYPERV;
                        address->Reserved = 0;
                        std::memcpy (&address->VmId, this->data.local, 2 * sizeof (GUID));
                    }
                    return Conversion::Success;
            }
            break;

        case Cell::Type::Data:
        case Cell::Type::Stream:
        case Cell::Type::Expression:
        case Cell::Type::Text:
            break;
    }
    // invalid type
    return Conversion::UnsupportedType;
}

Scada::Cell::Conversion Scada::Cell::Information::ToRawData (std::vector <std::byte> & buffer) const {
    bool direct = false;
    std::size_t length = 0;

    auto r = this->ToRawDataBufferSize (&length, &direct);
    if (r == Conversion::Success) {
        buffer.resize (length);

        if (length) {
            if (direct) {
                std::memcpy (buffer.data (), this->data.local, length);
            } else {
                r = this->ToRawDataSynthesize (buffer.data (), length);
            }
        }
    }
    return r;
}
Scada::Cell::Conversion Scada::Cell::Information::ToRawData (void * buffer, std::size_t * length) const {
    bool direct = false;
    auto capacity = *length;

    auto r = this->ToRawDataBufferSize (length, &direct);
    if (r == Conversion::Success) {
        if (length) {
            if (*length > capacity)
                return Conversion::InsufficientBuffer;

            if (direct) {
                std::memcpy (buffer, this->data.local, *length);
            } else {
                r = this->ToRawDataSynthesize (buffer, *length);
            }
        }
    }
    return r;
}

bool Scada::Cell::get (bool * value, std::size_t index, Conversion * error) {
    this->lock.AcquireShared ();
    auto e = this->value.ToBoolean (value, index);
    this->lock.ReleaseShared ();
    if (error) {
        *error = e;
    }
    return e == Conversion::Success;
}

void Scada::Cell::finalize () noexcept {
    /*if (this->value.flags.reserved2) {
        GetSystemTimePreciseAsFileTime (&this->t.ft);
    } else*/ {
        GetSystemTimeAsFileTime (&this->t.ft);
    }
    this->update_checksum ();
}
void Scada::Cell::finalize (TRIMCORE::Timestamp ts) noexcept {
    this->t = ts;
    this->update_checksum ();
}
void Scada::Cell::finalize (TRIMCORE::Timestamp ts, std::uint16_t crc) noexcept {
    this->t = ts;
    this->checksum = crc;
}

void Scada::Cell::acquire () noexcept {
    this->lock.acquire ();
}
void Scada::Cell::commit () noexcept {
    this->finalize ();
    this->lock.release ();
}

namespace {
    template <typename... Params>
    inline std::wstring to_chars_wstring (Params... params) {
        char tmp [32];
        auto r = std::to_chars (tmp, tmp + sizeof tmp, params...);

        if (r.ec == std::errc {}) {
            return std::wstring (&tmp [0], r.ptr);
        } else
            return std::wstring ();
    }
}

std::wstring TRIMCORE::Describe (const Scada::Cell::Information & cell, DescriptionFormatting * format) {
    std::wstring result;
    
    if (format) {
        ++format->skip_auto_string_formatting;
    }
    switch (cell.type) {
        case Scada::Cell::Type::Null:
            result = L"NULL";
            break;

        case Scada::Cell::Type::TimeStamp:
        case Scada::Cell::Type::Address:
        case Scada::Cell::Type::Data:
        case Scada::Cell::Type::Text:

            switch (auto failure = cell.ToString (&result, 0, format)) {

                case Scada::Cell::Conversion::Success:
                    switch (cell.type) {
                        case Scada::Cell::Type::TimeStamp:
                            /*if (cell.timestamp.flags.dst) {
                                result += L" DST";
                            }*/
                            /*if (cell.timestamp.flags.substituted) {
                                result += L"SUBSTITUTED";
                            }*/
                            break;

                        case Scada::Cell::Type::Data:
                            result.reserve (result.size () + 6);
                            result += L" ("sv;
                            result += to_chars_wstring (cell.count);
                            result += L"B)"sv;
                            break;

                        case Scada::Cell::Type::Text:
                            result.reserve (result.size () + 14);
                            result.insert (result.begin (), 1, L'\"');
                            result.append (L"\" ("sv);
                            result.append (to_chars_wstring (cell.count));

                            switch (cell.width) {
                                case 0: // UTF-8
                                    result.append (L") UTF-8"sv);
                                    break;
                                case 1: // ASCII
                                    result.append (L") ASCII"sv);
                                    break;
                                case 2: // UTF-16
                                    result.append (L") UTF-16"sv);
                                    break;
                                default:
                                    result.append (L") ???"sv);
                                    break;
                            }
                            break;

                        default:
                            break;
                    }
                    break;

                default:
                    result.assign (Describe (failure));
            }
            break;
        
        case Scada::Cell::Type::Integer:
        case Scada::Cell::Type::Unsigned:
        case Scada::Cell::Type::Float:
            if (cell.count) {
                result += L'(';
            }

            for (auto i = 0u; i != cell.count + 1u; ++i) {
                if (i) {
                    result += L',';
                }

                std::wstring value;
                switch (cell.type) {
                    default: break;

                    case Scada::Cell::Type::Float:
                        switch (cell.width) {
                            case 4: value = to_chars_wstring (cell.ieee754f [i]); break;
                            case 8: value = to_chars_wstring (cell.ieee754d [i]); break;
                            default:
                                value = L"?.?";
                                if (format) {
                                    format->invalid = true;
                                }
                        }
                        if (value.find (L'.') == std::wstring::npos) {
                            value += L".0";
                        }
                        break;

                    case Scada::Cell::Type::Integer:
                        switch (cell.width) {
                            case 0: value = to_chars_wstring ((cell.uint32 [i / 32] & (1u << (i % 32))) ? 1 : 0); break;
                            case 1: value = to_chars_wstring (cell.int8 [i]); break;
                            case 2: value = to_chars_wstring (cell.int16 [i]); break;
                            case 4: value = to_chars_wstring (cell.int32 [i]); break;
                            case 8: value = to_chars_wstring (cell.int64 [i]); break;
                            case 16: value = Describe (cell.int128 [i]); break;
                            case 32: value = Describe (cell.int256); break;

                            default:
                                value = L"?";
                                if (format) {
                                    format->invalid = true;
                                }
                        }
                        break;

                    case Scada::Cell::Type::Unsigned:
                        switch (cell.width) {
                            case 0: value = to_chars_wstring ((cell.uint32 [i / 32] & (1u << (i % 32))) ? 1 : 0); break;
                            case 1: value = to_chars_wstring (cell.uint8 [i]); break;
                            case 2: value = to_chars_wstring (cell.uint16 [i]); break;
                            case 4: value = to_chars_wstring (cell.uint32 [i]); break;
                            case 8: value = to_chars_wstring (cell.uint64 [i]); break;
                            case 16: value = Describe (cell.uint128 [i]); break;
                            case 32: value = Describe (cell.uint256); break;

                            default:
                                value = L'?';
                                if (format) {
                                    format->invalid = true;
                                }
                        }
                        if (!cell.count) {
                            value += L'u';
                        }
                        break;
                }
                result += value;
            }
            if (cell.count) {
                result += L')';
                if (cell.type == Scada::Cell::Type::Unsigned) {
                    result += L'u';
                }
            }
            
            result += L':';
            if (cell.count != 0) {
                result += to_chars_wstring (cell.count + 1u);
                result += L'\x00D7'; // TODO: \xXXXX
            }
            if (cell.width) {
                result += to_chars_wstring (8 * cell.width);
            } else {
                result += L'1';
            }
            break;

        case Scada::Cell::Type::Atom: {
            Scada::AtomPathView view (&cell.atom [0], cell.count);

            if (cell.width) {
                result.reserve (view.formatted_length () + 3 * std::size_t (cell.width));
                result.assign (L".."sv);
                for (auto i = 1u; i != cell.width; ++i) {
                    result.append (L"/.."sv);
                }
                result.append (Describe (view, format));
            } else {
                result.assign (Describe (view, format));
                result.erase (result.begin ());
            }
            result.append (L":"sv);
        } break;

        case Scada::Cell::Type::Stream:
            result = L"STREAM";
            break;

        case Scada::Cell::Type::Expression:
            for (auto & input : cell.expression.input) {
                if (input.operation != Scada::Cell::Expression::Operation::Nop) {
                    result += L' ';
                    result += Describe (input.operation, format);
                    result += L' ';
                    if (input.flags.absolute) {
                        result += L'$';
                    }
                    result += L'{';
                    result += Describe (input.cell, format);
                    result += L'}';
                    if (input.flags.dependent) {
                        result += L'!';
                    }
                    if (input.flags.convert) {
                        result += L" convert";
                    }
                }
            }
            break;

        default:
            result = L'?';
            if (format) {
                format->invalid = true;
            }
    }

    if (cell.flags.invalid) {
        result += L" !invalid";
    }

    if (format) {
        --format->skip_auto_string_formatting;
    }
    if (!format || !format->skip_auto_string_formatting) {
        return Describe (result, format);
    } else
        return result;
}

std::wstring TRIMCORE::Describe (const Scada::Cell & cell, DescriptionFormatting * format) {
    std::wstring result;

    if (format) {
        ++format->skip_auto_string_formatting;
    }

    if (cell.value.type == Scada::Cell::Type::Expression) {
        result += L"= {";
        result += Describe (cell.base);
        result += L"}";
    }

    result += Describe (cell.value, format);

    result += L" [";
    result += Describe (cell.t);
    result += L"]";

    if (!cell.verify_checksum ()) {
        result += L" CHECKSUM MISMATCH!";
    }

    if (cell.expression) {
        result += L" = ";
        result += Describe (cell.expression);
    }

    if (format) {
        --format->skip_auto_string_formatting;
    }
    if (!format || !format->skip_auto_string_formatting) {
        return Describe (result, format);
    } else
        return result;
}
