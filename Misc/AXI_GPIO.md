#  Xilinx Zynq-7000 AP SoC ZC706 Evaluation Kit (AXI-GPIO)

![Design Default view](AXI_GPIO_default.png)

![Design Interface View](AXI_GPIO_interface.png)

# Standalone_ps7_cortexa9_0

```c
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#define TARGET_GPIO_REG (*((volatile uint32_t *)0x41200000))

void write_to_GPIO(uint32_t value)
{
	TARGET_GPIO_REG = value;
}

const uint32_t DELAY_COUNT = 50000000;

void spin_delay(uint32_t count)
{
    while (count--);
}

int main()
{
	int vector;

    init_platform();

    print("Target address 0x4120_0000 will be periodically written with 0xF and 0x0...\n\r");

    while (1) {
    	 vector = 0xF;
         xil_printf("Wrote to 0x4120_0000: 0x%X\n\r", vector);
         write_to_GPIO(vector);
         spin_delay(DELAY_COUNT);


         vector = 0x0;
         xil_printf("Wrote to 0x4120_0000: 0x%X\n\r", vector);
         write_to_GPIO(vector);
         spin_delay(DELAY_COUNT);
    }

    cleanup_platform();
    return 0;
}
```

UART:
```
Target address 0x4120_0000 will be periodically written with 0xF and 0x0...
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
Wrote to 0x4120_0000: 0x0
Wrote to 0x4120_0000: 0xF
```
