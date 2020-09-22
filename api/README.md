*TRIM CORE SOFTWARE s.r.o.*
# ∆ SCADA SYSTEM API

**IMPORTANT**: API version 1 is currently under active development and not to be considered stable.
Until official public release the ABI exports may disappear or their behavior change.

All API functions made accessible by these header files are confined to namespace `Scada`

## ABI

ABI symbols are confined to `Scada::ABI` namespace.
ABI symbols are exported by [Scada.exe](../exe), with Scada.lib available for modules to link against in order to import them.
The modules are loaded into Scada.exe and OS loader resolves used symbols.

TBD: version

## General API

* Scada::GetManifoldName
* Scada::ABI::ApiVersion
* Scada::ABI::ApiDeclareSupport
* Scada::ABI::ApiRequireSupport

## Directory API

### General

* Scada::Directory::Construct
* Scada::Directory::Instantiate
* Scada::Directory::Listen
* Scada::Directory::ForEachCell
* Scada::Directory::ForEachSub

### Cell Access

* Scada::Object <T> template
* Scada::Access
* Scada::ABI::Api1CellCreate
* Scada::ABI::Api1CellCreateTemplate
* ...

## Data

* Scada::ABI::Api1CellRead
* Scada::ABI::Api1CellGetID
* Scada::ABI::Api1CellGetDataType
* Scada::ABI::Api1[S|G]et[xxx]

## Dependencies

TBD: concept of listening

* Scada::ABI::Api1CellListen
* Scada::ABI::Api1DirListenCell
* Scada::ABI::Api1DirListenSub

## Helpers

* Scada::ABI::Api1DirPathBreak[A|W][z|sv]

