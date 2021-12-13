*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration - Date/Time

TBD: general

Number of 100-nanosecond intervals since January 1, 1601 (UTC).

* [TRIMCORE::Timestamp](https://github.com/trimcore/trimcore-dll/blob/master/TRIMCORE_Now.h) type
* [FILETIME](https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime)

## Syntax

    name = T
    name = T = [...] // expression

**Limited implementation**: *The configuration format is currently limited to a single `T` which creates Date/Time type cell to be assigned
value later in code.*

## Cell data

    cell->value.type == Scada::Cell::Type::TimeStamp;
    cell->value.width == 0;
    cell->value.count == 1;
    
    cell->value.timestamp; // TRIMCORE::Timestamp -> union { std::uint64_t ull; FILETIME ft; }

## Expressions

* TBD: Processed as 64-bit unsigned integer.

## Notes

* TBD

---

« [Configuration](cfg.md)
