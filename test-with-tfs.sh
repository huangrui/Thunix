
# make a 1.44M disk
dd if=/dev/zero of=root.img bs=1024 count=1440

# cp the kernel
dd if=thunix.img of=root.img conv=notrunc

# put the tfs befind the boot image
dd if=tfs.img of=root.img bs=1024 seek=416 conv=notrunc
mv root.img thunix.img
