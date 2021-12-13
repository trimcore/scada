*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - Configuration - Address

TBD: General

The currently supported addresses are IPv4 and IPv6 only.

## Syntax

    <IP[v4]|IPv6> **address**[:**port**]

### IP Addresses

* TBD

## Cell data

    cell->value.type == Scada::Cell::Type::Address;
    cell->value.width == AF_INET; // AF_INET6
    cell->value.count == 1;
    
    cell->value.data.local;

## Expressions

* TBD

---

« [Configuration](cfg.md)
