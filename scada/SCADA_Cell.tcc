#ifndef SCADA_CELL_TCC
#define SCADA_CELL_TCC

#include "SCADA_Cell.hpp"
#include <type_traits>
#include <limits>

inline constexpr Scada::Cell::ID::ID () noexcept
    : index (0)
    , segment (0) {}
inline constexpr Scada::Cell::ID::ID (std::uint16_t segment, std::uint32_t index) noexcept
    : index (index)
    , segment (segment) {}
inline constexpr Scada::Cell::ID::operator bool () const noexcept {
    return this->index != 0
        || this->segment != 0;
}

inline Scada::Cell::Cell () noexcept {
    std::memset (this, 0, sizeof (Cell));
}
inline Scada::Cell::Cell (const Cell & other) noexcept {
    std::memcpy (this, &other, sizeof (Cell));
}
inline Scada::Cell & Scada::Cell::operator = (const Cell & other) noexcept {
    std::memcpy (this, &other, sizeof (Cell));
    return *this;
}

inline Scada::Cell::ConversionException::ConversionException (Conversion error)
    : std::runtime_error (TRIMCORE::s2ascii (TRIMCORE::Describe (error)))
    , error (error) {}

inline Scada::Cell::Information::Information () noexcept {
    std::memset (this, 0, sizeof (Information));
}
inline Scada::Cell::Information::Information (const Information & other) noexcept {
    std::memcpy (this, &other, sizeof (Information));
}
inline Scada::Cell::Information & Scada::Cell::Information::operator = (const Information & other) noexcept {
    std::memcpy (this, &other, sizeof (Information));
    return *this;
}

inline bool Scada::Cell::Information::initialize (bool value) {
    this->type = Cell::Type::Unsigned;
    this->width = 0;
    this->count = 0;
    this->flags.raw &= 0xF0;
    this->uint1 = value;
    return true;
}
inline bool Scada::Cell::Information::initialize (FILETIME value) {
    this->type = Cell::Type::TimeStamp;
    this->width = 0;
    this->count = 0;
    this->flags.raw &= 0xF0;
    this->timestamp.ft = value;
    return true;
}

template <typename T> inline
bool Scada::Cell::Information::CommonNumberInitialization (Cell::Type type, T value) {
    this->type = type;
    this->width = sizeof value;
    this->count = 0;
    this->flags.raw &= 0xF0;
    std::memcpy (this->data.local, &value, sizeof value);
    return true;
}

template <typename T> inline
bool Scada::Cell::Information::CommonCharInitialization (std::uint8_t width, T value) {
    this->type = Cell::Type::Text;
    this->width = width;
    this->count = 1;
    this->flags.raw &= 0xF0;
    switch (width) {
        case 0:
            this->text.local [0] = (char) value;
            break;
        default:
            std::memcpy (this->text.local, &value, sizeof value);
    }
    return true;
}

inline std::size_t Scada::Cell::Information::ElementCount () const noexcept {
    auto n = this->count;
    switch (this->type) {
        case Cell::Type::Unsigned:
        case Cell::Type::Integer:
        case Cell::Type::Float:
            ++n;
            break;
        default:
            break;
    }
    return n;
}

template <typename T> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeIntConversion (T * tgt, T src) {
    *tgt = src;
    return Conversion::Success;
}

template <typename T, typename S> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeIntConversion (T * tgt, const S & src) {

    if constexpr (std::is_same_v <T, Scada::Atom>) {

        if constexpr (std::is_signed_v <S>) {
            if (src < 0) {
                // TODO: consider trying to convert to Atom as string, i.e. "-127" should work
                return Conversion::ValueOutOfRangeBelow;
            }
        }
        if constexpr (std::is_same_v <S, std::uint64_t>) {
            if (src >= 0x8000'0000'0000'0000) {
                return Conversion::ValueOutOfRangeAbove;
            }
        }

        tgt->set (src);
        return Conversion::Success;

    } else {
        if constexpr (!std::is_floating_point_v <T>) {
            if (sizeof (T) < sizeof (S)) { // assigning to smaller type

                // will the value fit?
                if (src > S (std::numeric_limits <T>::max ()))
                    return Conversion::ValueOutOfRangeAbove;

                if (src < S (std::numeric_limits <T>::min ()))
                    return Conversion::ValueOutOfRangeBelow;

            } else {
                if constexpr (!std::is_signed_v <T> && std::is_signed_v <S>) { // if Source is negative, cannot assign
                    if (src < 0)
                        return Conversion::ValueOutOfRangeBelow;
                }

                if (sizeof (T) == sizeof (S)) {
                    if constexpr (std::is_signed_v <T> && !std::is_signed_v <S>) { // if Target is signed and S is not
                        if (src > std::numeric_limits <T>::max ())
                            return Conversion::ValueOutOfRangeAbove;
                    }
                }
            }
        }

        *tgt = (T) src;
        return Conversion::Success;
    }
}

