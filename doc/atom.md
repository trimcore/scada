*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Atoms and Atom Paths

Atoms are introduced to significantly reduce number of allocations of string objects and their overhead,
and for improved performance when searching [Directory](../doc/directory.md) or any other structure with Atom as a key.
Atoms are compared as simple 64-bit unsigned integers.

**Atom is internally 64-bit value representing either a short string (12 characters max) or an unsigned 63-bit number.**

AtomPath in general is a sequence of Atoms used to represent [Directory](directory.md) path, or group of Atoms for other purposes.
The [API](../api) provides [AtomPath](AtomPath.md) class to simplify path manipulation (see documentation for limits).

## Parsing

Input consisting of decimal or hexadecimal (prefix `#x`) digits is stored as a numeric Atom, otherwise parsing as string is attempted (if not prevented by `#` prefix).

Force parsing into numeric form with prefix `#`.  
Force parsing into string form by enclosing the input in single or double quotes.

## Numbers

Atoms can store any 63-bit unsigned value, i.e.:  0 to 9'223'372'036'854'775'807. Internally the most significant bit is set.

Parser supports following prefixes:

* `#` - to force parsing as number, and fail on presence of non-digit

* `#x` or `#X` - in addition to `#` parses the number as hexadecimal

Both prefixes suppress later attempts to parse input as string in case numeric parsing fails.

## Strings

Strings are encoded by 12 code-points (5-bit). Characters take 1 (lowercase) or 2 (uppercase) code-points.
Underscores, dashes and spaces are converted to code-point 0x00, and are ignored when trailing.
Internally, the remaining upper 4 bits of the 64-bit value are required to be 0.

Strings can contain only characters a-z, A-Z, 0-9 and space/dash/underscore. These are encoded into code-points as follows:

Character | Encoding | Character | Encoding
-|-|-|-
*space* /- / _ | 0x00 | A | 0x01 0x00
  |      | B | 0x01 0x01
a | 0x02 | C | 0x01 0x02
b | 0x03 | D | 0x01 0x03
c | 0x04 | E | 0x01 0x04
d | 0x05 | F | 0x01 0x05
e | 0x06 | G | 0x01 0x06
f | 0x07 | H | 0x01 0x07
g | 0x08 | I | 0x01 0x08
h | 0x09 | J | 0x01 0x09
i | 0x0A | K | 0x01 0x0A
j | 0x0B | L | 0x01 0x0B
k | 0x0C | M | 0x01 0x0C
l | 0x0D | N | 0x01 0x0D
m | 0x0E | O | 0x01 0x0E
n | 0x0F | P | 0x01 0x0F
o | 0x10 | Q | 0x01 0x10
p | 0x11 | R | 0x01 0x11
q | 0x12 | S | 0x01 0x12
r | 0x13 | T | 0x01 0x13
s | 0x14 | U | 0x01 0x14
t | 0x15 | V | 0x01 0x15
u | 0x16 | W | 0x01 0x16
v | 0x17 | X | 0x01 0x17
w | 0x18 | Y | 0x01 0x18
x | 0x19 | Z | 0x01 0x19
y | 0x1A | 4 | 0x01 0x1A
z | 0x1B | 5 | 0x01 0x1B
0 | 0x1C | 6 | 0x01 0x1C
1 | 0x1D | 7 | 0x01 0x1D
2 | 0x1E | 8 | 0x01 0x1E
3 | 0x1F | 9 | 0x01 0x1F

Numbers are by default encoded as numeric Atoms, to force encoding number as  string,
enclosed the input by single or double quotes. Then the number is stored as
sequence of code-points. **Warning:** Such way encoded number does not compare equal to regular numeric Atom.

## Notes

TBD: atom.exe tool


