
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

# DEMO

hwcap.c
```
#include <stdio.h>
#include <sys/auxv.h>
#include <asm/hwcap.h>

#define OPTIONS(flag) (hwcaps & flag?"\033[1;32;40msupport\033[0m":"\033[1;31;40mnonsupport\033[0m")

int main(void)
{
        long hwcaps= getauxval(AT_HWCAP);


        printf("   [cpuid] : Some CPU ID registers readable at user-level.                                  (%s)\n", OPTIONS(HWCAP_CPUID));
        printf(" [atomics] : Large System Extensions.                                                       (%s)\n", OPTIONS(HWCAP_ATOMICS));
        printf(" [evtstrm] : Generic timer is configured to generate 'events' at frequency of about 100KHz. (%s)\n", OPTIONS(HWCAP_EVTSTRM));
        printf("    [fphp] : Half-precision floating point.                                                 (%s)\n", OPTIONS(HWCAP_FPHP));
        printf("      [fp] : Single-precision and double-precision floating point.                          (%s)\n", OPTIONS(HWCAP_FP));
        printf("   [jscvt] : Javascript-style double->int convert.                                          (%s)\n", OPTIONS(HWCAP_JSCVT));
        printf("   [lrcpc] : Weaker release consistency.                                                    (%s)\n", OPTIONS(HWCAP_LRCPC));
        printf("   [dcpop] : Data cache clean to Point of Persistence.                                      (%s)\n", OPTIONS(HWCAP_DCPOP));
        printf("   [crc32] : CRC32/CRC32C instructions.                                                     (%s)\n", OPTIONS(HWCAP_CRC32));
        printf("    [sha1] : SHA-1 instructions.                                                            (%s)\n", OPTIONS(HWCAP_SHA1));
        printf("    [sha2] : SHA-2 instructions.                                                            (%s)\n", OPTIONS(HWCAP_SHA2));
        printf("    [sha3] : SHA-3 instructions.                                                            (%s)\n", OPTIONS(HWCAP_SHA3));
        printf("  [sha512] : SHA512 instructions.                                                           (%s)\n", OPTIONS(HWCAP_SHA512));
        printf("     [aes] : AES instructions.                                                              (%s)\n", OPTIONS(HWCAP_AES));
        printf("     [sm3] : SM3 instructions.                                                              (%s)\n", OPTIONS(HWCAP_SM3));
        printf("     [sm4] : SM4 instructions.                                                              (%s)\n", OPTIONS(HWCAP_SM4));
        printf("   [asimd] : Advanced single-instruction-multiple-data.                                     (%s)\n", OPTIONS(HWCAP_ASIMD));
        printf("   [pmull] : Polynomial Multiply Long instructions.                                         (%s)\n", OPTIONS(HWCAP_PMULL));
        printf(" [asimddp] : SIMD Dot Product.                                                              (%s)\n", OPTIONS(HWCAP_ASIMDDP));
        printf("[asimdrdm] : Rounding Double Multiply Accumulate/Subtract.                                  (%s)\n", OPTIONS(HWCAP_ASIMDRDM));
        printf("     [sve] : Scalable Vector Extension.                                                     (%s)\n", OPTIONS(HWCAP_SVE));

        return 0;
}
```

```
[root@jerrydai ~]# gcc hwcap.c -o cpu_features
[root@jerrydai ~]# ./cpu_features
   [cpuid] : Some CPU ID registers readable at user-level.                                  (support)
 [atomics] : Large System Extensions.                                                       (nonsupport)
 [evtstrm] : Generic timer is configured to generate 'events' at frequency of about 100KHz. (support)
    [fphp] : Half-precision floating point.                                                 (nonsupport)
      [fp] : Single-precision and double-precision floating point.                          (support)
   [jscvt] : Javascript-style double->int convert.                                          (nonsupport)
   [lrcpc] : Weaker release consistency.                                                    (nonsupport)
   [dcpop] : Data cache clean to Point of Persistence.                                      (nonsupport)
   [crc32] : CRC32/CRC32C instructions.                                                     (support)
    [sha1] : SHA-1 instructions.                                                            (nonsupport)
    [sha2] : SHA-2 instructions.                                                            (nonsupport)
    [sha3] : SHA-3 instructions.                                                            (nonsupport)
  [sha512] : SHA512 instructions.                                                           (nonsupport)
     [aes] : AES instructions.                                                              (nonsupport)
     [sm3] : SM3 instructions.                                                              (nonsupport)
     [sm4] : SM4 instructions.                                                              (nonsupport)
   [asimd] : Advanced single-instruction-multiple-data.                                     (support)
   [pmull] : Polynomial Multiply Long instructions.                                         (nonsupport)
 [asimddp] : SIMD Dot Product.                                                              (nonsupport)
[asimdrdm] : Rounding Double Multiply Accumulate/Subtract.                                  (nonsupport)
     [sve] : Scalable Vector Extension.                                                     (nonsupport)
```