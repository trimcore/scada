#ifndef SCADA_SCADA_H
#define SCADA_SCADA_H

#include <cstddef>
#include <vector>

#include "TRIMCORE.h"
#include "ScadaABI.h"

// TODO: Scada::Aggregate
//  - group of Objects referencing Cells
//  - active after Pending construction tracking (directory path, cells for all objects (some optional))

namespace Scada {

    // Scada::GetManifoldName
    //  - retrieves 'Atom' name of the current manifold process
    //  - use to access proper /manifolds/<MANIFOLD>/... directory path
    //  - empty when connected from other process
    //  - empty in primary service process
    //
    inline const Atom & GetManifoldName () noexcept {
        return *reinterpret_cast <const Atom *> (&ABI::ScadaManifoldName);
    }

    // Connect
    //  - 
    //  - TODO: versions with machine name, pipe name
    //
    inline bool Connect (Scada::Atom software, Scada::Atom identity, const ABI::ConnectionCallbacks * callbacks) noexcept {
        ABI::ConnectParameters connect;
        connect.cb = sizeof connect;
        connect.callbacks = callbacks;
        connect.software = software;
        connect.identity = identity;

        return ABI::ScadaConnectName (nullptr, &connect);
    }

    // inline bool Connect (const sockaddr * remote);

    inline bool Disconnect (DWORD reason) noexcept {
        ABI::DisconnectNote note;
        note.reason = reason;
        return ABI::ScadaConnectionClose (&note);
    }

    // Scada::Access
    //  - 
    //
    inline Cell::Handle Access (const AtomPathView & path) noexcept {
        if (!path.empty ()) {
            Cell::Handle handle;
            if (ABI::ScadaAccess (path.data (), path.size () - 1,
                                  path.data () + (path.size () - 1), 1, &handle)) {
                return handle;
            }
        }
        return 0;
    }
    inline Cell::Handle Access (const AtomPathView & path, Atom name) noexcept {
        Cell::Handle handle;
        if (ABI::ScadaAccess (path.data (), path.size (), &name, 1, &handle)) {
            return handle;
        } else
            return 0;
    }
    inline Cell::Handle Access (const AtomPathView & path, std::string_view name) noexcept {
        Atom a (name);
        Cell::Handle handle;
        if (ABI::ScadaAccess (path.data (), path.size (), &a, 1, &handle)) {
            return handle;
        } else
            return 0;
    }
    inline Cell::Handle Access (const AtomPathView & path, std::wstring_view name) noexcept {
        Atom a (name);
        Cell::Handle handle;
        if (ABI::ScadaAccess (path.data (), path.size (), &a, 1, &handle)) {
            return handle;
        } else
            return 0;
    }
    inline Cell::Handle Access (Scada::Cell::ID id) noexcept {
        return ABI::ScadaAccessID (id);
    }

    inline std::size_t Release (Cell::Handle h) noexcept {
        return ABI::ScadaRelease (1, &h);
    }
    

    using CellEntry = ABI::CellEntry;
    using ListenCallback = ABI::ListenCallback;
    using NewCellCallback = ABI::NewCellCallback;
    using NewSubDirCallback = ABI::NewSubDirCallback;
    using FinalizeCallback = ABI::FinalizeCallback;

    namespace Directory {

        // Scada::Directory::Construct
        //  -
        //
        inline std::size_t Construct (const AtomPathView & path) noexcept {
            return ABI::ScadaDirConstruct (path.data (), path.size ());
        }

        // Scada::Directory::Instantiate

        inline std::size_t Instantiate (const AtomPathView & source, const AtomPathView & target) noexcept {
            return ABI::ScadaDirInstantiate (source.data (), source.size (), target.data (), target.size (), nullptr, 0);
        }
        inline std::size_t Instantiate (const AtomPathView & source, const AtomPathView & target, const AtomPathView & diffs) noexcept {
            return ABI::ScadaDirInstantiate (source.data (), source.size (), target.data (), target.size (), diffs.data (), diffs.size ());
        }

        /*// Scada::Directory::ForEachCell
        //  - 
        //
        template <typename Callback>
        inline std::size_t ForEachCell (const AtomPath & path, Callback callback);

        // Scada::Directory::ForEachSub
        //  - 
        //
        template <typename Callback>
        inline std::size_t ForEachSub (const AtomPath & path, Callback callback);

        */

        // Listen
        //  - invokes callback routine when new sub-DIRECTORY is added directly into provided directory path
        //
        inline
        bool Listen (const AtomPathView & path, NewSubDirCallback callback, void * context) noexcept {
            return ABI::ScadaDirListenSub (path.data (), path.size (), callback, nullptr, context);
        }

        // Listen
        //  - invokes callback member function on 'self' when new sub-DIRECTORY is added directly into provided directory path
        //  - TODO: extend all Listen/Register functions with this pattern
        //
        template <typename T> inline
        bool Listen (const AtomPathView & path, T * self,
                     void (TRIMCORE_APIENTRY T::*callback) (const Atom * path, std::size_t depth, Atom name)) noexcept {

            if (auto function = ABI::IsMemberFunctionDirectlyCallable (callback)) {
                return ABI::ScadaDirListenSub (path.data (), path.size (), function, nullptr, self);

            } else {
                struct ListenDelegate {
                    T * self;
                    void (TRIMCORE_APIENTRY T:: * callback) (const Atom * path, std::size_t depth, Atom name);
                };
                auto ListenAdapter = [] (void * context, const Atom * path, std::size_t depth, Atom name) {
                    auto delegate = static_cast <ListenDelegate *> (context);
                    return ((delegate->self)->*(delegate->callback)) (path, depth, name);
                };
                auto FinalizeAdapter = [] (void * context) {
                    delete static_cast <ListenDelegate *> (context);
                };
                return ABI::ScadaDirListenSub (path.data (), path.size (),
                                               ListenAdapter, FinalizeAdapter, new ListenDelegate { self, callback });
            }
        }

        /*template <typename T> inline
        bool RegisterCallback (std::wstring_view path, T * self,
                               void (T::*member) (const void *, std::size_t, int)) noexcept {

            return ABI::RegisterCallback (path.data (), path.size (), output, self);
        }*/

        // Listen
        //  - invokes callback routine when new CELL is added directly into provided directory path
        //
        inline
        bool Listen (const AtomPathView & path, NewCellCallback callback, void * context) noexcept {
            return ABI::ScadaDirListenCell (path.data (), path.size (), callback, nullptr, context);
        }
        
    }
}

#include "Scada_Object.h"
#include "Scada.tcc"
#endif
