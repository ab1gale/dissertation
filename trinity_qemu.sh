qemu-system-i386 -kernel ../obj/linux-x86-basic/arch/x86/boot/bzImage -hda ../wheezy_image/wheezy.img -net user,hostfwd=tcp::10021-:22 -net nic  -nographic -append "debug console=ttyS0 root=/dev/sda" -boot c -s