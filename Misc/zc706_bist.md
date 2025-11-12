#  Xilinx Zynq-7000 AP SoC ZC706 Evaluation Kit (BIST)

```
********************************************************
********************************************************
**    Xilinx Zynq-7000 AP SoC ZC706 Evaluation Kit    **
********************************************************
********************************************************
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
1

********************************************************
********************************************************
**                 ZC706 - UART Test                  **
********************************************************
********************************************************
Testing UART
115200,8,N,1
Hello world!
UART Test Passed

Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
2

********************************************************
********************************************************
**     ZC706 - IIC EEPROM Test                        **
********************************************************
********************************************************
WriteBuffer[00]=0x10  ReadBuffer[00]=0x10
WriteBuffer[01]=0x11  ReadBuffer[01]=0x11
WriteBuffer[02]=0x12  ReadBuffer[02]=0x12
WriteBuffer[03]=0x13  ReadBuffer[03]=0x13
WriteBuffer[04]=0x14  ReadBuffer[04]=0x14
WriteBuffer[05]=0x15  ReadBuffer[05]=0x15
WriteBuffer[06]=0x16  ReadBuffer[06]=0x16
WriteBuffer[07]=0x17  ReadBuffer[07]=0x17
WriteBuffer[08]=0x18  ReadBuffer[08]=0x18
WriteBuffer[09]=0x19  ReadBuffer[09]=0x19
WriteBuffer[10]=0x1A  ReadBuffer[10]=0x1A
WriteBuffer[11]=0x1B  ReadBuffer[11]=0x1B
WriteBuffer[12]=0x1C  ReadBuffer[12]=0x1C
WriteBuffer[13]=0x1D  ReadBuffer[13]=0x1D
WriteBuffer[14]=0x1E  ReadBuffer[14]=0x1E
WriteBuffer[15]=0x1F  ReadBuffer[15]=0x1F
Successfully ran IIC EEPROM Interrupt Example Test

********************************************************
********************************************************
** IIC Read ADV7511 Programmable Clock Interrupt Test **
********************************************************
********************************************************
Register 00 = 0x14
Successfully ran IIC ADV7511 Interrupt Example Test

********************************************************
********************************************************
**          IIC Read TCA6416A Interrupt Test          **
********************************************************
********************************************************
Register 00 = 0x00
Successfully ran IIC TCA6416A Interrupt Example Test

********************************************************
********************************************************
** IIC Read RTC8564 Programmable Clock Interrupt Test **
********************************************************
********************************************************
IIC Read RTC8564 Programmable Clock Interrupt Example Test
Register 00 = 0x08
Register 01 = 0x80
Register 02 = 0x91
Register 03 = 0xA0
Register 04 = 0x82
Register 05 = 0x85
Register 06 = 0x92
Register 07 = 0x08
Register 08 = 0x01
Register 09 = 0x80
Register 10 = 0x88
Register 11 = 0xA0
Register 12 = 0xA7
Register 13 = 0x90
Register 14 = 0x13
Register 15 = 0x20
Successfully ran IIC RTC8564 Interrupt Example Test

********************************************************
********************************************************
**  IIC Read Si570 Programmable Clock Interrupt Test  **
********************************************************
********************************************************
IIC Read Si570 Programmable Clock Interrupt Example Test
Register 07 = 0x01
Register 08 = 0xC2
Register 09 = 0xBB
Register 10 = 0xBD
Register 11 = 0x6B
Register 12 = 0xFE
Register 13 = 0x07
Register 14 = 0xC2
Register 15 = 0xC0
Register 16 = 0x00
Register 17 = 0x00
Register 18 = 0x00
Successfully ran IIC Si570 Interrupt Example Test

********************************************************
********************************************************
********** IIC Read DDR3 Interrupt Test ****************
********************************************************
********************************************************
Register 00 = 0x92
Register 01 = 0x11
Register 02 = 0x0B
Register 03 = 0x03
Register 04 = 0x02
Register 05 = 0x11
Register 06 = 0x00
Register 07 = 0x01
Register 08 = 0x03
Register 09 = 0x11
Register 10 = 0x01
Register 11 = 0x08
Register 12 = 0x0A
Register 13 = 0x00
Register 14 = 0xFE
Register 15 = 0x00
Successfully ran IIC DDR3 Interrupt Example Test

********************************************************
********************************************************
** IIC Read si5324 Interrupt Test **
********************************************************
********************************************************
Register 00 = 0x01
Register 01 = 0x82
Register 02 = 0x00
Register 03 = 0x00
Register 04 = 0x0F
Register 05 = 0xFF
Register 06 = 0x00
Register 07 = 0x00
Register 08 = 0x00
Register 09 = 0x00
Register 10 = 0x00
Register 11 = 0x00
Register 12 = 0x00
Register 13 = 0x00
Register 14 = 0x00
Register 15 = 0x00
Successfully ran IIC si5324 Interrupt Example Test
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
3

********************************************************
********************************************************
**                 ZC706 - Timer Test                 **
********************************************************
********************************************************
TTC Interrupt Example Test
Successfully ran TTC Interrupt Example Test
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
4

********************************************************
********************************************************
**               ZC706 SCU / GIC Test                 **
********************************************************
********************************************************
ScuGicSelfTestExample PASSED
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
5

********************************************************
********************************************************
**                 ZC706 - DCFG Test                  **
********************************************************
********************************************************
DcfgSelfTestExample PASSED
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
6

********************************************************
********************************************************
**            ZC706 - DDR3 PS Memory TEST             **
********************************************************
********************************************************
NOTE: This application runs with D-Cache disabled.
As a result, cacheline requests will not be generated
Testing memory region: ps7_ddr_0
    Memory Controller: ps7_ddr
         Base Address: 0x00200000
                 Size: 0x3fd00000 bytes
          32-bit test: PASSED!
          16-bit test: PASSED!
           8-bit test: PASSED!
Testing memory region: ps7_ram_1
    Memory Controller: ps7_ram
         Base Address: 0xffff0000
                 Size: 0x0000fe00 bytes
          32-bit test: PASSED!
          16-bit test: PASSED!
           8-bit test: PASSED!
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
7

********************************************************
********************************************************
**               ZC706 Interrupt Tests                **
********************************************************
********************************************************
ScuGic Interrupt Setup PASSED

 Running Interrupt Test  for ps7_ethernet_0...
EmacPsDmaIntrExample PASSED

 Running Interrupt Test  for ps7_scutimer_0...
ScuTimerIntrExample PASSED

 Running Interrupt Test  for ps7_scuwdt_0...
ScuWdtIntrExample PASSED
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
8

********************************************************
********************************************************
**             ZC706 Watchdog Timer Test              **
********************************************************
********************************************************
WdtPsSelfTestExample PASSED
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
9

********************************************************
********************************************************
**                  ZC706 - LED Test                  **
********************************************************
********************************************************

Running GpioOutputExample() for axi_gpio_led...
GpioOutputExample PASSED.
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
A

********************************************************
********************************************************
**            ZC706 - GPIO DIP Switch Test            **
********************************************************
********************************************************

Running GpioOutputExample() for AXI GPIO DIP Switches...
GpioInputExample PASSED. Read data:0x0
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
B

********************************************************
********************************************************
**        ZC706 - GPIO Push Button Switch Test        **
********************************************************
********************************************************

Running GpioOutputExample() for AXI GPIO Push Buttons...
GpioInputExample PASSED. Read data:0x0
Press any key to return to main menu
Choose Feature to Test:
1: PS UART Test
2: PS IIC Test
3: PS TIMER Test
4: PS SCUGIC Test
5: PS DCFG Test
6: PS DDR3 Memory Test
7: PS Interrupt Test
8: PS Watchdog Timer Test
9: PL LED Test
A: PL DIP SWITCH Test
B: PL Push Button
0: Exit
0
Good-bye!
```
