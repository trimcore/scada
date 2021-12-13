*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration - String/Binary

TBD: General

## Strings

Syntax: "text" [storage]

Strings are enclosed by double quotes character: `"string"`  
To represent double quotes character within a string, double it: `"abc""def"` turns into `abc"def`

The optional `storage` specification defines how the string is stored in memory.

Storage types available (may also be lowercase):

* **UTF-8**
* **UTF-16**
* **ASCII** - string is restricted to 7-bit ASCII characters, international characters are skipped

String limits currently stem from 32 byte cell data limit.
Strings are assigned directly, the storage type changes dynamically and optionally converted on read.

    TBD example

## Cell data

    cell->value.type == Scada::Cell::Type::String;
    cell->value.width == storage; // 0 = UTF-8, 1 = ASCII, 2 = UTF-16
    cell->value.count == length; // in characters, in bytes for UTF-8
    
    cell->value.text.local; // raw buffer

## Binary blobs

Syntax: `$` followed by pairs of hexadecimal digits, optionally separated by space, tab, colon, comma or dash.

Binary blobs are currently **limited to 32 bytes**.

    name = $ 01 23 45 67 89 ab cd ef 00 00
    name = $0123-4567-89ab-cdef-0000

Allowed byte separators, otherwise ignored, are:

* `$`
* `-` *dash*
* `:` *colon*
* `,` *comma*
* white-space characters: SPACE and TAB

## Expressions

* TBD: subscript per byte (or word for UTF-16)
* 

---

« [Configuration](cfg.md)
