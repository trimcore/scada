*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration - General overview

Each directive or definition must appear on a separate line.

The order of appearance of particular entries in .cfg file is mostly irrelevant. Except for direct dependencies and [expressions](cfg-expr.md).

If multiple definitions of a named Cell are present, the last Cell retains that name; the former ones can only be accessed by Cell ID.  
If Directory path is stated repeatedly, the defined content is merged together.

## Character set

Loader checks each file for leading UTF-8 `EF BB BF` or UTF-16 LE `FF FE` [BOM](https://en.wikipedia.org/wiki/Byte_order_mark).  
If present, any further charset directive is ignored.

If no BOM is present and the file starts with `!utf-8` directive then UTF-8 is assumed.  
If the file starts with `!utf-16` directive (in UTF-16 LE encoding) then UTF-16 LE is assumed.

If neither condition is detected, current 8-bit (*ANSI*/*CP_ACP*) code page is assumed, i.e.
interpretation of non-ASCII characters depends on current user/system locale and settings.
**Do NOT rely on this!**

## Nesting configuration files

Configuration files can include other files using `!include "file.cfg"` directive.  
Make sure to enclose filename in double quotes `"`.

## Comments and strings

Comments are introduced with two forward slashes: `// comment`  
Within strings the two forward slashes are not interpreted as a comment: `"// not comment"`  

All comments, and all leading and trailing whitespace, are removed before interpreting.

Strings are enclosed by double quotes character: `"string"`  
To represent double quotes character within a string, double it: `"abc""def"` turns into `abc"def`

---

« [Configuration](cfg.md)
