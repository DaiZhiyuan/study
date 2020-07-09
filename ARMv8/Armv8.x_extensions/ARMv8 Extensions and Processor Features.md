
# ARMv8 Extensions and Processor Features

ARMv8 has many versions (ARMv8.1, etc), which define mandatory and optional features. 
The Linux kernel exposes the presence of some of these features via hwcaps. 
These values are displayed in /proc/cpuinfo.

Name | Version | Feature Supported
---|---|---
fp|-|Single-precision and double-precision floating point.
asimd|-|Advanced SIMD.
evtstrm|N/A|Generic timer is configured to generate "events" at frequency of about 100KHz.
aes|-|AES instructions (AESE, etc)
pmull|-|Polynomial Multiply Long instructions (PMULL/PMULL2)
sha1|-|SHA-1 instructions (SHA1C, etc)
sha2|-|SHA-2 instructions (SHA256H, etc)
crc32|[ARMv8.0], ARMv8.1 ... | CRC32/CRC32C instructions
atomics|ARMv8.1 ...	|Large System Extensions (LSE) - (CAS/SWP/LD[op])
fphp| ARMv8.2-FP16 | Half-precision floating point.
cpuid |N/A |Some CPU ID registers readable at user-level.
asimdrdm |ARMv8.1 | Rounding Double Multiply Accumulate/Subtract (SQRDMLAH/SQRDMLSH)
jscvt| ARMv8.3 | Javascript-style double->int convert (FJCVTZS)
lrcpc | ARMv8.3 | Weaker release consistency (LDAPR, etc)
dcpop | ARMv8.2 | Data cache clean to Point of Persistence (DC CVAP)
sha3 | ARMv8.2-SHA | SHA-3 instructions (EOR3, RAX1, XAR, BCAX)
sm3 |ARMv8.2-SM	| SM3 instructions
sm4	| ARMv8.2-SM | SM4 instructions
asimddp | ARMv8.2-DotProd | SIMD Dot Product
sha512 | ARMv8.2-SHA | SHA512 instructions
sve	| ARMv8.2-SVE | Scalable Vector Extension (SVE)

# ARM64 ELF hwcaps(exposed in AT_HWCAP)



HWCAP_FP

    Functionality implied by ID_AA64PFR0_EL1.FP == 0b0000.

HWCAP_ASIMD

    Functionality implied by ID_AA64PFR0_EL1.AdvSIMD == 0b0000.

HWCAP_EVTSTRM

    The generic timer is configured to generate events at a frequency of
    approximately 100KHz.

HWCAP_AES

    Functionality implied by ID_AA64ISAR0_EL1.AES == 0b0001.

HWCAP_PMULL

    Functionality implied by ID_AA64ISAR0_EL1.AES == 0b0010.

HWCAP_SHA1

    Functionality implied by ID_AA64ISAR0_EL1.SHA1 == 0b0001.

HWCAP_SHA2

    Functionality implied by ID_AA64ISAR0_EL1.SHA2 == 0b0001.

HWCAP_CRC32

    Functionality implied by ID_AA64ISAR0_EL1.CRC32 == 0b0001.

HWCAP_ATOMICS

    Functionality implied by ID_AA64ISAR0_EL1.Atomic == 0b0010.

HWCAP_FPHP

    Functionality implied by ID_AA64PFR0_EL1.FP == 0b0001.

HWCAP_ASIMDHP

    Functionality implied by ID_AA64PFR0_EL1.AdvSIMD == 0b0001.

HWCAP_CPUID

    EL0 access to certain ID registers is available, to the extent
    described by Documentation/arm64/cpu-feature-registers.txt.

    These ID registers may imply the availability of features.

HWCAP_ASIMDRDM

    Functionality implied by ID_AA64ISAR0_EL1.RDM == 0b0001.

HWCAP_JSCVT

    Functionality implied by ID_AA64ISAR1_EL1.JSCVT == 0b0001.

HWCAP_FCMA

    Functionality implied by ID_AA64ISAR1_EL1.FCMA == 0b0001.

HWCAP_LRCPC

    Functionality implied by ID_AA64ISAR1_EL1.LRCPC == 0b0001.

HWCAP_DCPOP

    Functionality implied by ID_AA64ISAR1_EL1.DPB == 0b0001.

HWCAP2_DCPODP

    Functionality implied by ID_AA64ISAR1_EL1.DPB == 0b0010.

HWCAP_SHA3

    Functionality implied by ID_AA64ISAR0_EL1.SHA3 == 0b0001.

HWCAP_SM3

    Functionality implied by ID_AA64ISAR0_EL1.SM3 == 0b0001.

HWCAP_SM4

    Functionality implied by ID_AA64ISAR0_EL1.SM4 == 0b0001.

HWCAP_ASIMDDP

    Functionality implied by ID_AA64ISAR0_EL1.DP == 0b0001.

HWCAP_SHA512

    Functionality implied by ID_AA64ISAR0_EL1.SHA2 == 0b0010.

HWCAP_SVE

    Functionality implied by ID_AA64PFR0_EL1.SVE == 0b0001.

HWCAP2_SVE2

    Functionality implied by ID_AA64ZFR0_EL1.SVEVer == 0b0001.

HWCAP2_SVEAES

    Functionality implied by ID_AA64ZFR0_EL1.AES == 0b0001.

HWCAP2_SVEPMULL

    Functionality implied by ID_AA64ZFR0_EL1.AES == 0b0010.

HWCAP2_SVEBITPERM

    Functionality implied by ID_AA64ZFR0_EL1.BitPerm == 0b0001.

HWCAP2_SVESHA3

    Functionality implied by ID_AA64ZFR0_EL1.SHA3 == 0b0001.

HWCAP2_SVESM4

    Functionality implied by ID_AA64ZFR0_EL1.SM4 == 0b0001.

HWCAP_ASIMDFHM

   Functionality implied by ID_AA64ISAR0_EL1.FHM == 0b0001.

HWCAP_DIT

    Functionality implied by ID_AA64PFR0_EL1.DIT == 0b0001.

HWCAP_USCAT

    Functionality implied by ID_AA64MMFR2_EL1.AT == 0b0001.

HWCAP_ILRCPC

    Functionality implied by ID_AA64ISAR1_EL1.LRCPC == 0b0010.

HWCAP_FLAGM

    Functionality implied by ID_AA64ISAR0_EL1.TS == 0b0001.

HWCAP_SSBS

    Functionality implied by ID_AA64PFR1_EL1.SSBS == 0b0010.

HWCAP_PACA

    Functionality implied by ID_AA64ISAR1_EL1.APA == 0b0001 or
    ID_AA64ISAR1_EL1.API == 0b0001, as described by
    Documentation/arm64/pointer-authentication.txt.

HWCAP_PACG

    Functionality implied by ID_AA64ISAR1_EL1.GPA == 0b0001 or
    ID_AA64ISAR1_EL1.GPI == 0b0001, as described by
    Documentation/arm64/pointer-authentication.txt.

