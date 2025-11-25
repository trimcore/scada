#ifndef SCADA_ATOMPATH_HPP
#define SCADA_ATOMPATH_HPP

#include "SCADA_Atom.hpp"

#ifndef SCADA_DLL_IMPORT
#define SCADA_DLL_IMPORT_DEFINED_ATOMPATH
#ifndef SCADA_DLL
#define SCADA_DLL_IMPORT extern
#else
#define SCADA_DLL_IMPORT 
#endif
#endif

namespace Scada::ABI {
    SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY AtomPathParseA (const char * path, std::size_t length, Scada::Atom * atoms, std::size_t max, std::size_t * depth, Atom::InvalidInputReason * error = nullptr) noexcept;
    SCADA_DLL_IMPORT bool TRIMCORE_APIENTRY AtomPathParseW (const wchar_t * path, std::size_t length, Scada::Atom * atoms, std::size_t max, std::size_t * depth, Atom::InvalidInputReason * error = nullptr) noexcept;

    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY AtomPathCapacityGuessA (const char * path, std::size_t length) noexcept;
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY AtomPathCapacityGuessW (const wchar_t * path, std::size_t length) noexcept;
}

namespace Scada {
    class AtomPathView;

    // AtomPathParse
    //  - attempts to parse '/'-separated list of strings into vector of Atoms
    
    inline bool AtomPathParse (std::string_view path, Scada::Atom * atoms, std::size_t max, std::size_t * depth, Atom::InvalidInputReason * error = nullptr) noexcept {
        return ABI::AtomPathParseA (path.data (), path.size (), atoms, max, depth, error);
    }
    inline bool AtomPathParse (std::wstring_view path, Scada::Atom * atoms, std::size_t max, std::size_t * depth, Atom::InvalidInputReason * error = nullptr) noexcept {
        return ABI::AtomPathParseW (path.data (), path.size (), atoms, max, depth, error);
    }

    // AtomPathCapacityGuess
    //  - fast heuristics on how long the path is
    //  - is never smaller than actual size determined by AtomPathParse
    
    inline std::size_t AtomPathCapacityGuess (std::string_view path) noexcept { return ABI::AtomPathCapacityGuessA (path.data (), path.size ()); };
    inline std::size_t AtomPathCapacityGuess (std::wstring_view path) noexcept { return ABI::AtomPathCapacityGuessW (path.data (), path.size ()); };

    // AtomPathBase
    //  - base type for AtomPath allowing usage of custom container
    //  - THE CONTAINER MUST BE CONTINUOUS!
    //
    template <template <class, class> class ContainerTemplate, typename Allocator = std::allocator <Atom>>
    class AtomPathBase : public ContainerTemplate <Atom, Allocator> {
        using Container = ContainerTemplate <Atom, Allocator>;

    public:
        // initialize empty
        inline constexpr AtomPathBase () = default;

        // initialize from string
        inline AtomPathBase (const char * s) { this->initialize_from_string (s); }
        inline AtomPathBase (const wchar_t * s) { this->initialize_from_string (s); }
        inline AtomPathBase (std::string_view s) { this->initialize_from_string (s); }
        inline AtomPathBase (std::wstring_view s) { this->initialize_from_string (s); }

        // parse
        inline bool parse (const char * s, Atom::InvalidInputReason * r = nullptr) { return this->parse_from_string (s, r); }
        inline bool parse (const wchar_t * s, Atom::InvalidInputReason * r = nullptr) { return this->parse_from_string (s, r); }
        inline bool parse (std::string_view s, Atom::InvalidInputReason * r = nullptr) { return this->parse_from_string (s, r); }
        inline bool parse (std::wstring_view s, Atom::InvalidInputReason * r = nullptr) { return this->parse_from_string (s, r); }

        // initialize from single Atom
        inline AtomPathBase (Atom atom, std::size_t extra_capacity = 0) {
            this->reserve (extra_capacity + 1);
            this->push_back (atom);
        }
        template <template <class, class> class ContainerTemplate2, typename Allocator2 = std::allocator <Atom>>
        inline AtomPathBase (const AtomPathBase <ContainerTemplate2, Allocator2> & path) {
            this->insert (this->end (), path.begin (), path.end ());
        }

        // concatenation

