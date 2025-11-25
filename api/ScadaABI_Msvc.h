#ifndef SCADA_SCADAABI_MSVC_H
#define SCADA_SCADAABI_MSVC_H

namespace Scada::ABI {
#ifdef _MSC_VER

    template <typename Class, typename Return, typename... Args> inline
    Return (*IsMemberFunctionDirectlyCallable (Return (Class:: * callback) (Args...))) (void *, Args ...) {

        typedef Return (Class:: * MemberFunction) (Args...);
        typedef Return (*FirstClassFunction) (void *, Args...);
        constexpr auto n = (sizeof (MemberFunction) - sizeof (FirstClassFunction)) / sizeof (std::intptr_t);

        union {
            MemberFunction mfn;
            struct {
                FirstClassFunction fn;
                std::intptr_t adjustments [n ? n : 1];
            } layout;
        };

        mfn = callback;

        for (auto i = 0uz; i != n; ++i)
            if (layout.adjustments [i] != 0x00)
                return nullptr;

        return layout.fn;
    }

#else
    template <typename Class, typename Return, typename... Args> inline
    Return (*IsMemberFunctionDirectlyCallable (Return (Class:: * callback) (Args...))) (void *, Args ...) {
        return nullptr;
    }
#endif
}

#endif
