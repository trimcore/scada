*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration (.cfg) file syntax

The purpose of .cfg files is to pre-create [Directory](directory.md) levels and named Cells, that are,
upon initialization, used to actually initialize the system and the solutions
it is modeling. After the configuration is loaded, the system is [bootstrapped](boot.md).

First configuration file loaded is **SCADA.cfg** which may reference other files.

## Example

    !utf-8
    !system segment control size 4M
    
    system/log:
        level = "warning" // comment
        detailed = true
    
    settings/manifolds/AppProcess:
    settings/models/ModelName:
        manifold = "AppProcess"
    
    !include "model.cfg"
    
    settings/model/ModelName/drivers:
        database = "PostgreSQL.dll"
    settings/model/ModelName/database:
        port = 5432
    
    // ...

## Contents

* **[General](cfg-general.md)**  
  *cfg (text) files, including/nesting and character set directives, comments, strings*

* **[Directory structure](cfg-structure.md)**  
  *hierarchy, paths, names, atoms*  
  *e.g.:* `system/log:`

* Cell definitions  
  *take simple form of a single row:* `name = value` *where value can*

  * **null**  
    *lowercase `null` specifies cell that contains no value, it is a separate data type*  
    *null cells can still be signalled, carry timestamp or specify flags*

  * **[Numeric values](cfg-numeric.md)**  
    *integers, floats, vectors*

  * **[Strings and binary blobs](cfg-string.md)**

  * **Atoms, Atom Paths**  
    *definition starts with `/` character and 0 to 4 `/`-separated [Atom](atom.md)s, optionally ending with `:` (ignored)*

  * **[Date/Time](cfg-datetime.md)**

  * **[Addresses](cfg-address.md)**  
    *communication endpoint addresses, both local and network, e.g.: IP addresses*

* **[Templates](cfg-template.md)**  
  *define and instantiate template for directory path and contained cells replacing parameters*

* **[Expressions](cfg-expr.md)**  
  *describe short dynamic computations on cell values*

## Notes

* no protection against endless cycles in files including each other is implemented, file nesting and configuration file sizes are limited only by available memory
