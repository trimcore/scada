*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - HW/SW Requirements

## Hardware

CPU ISA
* x86-64 - SSE 4.1/CMPXCHG16B
  * Intel Core/[Penryn](https://en.wikipedia.org/wiki/Penryn_(microarchitecture)) (2008+)
  * AMD Bulldozer/Jaguar (2011+)
* x86-32 - SSE 4.1
* AArch64 - ARM v8 CRC32, AES, SHA1, FMAC and NEON vector extensions
  * Qualcomm Snapdragon 835 (2017+)

2 hardware threads
* 2 cores or 1 core with HT/SMT), *logical processors* in Windows kernel nomenclature  
  *Note this especially when allocating resources for Virtual Machine*

1 GB of RAM
* TBD: lock, 2MB/1GB pages, see [Additional Privileges](privileges.md) on how to enable

## Software
### Operating System

NT 6.2 kernel based or newer.  
Testing is currently performed on following SKUs:

* [Windows Server](https://www.microsoft.com/cs-cz/windows-server) 2012 R2, 2016 and 2019
* Windows 8.1, Windows 10 - **no support will be provided for deployments on client SKUs**
* [Hyper-V Server](https://www.microsoft.com/en-us/evalcenter/evaluate-hyper-v-server-2019) 2016 and 2019
* [Azure Stack HCI](https://azure.microsoft.com/en-us/products/azure-stack/hci/)

*NOTE: Consider strong licensing restrictions and limitations that apply to non-Windows SKUs.*

### [Microsoft Visual C++ runtime redistributables](https://github.com/trimcore/scada/tree/master/redist)
* [14.26 x86-64](https://github.com/trimcore/scada/raw/master/redist/msvc_redist_x64_14.26.exe)
* [14.26 x86-32](https://github.com/trimcore/scada/raw/master/redist/msvc_redist_x84_14.26.exe)
* [14.26 AArch64](https://github.com/trimcore/scada/raw/master/redist/msvc_redist_arm64_14.26.exe)

### TRIMCORE Software Management Service

*Optional*

For regular commercial operation, unless specially licensed, the software requires TRIMCORE Software Management
Service installed and running with Licensing functions enabled.

## License

TBD


