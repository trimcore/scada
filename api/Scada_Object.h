#ifndef SCADA_OBJECT_HPP
#define SCADA_OBJECT_HPP

#include <compare>
#include <string>
#include "SCADA_AtomPath.hpp"

namespace Scada {

    //template <typename T>
    //class ObjectSpecialization;

    // Object
    //  - 
    //  - TODO: T[N] and std::vector<T> overloads
    //
    template <typename T>
    class Object /*final : public ObjectSpecialization <T>*/ {
        Cell::Handle handle;

    public:
        explicit Object (Cell::ID id);
        explicit Object (const AtomPathView & path);
        explicit Object (const AtomPathView & path, Atom name);
        explicit Object (const AtomPathView & path, std::string_view name);
        explicit Object (const AtomPathView & path, std::wstring_view name);
        ~Object ();

        Object (const Object & other) = delete;
        Object & operator = (const Object & other) = delete;

    public:
        Cell::ID identify () const noexcept;
        
        void signal () const {
            if (!ABI::ScadaSignal (this->handle)) {
                this->throw_on_failure ();
            }
        }

        // using ObjectSpecialization <T>::operator =;

        // TODO: implement also as overriding; user inherits from Object<bool>, overrides 'OnChange' and calls 'listen'

        bool listen (ABI::ListenCallback callback, void * context) const noexcept {
            return ABI::ScadaListen (this->handle, callback, context);
        }
        bool valid () const noexcept {
            Cell::Specs specs;
            if (ABI::ScadaGetCellSpecs (this->handle, &specs)) {
                return !specs.flags.invalid;
            } else
                return false;
        }

        // get
        //  -
        //
        bool get (T * result, std::size_t index = 0, Cell::Conversion * error = nullptr) const noexcept {
            Cell::Information information;
            if (ABI::ScadaGetValue (this->handle, &information)) {
                auto e = information.to (result, index);
                if (error) {
                    *error = e;
                }
                return e == Cell::Conversion::Success;
            } else {
                if (error) {
                    *error = Cell::Conversion::NotConversionError;
                }
                return false;
            }
        }

        bool set (const T & value) noexcept { return this->set_implementation (value, false); }
        bool update (const T & value) noexcept { return this->set_implementation (value, true); }

        operator T () const {
            T r;
            Cell::Conversion e;
            if (this->get (&r, 0, &e)) {
                return r;
            } else
                throw TRIMCORE::Log::Exception (nullptr, "Scada::Object read failure {1}, error {ERR}", e);
        }

        Object & operator = (const T & value) {
            if (!this->set (value))
                throw TRIMCORE::Log::Exception (nullptr, "Scada::Object set failure, error {ERR}");

            return *this;
        }

    private:
        void validate_access ();
        void validate_access (const AtomPathView &);
        void validate_access (const AtomPathView &, Atom);

        inline void throw_on_failure () const {
            switch (GetLastError ()) {
                case ERROR_SUCCESS:
                    return;
                default:
                    throw TRIMCORE::Log::Exception (nullptr, "Scada::Object error {ERR}");
            }
        }

        bool set_implementation (const T & value, bool update) noexcept {
            Cell::Information information;
            if (ABI::ScadaGetCellSpecs (this->handle, &information)) {
                information.initialize (value);
                return ABI::ScadaSetValue (this->handle, &information, update ? ABI::UpdateOnChangeOnly : ABI::DefaultSetMode);
            } else
                return false;
        }
    };

    /*
    // ObjectSpecialization
    //  - 

    template <>
    class ObjectSpecialization <bool> {
    protected:
        void assign_to_information (Cell::Information & information, bool value) noexcept {
            information.flags = Cell::Flags {};
            information.type = Cell::Type::Unsigned;
            information.count = 0;
            information.width = 0;
            information.uint1 = value;
        }
    };

    template <>
    class ObjectSpecialization <std::uint64_t> {
    protected:
        ObjectSpecialization & operator = (std::uint64_t value) {
            if (!ABI::Api1CellSetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    };
    template <>
    class ObjectSpecialization <std::uint32_t> {
    protected:
        ObjectSpecialization & operator = (std::uint64_t value) {
            value &= 0xFFFF'FFFF;
            if (!ABI::Api1CellSetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    };
    template <>
    class ObjectSpecialization <std::uint16_t> {
    protected:
        ObjectSpecialization & operator = (std::uint64_t value) {
            value &= 0xFFFF;
            if (!ABI::Api1CellSetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    };
    template <>
    class ObjectSpecialization <std::uint8_t> {
    protected:
        ObjectSpecialization & operator = (std::uint64_t value) {
            value &= 0xFF;
            if (!ABI::Api1CellSetUnsigned64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    };
    // float

    template <>
    class ObjectSpecialization <float> {
    protected:
        ObjectSpecialization & operator = (float value) {
            if (!ABI::Api1CellSetFloat32 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    };

    // double

    template <>
    class ObjectSpecialization <double> {
    protected:
        ObjectSpecialization & operator = (double value) {
            if (!ABI::Api1CellSetFloat64 (this->GetHandle (), &value, 1u)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    };

    // string

    template <>
    class ObjectSpecialization <std::string> {
    protected:
        ObjectSpecialization & operator = (std::string_view sv) {
            if (!ABI::Api1CellSetStringAsv (this->GetHandle (), sv.data (), sv.size ())) {
                this->throw_on_failure ();
            }
            return *this;
        }
    };

    // AtomPath

    template <>
    class ObjectSpecialization <AtomPath> {
    protected:
        ObjectSpecialization & operator = (const AtomPath &) {
            // TBD
            return *this;
        }
    };

    // Manifold::Address

    template <>
    class ObjectSpecialization <Manifold::Address> {
    protected:
        ObjectSpecialization & operator = (const Manifold::Address & address) {
            if (!ABI::Api1CellSetAddress (this->GetHandle (), &address)) {
                this->throw_on_failure ();
            }
            return *this;
        }
    public:

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
    };*/
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
