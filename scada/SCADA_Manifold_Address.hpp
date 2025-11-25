#ifndef SCADA_MANIFOLD_ADDRESS_HPP
#define SCADA_MANIFOLD_ADDRESS_HPP

#include "SCADA_Manifold.hpp"

#include <ws2tcpip.h>
#include <hvsocket.h>
#include <af_irda.h>
#include <afunix.h>
#include <ws2bth.h>

#include <compare>

#ifndef SCADA_DLL_IMPORT
#define SCADA_DLL_IMPORT_DEFINED_MANIFOLD_ADDRESS
#ifndef SCADA_DLL
#define SCADA_DLL_IMPORT extern
#else
#define SCADA_DLL_IMPORT 
#endif
#endif

namespace Scada::ABI {
    SCADA_DLL_IMPORT DWORD TRIMCORE_APIENTRY ScadaAddrInitA (Manifold::Address *, const char * string, std::size_t length, short family, const char * service, std::size_t svclen) noexcept;
    SCADA_DLL_IMPORT DWORD TRIMCORE_APIENTRY ScadaAddrInitW (Manifold::Address *, const wchar_t * string, std::size_t length, short family, const char * service, std::size_t svclen) noexcept;
    SCADA_DLL_IMPORT DWORD TRIMCORE_APIENTRY ScadaAddrInitType (Manifold::Address *, int, short family, const char * service, std::size_t svclen) noexcept;
    SCADA_DLL_IMPORT int TRIMCORE_APIENTRY ScadaAddrCmp (const Manifold::Address *, const Manifold::Address *, int) noexcept;
}

namespace Scada {

    // Address
    //  - socket address abstraction for all types supported by Scada Manifold facility
    //  - standard layout structure, can be casted to sockaddr*
    //
    class Manifold::Address {
    public:
        union {
            short           family;
            SOCKADDR_HV     hv;   // AF_HYPERV
            SOCKADDR_BTH    bth;  // AF_BTH
            SOCKADDR_IRDA   irda; // AF_IRDA
            SOCKADDR_UN     unix; // AF_UNIX
            SOCKADDR_IN     ipv4; // AF_INET
            SOCKADDR_IN6    ipv6; // AF_INET6
            char            storage [112];
        };

        friend class Manifold::Socket;

    public:
        enum class Special {
            Listener, // address to bind for listening on all interfaces
            Loopback,
            Broadcast,
        };

        // Initialize
        //  - initializes 'address' to 'Special' address type
        //  - if 'address' is nullptr, then function only validates parameters
        //  - requirements on 'service' for supported 'family' are:
        //     - AF_HYPERV: GUID in Microsoft format
        //     - AF_BTH: ignored
        //     - AF_IRDA: service name, e.g.: "irDA:irCOMM"
        //     - AF_UNIX: unix pipe file name
        //     - AF_INET(6): number 1...65535 (hexadecimal if prefixed with 0x or #x)
        //  - returns:
        //     - ERROR_SUCCESS - on success (if the combination is supported and parameters are valid)
        //     - ERROR_INVALID_ADDRESS - the input string was unrecognized or doesn't match specified family
        //     - ERROR_INVALID_PARAMETER - issue with 'service' parameter, usually port number out of range
        //     - ERROR_INVALID_SERVICENAME - failed fallback or use of 'service' if the input string was missing
        //     - ERROR_FILENAME_EXCED_RANGE - 'input' or 'service' provided is too long (unix/irda path too long)
        //     - ERROR_NOT_SUPPORTED - 'family' or combination of 'type' and 'family' is not supported (makes no sense)
        //     - ERROR_OUTOFMEMORY - 
        //     - ERROR_UNHANDLED_EXCEPTION - internal error happened
        //
        static inline DWORD Initialize (Address * address, Special type, short family, std::string_view service) noexcept {
            return Scada::ABI::ScadaAddrInitType (address, (int) type, family, service.data (), service.size ());
        }
        
        // Initialize
        //  - returns 'Special' address type of given 'family' and 'service'
        //  - throws std::invalid_argument on any error
        //
        static inline Address Initialize (Special type, short family, std::string_view service);

