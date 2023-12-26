*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration - Templates

## Template definition

    !template NAME p1 p2 p3 /some/directory/path/%p1%/%p2%
        text%p3% = "string%p3%"
        int%p3% = %p3%i:64

* Defining different template with the same name overrides the old one.
* The number of parameters is not restricted.

## Instantiation

    !instantiate NAME aaa bbb 123

Generates following:

    /some/directory/path/aaa/bbb:
        text123 = "string123"
        int123 = 123i:64

## Included files

* All templates are visible in nested (included) files, but have lower priority.
* Templates of the same name, defined in the closest file, have priority.
* Templates are not exported outside of included files.

---

« [Configuration](cfg.md)
