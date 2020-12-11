#!/bin/bash

BOOTABLE_ARGS="-b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -eltorito-alt-boot -e images/efiboot.img -no-emul-boot"

# product-version-arch
VOLUME="CentOS-7-x86_64"

# product is CentOS
# version is 7
# arch is x86_64
# DVD is type for laser disc
# release is 1607
# iso is type for file
OUTFILE="/root/centos_isos/CentOS-7-x86_64-DVD-1607.iso"

BUILDTREE="/root/centos_tree"

echo Generating ISO9660 images...
mkisofs -V ${VOLUME} -R -J -T -graft-points -verbose ${BOOTABLE_ARGS} -o ${OUTFILE} ${BUILDTREE} 

echo "Set hybrid boot for uefi..."	
isohybrid --uefi "${OUTFILE}"

echo Created "${OUTFILE}"
echo "Implant ISO MD5 for disc ..."
implantisomd5 --force "${OUTFILE}"  

echo generate md5sum...
md5sum "${OUTFILE}" > "${OUTFILE}.md5sum"  
echo Finished at `date`.