        AtomPathBase & operator += (Atom a) {
            this->push_back (a);
            return *this;
        }
        template <template <class, class> class ContainerTemplate2, typename Allocator2 = std::allocator <Atom>>
        AtomPathBase & operator += (const AtomPathBase <ContainerTemplate2, Allocator2> & path) {
            this->insert (this->end (), path.begin (), path.end ());
            return *this;
        }

        // view

        inline AtomPathBase (const AtomPathView &);
        inline AtomPathBase & operator += (const AtomPathView &);

        // copyable
        //  - must be both, because of template AtomPath (T && string) above

        AtomPathBase (AtomPathBase &) = default;
        AtomPathBase (const AtomPathBase &) = default;

        // movable

        AtomPathBase (AtomPathBase && other) noexcept = default;
        AtomPathBase & operator = (AtomPathBase && other) noexcept = default;

    private:
        template <typename T>
        inline void initialize_from_string (T && string/*, std::size_t extra_capacity = 0*/) {
            Atom::InvalidInputReason error = Atom::InvalidInputReason::None;
            if (!this->parse (string, &error))
                throw Atom::InvalidInputException (error, string);
        }

        template <typename T>
        inline bool parse_from_string (T && string, Atom::InvalidInputReason * result = nullptr) {
            this->resize (Scada::AtomPathCapacityGuess (string));
        
            // auto guess = Scada::AtomPathCapacityGuess (string);
            // this->reserve (guess + extra_capacity);
            // this->resize (guess);

            std::size_t actual_size = 0;
            if (Scada::AtomPathParse (string, this->data (), this->size (), &actual_size, result)) {
                this->resize (actual_size);
                return true;
            } else
                return false;
        }
    };

    using AtomPath          = AtomPathBase <std::vector>;
    using AtomPathTemporary = AtomPathBase <std::vector, ::TRIMCORE::Allocator64kB <Atom>>;


    // AtomPathView
    //  - non-owning view into span of Atoms, that also can be initialized from a string
    //
    class AtomPathView {
        const Atom * view_data = nullptr;
        std::size_t view_allocated : 1 = 0;
        std::size_t view_size : (8 * sizeof (std::size_t) - 1) = 0;

    public:
        bool empty () const { return this->view_size == 0; }
        const Atom * data () const noexcept { return this->view_data; }
        std::size_t  size () const noexcept  { return this->view_size; }

        const Atom * begin () const noexcept { return this->data (); }
        const Atom * end ()   const noexcept  { return this->data () + this->size (); }

        const Atom & operator [] (std::size_t i) const noexcept  { return this->data () [i]; }

        Atom front () const noexcept  { return this->view_data [0]; }
        Atom back () const noexcept { return this->view_data [this->size () - 1]; }
        
        AtomPathView sub (std::size_t offset, std::size_t n = ~0) const {
            if (offset < this->size ()) {
                return AtomPathView (this->data () + offset, std::min (n, this->size () - offset));
            } else
                return AtomPathView ();
        }

    public:
        // initialize empty
        inline constexpr AtomPathView () {}

        // initialize from string
        inline AtomPathView (const char * s) { this->initialize_from_string (s); }
        inline AtomPathView (const wchar_t * s) { this->initialize_from_string (s); }
        inline AtomPathView (std::string_view s) { this->initialize_from_string (s); }
        inline AtomPathView (std::wstring_view s) { this->initialize_from_string (s); }

        inline AtomPathView (const AtomPath & path)
            : view_data (&path [0])
            , view_size (path.size ())
            , view_allocated (false) {}

        template <template <class, class> class ContainerTemplate2, typename Allocator2 = std::allocator <Atom>>
        inline AtomPathView (const AtomPathBase <ContainerTemplate2, Allocator2> & path)
            : view_data (&path [0])
            , view_size (path.size ())
            , view_allocated (false) {}

        inline constexpr AtomPathView (const Atom * path, std::size_t depth)
            : view_data (path)
            , view_size (path ? depth : 0)
            , view_allocated (false) {
        }
        inline constexpr AtomPathView (const Atom * begin, const Atom * end)
            : view_data (begin)
            , view_size (end - begin)
            , view_allocated (false) {
        }

        inline ~AtomPathView () {
            if (this->view_allocated) {
                TRIMCORE::Free64kB (const_cast <Atom *> (this->view_data));
            }
        }

        // TODO: make following common for all AtomPathXXXX

