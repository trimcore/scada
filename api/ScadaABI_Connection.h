#ifndef SCADA_SCADAABI_CONNECTION_H
#define SCADA_SCADAABI_CONNECTION_H

// Manifold ABI, version 1, Uplink API
//  - 

namespace Scada::ABI {

    // ConnectionInformation
    //  - 
    //
    struct ConnectionInformation {
        std::size_t cb; // number of bytes
        DWORD       session; // Session ID
        DWORD       process; // SCADA process PID
    };
    
    // ConnectionCallbacks
    //  - 
    //
    struct ConnectionCallbacks {
        std::size_t cb = 0; // set to: sizeof (ConnectionCallbacks)
        void *      context = nullptr;

        void (CALLBACK * onScadaConnect) (void * context, const ConnectionInformation *) = nullptr;
        void (CALLBACK * onScadaDisconnect) (void * context) = nullptr;
        void (CALLBACK * onScadaConnectFailure) (void * context) = nullptr; // call GetLastError
    };

    // ConnectParameters
    //  - Scada::ABI::ScadaConnectName
    //  - Scada::ABI::ScadaConnectAddr
    //
    struct ConnectParameters {
        std::uint16_t               cb = 0; // sizeof (ConnectParameters)
        std::uint16_t               flags = 0; // TODO: sync/async notifications, sync access to 'db', 
        std::uint32_t               reserved = 0;
        Scada::Atom                 software;   // software type, e.g.: HMI
        Scada::Atom                 identity;   // instance name
        const wchar_t *             pipe = nullptr; // SCADA pipe name, defaults to L"TRIMCORE:SCADA"
        const ConnectionCallbacks * callbacks = nullptr;

        // TODO: credentials
    };

    // DisconnectNote
    //  - 
    //
    struct DisconnectNote {
        std::uint16_t cb = sizeof (DisconnectNote);
        std::uint16_t reserved = 0;
        DWORD         reason = 0; // Win32 or HRESULT
    };

    // ScadaConnectName
    //  - 
    //  - 'peer' - local network name of the remote computer running SCADA software
    //           - NULL means only establish local memory-mapping connection
    //  - returns true/false result
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaConnectName (const wchar_t * peer, const ConnectParameters *);

    // ScadaConnectAddr
    //  -
    //  - 'peer' - socket address ofthe remote computer running SCADA software
    //           - NULL means only establish local memory-mapping connection
    //  - returns true/false result
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaConnectAddr (const sockaddr * peer, const ConnectParameters *);

    // ScadaConnectionPump
    //  - to synchronously receive notifications, callbacks, and drive possible reconnect
    // 
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaConnectionPump ();

    // ScadaConnectionClose
    //  - closes connection, if any, and stops ScadaConnectionPump from reestablishing one
    //  - optionally sends the DisconnectNote (may be nullptr)
    //  - result is bitmask reporting what was disconnected
    //     - 0x01 - always set on success, if not then nothing was disconnected
    //     - 0x02 - named pipe was disconnected
    //     - 0x04 - direct local db mapping was used and successfully closed
    //     - 0x08 - DisconnectNote was successfully sent
    //
    SCADA_DLL_IMPORT std::size_t TRIMCORE_APIENTRY ScadaConnectionClose (const DisconnectNote *) noexcept;
}

#endif
