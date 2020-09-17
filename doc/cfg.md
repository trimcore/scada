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
Each directive, Directory branch change or Cell definition must appear on a separate line.

### Comments and strings

Comments are introduced with two forward slashes: `// comment`  
Two forward slashes within string are not interpreted as a comment: `"// not comment"`  
All comments are removed before next parsing step.

Strings are enclosed by double quotes character: `"string"`  
To represent double quotes character within a string, double it: `"this "" is double-quote"`

### Character set

Loader checks each file for UTF-8 (*EF BB BF*) or UTF-16 LE BOM (*FF FE*). If present, all further charset directives are ignored.

If no BOM is present and the file starts with `!utf-8` directive or `!utf-16` directive (in UTF-16 charset),
then UTF-8 or UTF-16 LE character set respective is assumed.

If neither condition is detected, current 8-bit (*ANSI*/*CP_ACP*) code page is assumed, i.e. ...

### Nesting configuration files

Configuration files can include other files using `!include file.cfg` directive.  
No quotes for filenames containing spaces are necessary.

### TBD: Atoms

## Directory paths

`/aa/bb/cc:`

## Cell definitions

`name = value`

## Notes

* no macros or automatic replacements are currently supported
