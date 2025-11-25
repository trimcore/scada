*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM - HW/SW Requirements

On startup, the software will check for required CPU/OS features and refuse to continue if the minimal
requirements are not met. It will also report on presence of features that might be required by certain
advanced features or future versions.

## Hardware

CPU ISA
* x86-64 - AVX2 (and consequently SSE 4.2/CRC32/CMPXCHG16B)
  * Intel [Haswell](https://en.wikipedia.org/wiki/Haswell_\(microarchitecture\)) architecture (2013+)
  * AMD Excavator (2015+)
* x86-32 - SSE 4.2 (CRC32) and POPCNT
  * Intel [Nehalem](https://en.wikipedia.org/wiki/Nehalem_\(microarchitecture\)) architecture (2008+)
  * AMD Bulldozer/Jaguar (2011+)
* AArch64 - ARMv8.2-A (CRC32, AES, LSE and NEON vector extensions) + LRCPC instructions
  * Qualcomm Snapdragon 850 (2018+)

2 hardware threads
* 2 cores (or 1 core with HT/SMT), *logical processors* in Windows kernel nomenclature  
  *Note this especially when allocating resources for virtual guests running the software.*

1 GB of RAM
* In-memory design requires direct workload including system overhead must fit the memory
* The intermediate workload is always locked into physical memory (see [Additional Privileges](privileges.md) on how to enable)  
  *While the software will run without the privilege, this is unsupported scenario with limited performance.*
* NOTE: Assign memory in multiples of 1 GB for best performance

## Software
### Operating System

The SCADA Software at minimum requires Windows OS based on NT kernel version **6.3**  
That corresponds to SKUs **Windows Server 2012 R2**, optionally **Windows 8.1**, and later.

Testing is currently performed on following LTSC SKUs:

* [Windows Server](https://www.microsoft.com/cs-cz/windows-server) 2012 R2, 2016, 2019, 2022 and 2025
* [Hyper-V Server](https://www.microsoft.com/en-us/evalcenter/evaluate-hyper-v-server-2019) 2016 and 2019
* [Azure Stack HCI](https://azure.microsoft.com/en-us/products/azure-stack/hci/) 20349, 25398 and 26100
* Windows 8.1, Windows 10 LTSC, Windows 11 - **no support will be provided for deployments on client SKUs**

**NOTE:** Consider strong licensing restrictions and limitations that apply to non-Windows Server SKUs.

*Only system/software administrative tools can be run directly on Hyper-V Server.*  
*On Azure Stack HCI the license allows to run software only if the particular usage falls within terms of Software Defined Networking.*

Consult EULA!

### [Microsoft Visual C++ runtime redistributables](https://github.com/trimcore/redist/tree/main/msvcrt)
* [14.44 x86-64](https://github.com/trimcore/redist/raw/main/msvcrt/x86-64/msvc_redist_14.44.35112_x64.exe)
* [14.44 x86-32](https://github.com/trimcore/redist/raw/main/msvcrt/x86-32/msvc_redist_14.44.35112_x86.exe)
* [14.44 AArch64](https://github.com/trimcore/redist/raw/main/msvcrt/AArch64/msvc_redist_14.44.35112_arm64.exe)

### Installation

Use provided [installer.exe](https://github.com/trimcore/scada/tree/master/exe/installer) tool, to deploy the software as a service.

While the software, for debugging purposes, can be started in console, as an application, this mode offers limited functionality.
For commercial operation and production, unless specially licensed, the software is supported only when run as a service.

### TRIMCORE Software Management Service

*Optional*

For commercial operation, unless specially licensed, the software requires [TRIMCORE Software Management
Service](https://github.com/trimcore/service) installed and running with licensing functions enabled.
