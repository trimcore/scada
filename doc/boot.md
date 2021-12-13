*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Bootstrap and conventions

...

## Runtime Environment

* /system/heartbeat
* /system/log applied (and further reflected)
* /system/debug/database/dump

## Directory initialization

* **/settings/models/\<MODEL\>** instantiated to **/models/\<MODEL\>**
* **/settings/manifolds/\<MANIFOLD\>** instantiated to **/manifolds/\<MANIFOLD\>**
  * manifold processes dispatched immediately

* instantiations during initialization do not perform [expression remapping](cfg-expr.md)

## Manifold processes and Module loading

...

## Shutdown

* manifolds teardown
* performance reports
* database dump
