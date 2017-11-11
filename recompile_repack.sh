filename=$1
gcc -static -pthread -o ${filename} ${filename}.c
cp ./${filename} /home/ab1gale/Desktop/linux_qemu/initramfs/x86-busybox/usr/exp/
OriginDir=`pwd`
cd /home/ab1gale/Desktop/linux_qemu/initramfs/x86-busybox
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../../obj/initramfs-busybox-x86.cpio.gz
cd $OriginDir