template <typename T, typename LO, typename HI> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeIntConversion (T * tgt, const double_integer <LO, HI> & src) {

    if constexpr (std::is_same_v <T, Scada::Atom>) {

        if constexpr (std::is_signed_v <double_integer <LO, HI>>) {
            if (src < 0) {
                return Conversion::ValueOutOfRangeBelow;
            }
        }
        if (src >= double_integer <LO, HI> (0x8000'0000'0000'0000)) {
            return Conversion::ValueOutOfRangeAbove;
        }

        std::uint64_t ull;
        std::memcpy (&ull, &src, sizeof ull);
        tgt->set (ull);
        return Conversion::Success;

    } else
    if constexpr (std::is_floating_point_v <T>) {
        *tgt = (T) (double) src;
        return Conversion::Success;
    } else {

        if (src > double_integer <LO, HI> (std::numeric_limits <T>::max ()))
            return Conversion::ValueOutOfRangeAbove;

        if (src < double_integer <LO, HI> (std::numeric_limits <T>::min ()))
            return Conversion::ValueOutOfRangeBelow;

        std::memcpy (tgt, &src, sizeof (T));
        return Conversion::Success;
    }
}

template <typename T, typename S> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeFltConversion (T * tgt, S src) {
    if constexpr (!std::is_floating_point_v <T>) {
        src = std::round (src);

        // will the value fit?
        if (src > S (std::numeric_limits <T>::max ()))
            return Conversion::ValueOutOfRangeAbove;

        if (src < S (std::numeric_limits <T>::min ()))
            return Conversion::ValueOutOfRangeBelow;
    }

    *tgt = T (src);
    return Conversion::Success;
}

template <typename T, typename S, std::size_t N> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeIntExtraction (T * tgt, const S (&src) [N], std::size_t index) {
    if (index < N) {
        return SafeIntConversion (tgt, src [index]);
    } else
        return Conversion::IndexOutOfSupportedBounds;
}

template <typename T, typename S, std::size_t N> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeFltExtraction (T * tgt, const S (&src) [N], std::size_t index) {
    if (index < N) {
        return SafeFltConversion (tgt, src [index]);
    } else
        return Conversion::IndexOutOfSupportedBounds;
}

template <typename T, typename S> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeIntExtraction (T * tgt, const S & src, std::size_t index) {
    if (index == 0) {
        return SafeIntConversion (tgt, src);
    } else
        return Conversion::IndexOutOfSupportedBounds;
}

template <typename T> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeBitAssignment (T * tgt, const std::uint32_t (&bits) [8], std::size_t index) {
    if (index < 256) {
        if (_bittest ((const long *) &bits [index / 32], index % 32)) {
        //if (bits [index / 32] & (1u << (index % 32))) { // _bittest?
            *tgt = T (1u);
        } else {
            *tgt = T (0u);
        }
        return Conversion::Success;
    } else
        return Conversion::IndexOutOfSupportedBounds;
}

template <typename T> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeStrConversion (T * tgt, std::string_view string) {
    using namespace std::literals;

    string.remove_prefix (string.find_first_not_of (" \t\r\n\v\f"sv)); // TODO: NUL byte?
    string = string.substr (0, string.find_last_not_of (" \t\r\n\v\f"sv) + 1);

    int base = 10;
    if (string.starts_with ("0x") || string.starts_with ("0X")) { string.remove_prefix (2); base = 16; }
    if (string.starts_with ("0b") || string.starts_with ("0B")) { string.remove_prefix (2); base = 2; }

    if (string.empty ())
        return Conversion::FailedConversion;

    T result;

    auto svb = string.data ();
    auto sve = string.size () + svb;
    auto [end, error] = std::from_chars (svb, sve, result);
    switch (error) {
        case std::errc::result_out_of_range:
            return Conversion::ValueOutOfRangeAbove;

        case (std::errc) 0:
            if (end == sve) {
                *tgt = result;
                return Conversion::Success;
            }

            [[ fallthrough ]];
        default:
            return Conversion::FailedConversion;
    }
}

template <typename T> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeStrConversion (T * tgt, std::wstring_view string) {
    char buffer [32];
    auto length = TRIMCORE::s2ascii (string, buffer, sizeof buffer);
    return SafeStrConversion (tgt, std::string_view (buffer, length));
}

template <typename T> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeStrConversion (T * tgt, std::u8string_view string) {
    char buffer [32];
    auto length = TRIMCORE::s2ascii (string, buffer, sizeof buffer);
    return SafeStrConversion (tgt, std::string_view (buffer, length));
}

template <typename T>
Scada::Cell::Conversion
Scada::Cell::Information::SafeVectorExtract (T * result, std::size_t index) const {
    switch (this->type) {
        case Cell::Type::Integer:
            if (index > this->count)
                return Scada::Cell::Conversion::IndexOutOfTypeBounds;

            switch (this->width) {
                case 0: return SafeBitAssignment (result, this->uint32, index);
                case 1: return SafeIntExtraction (result, this->int8, index);
                case 2: return SafeIntExtraction (result, this->int16, index);
                case 4: return SafeIntExtraction (result, this->int32, index);
                case 8: return SafeIntExtraction (result, this->int64, index);
                case 16: return SafeIntExtraction (result, this->int128, index);
                case 32: return SafeIntExtraction (result, this->int256, index);
            }
            return Scada::Cell::Conversion::UnsupportedWidth;

        case Cell::Type::Unsigned:
            if (index > this->count)
                return Scada::Cell::Conversion::IndexOutOfTypeBounds;

            switch (this->width) {
                case 0: return SafeBitAssignment (result, this->uint32, index);
                case 1: return SafeIntExtraction (result, this->uint8, index);
                case 2: return SafeIntExtraction (result, this->uint16, index);
                case 4: return SafeIntExtraction (result, this->uint32, index);
                case 8: return SafeIntExtraction (result, this->uint64, index);
                case 16: return SafeIntExtraction (result, this->uint128, index);
                case 32: return SafeIntExtraction (result, this->uint256, index);
            }
            return Scada::Cell::Conversion::UnsupportedWidth;

        case Cell::Type::Float:
            if constexpr (!std::is_same_v <T, Scada::Atom>) {
                if (index > this->count)
                    return Scada::Cell::Conversion::IndexOutOfTypeBounds;

                switch (this->width) {
                    case 4: return SafeFltExtraction (result, this->ieee754f, index);
                    case 8: return SafeFltExtraction (result, this->ieee754d, index);
                }
                return Scada::Cell::Conversion::UnsupportedWidth;
            } else
                return Conversion::UnsupportedType;

        default:
            return Conversion::UnsupportedType;
    }
}

template <typename T>
Scada::Cell::Conversion Scada::Cell::Information::ToInteger (T * result, std::size_t index) const {
    // this->flags.invalid; // report

    switch (this->type) {
        case Cell::Type::Null:
        case Cell::Type::Address:
        case Cell::Type::Stream:
        case Cell::Type::Expression:
            break;

        case Cell::Type::TimeStamp:
            if (index > this->count)
                return Scada::Cell::Conversion::IndexOutOfTypeBounds;

            return SafeIntExtraction (result, this->timestamp.ull, index);

        case Cell::Type::Atom:
            if (index < this->width) {
                *result = T ();
                return Conversion::Success;
            }
            
            index -= this->width;

            if (index >= this->count)
                return Scada::Cell::Conversion::IndexOutOfTypeBounds;

            if (index < sizeof this->atom / sizeof this->atom [0]) {

                std::uint64_t numeric;
                if (this->atom [index].is_numeric (&numeric)) {
                    return SafeIntConversion (result, numeric);
                } else {
                    char buffer [20];
                    auto length = this->atom [index].format (buffer, sizeof buffer);
                    return SafeStrConversion (result, std::string_view (buffer, length));
                }
            } else
                return Scada::Cell::Conversion::IndexOutOfSupportedBounds;

        case Cell::Type::Float:
        case Cell::Type::Integer:
        case Cell::Type::Unsigned:
            return this->SafeVectorExtract (result, index);

        case Cell::Type::Text:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;

            if (this->flags.external) {
                return Conversion::External;

            } else {
                switch (this->width) {
                    case 0: return SafeStrConversion (result, std::u8string_view ((const char8_t *) this->text.local, this->count)); // UTF-8
                    case 1: return SafeStrConversion (result, std::string_view (this->text.local, this->count)); // ASCII
                    case 2: return SafeStrConversion (result, std::wstring_view ((const wchar_t *) this->text.local, this->count)); // UTF-16
                }
                return Scada::Cell::Conversion::UnsupportedWidth;
            }

        case Cell::Type::Data:
            if (index != 0)
                return Conversion::IndexOutOfTypeBounds;

            if (this->flags.external) {
                return Conversion::External;

            } else {
                if (this->count <= sizeof (this->data.local)) {
                    if (this->count) {
                        uint256_t value;
                        std::memcpy (&value, this->data.local, this->count);
                        return SafeIntExtraction (result, value, index);
                    } else {
                        return SafeIntExtraction (result, 0, index);
                    }
                }
            }
            break;
    }

    // invalid type
    return Scada::Cell::Conversion::UnsupportedType;
}

template <typename T>
Scada::Cell::Conversion Scada::Cell::Information::ToFloat (T * result, std::size_t index) const {
    switch (this->type) {
        case Cell::Type::TimeStamp:
            if (index > this->count)
                return Scada::Cell::Conversion::IndexOutOfTypeBounds;
            if (index > 0)
                return Scada::Cell::Conversion::IndexOutOfSupportedBounds;

            *result = (T) (this->timestamp.ull / 1000'000'0.0);
            return Scada::Cell::Conversion::Success;

        default:
            return this->ToInteger (result, index);
    }
}

inline
Scada::Cell::Conversion
Scada::Cell::Information::ToString (std::string * result, std::size_t index, std::wstring_view fmt) const {
    TRIMCORE::DescriptionFormatting format (fmt);
    return this->ToString (result, index, &format);
}
inline
Scada::Cell::Conversion
Scada::Cell::Information::ToString (std::wstring * result, std::size_t index, std::wstring_view fmt) const {
    TRIMCORE::DescriptionFormatting format (fmt);
    return this->ToString (result, index, &format);
}
inline
Scada::Cell::Conversion
Scada::Cell::Information::ToString (std::u8string * result, std::size_t index, std::wstring_view fmt) const {
    TRIMCORE::DescriptionFormatting format (fmt);
    return this->ToString (result, index, &format);
}


template <typename S> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeBoolConversion (bool * tgt, const S & src, std::size_t index) {
    if (index == 0) {
        *tgt = (src != S (0)); // TODO: NaN?
        return Conversion::Success;
    } else
        return Conversion::IndexOutOfSupportedBounds;
        
}
template <typename S, std::size_t N> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeBoolConversion (bool * tgt, const S (&src) [N], std::size_t index) {
    if (index < N) {
        return SafeBoolConversion (tgt, src [index], 0);
    } else
        return Conversion::IndexOutOfSupportedBounds;
}

template <typename S> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeStringConversion (std::wstring * tgt, const S & src, std::size_t index, TRIMCORE::DescriptionFormatting * fmt) {
    if (index == 0) {
        tgt->assign (TRIMCORE::Describe (src, fmt));
        return Conversion::Success;
    } else
        return Conversion::IndexOutOfSupportedBounds;
}

template <typename S, std::size_t N> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeStringConversion (std::wstring * tgt, const S (&src) [N], std::size_t index, TRIMCORE::DescriptionFormatting * fmt) {
    if (index < N) {
        return SafeStringConversion (tgt, src [index], 0, fmt);
    } else
        return Conversion::IndexOutOfSupportedBounds;
}

template <typename LO, typename HI> inline
Scada::Cell::Conversion
Scada::Cell::Information::SafeStringConversion (std::wstring * tgt, const double_integer <LO, HI> & src, std::size_t index, TRIMCORE::DescriptionFormatting * fmt) {
    if (index == 0) {
        tgt->assign (TRIMCORE::Describe (src, fmt));
        return Conversion::Success;
    } else
        return Conversion::IndexOutOfSupportedBounds;
}

template <typename T>
bool Scada::Cell::get (T * value, std::size_t index, Conversion * error) {
    this->lock.AcquireShared ();
    auto e = this->value.to <T> (value, index);
    this->lock.ReleaseShared ();
    if (error) {
        *error = e;
    }
    return e == Conversion::Success;
}

template <typename C>
bool Scada::Cell::get (std::basic_string <C> * value, std::size_t index, Conversion * error) {
    this->lock.AcquireShared ();
    auto e = this->value.ToString (value, index);
    this->lock.ReleaseShared ();
    if (error) {
        *error = e;
    }
    return e == Conversion::Success;
}

template <typename T>
inline T Scada::Cell::get (std::size_t index) { // throws ConversionException
    T result;
    Conversion error;

    if (this->get (&result, index, &error)) {
        return result;
    } else
        throw ConversionException (error);
}

inline constexpr bool Scada::operator == (const Cell::ID & a, const Cell::ID & b) noexcept {
    return a.segment == b.segment
        && a.index == b.index;
}
inline constexpr bool Scada::operator != (const Cell::ID & a, const Cell::ID & b) noexcept {
    return a.segment != b.segment
        || a.index != b.index;
}
inline constexpr bool Scada::operator < (const Cell::ID & a, const Cell::ID & b) noexcept {
    return a.segment < b.segment
        || (a.segment == b.segment && (a.index < b.index));
}


#endif
