#ifndef SCADA_OBJECT_TCC
#define SCADA_OBJECT_TCC

template <typename T>
inline Scada::Object <T>::Object (Cell::ID id)
    : handle (Scada::Access (id)) {

    this->validate_access ();
}
template <typename T>
inline Scada::Object <T>::Object (const AtomPathView & path)
    : handle (Scada::Access (path)) {

    this->validate_access (path);
}
template <typename T>
inline Scada::Object <T>::Object (const AtomPathView & path, Atom name)
    : handle (Scada::Access (path, name)) {

    this->validate_access (path, name);
}
template <typename T>
inline Scada::Object <T>::Object (const AtomPathView & path, std::string_view name)
    : handle (Scada::Access (path, Atom (name))) {

    this->validate_access (path, Atom (name));
}
template <typename T>
inline Scada::Object <T>::Object (const AtomPathView & path, std::wstring_view name)
    : handle (Scada::Access (path, Atom (name))) {

    this->validate_access (path, Atom (name));
}
template <typename T>
inline Scada::Object <T>::~Object () {
    Scada::Release (this->handle);
}

template <typename T>
inline Scada::Cell::ID Scada::Object <T>::identify () const noexcept {
    return ABI::ScadaGetID (this->handle);
}

/*template <typename T>
inline Scada::Object <T>::Object (const Object & other)
    : handle (ABI::ScadaDuplicateHandle (other.handle)) {

    this->validate_access ();
}

template <typename T>
Scada::Object <T> & Scada::Object <T> ::operator = (const Object & other) {
    Scada::Release (this->handle);
    this->handle = ABI::ScadaDuplicateHandle (other.handle);
    this->validate_access ();
    return *this;
}*/

template <typename T>
inline void Scada::Object <T>::validate_access () {
    if (!this->handle) {
        this->throw_on_failure ();
    }
}

template <typename T>
inline void Scada::Object <T>::validate_access (const AtomPathView & path) {
    if (!this->handle) {
        TRIMCORE::log (TRIMCORE::Log::Error, "Scada::Object failure accessing {1}", path);
        this->throw_on_failure ();
    }
}

template <typename T>
inline void Scada::Object <T>::validate_access (const AtomPathView & path, Atom name) {
    if (!this->handle) {
        TRIMCORE::log (TRIMCORE::Log::Error, "Scada::Object failure accessing {1}/{2}", path, name);
        this->throw_on_failure ();
    }
}

/*template <typename T>
inline Scada::Cell::Handle Scada::Object <T>::GetHandle () const noexcept {
    return this->handle;
}*/

#endif
