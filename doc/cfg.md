*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration (.cfg) file syntax

The purpose of .cfg files is to pre-create [Directory](directory.md) levels and named Cells, that are,
upon initialization of the software and loaded modules, used to actually initialize ty system and solution
it is modeling.

First configuration file loaded is **SCADA.cfg** which may reference other files.  

The order of appearance of particular entries is mostly irrelevant. If multiple definitions of a named Cell
are present, the last Cell retains the name; the former ones can be accessed by Cell ID. If Directory path
is stated repeatedly, the defined content is merged together.

## General
Each directive or definition must appear on a separate line.

### Comments and strings

Comments are introduced with two forward slashes: `// comment`  
Within strings the two forward slashes are not interpreted as a comment: `"// not comment"`  

All comments, and all leading and trailing whitespace, are removed before interpreting.

Strings are enclosed by double quotes character: `"string"`  
To represent double quotes character within a string, double it: `"abc""def"` turns into `abc"def`

### Character set

Loader checks each file for UTF-8 (*EF BB BF*) or UTF-16 LE BOM (*FF FE*).  
If present, any further charset directive is ignored.

If no BOM is present and the file starts with `!utf-8` directive then UTF-8 is assumed.  
If the file starts with `!utf-16` directive (in UTF-16 charset) then UTF-16 LE is assumed.

If neither condition is detected, current 8-bit (*ANSI*/*CP_ACP*) code page is assumed, i.e.
interpretation of non-ASCII characters depends on current user/system locale and settings.
**Do NOT rely on this!**

### Nesting configuration files

Configuration files can include other files using `!include file.cfg` directive.  
Do not use quotes to enclose filenames.

### Atoms

Both directory path levels and cell definition names are internally stored as [Atoms](atom.md) so
appropriate restrictions on names apply.

## Directory paths

`aa/bb/cc:`

## Cell definitions

`name = value`

### NULL

### Numeric values

Integer syntax: [0b|0o|0x] \# [u|i][k|K|M|G|T] [:[**N**<x|×>][**W**]]

Floats: \#.\# [u|i][k|K|M|G|T] [:[**N**<x|×>][**W**]]

* 0b, 0o, 0x - prefix (integers only) for binary, octal or hexadecimal notation
* \#[.\#] - the number, integer or float
* i - integer is signed (default)
* u - integer is unsigned (overrides any `i`)
* k|K|M|G|T - multipliers (e.g.: 1k = 1024)
* **N** - specifies vector width, default is 1
* **W** - specifies storage width, in bits; supported widths are: 1, 8, 16, 32, 64 (default) for integers, 32 and 64 for floats

Constants true/false are aliases to 1u:1/0u:1 respectively.

### Strings

Syntax: "text" [storage]

Storage:

* **utf-8** -
* **utf-16** - 
* **ascii** -

### Binary blobs

Syntax: $ followed by pairs of hexadecimal digits, optionally separated by space, tab, colon, comma or dash.

### Atoms/AtomPaths

### Date/Time

### IP Addresses


## Notes

* no macros or automatic replacements are currently supported
* file nesting and configuration file sizes are limited by available memory
