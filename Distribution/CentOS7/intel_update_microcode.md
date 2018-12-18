# 1. Download MicroCode Firmware

intel官方下载地址：https://downloadcenter.intel.com/download/27591/Linux-Processor-Microcode-Data-File

# 2. Intel Processor Microcode Package for Linux Release

This package contains Intel microcode files in two formats:
- microcode.dat
- intel-ucode directory

microcode.dat is in a traditional text format. It is still used in some Linux distributions.

intel-ucode directory contains binary microcode files named in family-model-stepping pattern. The file is supported in most modern Linux distributions. It's generally located in the /lib/firmware directory, and can be updated through the microcode reload interface.

# 3. To update the intel-ucode package to the system
1. Ensure the existence of /sys/devices/system/cpu/microcode/reload
2. Copy intel-ucode directory to /lib/firmware, overwrite the files in /lib/firmware/intel-ucode/ 
3. Write the reload interface to 1 to reload the microcode files, e.g.
```bash
echo 1 > /sys/devices/system/cpu/microcode/reload
```