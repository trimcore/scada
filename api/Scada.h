#ifndef SCADA_SCADA_H
#define SCADA_SCADA_H

#include <cstddef>
#include <vector>

#include "TRIMCORE.h"
#include "ScadaABI.h"
#include "Scada_Object.h"

namespace Scada {

    // Scada::GetManifoldName
    //  - retrieves 'Atom' name of the current manifold process
    //  - use to access proper /manifolds/<MANIFOLD>/... directory path
    //  - empty in primary service process
    //
    inline const Atom & GetManifoldName () noexcept {
        return *reinterpret_cast <const Atom *> (&ABI::ManifoldName);
    }

    // Scada::Access
    //  - 
    //
    inline Cell::Handle Access (const AtomPath & path, ABI::Api1AccessLevel level) {
        return ABI::Api1CellAccessPath (path.data (), path.depth, level);
    }
    inline Cell::Handle Access (Scada::Cell::ID id, ABI::Api1AccessLevel level) {
        return ABI::Api1CellAccess (id, level);
    }

    namespace Directory {

        // Scada::Directory::Construct
        //  -
        //
        inline std::size_t Construct (const AtomPath & path) {
            return ABI::Api1DirConstruct (path.data (), path.depth);
        }

        // Scada::Directory::Instantiate

        inline std::size_t Instantiate (const AtomPath & source, const AtomPath & target) {
            return ABI::Api1DbInstantiate (source.data (), source.depth, target.data (), target.depth, nullptr, 0);
        }
        inline std::size_t Instantiate (const AtomPath & source, const AtomPath & target, const AtomPath & diffs) {
            return ABI::Api1DbInstantiate (source.data (), source.depth, target.data (), target.depth, diffs.data (), diffs.depth);
        }

        // Scada::Directory::ForEachCell
        //  - 
        //
        template <typename Callback>
        inline std::size_t ForEachCell (const AtomPath & path, Callback callback);

        // Scada::Directory::ForEachSub
        //  - 
        //
        template <typename Callback>
        inline std::size_t ForEachSub (const AtomPath & path, Callback callback);

        // Listen

        /*inline
        bool Listen (const AtomPath & path,
                     void (*callback) (void * context, Cell::Handle handle, FILETIME t, Cell::Flags flags, Scada::Cell::DataType, Cell::Information value),
                     void * context) {
            if (path.valid) {
                return ABI::Api1CellListen (path.data (), path.depth, callback, context);
            } else
                return false;
        }*/
        inline
        bool Listen (const AtomPath & path,
                     void (*callback) (void * context, const Atom * path, std::size_t depth, Atom sub),
                     void * context) {
            if (path.valid) {
                return ABI::Api1DirListenSub (path.data (), path.depth, callback, context);
            } else
                return false;
        }
        inline
        bool Listen (const AtomPath & path,
                     void (*callback) (void * context, const Atom * path, std::size_t depth, Atom cell, Cell::ID id),
                     void * context) {
            if (path.valid) {
                return ABI::Api1DirListenCell (path.data (), path.depth, callback, context);
            } else
                return false;
        }
    }
}

#include "Scada.tcc"
#endif