        // Validate
        //  - verifies if special address, given the 'family' and 'parameter', is supported and can be constructed
        //
        static inline bool Validate (Special type, short family, std::string_view service, DWORD * error = nullptr) noexcept {
            auto r = Scada::ABI::ScadaAddrInitType (nullptr, (int) type, family, service.data (), service.size ());
            if (error) {
                *error = r;
            }
            return r == ERROR_SUCCESS;
        }

        // Any/Loopback/Broadcast
        //  - helper functions
        //  - throws std::invalid_argument

        static inline Address Any (short family, std::string_view parameter) { return Initialize (Special::Listener, family, parameter); }
        static inline Address Loopback (short family, std::string_view parameter) { return Initialize (Special::Loopback, family, parameter); }
        static inline Address Broadcast (short family, std::string_view parameter) { return Initialize (Special::Broadcast, family, parameter); }

    public:
        Address (const Address &) = default;
        Address & operator = (const Address &) = default;

    public:
        inline Address () noexcept { std::memset (&this->storage, 0, sizeof this->storage); }

        Address (const SOCKET_ADDRESS & address) {
            if (address.iSockaddrLength >= 2 && address.iSockaddrLength <= sizeof this->storage) {
                std::memcpy (&this->storage, address.lpSockaddr, address.iSockaddrLength);
            } else
                throw std::length_error ("Unsupported/Invalid length for Scada::Manifold::Address");
        }

        Address (const struct sockaddr * addr, std::size_t len) {
            if (len >= 2 && len <= sizeof this->storage) {
                std::memcpy (&this->storage, addr, len);
            } else
                throw std::length_error ("Unsupported/Invalid length for Scada::Manifold::Address");
        }

        inline Address (const SOCKADDR_HV & address) noexcept { this->hv = address; }
        inline Address (const SOCKADDR_BTH & address) noexcept { this->bth = address; }
        inline Address (const SOCKADDR_IRDA & address) noexcept { this->irda = address; }
        inline Address (const SOCKADDR_UN & address) noexcept { this->unix = address; }
        inline Address (const SOCKADDR_IN & address) noexcept { this->ipv4 = address; }
        inline Address (const SOCKADDR_IN6 & address) noexcept { this->ipv6 = address; }

        // Address
        //  - parses the string and constructs sockaddr compatible structure
        //  - 'service' is default parameter (port for IP, service GUID for Hyper-V)
        //  - family specifies what address type the string defines
        //     - if specified, relaxed 
        //     - if AF_UNSPEC (0) or not used, then family is detected from string
        //  - if default port is "0" then 0 as port is allowed in address and won't throw
        //  - supported formats:
        //     - IPv4: 123.45.67.89:12345, default service is port number used if omitted from string
        //     - IPv6: [ff01::1234]:12345, default service is port number used if omitted from string
        //     - Hyper-V: vm[bus]:{guid}[:{guid}], first GUID is partition, second is service GUID
        //        - partition can be:
        //          "parent", "broadcast", "this"/"localhost" (loopback), "silo"/"silohost", "children"
        //        - default service is service GUID used if omitted from string
        //        - braces { } are optional
        //     - Unix (WSL): unix:/unix/path/here
        //        - conversion from 'wchar_t' must fit to 108 UTF-8 bytes, 'char' version copies directly
        //     - ?? TODO IrDA: 4 chars device, 25 chars service
        //  - throws
        //     - Manifold::Address::Exception - for failed conversion (inherits from std::invalid_argument)
        //     - std::bad_alloc - if internals failed to allocate temporary conversion buffers
        
        explicit Address (std::string_view string, short family, std::string_view service = std::string_view ()) {
            std::memset (&this->storage, 0, sizeof this->storage);
            HandleInitResult (Scada::ABI::ScadaAddrInitA (this, string.data (), string.size (),
                                                          family, service.data (), service.size ()),
                              family, service);
        }
        explicit Address (std::wstring_view string, short family, std::string_view service = std::string_view ()) {
            std::memset (&this->storage, 0, sizeof this->storage);
            HandleInitResult (Scada::ABI::ScadaAddrInitW (this, string.data (), string.size (),
                                                          family, service.data (), service.size ()),
                              family, service);
        }

        explicit Address (std::wstring_view string, std::string_view service = std::string_view ()) : Address (string, AF_UNSPEC, service) {};
        explicit Address (std::string_view string,  std::string_view service = std::string_view ()) : Address (string, AF_UNSPEC, service) {};