        inline std::size_t formatted_length () const {
            if (auto length = this->size ()) {
                for (auto a : *this) {
                    if (a == Atom ()) {
                        length += 2; // ".."
                    } else {
                        length += a.formatted_length ();
                    }
                }
                return length - 1;
            } else
                return 0;
        }
        inline std::string formatted () const {
            using namespace std::literals;
            
            std::string s;
            s.reserve (this->formatted_length () + 1);

            for (const auto & a : *this) {
                if (a == Scada::Atom {}) {
                    s += "/.."sv;
                } else {
                    s += "/"sv;
                    s += a.format ();
                }
            }
            return s;
        }

    private:
        template <typename T>
        inline void initialize_from_string (T && string) {
            if (this->view_data = static_cast <Atom *> (TRIMCORE::Alloc64kB ())) {
                this->view_allocated = true;
                
                std::size_t actual_size = 0;
                auto error = Atom::InvalidInputReason::None;

                if (Scada::AtomPathParse (string, (Atom *) this->view_data, 65536 / sizeof (Atom), &actual_size, &error)) {
                    this->view_size = actual_size;
                } else {
                    TRIMCORE::Free64kB (const_cast <Atom *> (this->view_data));
                    throw Atom::InvalidInputException (error, string);
                }
            } else
                throw TRIMCORE::BadAlloc64kB ();
        }
    };

    // conversion from AtomPathView to any kind of AtomPath

    template <template <class, class> class ContainerTemplate, typename Allocator>
    inline AtomPathBase <ContainerTemplate, Allocator> ::AtomPathBase (const AtomPathView & view) {
        this->insert (this->end (), view.begin (), view.end ());
    }

    template <template <class, class> class ContainerTemplate, typename Allocator>
    inline AtomPathBase <ContainerTemplate, Allocator> & AtomPathBase <ContainerTemplate, Allocator> ::operator += (const AtomPathView & view) {
        this->insert (this->end (), view.begin (), view.end ());
        return *this;
    }

    // operator +
    //  - concatenates two AtomPaths (one can even be a string)

    inline AtomPathTemporary operator + (const AtomPathView & a, const AtomPathView & b) {
        AtomPathTemporary p = a;
        p += b;
        return p;
    }
    inline AtomPathTemporary operator + (const AtomPathView & a, Atom b) {
        AtomPathTemporary p = a;
        p += b;
        return p;
    }
    inline AtomPathTemporary operator + (Atom a, const AtomPathView & b) {
        AtomPathTemporary p = a;
        p += b;
        return p;
    }
}

// enable TRIMCORE::Log to log Atoms

#ifdef TRIMCORE
namespace TRIMCORE {
    inline Serialized Serialize (const Scada::AtomPath & path, Temporary64kB <std::uint8_t> &) {
        return { 'Apth', path.data (), path.size () * sizeof (Scada::Atom) };
    }
    inline Serialized Serialize (const Scada::AtomPathView & path, Temporary64kB <std::uint8_t> &) {
        return { 'Apth', path.data (), path.size () * sizeof (Scada::Atom) };
    }
    inline Serialized Serialize (const Scada::AtomPathTemporary & path, Temporary64kB <std::uint8_t> &) {
        return { 'Apth', path.data (), path.size () * sizeof (Scada::Atom) };
    }

    inline std::wstring Describe (const Scada::AtomPathView & path, DescriptionFormatting * fmt = nullptr) {
        if (!path.empty ()) {
            return Describe (path.formatted (), fmt);
        } else {
            return Describe (L'/', fmt);
        }
    }

    template <template <class> class ContainerTemplate>
    inline std::wstring Describe (const ContainerTemplate <Scada::Atom> & path, DescriptionFormatting * fmt = nullptr) {
        return Describe (Scada::AtomPathView (path.begin (), path.end ()), fmt);
    }
    inline std::wstring Describe (const Scada::AtomPath & path, DescriptionFormatting * fmt = nullptr) {
        return Describe (Scada::AtomPathView (path), fmt);
    }
    inline std::wstring Describe (const Scada::AtomPathTemporary & path, DescriptionFormatting * fmt = nullptr) {
        return Describe (Scada::AtomPathView (path), fmt);
    }
}
#endif

#ifdef SCADA_DLL_IMPORT_DEFINED_ATOMPATH
#undef SCADA_DLL_IMPORT_DEFINED_ATOMPATH
#undef SCADA_DLL_IMPORT
#endif

#endif
