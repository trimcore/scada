*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuring system privileges

TBD: descriptions

## SeLockMemoryPrivilege

The high performance capability relies significantly on preventing internal database from being swapped out from physical memory.
The software attempts to lock important memory regions as large pages in physical RAM. This will fail on regular OS installation as
the feature requires nonstandard privileges to be assigned to the accounts under whose the software runs.

* [Steps to properly configure the privilege](https://docs.microsoft.com/en-us/sql/database-engine/configure-windows/enable-the-lock-pages-in-memory-option-windows)

*The software will continue to function if this locking fails, but the potential for degraded performance is reported as warning in the logs.*

### Briefly in steps

* **Win+R** `gpedit.msc`
* Computer Configuration / Windows Settings / Security Settings / Local Policies / User Rights Assignment
* Lock pages in memory
* Add User or Group: `SYSTEM`
* Add User or Group: `trimcore` *or appropriate username under which the software runs*
