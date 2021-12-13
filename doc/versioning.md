*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Versioning

The version number consists of 4 non-negative numbers separated by `.`

In general, software developed by TRIM CORE SOFTWARE s.r.o. uses
[semantic versioning](https://en.wikipedia.org/wiki/Semantic_versioning),
which differs slightly when in development and after release for general production use.

* Production stage: `major . minor . patch . build`
* Development stage: `0 . major . minor . build`

## Production Major/Minor

**Major** version number starts with 1 and is incremented rarely, typically for one of following reasons:

* Important commercial release
* Addition of a wide-impact feature
* Change that will create incompatibilities
* Large internal rewrite causing significant improvements

**Minor** version number is reset to 0 every time major number is incremented.
The number is incremented for every distinct public release that is not a **patch**, and can increase by more than 1.

## Development Major/Minor

Major number, generally starting at 1 and ending with 9 (release preview),
during development represents continuous introduction of primary features.
Version 0.5 should be roughly half complete in regards to originally planned feature set.

Minor number during development also takes on a role of a patch.
It's incremented for both, small improvement or patch.
During development the software isn't available to general public, thus preserving the distinction isn't very important.

## Patch

Incremented whenever a product/module is released with only a bug or important issue fixed.  
The number is reset to 0 every time major or minor number is incremented.

The patch number is usually not part of the marketing texts, but it easily reachable in the software
to verify whether the latest version is running.

## Build

Build number is independent on all other numbers.
It's incremented independently every time the developer makes any change and generates the executable. 

## Product vs file

In string fields/representation, for the purposes of product/file version number distinction
the product version usually consist only of **major** and **minor** number, unless the **patch**
number is important for the business purpose of the product.

TBD: Numeric vs. strings

## EXE/DLL

TBD

## Technical details

* Each number is limited to range 0 to 65535. This is due to the Windows'
  [VS_FIXEDFILEINFO](https://docs.microsoft.com/en-us/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo)
  structure (part of the EXE/DLL) where the values are stored.
 