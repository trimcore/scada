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
* 2 cores (or 1 core with HT/SMT), *logical processors* in Windows kernel nomenclature  
  *Note this especially when allocating resources for virtual guests running the software.*

1 GB of RAM
* In-memory design requires direct workload including system overhead must fit the memory
* The intermediate workload is always locked into physical memory (see [Additional Privileges](privileges.md) on how to enable)  
  *While the software will run without the privilege, this is unsupported scenario with limited performance.*

## Software
### Operating System

Windows NT 6.3 kernel-based or newer.  
Testing is currently performed on following SKUs:

* [Windows Server](https://www.microsoft.com/cs-cz/windows-server) 2012 R2, 2016, 2019 and 2022
* [Hyper-V Server](https://www.microsoft.com/en-us/evalcenter/evaluate-hyper-v-server-2019) 2016 and 2019
* [Azure Stack HCI](https://azure.microsoft.com/en-us/products/azure-stack/hci/) 17784 and 20348
* Windows 8.1, Windows 10 LTSC, Windows 11 - **no support will be provided for deployments on client SKUs**

**NOTE:** Consider strong licensing restrictions and limitations that apply to non-Windows SKUs.

*Only system/software administrative tools can be run directly on Hyper-V Server.*  
*On Azure Stack HCI the license allows to run software only if the particular usage falls within terms of Software Defined Networking*

Consult EULA!

### [Microsoft Visual C++ runtime redistributables](https://github.com/trimcore/redist/tree/main/msvcrt)
* [14.32 x86-64](https://github.com/trimcore/redist/raw/main/msvcrt/x86-64/msvc_redist_14.32.31326_x64.exe)
* [14.32 x86-32](https://github.com/trimcore/redist/raw/main/msvcrt/x86-32/msvc_redist_14.32.31326_x86.exe)
* [14.32 AArch64](https://github.com/trimcore/redist/raw/main/msvcrt/AArch64/msvc_redist_14.32.31326_arm64.exe)

### Installation

Use provided [installer.exe](https://github.com/trimcore/scada/tree/master/exe/installer) tool, to deploy the software as a service.

While the software, for debugging purposes, can be started in console, as an application, this mode offers limited functionality.
For commercial operation and production, unless specially licensed, the software is supported only when run as a service.

### TRIMCORE Software Management Service

*Optional*

For commercial operation, unless specially licensed, the software requires [TRIMCORE Software Management
Service](https://github.com/trimcore/service) installed and running with licensing functions enabled.

