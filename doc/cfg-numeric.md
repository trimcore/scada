*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration - Numeric values

## True/False

Lowecase constants `true`/`false` are aliases to 1u:1/0u:1 respectively.

## Syntax

**Integer**:

    [0b|0o|0x] # [u|i][k|K|M|G|T] [:[N<x|×>][W]]

**Float**:

    #.# [k|K|M|G|T] [:[N<x|×>][W]]

**Legend**:

* `0b`, `0o`, `0x` - prefix (integers only) for binary, octal or hexadecimal notation respectively
* \#[.\#] - the number, integer or float
* `i` - integer is signed (default)
* `u` - integer is unsigned (overrides any `i`)
* `k`,`K`,`M`,`G`,`T` - multipliers (e.g.: 1k = 1024, 1M = 1048576, ...)
* `N` - specifies vector width, default is 1
* `W` - specifies storage width, in bits; supported widths are: 1, 8, 16, 32, **64 (default)**, 128 and 256 for integers, 32 and 64 for floats

Limits: Some combinations of parameters are not supported. The loader will log warning.

**Integer syntax notes:**

`123` same as `123:64` same as `0x7Bi:1×64` - signed integer  
`1u` initializes unsigned integer

## Vector syntax

    floats = (1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0)f:32
    longlongs = (123, 456, 789, 0xABC)u:4×64

* Types (integer/unsigned/float) of all vector values must match.
* Vectors are limited to 32 bytes. Size of element × number of elements must be 256b or less.
* If type width is not defined, 64b is assumed (for both floats and integers).
* If number of elements is not explicitly defined, it's inferred from the specification.
* Other mismatches are reported as errors on startup.

## Notes

* The numeric definition may be followed by [expression](cfg-expr.md) specification.
  In such case it specifies result type and initial value.

---

« [Configuration](cfg.md)
