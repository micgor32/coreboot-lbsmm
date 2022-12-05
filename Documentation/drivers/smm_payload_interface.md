# coreboot-mediated payload SMM initialisation
**Purpose**: Enable UEFI Authenticated Variables to utilise the
dependent features. It is critical that authentication and write be
performed together in a trusted/trustworthy environment, or arbitrary
data could be written. Therefore, only a `SetVariable()` call should
result in writes into the variable store. Since the current SMMSTORE
wasn't designed for this, we enable a full variable stack using EDK2.

Then, Secure Boot can be enabled to the user's desired configuration.
Additionally, it's possible to dynamically enable features such as BIOS
Lock in a setup menu, rather than at compile-time, using separate work
to bringup UEFI variables in coreboot as another option backend.
Specifically, these settings shall not have the runtime attribute. Now,
variables can only be written using functions in the UEFI environment.
In contrast with the CMOS backend, which is always writeable, there is
more versatility and security to UEFI variables.

## Implementation details
### Addressing challenges of S0 boot
- One payload should support many silicon generations, with differing register definitions
  - **Solution**: coreboot produces a new CBTABLE with register definitions
- Payload does not know SPI layout
  - **Solution**: coreboot produces a new CBTABLE with offset of variable region

### Addressing challenges of S3 boot
- Payload execution is skipped, but at least lockdown must be performed
  - The first 4K of SMRAM is reserved as a data structure, containing the payload's chosen SWSMI number
  - Given that SMI entry address value is valid, a SMI IPI is sufficient to relocate the CPU's SMBASE to existing TSEG
    - **Background**: Unlike coreboot SMI handlers, EDK2 handlers shall maintain their state through S3. They are not reinitialised
  - **Solution**: coreboot performs this minimal SMM relocation (without installation), then triggers the payload SWSMI

### Design flow
**Cold/Warm boot**
1. `mp_init_with_smm()` skips SMM initialisation
2. coreboot produces required data
3. UefiPayloadPkg/CbParseLib receives data, produces HOBs for EDK2
4. UefiPayloadPkg/{SmmAccessDxe,SmmControlDxe} protocols support PiSmmIpl to initialise SMM. SMM dispatcher begins
  - UefiPayloadPkg/BlSupportSmm locates reserved 4K of SMRAM, produces HOB struct with SWSMI number here and CPU SMBASEs
  - UefiPayloadPkg/FvbRuntimeDxe produces a variable store with Authenticated Variables support

**S3 suspend-resume**
1. `mp_init_with_smm()` performs SMBASE relocation
  - Uses coreboot driver to retrieve CPU SMBASEs
2. coreboot driver retrieves SWSMI number to trigger from reserved 4K communication region
3. Payload SWSMI handler performs SMM lockdown. Other SMM protocols have remained resident, such as the variable-stack

## Progress
**Current stage**: Complete, successful. Requires further testing on more platforms.
- **TODO**: Optionally test new support for MCH SMRAMC register to support older platforms.

**Instructions to reproduce**:
- **coreboot cherry-picks**: [CB:70376](https://review.coreboot.org/c/coreboot/+/70376) [CB:70377](https://review.coreboot.org/c/coreboot/+/70377), [CB:70378](https://review.coreboot.org/c/coreboot/+/70378)
- **EDK2 cherry-picks**: [4378ab1](https://github.com/benjamindoron/edk2/commit/4378ab15c879213f10bb06252404f55f50f54648), [a596060](https://github.com/benjamindoron/edk2/commit/a5960607ddfdd0430c3ba20aa6d8b87cfca58cb0), [ae11732](https://github.com/benjamindoron/edk2/commit/ae11732dcf12cf72913be3bc8e900974c6ffe8f6), [e0a1921](https://github.com/benjamindoron/edk2/commit/e0a1921151eb04da4230197dfa67a5ea5af1c763)
- Build image with `CONFIG_SMM_PAYLOAD_INTERFACE=y`, `CONFIG_PAYLOAD_EDK2=y`, `CONFIG_EDK2_CUSTOM_BUILD_PARAMS="-D SMM_SUPPORT=TRUE -D VARIABLE_SUPPORT=SPI"`
