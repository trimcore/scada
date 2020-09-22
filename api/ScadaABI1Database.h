#ifndef SCADA_SCADAABI1DATABASE_H
#define SCADA_SCADAABI1DATABASE_H

// Manifold ABI, version 1, Database API
//  - 

namespace Scada::ABI {

    // Api1DbInstantiate
    //  - 
    //
    SCADA_DLL_IMPORT std::size_t Api1DbInstantiate (const Atom * source, std::size_t,
                                                    const Atom * target, std::size_t,
                                                    const Atom * differences, std::size_t) noexcept;
}

#endif
