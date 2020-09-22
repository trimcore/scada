#ifndef SCADA_SCADA_TCC
#define SCADA_SCADA_TCC

namespace Scada::Directory {


    namespace ImplementationDetails {
        static inline constexpr std::size_t Align (std::size_t n, std::size_t alignment) noexcept {
            return (n + alignment - 1) & ~(alignment - 1);
        }

        template <typename Callback, typename Item>
        inline bool TryForEachCommon (Callback callback, Item * buffer, std::size_t & length, std::size_t n) {
            if (n <= length) {
                for (std::size_t i = 0; i != n; ++i) {
                    callback (buffer [i]);
                }
                length = n;
                return true;
            } else {
                length = n;
                return false;
            }
        }

        template <typename Callback>
        inline bool TryForEachCell (const AtomPath & path, Callback callback, ABI::Api1DirListCellInfo * buffer, std::size_t & length) {
            auto n = ABI::Api1DirListCells (path.data (), path.depth, buffer, length);
            return TryForEachCommon (callback, buffer, length, n);
        }

        template <typename Callback>
        inline bool TryForEachSub (const AtomPath & path, Callback callback, Atom * buffer, std::size_t & length) {
            auto n = ABI::Api1DirListSubs (path.data (), path.depth, buffer, length);
            return TryForEachCommon (callback, buffer, length, n);
        }

        template <typename ItemType, typename Function, typename Callback>
        inline std::size_t ForEachCommon (const AtomPath & path, Function function, Callback callback) {

            // try temporary buffer first (8k Atoms or 4k CellInfos)

            TRIMCORE::Temporary64kB <ItemType> fast;
            auto length = fast.size ();
            if (function (path, callback, fast.data (), length))
                return length;

            // allocated buffer
            //  - add a little space for a subs that might've been added in parallel
            //  - align up to 4k page size to improve memory utilization and prevent fragmentation

            std::vector <ItemType> slow;
            while (true) {
                length = ImplementationDetails::Align (length + 64, 4096 / sizeof (ItemType));
                slow.resize (length);

                if (function (path, callback, slow.data (), length))
                    return length;
            }
        }
    }

    // Scada::Directory::ForEachCell
    //  - 
    //
    template <typename Callback>
    inline std::size_t ForEachCell (const AtomPath & path, Callback callback) {
        return ImplementationDetails::ForEachCommon <ABI::Api1DirListCellInfo> (path, ImplementationDetails::TryForEachCell <Callback>, callback);
    }

    // Scada::Directory::ForEachSub
    //  - 
    //
    template <typename Callback>
    inline std::size_t ForEachSub (const AtomPath & path, Callback callback) {
        return ImplementationDetails::ForEachCommon <Atom> (path, ImplementationDetails::TryForEachSub <Callback>, callback);
    }
}

#endif
