*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuring system privileges

The software is primarily designed to run on dedicated hardware or, with small performance penalty, within dedicated virtual machine.
To achieve full potential, performance and otherwise, certain additional configuration changes must be performed on the Operating System.

**Running the software without any of these privileges is unsupported!**

The software will continue to function without this properly configured, but the runtime performance may degrade apparently at random,
not meeting the warranted processing rate. Failing to acquire these privileges is reported as warning in the logs.

Following system privileges need to be configured:

## SeLockMemoryPrivilege

The high performance capability relies significantly on preventing internal database from being swapped out from physical memory.
The software attempts to lock important memory regions as large pages in physical RAM. This will fail on regular OS installation as
the feature requires nonstandard privileges to be assigned to the accounts under whose the software runs.

* [Steps to properly configure the privilege](https://docs.microsoft.com/en-us/sql/database-engine/configure-windows/enable-the-lock-pages-in-memory-option-windows)

### Briefly in steps

* **Win+R** `gpedit.msc`
* Computer Configuration / Windows Settings / Security Settings / Local Policies / User Rights Assignment
* Lock pages in memory
* Add User or Group: `SYSTEM`
* Add User or Group: `trimcore` *or appropriate username under which the software runs*

## SeManageVolumePrivilege

TBD

## SeShutdownPrivilege

In edge cases, when catastrophic environment or memory corruption is detected, the software may attempt to reboot the operating system.
This privilege is generally available to all non-guests, there is no need for additional setup.
