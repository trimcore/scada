*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration - Expressions

Expressions are short dynamic computations on cell values implemented into signalling system,
having higher priority, i.e. evaluated before callbacks.

Expressions are supported on following type of cells: Numeric, Data, Text and Timestamps.

**Important**: The performance of expression evaluation will always be significantly lower than
alternative native code implementation in a DLL driver.

## Syntax

    name = <init> = <base>[!] (0~N)[<op> [$]<input>[!] [ext]] [restrict [$]<input>[!] (0~N)[<op> [$]<input>]]

### Notes

* Always separate every part of the expression by one or more white-space characters!
  Operators must be separated on both left and right, otherwise they are interpreted as part
  of keyword or input.
* Prefix and suffix, like `$` or `!`, must NOT be white-space separated.

### Components

* **init**  
  Standard part of cell definition syntax.
  Sets default value and target type of the cell: number, text or timestamp.
  See general [configuration](cfg.md) and individual data types.

* **base**  
  A relative [Atom Path](cfg-atom.md) naming a cell that the initial value is loaded, when the expression is evaluated.
  Base can specify suffix `!` just as inputs, see below.

* **op**  
  See [Operators](#operators) below.

* **input**  
  A relative [Atom Path](cfg-atom.md) naming a cell that the operand value is loaded.  
  Or a constant immediate value.

  `$` prefix designates cell that will not be duplicated on instantiation  
  `!` suffix designates cell whose change/signalling will trigger expression reevaluation

* **ext**  
  Optional additional actions on the particular input value:

  * `n2h` or `byteswap`  
    If the current value at that point is 16-bit or 32-bit integer (signed or unsigned), the bytes are swapped.
    Only first element of a vector is swapped.
    Works differently when applied to Extract Operation (see below).

  * `convert`  
    Up until the encountering the keyword `convert` the expression is evaluated in type of **base** cell.
    At this point, the intermediate value is converted into **initial** type.
    If not specified, the conversion is performed last, just before assigning result to the cell.

  The keywords can be separated by comma `,` for readibility.

* `restrict`  
  Optional keyword that ends the computation, and begins the conditional part of the expression.
  If present, the conditional part is evaluated separately, and only if non-zero, the final assignment performed.
  The same rules apply for evaluation of the conditional part, only the intermediate is separate.

## Operators

The following operations are supported in current version of the software:

* `+` (plus)  
  Adds value loaded from **input** into current intermediate.

      sum = 0u:64 = /raw/input1! + /raw/input2! + $1000u:64

* `-` (hyphen-minus) or `−` (mathematical minus)  
  Subtracts value loaded from **input** into current intermediate.

* `*` (star) or `×` (multiply sign)  
  Multiplies the current intermediate by the value loaded from **input**.

* `/` (slash), `∕` (division slash), or `÷` (division sign)  
  Divider the current intermediate by the value loaded from **input**.
  
      temperature = 0.0:64 = /raw/data! ÷ $10.0:64

* `==` (equality comparison) or `<>` (inequality comparison, alternatively `≠`)  
  Compares current intermediate to the value loaded from **input**
  and assigns resulting 1 (true) or 0 (false) to the intermediate.
  Regular vector characteristics apply, see below.

* `==:` (equals ANY of)  
  Equality comparison with different vector characteristics:
  Each of the current intermediate elements is compared to EVERY element of the value loaded from **input**
  and if ANY matches, the result is 1 (true).

      signal = 0u:8 = /raw/signal! restrict /raw/signal <> $0u

  Updates signal only when raw signal is nonzero.

* `[]` Extraction  
  Extracts a single element from a vector, range of bits from a value (per each vector element), or both.

  Variations:

  * `= /raw/data [1]` extracts 2nd element from a /raw/data vector
  * `= /raw/data [8..23]` extracts 2nd and 3rd byte from in /raw/data (from each element if vector)
  * `= /raw/data [0][8..23]` combines the above, resulting in single value, even if /raw/data was vector

  Additional modifiers supported:

  * `$` - preceeding square brackets means the extraction parameters are not duplicated on instantiation
  * `signed` - sign-extends the extracted pattern
  * `unsigned` - default, implied, the bits
  * `n2h` or `byteswap` may follow, indicating that the extracted bytes should be swapped.
    Currently supported on 16-bit or 32-bit extractions, and the expression will fail otherwise.  
    **Important:** Byte swap is always performed before sign extension.
  
  Example:  
  `/raw/data $[1][8..23] signed n2h` on 4×32-bit unsigned vector would be processed in following steps
  1. `(0x01234567u, 0x89ABCDEFu, 0x01020304u, 0x05060708u):4×32` - Vector of four 32-bit Unsigned integers
  2. `0x89ABCDEFu:1×32` - 2nd element extracted
  3. `0x0000ABCDu:1×32` - bits 8..23 extracted
  4. `0x0000CDABu:1×32` - n2h swapped
  5. `0xFFFFCDABu:1×32` - sign extension from 16-bit to 32-bit
  
  **Failures:**  
  If the extraction range is out of bounds for the input, the target cell value is left unchanged and marked invalid.

## Instantiation (remapping)

If the **base** or any **input** is relative path, then the expression is a template to be instantiated.
See [Database Instantiation](database.md#instantiation) for details.

The paths are applied to current target instantiation path, and the input is replaced by ID of a cell
described by that path. The evaluation triggers are then registered on the cells, if designated by `!`.
When such cell is signalled (on change), the expression is reevaluated.

Note: The expression paths are not remapped when instantiating from `/settings` to `/manifolds` or `/models`.

<!---
## Evaluation

* TBD: Conversion
* TBD: Vector characteristics
* TBD: Assignment: 
--->

## Limitations

* Currently the expressions are limited to `N` = 4 operands.

## Notes

* TBD: database storage specs link
* Timestamps are considered 64-bit unsigned integers

---

« [Configuration](cfg.md)
