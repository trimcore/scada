#ifndef SCADA_OBJECT_HPP
#define SCADA_OBJECT_HPP

#include <compare>
#include <string>
#include "SCADA_AtomPath.hpp"

namespace Scada {

    // ObjectSpecializationBase
    //  - internal inteface for access from specializations to base class
    //
    class ObjectSpecializationBase/* : virtual TRIMCORE::Log::Provider*/ {
    protected:
        inline void throw_on_failure () const;
        virtual Cell::Handle GetHandle () const noexcept = 0;
    };

    template <typename T>
    class ObjectSpecialization;

    // AccessLevel
    //  - 
    //
    using AccessLevel = ABI::Api1AccessLevel;

    // Object
    //  - 
    //
    template <typename T>
    class Object final
        : virtual ObjectSpecializationBase
        , public ObjectSpecialization <T> {

        Cell::Handle handle;

    public:
        explicit Object (Cell::ID id, AccessLevel level);
        explicit Object (const AtomPath & path, AccessLevel level);

        Object (const Object & other);
        Object & operator = (const Object & other);
        ~Object ();

    public:
        Cell::ID identify () const noexcept;
        
        void signal () const noexcept {
            if (!ABI::Api1CellSignal (this->handle)) {
                this->throw_on_failure ();
            }
        }

        using ObjectSpecialization <T>::operator =;

        // TODO: implement also as overriding; user inherits from Object<bool>, overrides 'OnChange' and calls 'listen'
        bool listen (void (*callback) (void * context, Cell::Handle handle, FILETIME t, Cell::Information value), void * context) const noexcept {
            return ABI::Api1CellListen (this->handle, callback, context);
        }
        bool valid () const noexcept {
            auto type = ABI::Api1CellGetDataType (this->handle);
            return !type.flags.invalid;
        }

    private:
        void validate_access ();
        void validate_access (const AtomPath &);
        virtual Cell::Handle GetHandle () const noexcept override; // ObjectSpecializationBase
    };


    // ObjectSpecialization
    //  - 

    template <>
    class ObjectSpecialization <bool> : virtual ObjectSpecializationBase {
        //virtual bool ???

