#ifndef SCADA_RWSPINLOCK_TCC
#define SCADA_RWSPINLOCK_TCC

#include "SCADA_RwSpinLock.hpp"

// RwSpinLockScopeExclusive

template <typename StateType>
inline Scada::RwSpinLockScopeExclusive <StateType>::~RwSpinLockScopeExclusive () noexcept {
    if (this->lock) {
        this->release ();
    }
}

template <typename StateType>
inline void Scada::RwSpinLockScopeExclusive <StateType>::release () noexcept {
    this->lock->ReleaseExclusive ();
    this->lock = nullptr;
}

// RwSpinLockScopeShared

template <typename StateType>
inline Scada::RwSpinLockScopeShared <StateType>::RwSpinLockScopeShared (const Scada::RwSpinLockScopeShared <StateType> & from) noexcept : lock (from.lock) { this->lock->AcquireShared (); }

template <typename StateType>
inline Scada::RwSpinLockScopeShared <StateType> & Scada::RwSpinLockScopeShared <StateType>::operator = (const Scada::RwSpinLockScopeShared <StateType> & from) noexcept {
    if (this->lock) {
        this->release ();
    }
    this->lock = from.lock;
    if (this->lock) {
        this->lock->AcquireShared ();
    }
    return *this;
}

template <typename StateType>
inline Scada::RwSpinLockScopeShared <StateType>::~RwSpinLockScopeShared () noexcept {
    if (this->lock) {
        this->release ();
    }
}

template <typename StateType>
inline void Scada::RwSpinLockScopeShared <StateType>::release () noexcept {
    this->lock->ReleaseShared ();
    this->lock = nullptr;
}

// RwSpinLock

template <typename StateType>
inline void Scada::RwSpinLock <StateType>::AcquireExclusive (std::uint32_t * rounds) noexcept {
    std::uint32_t r = 0;
    while (!this->TryAcquireExclusive ()) {
        if (++r <= Parameters::Exclusive::Yields) {
            YieldProcessor ();
        } else {
            this->Spin <Parameters::Exclusive> (r);
        }
    }
    if (rounds) {
        *rounds = r;
    }
}

template <typename StateType>
inline void Scada::RwSpinLock <StateType>::AcquireShared (std::uint32_t * rounds) noexcept {
    std::uint32_t r = 0;
    while (!this->TryAcquireShared ()) {
        if (++r <= Parameters::Shared::Yields) {
            YieldProcessor ();
        } else {
            this->Spin <Parameters::Shared> (r);
        }
    }
    if (rounds) {
        *rounds = r;
    }
}

template <typename StateType>
inline void Scada::RwSpinLock <StateType>::UpgradeToExclusive (std::uint32_t * rounds) noexcept {
    std::uint32_t r = 0;
    while (!this->TryUpgradeToExclusive ()) {
        if (++r <= Parameters::Upgrade::Yields) {
            YieldProcessor ();
        } else {
            this->Spin <Parameters::Upgrade> (r);
        }
    }
    if (rounds) {
        *rounds = r;
    }
}

template <typename StateType>
[[nodiscard]] inline bool Scada::RwSpinLock <StateType>::AcquireExclusive (std::uint64_t timeout, std::uint32_t * rounds) noexcept {
    std::uint32_t r = 0;

    while (!this->TryAcquireExclusive ()) {
        if (++r <= Parameters::Exclusive::Yields) {
            YieldProcessor ();
        } else {
            if (timeout) {
                auto t = GetTickCount64 () + timeout;
                SwitchToThread ();

                // contested case, with backoff
                while (!this->TryAcquireExclusive ()) {
                    if (GetTickCount64 () < t) {
                        this->Spin <Parameters::Exclusive> (++r);
                    } else {
                        if (rounds) {
                            *rounds = r;
                        }
                        return false;
                    }
                }
            }
            break;
        }
    }
    if (rounds) {
        *rounds = r;
    }
    return true;
}

template <typename StateType>
[[nodiscard]] inline bool Scada::RwSpinLock <StateType>::AcquireShared (std::uint64_t timeout, std::uint32_t * rounds) noexcept {
    std::uint32_t r = 0;

    while (!this->TryAcquireShared ()) {
        if (++r <= Parameters::Shared::Yields) {
            YieldProcessor ();
        } else {
            if (timeout) {
                auto t = GetTickCount64 () + timeout;
                SwitchToThread ();

                // contested case, with backoff
                while (!this->TryAcquireShared ()) {
                    if (GetTickCount64 () < t) {
                        this->Spin <Parameters::Shared> (++r);
                    } else {
                        if (rounds) {
                            *rounds = r;
                        }
                        return false;
                    }
                }
            }
            break;
        }
    }
    if (rounds) {
        *rounds = r;
    }
    return true;
}

template <typename StateType>
[[nodiscard]] inline bool Scada::RwSpinLock <StateType>::UpgradeToExclusive (std::uint64_t timeout, std::uint32_t * rounds) noexcept {
    std::uint32_t r = 0;

    while (!this->TryUpgradeToExclusive ()) {
        if (++r <= Parameters::Upgrade::Yields) {
            YieldProcessor ();
        } else {
            if (timeout) {
                auto t = GetTickCount64 () + timeout;
                SwitchToThread ();

                // contested case, with backoff
                while (!this->TryUpgradeToExclusive ()) {
                    if (GetTickCount64 () < t) {
                        this->Spin <Parameters::Upgrade> (++r);
                    } else {
                        if (rounds) {
                            *rounds = r;
                        }
                        return false;
                    }
                }
            }
            break;
        }
    }
    if (rounds) {
        *rounds = r;
    }
    return true;
}

// internals

template <typename StateType>
template <typename Timings>
inline void Scada::RwSpinLock <StateType>::Spin (std::uint32_t round) {
    DWORD n = 0;
    if (round > Timings::Yields + Timings::Sleep0s) {
        n = 1;
    }
    Sleep (n);
}

// if scope

template <typename StateType>
[[nodiscard]] inline Scada::RwSpinLockScopeExclusive <StateType> Scada::RwSpinLock <StateType>::exclusively (std::uint32_t * rounds) noexcept {
    this->AcquireExclusive (rounds);
    return this;
}
template <typename StateType>
[[nodiscard]] inline Scada::RwSpinLockScopeExclusive <StateType> Scada::RwSpinLock <StateType>::exclusively (std::uint64_t timeout, std::uint32_t * rounds) noexcept {
    if (this->AcquireExclusive (timeout, rounds))
        return this;
    else
        return nullptr;
}

template <typename StateType>
[[nodiscard]] inline Scada::RwSpinLockScopeShared <StateType> Scada::RwSpinLock <StateType>::share (std::uint32_t * rounds) noexcept {
    this->AcquireShared (rounds);
    return this;
}
template <typename StateType>
[[nodiscard]] inline Scada::RwSpinLockScopeShared <StateType> Scada::RwSpinLock <StateType>::share (std::uint64_t timeout, std::uint32_t * rounds) noexcept {
    if (this->AcquireShared (timeout, rounds))
        return this;
    else
        return nullptr;
}


#endif
