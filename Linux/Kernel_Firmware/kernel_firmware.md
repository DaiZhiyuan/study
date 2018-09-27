***kernel_firmware***

[TOC]

# 1. Scope

Linux kernel provides a function "request_firmware()" to load firmware binary to configure and enable devices.

# 2. Usage of "request_firmware()"

Declaration of "request_firmware()":

```cpp
int request_firmware( const struct firmware **firmware_p, const char *name, 
                        struct device *device )
        // @firmware_p : pointer to firmware pointer
        // @name       : name of firmware file
        // @device     : device for which firmware is loaded
```

Release the resource in kernel with:

```
void release_firmware(struct firmware *firmware)
```

Example of calling "request_firmware()" in kernel to load firmware:

```
    int ret;
    ret = request_firmware(&fw, "abc.bin", dev);
    if(ret < 0)
        pr_err("failed to load firmware\n");
    else
        update_device_firmware();     // config or enable device with this firmware
```

# 3. Basic work flow of "request_firmware()"

```cpp
                                request_firmware()
                                         |
                                         |                     Y
                       firmware built in kernel image? ------------------- load from kernel image -------
                                         |                                                               |
                                         |  N                                                            |
                                         |                     Y                                         |
                              load directly from fs ? ------------------  load directly from "path" -    |
                                         |                                                           |   |
                                         |  N                                                        |   |
                                         |                                                           |   |
                      load with file node "/sys/class/firmware/xx.bin/"                              |   |
                                         |                                                           |   |
                                         |                                                           |   |
                                        exit                                                          exit
```

# 4. Load firmware from kernel image

Linux kernel provides methods to build firmware into kernel image. But please take caution because lots of firmware content is non-GPL, this will dirty the kernel. So do not build the firmware into kernel image if not necessary.

Enable "CONFIG_FIRMWARE_IN_KERNEL", "CONFIG_EXTRA_FIRMWARE_DIR" and "CONFIG_EXTRA_FIRMWARE" to build firmware into kernel image：

```cpp
        CONFIG_FIRMWARE_IN_KERNEL=y
        CONFIG_EXTRA_FIRMWARE_DIR="firmware"          // this means $(source_dir)/firmware
        CONFIG_EXTRA_FIRMWARE="fw_sst_0f28.bin"
```

If several firmwares are required, add them as:

```
        CONFIG_EXTRA_FIRMWARE="fw_sst_0f28.bin  abc.bin  efg.bin"
```

# 5. Mechanism of firmware builtin

When run command "make bzImage", Makefile under "firmware" generates an assembly file "fw_sst_0f28.bin.gen.S":

```cpp
           .section .rodata
        _fw_bin:
           .incbin "/home/caoxin/linux3.8/firmware/fw_sst_0f28.bin"

           .section .rodata.str
        _fw_bin_name:
           .string "fw_sst_0f28.bin"   
                                                                                                                                        
           .section .builtin_fw
           .long _fw_bin_name
           .long _fw_bin
           .long _fw_end - _fw_bin
```

This assembly file contains three sections: ".rodata", "rodata.str" and ".builtin_fw".
- section ".rodata" saves the firmware data in "fw_sst_0f28.bin"
- section ".rodata.str" saves the name of the file "fw_sst_0f28.bin"
- section ".builtin_fw" saves the address of the firmware name and the address of firmware data

The link-script vmlinux.lds involves the following section:

```cpp
    .builtin_fw      : {
          __start_builtin_fw = .
          *(.builtin_fw)
          __end_builtin_fw = .
     }
```

GNU linker locates this section in all the object files into the ".builtin_fw" section of the kernel image. After kernel image is running in memory, "__start_builtin_fw" and "__end_builtin_fw" are the start and end address of this section.

When kernel calls "request_firmware()", firmware names in section ".builtin_fw" will be checked to match the firmware name passed in. If matched, kernel copy the firmware data to a buffer and "request_firmware()" returns successfully.

# 6. Load one target firmware from fs(file system) directly

Kernel searchs the firmware with the firmware name in the following path:
1. fw_path_para(defined in kernel command line with "firmware_class.path = xxx")
2. "/lib/firmware/updates" UTS_RELEASE,
3. "/lib/firmware/updates",
4. "/lib/firmware" UTS_RELEASE,
5. "/lib/firmware",

Notes: 
1. UTS_RELEASE is the kernel release version number , such as "3.8.10-07312013"
2. Search priority is from No.1 to No.5.

If target firmware file is found, kernel grows a buffer to hold the binary files and "request_firmware()" returns sucessfully.

# 7. Load firmware in userspace

Kernel creates a directory under "/sys/class/firmware/". For example, "/sys/class/firmware/fw_sst_0f28.bin/{loading, data}".

Steps of loading firmware in userspace:
```
  1). echo 1 > /sys/class/firmware/fw_sst_0f28.bin/loading
  2). cat appropriate_firmware_image > /sys/class/firmware/fw_sst_0f28.bin/data
  3). echo 0 > /sys/class/firmware/fw_sst_0f28.bin/loading
```

Kernel grows a buffer to hold the binary file as it comes in, and "request_firmware()" returns successfully.

# 8. Reference

- kernel/Documentation/firmware_class/README
- 《Using ld, the GNU linker》 by Jeffrey, Osier