    protected:
        ObjectSpecialization & operator = (bool value) {
            if (!ABI::Api1CellSetBoolean (this->GetHandle (), value)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator bool () const {
            bool value = false;
            if (!ABI::Api1CellGetBoolean (this->GetHandle (), &value)) {
                this->throw_on_failure ();
            }
            return value;
        }
    };

    template <>
    class ObjectSpecialization <std::uint64_t> : virtual ObjectSpecializationBase {
    protected:
        ObjectSpecialization & operator = (std::uint64_t value) {
            if (!ABI::Api1CellSetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator std::uint64_t () const {
            std::uint64_t value = 0;
            if (!ABI::Api1CellGetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return value;
        }
    };
    template <>
    class ObjectSpecialization <std::uint32_t> : virtual ObjectSpecializationBase {
    protected:
        ObjectSpecialization & operator = (std::uint64_t value) {
            value &= 0xFFFF'FFFF;
            if (!ABI::Api1CellSetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator std::uint32_t () const {
            std::uint64_t value = 0;
            if (!ABI::Api1CellGetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return (std::uint32_t) value;
        }
    };
    template <>
    class ObjectSpecialization <std::uint16_t> : virtual ObjectSpecializationBase {
    protected:
        ObjectSpecialization & operator = (std::uint64_t value) {
            value &= 0xFFFF;
            if (!ABI::Api1CellSetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator std::uint16_t () const {
            std::uint64_t value = 0;
            if (!ABI::Api1CellGetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return (std::uint16_t) value;
        }
    };
    template <>
    class ObjectSpecialization <std::uint8_t> : virtual ObjectSpecializationBase {
    protected:
        ObjectSpecialization & operator = (std::uint64_t value) {
            value &= 0xFF;
            if (!ABI::Api1CellSetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator std::uint8_t () const {
            std::uint64_t value = 0;
            if (!ABI::Api1CellGetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return (std::uint8_t) value;
        }
    };
    // float

    template <>
    class ObjectSpecialization <float> : virtual ObjectSpecializationBase {
    protected:
        ObjectSpecialization & operator = (float value) {
            if (!ABI::Api1CellSetFloat32 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator float () const {
            float value = 0.0;
            if (!ABI::Api1CellGetFloat32 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return value;
        }
    };

    // double

    template <>
    class ObjectSpecialization <double> : virtual ObjectSpecializationBase {
    protected:
        ObjectSpecialization & operator = (double value) {
            if (!ABI::Api1CellSetFloat64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator double () const {
            double value = 0.0;
            if (!ABI::Api1CellGetFloat64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return value;
        }
    };

    // string

    template <>
    class ObjectSpecialization <std::string> : virtual ObjectSpecializationBase {
    protected:
        ObjectSpecialization & operator = (std::string_view sv) {
            if (!ABI::Api1CellSetStringAsv (this->GetHandle (), sv.data (), sv.size ())) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator std::string () const {
            char buffer [32];
            auto length = ABI::Api1CellGetStringA (this->GetHandle (), buffer, sizeof buffer);
            if (length == 0) {
                this->throw_on_failure ();
            }
            return std::string (&buffer [0], &buffer [length]);
        }
    };

    // AtomPath

    template <>
    class ObjectSpecialization <AtomPath> : virtual ObjectSpecializationBase {
    protected:
        /*ObjectSpecialization & operator = (const AtomPath &) {
            // TBD
            return *this;
        }*/
    public:
        AtomPath get () const {
            AtomPath path;
            Atom::InvalidInputReason reason;
            path.depth = ABI::Api1CellConvertToAtoms (this->GetHandle (), path.data (), 65536 / sizeof (Atom), &reason);
            path.valid = reason == Atom::InvalidInputReason::None;
            return path;
        }

        operator AtomPath () const {
            AtomPath path;
            Atom::InvalidInputReason reason;
            path.depth = ABI::Api1CellConvertToAtoms (this->GetHandle (), path.data (), 65536 / sizeof (Atom), &reason);
            path.valid = reason == Atom::InvalidInputReason::None;
            return path;
        }
    };

    // Manifold::Address

    template <>
    class ObjectSpecialization <Manifold::Address> : virtual ObjectSpecializationBase {
    protected:
        ObjectSpecialization & operator = (const Manifold::Address & address) {
            if (!ABI::Api1CellSetAddress (this->GetHandle (), &address)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator Manifold::Address () {
            Manifold::Address address;
            if (!ABI::Api1CellGetAddress (this->GetHandle (), &address)) {
                this->throw_on_failure ();
            }
            return address;
        }

        // update
        //  - listeners on Cell of this Object are notified only if 'address' is different from previous one
        //    (the difference is that for assignement (Set), the listeners are notified always)
        //
        bool update (const Manifold::Address & address) {
            if (!ABI::Api1CellUpdateAddress (this->GetHandle (), &address)) {
                this->throw_on_failure ();
                return false;
            } else
                return true;
        }
    };

    // FILETIME

    template <>
    class ObjectSpecialization <FILETIME>: virtual ObjectSpecializationBase {
    protected:
        ObjectSpecialization & operator = (const FILETIME & t) {
            if (!ABI::Api1CellSetTimeStamp (this->GetHandle (), &t)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:
        operator FILETIME () {
            FILETIME ft;
            if (!ABI::Api1CellGetTimeStamp (this->GetHandle (), &ft)) {
                this->throw_on_failure ();
            }
            return ft;
        }
    };
}

// enable TRIMCORE::Log to log Objects

/*#ifdef TRIMCORE
namespace TRIMCORE {
    //template <typename T>
    //inline std::size_t DescriptionLengthEst (Scada::Object <T>) { return DescriptionLengthEst (...); }

    template <typename T>
    inline std::wstring Describe (const Scada::Object <T> & v, DescriptionFormatting * format = nullptr) {
        return Describe ((T) v, format);
    }
}
#endif // */

#include "SCADA_Object.tcc"
#endif
