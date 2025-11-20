# Raspberry Pi minimal image

This guide intends to summarize the essencial steps performed to build a Raspberry Pi minmal image using Yocto.

## Download software dependencies

To build the image we are going to use the [Poky](http://git.yoctoproject.org/cgit/cgit.cgi/poky) and the layers [meta-openembedded](http://git.openembedded.org/meta-openembedded) , [meta-raspberrypi](https://github.com/agherzan/meta-raspberrypi) .

The list of all meta-raspberrypi dependencies are contained in the [Readme.md](https://github.com/agherzan/meta-raspberrypi/blob/sumo/README.md#dependencies) the provided MACHINE's are  listed on the [layer-contentens.md](https://github.com/agherzan/meta-raspberrypi/blob/sumo/docs/layer-contents.md#supported-machines)

- Use Git to clone Poky repo using the apropriate [version](https://wiki.yoctoproject.org/wiki/Releases) branch.
```
$ git clone -b sumo git://git.yoctoproject.org/poky.git
```

- Enter into the Poky folder  
```
$ cd poky
```

- Use Git to clone the meta-rasperrypi layer repo using the apropriate [version](https://wiki.yoctoproject.org/wiki/Releases) branch.  
```
$ git clone -b sumo git://git.yoctoproject.org/meta-raspberrypi
```

- Use Git to clone the meta-openembedded layer repo using the apropriate [version](https://wiki.yoctoproject.org/wiki/Releases) branch.  
```
$ git clone -b sumo git://git.openembedded.org/meta-openembedded
```

## Build Your Image

- Initialize the build env  
```
$ source oe-init-build-env
```
- Modify your local.conf file  
    - Set your machine to `MACHINE = "raspberrypi4-64"` according to your Raspberry Pi [model](https://github.com/agherzan/meta-raspberrypi/blob/sumo/docs/layer-contents.md#supported-machines).
    - (Optional) Set the package manager to deb `PACKAGE_CLASSES ?= "package_deb"`
    - (Optional--[u007d] didn't seem to help for me) Set the SSTate Mirrors to save time in the builds.  
    
    ```
    SSTATE_MIRRORS = "\
        file://.* http://sstate.yoctoproject.org/dev/PATH;downloadfilename=PATH \n \
        file://.* http://sstate.yoctoproject.org/2.4/PATH;downloadfilename=PATH \n \
        file://.* http://sstate.yoctoproject.org/2.5/PATH;downloadfilename=PATH \n \"
    ```  
    
    - Modify the bbplayers.conf file and add all the listed dependencies listed on the [Readme.md](https://github.com/agherzan/meta-raspberrypi/blob/sumo/README.md#dependencies).  
    
    ```
    BBLAYERS ?= " \
        /workdir/poky/meta \
        /workdir/poky/meta-poky \
        /workdir/poky/meta-yocto-bsp \
        /workdir/poky/meta-openembedded/meta-oe \
        /workdir/poky/meta-openembedded/meta-multimedia \
        /workdir/poky/meta-openembedded/meta-networking \
        /workdir/poky/meta-openembedded/meta-python \  
        /workdir/poky/meta-raspberrypi \
        "
    ```  

    - Build the image  *minimal* ```$ bitbake -k core-image-minimal```.  
    Other available images are listed [here](https://github.com/agherzan/meta-raspberrypi/blob/sumo/docs/layer-contents.md#images).

## Transfer the image to the sd card

**Check what is your sd card device. Example uses dummy device /dev/XYZ. Be carefull this operation is not reversible.**

To check what is your sd card device use ```$ sudo lsblk -io KNAME,TYPE,SIZE,MODEL --nodep ```

## Copy the image out of the Docker container
* In Finder, `Connect to Server` (Cmd-K)
* Connect to `127.0.0.2`
* Log in as `Guest`
* Navigate to desired content
```

## Create `.img` file
```
# cd ~/Development/3rd_party/qemu/0xmirasio/qemu-patch-raspberry4/build
./qemu-img convert -f raw -O qcow2 core-image-minimal-raspberrypi4-64.ext3 core-image-minimal-raspberrypi4-64.img
./qemu-img resize ~/Development/core-image-minimal-raspberrypi4-64.img 256M # or some other power-of-two

```
$ sudo dd if=build/tmp/deploy/images/raspberrypi/core-image-minimal-raspberrypi-20180810110222.rootfs.rpi-sdimg of=/dev/XYZ status=progress
```

## Boot the image

Insert the sd card into your Raspberry Pi.  
If everything went smoothly you should be able to see a login prompt. Just use *root* without any password.

## Boot the image (`qemu`)
Try:
```
# cd ~/Development/3rd_party/qemu/0xmirasio/qemu-patch-raspberry4/build`
./qemu-system-aarch64 -M raspi4b2g -dtb bcm2711-rpi-4-b.dtb -sd core-image-full-cmdline-raspberrypi4-64.rpi-sdimg -kernel kernel8.img -nographic -append "rw earlyprintk loglevel=8 console=ttyAMA0,115200 dwc_otg.lpm_enable=0 root=/dev/mmcblk0p2"
```
