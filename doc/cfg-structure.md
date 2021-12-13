*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration - Directory Structure

TBD: tree hierarchy  
TBD: top level entries

## Atoms

Configuration **names**, i.e. database directory paths and cell names, are internally stored as [Atoms](atom.md). 
See the page for explanation. Appropriate restrictions apply.

## Directory paths

Slash-separated list of atoms `aa/bb/cc:` terminated by colon. Each atom represents nested level in
[Directory of named Cells](directory.md). The directory path is set for all subsequently following
cell definitions.

Only a small number of top-level levels can be created. Number of nested levels is generally not limited,
but huge number of cells or sub-levels in one directory can hinder performance; prefer deeper nesting.

## TBD

* example
* predefined paths
* fixed meaning

---

« [Configuration](cfg.md)
