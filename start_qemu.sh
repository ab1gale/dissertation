qemu-system-i386 -kernel ../obj/linux-x86-basic/arch/x86/boot/bzImage -initrd ../obj/initramfs-busybox-x86.cpio.gz -nographic -append "debug console=ttyS0" -boot c -s