        // Comparison
        //  - 
        //
        enum class Comparison {
            Binary,     // memcmp
            Full,       // include all usable members
            Standard,   // standard comparison
            NoService,  // ignore service/port, address only
        };

        // compare
        //  - base comparison function, lexicographic
        //  - returns -1,0,+1 like memcmp/strcmp, 0 means equal
        //
        inline int compare (const Address & other, Comparison cmp = Comparison::Standard) const noexcept {
            return Scada::ABI::ScadaAddrCmp (this, &other, (int) cmp);
        }
        
        // comparison operators

        inline bool operator == (const Address & other) const { return this->compare (other, Comparison::Standard) == 0; }
        inline bool operator != (const Address & other) const { return this->compare (other, Comparison::Standard) != 0; };
        inline bool operator  < (const Address & other) const { return this->compare (other, Comparison::Standard)  < 0; }
        inline bool operator <= (const Address & other) const { return this->compare (other, Comparison::Standard) <= 0; };
        inline bool operator  > (const Address & other) const { return this->compare (other, Comparison::Standard)  > 0; }
        inline bool operator >= (const Address & other) const { return this->compare (other, Comparison::Standard) >= 0; };
        
    public:
        class Exception
            : public std::invalid_argument {

        public:
            DWORD code;
            short family;

            std::string service;

        public:
            Exception (DWORD code, short family, std::string_view service)
                : std::invalid_argument (
                    [&] {
                        switch (code) {
                            case ERROR_NOT_SUPPORTED:
                                return std::invalid_argument ("unsupported family " + std::to_string ((unsigned short) family));
                            case ERROR_INVALID_SERVICENAME:
                                return std::invalid_argument ("unrecognized service: " + std::string (service));
                            case ERROR_INVALID_PARAMETER:
                                return std::invalid_argument ("port number " + std::string (service) + " out of range");

                            default:
                            case ERROR_INVALID_ADDRESS:
                            case ERROR_FILENAME_EXCED_RANGE:
                                return std::invalid_argument ("invalid format");
                        }
                    } ()
                )
                , code (code)
                , family (family)
                , service (service) {};

            inline Exception (const Exception &) = default;
            inline Exception (Exception && from) noexcept = default;
            inline Exception & operator = (Exception && from) noexcept = default;
        };

    private:
        static inline void HandleInitResult (DWORD error, short family, std::string_view service) {
            switch (error) {
                case ERROR_SUCCESS:
                    break;

                case ERROR_OUTOFMEMORY:
                    throw std::bad_alloc ();
                case ERROR_UNHANDLED_EXCEPTION:
                    throw std::exception ();

                default:
                    throw Exception (error, family, service);
            }
        }
    };
}

inline Scada::Manifold::Address Scada::Manifold::Address::Initialize (Special type, short family, std::string_view service) {
    Address address;
    Address::HandleInitResult (Address::Initialize (&address, type, family, service), family, service);
    return address;
}

// enable TRIMCORE::Log to log Manifold::Address

namespace TRIMCORE {
    inline std::wstring Describe (const Scada::Manifold::Address & address, DescriptionFormatting * format = nullptr) {
        return TRIMCORE::Describe ((const sockaddr *) &address, format);
    }
    inline Serialized Serialize (const Scada::Manifold::Address & addr, Temporary64kB <std::uint8_t> & scratch) {
        switch (addr.family) {
            case AF_INET: return Serialize (addr.ipv4, scratch);
            case AF_INET6: return Serialize (addr.ipv6, scratch);
            case AF_HYPERV: return Serialize (addr.hv, scratch);
            case AF_UNIX: return Serialize (addr.unix, scratch);
            //case AF_BTH: return Serialize (addr.bth, scratch);
            //case AF_IRDA: return Serialize (addr.irda, scratch);
            default:
                return SerializeTrivially ('addr', addr);
        }
    }
}

#ifdef SCADA_DLL_IMPORT_DEFINED_MANIFOLD_ADDRESS
#undef SCADA_DLL_IMPORT_DEFINED_MANIFOLD_ADDRESS
#undef SCADA_DLL_IMPORT
#endif

#endif

