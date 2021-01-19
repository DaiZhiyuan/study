# Power State Coordination Interface (PSCI)
Firmware implementing the PSCI functions described in ARM document number
ARM DEN 0022A ("Power State Coordination Interface System Software on ARM
processors") can be used by Linux to initiate various CPU-centric power
operations.

Issue A of the specification describes functions for CPU suspend, hotplug
and migration of secure software.

Functions are invoked by trapping to the privilege level of the PSCI
firmware (specified as part of the binding below) and passing arguments
in a manner similar to that specified by AAPCS:
	 r0		=> 32-bit Function ID / return value
	{r1 - r3}	=> Parameters

> Note that the immediate field of the trapping instruction must be set
to #0.

```
Main node required properties:
 - compatible    : should contain at least one of:
				 * "arm,psci" : for implementations complying to PSCI versions prior to
					0.2. For these cases function IDs must be provided.
				 * "arm,psci-0.2" : for implementations complying to PSCI 0.2. Function
					IDs are not required and should be ignored by an OS with PSCI 0.2
					support, but are permitted to be present for compatibility with
					existing software when "arm,psci" is later in the compatible list.
				 * "arm,psci-1.0" : for implementations complying to PSCI 1.0.
 - method        : The method of calling the PSCI firmware. Permitted
                   values are:
                   "smc" : SMC #0, with the register assignments specified
		           in this binding.
                   "hvc" : HVC #0, with the register assignments specified
		           in this binding.
Main node optional properties:
 - cpu_suspend   : Function ID for CPU_SUSPEND operation
 - cpu_off       : Function ID for CPU_OFF operation
 - cpu_on        : Function ID for CPU_ON operation
 - migrate       : Function ID for MIGRATE operation
Device tree nodes that require usage of PSCI CPU_SUSPEND function (ie idle
state nodes, as per bindings in [1]) must specify the following properties:
- arm,psci-suspend-param
		Usage: Required for state nodes[1] if the corresponding
                       idle-states node entry-method property is set
                       to "psci".
		Value type: <u32>
		Definition: power_state parameter to pass to the PSCI
			    suspend call.
Example:
Case 1: PSCI v0.1 only.
	psci {
		compatible	= "arm,psci";
		method		= "smc";
		cpu_suspend	= <0x95c10000>;
		cpu_off		= <0x95c10001>;
		cpu_on		= <0x95c10002>;
		migrate		= <0x95c10003>;
	};
Case 2: PSCI v0.2 only
	psci {
		compatible	= "arm,psci-0.2";
		method		= "smc";
	};
Case 3: PSCI v0.2 and PSCI v0.1.
	A DTB may provide IDs for use by kernels without PSCI 0.2 support,
	enabling firmware and hypervisors to support existing and new kernels.
	These IDs will be ignored by kernels with PSCI 0.2 support, which will
	use the standard PSCI 0.2 IDs exclusively.
	psci {
		compatible = "arm,psci-0.2", "arm,psci";
		method = "hvc";
		cpu_on = < arbitrary value >;
		cpu_off = < arbitrary value >;
		...
	};
```

## Mandatory and optional functions for a given version of PSCI

```
FUNCTION            PSCI 0.2
PSCI_VERSION        Mandatory        0x84000000     Return the version of PSCI implemented.
CPU_SUSPEND         Mandatory        0xc4000001     Suspend execution on a core or higher-level topology node.
CPU_OFF             Mandatory        0x84000002     Power down the calling core. 
CPU_ON              Mandatory        0xc4000003     Power up a core.
AFFINITY_INFO       Mandatory        0xc4000004     Enable the caller to request status of an affinity instance.
SYSTEM_OFF          Mandatory        0x84000008     Shut down the system. 
SYSTEM_RESET        Mandatory        0x84000009     Reset the system. 
```

FT2000plus DTS:
```
psci {
	compatible = "arm,psci-1.0";
	method = "smc";
	cpu_suspend = <0xc4000001>;
	cpu_off = <0x84000002>;
	cpu_on = <0xc4000003>;
	sys_poweroff = <0x84000008>;
	sys_reset = <0x84000009>;
};
```

Kernel Messages:
```
psci: probing for conduit method from ACPI.
psci: PSCIv1.0 detected in firmware.
psci: Using standard PSCI v0.2 function IDs
psci: MIGRATE_INFO_TYPE not supported.
psci: SMC Calling Convention v1.0
```

SMC Calling:
```
For versions using 64-bit parameters, the arguments are passed in X0 to
X3, with return values in X0 or W0, depending on the return parameter size.
In line with the SMC Calling Conventions, the immediate value used with an
SMC (or HVC) instruction must be 0.
```

```
"FACP"    [Fixed ACPI Description Table (FADT)]

     Value to cause reset : 00
ARM Flags (decoded below) : 0001
           PSCI Compliant : 1
    Must use HVC for PSCI : 0
```

> MSI Not Supported
> PCIe ASPM Not Supported


> The OSPM has to
provide the necessary power management software infrastructure to
determine the correct choice of state.