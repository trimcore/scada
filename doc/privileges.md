*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuring system privileges

TBD: Descriptions

## SeLockMemoryPrivilege

* Win+R "gpedit.msc"
* Computer Configuration / Windows Settings / Security Settings / Local Policies / User Rights Assignment
* Lock pages in memory
* Add User or Group: "SYSTEM"
* Add User or Group: "trimcore"

https://docs.microsoft.com/en-us/sql/database-engine/configure-windows/enable-the-lock-pages-in-memory-option-windows