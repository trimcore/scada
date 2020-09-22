#ifndef SCADA_OBJECT_TCC
#define SCADA_OBJECT_TCC

template <typename T>
inline Scada::Object <T>::Object (Cell::ID id, AccessLevel level)
    : handle (ABI::Api1CellAccess (id, level)) {

    this->validate_access ();
}
template <typename T>
inline Scada::Object <T>::Object (const AtomPath & path, AccessLevel level)
    : handle (ABI::Api1CellAccessPath (path.data (), path.depth, level)) {

    this->validate_access (path);
}
template <typename T>
inline Scada::Object <T>::~Object () {
    if (this->handle) {
        ABI::Api1CellRelease (this->handle);
    }
}

template <typename T>
inline Scada::Cell::ID Scada::Object <T>::identify () const noexcept {
    return ABI::Api1CellGetID (this->handle);
}

template <typename T>
inline Scada::Object <T>::Object (const Object & other)
    : handle (ABI::Api1CellDuplicateHandle (other.handle, AccessLevel::Same)) {

    this->validate_access ();
}

template <typename T>
Scada::Object <T> & Scada::Object <T> ::operator = (const Object & other) {
    if (this->handle) {
        ABI::Api1CellRelease (this->handle);
        this->handle = 0;
    }
    this->handle = ABI::Api1CellDuplicateHandle (other.handle, AccessLevel::Same);
    this->validate_access ();
    return *this;
}

template <typename T>
inline void Scada::Object <T>::validate_access () {
    // TODO: validate that Api1CellGetDataType is proper for <T>
    if (!this->handle) {
        this->throw_on_failure ();
    }
}

template <typename T>
inline void Scada::Object <T>::validate_access (const AtomPath & path) {
    // TODO: validate that Api1CellGetDataType is proper for <T>
    if (!this->handle) {
        TRIMCORE::log (TRIMCORE::Log::Error, "Scada::Object failure accessing {1}", path);
        this->throw_on_failure ();
    }
}

inline void Scada::ObjectSpecializationBase::throw_on_failure () const {
    switch (GetLastError ()) {
        case ERROR_SUCCESS:
            return;
        default:
            throw TRIMCORE::Log::Exception (nullptr, "Scada::Object error {ERR}");
    }
}

template <typename T>
inline Scada::Cell::Handle Scada::Object <T>::GetHandle () const noexcept {
    return this->handle;
}

#endif
