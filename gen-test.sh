#!/bin/bash

TFS_DIR=${HOME}/project/fstk/fs/tfs

# make a 1.44M disk
dd if=/dev/zero of=root.img bs=1024 count=1440

dd if=/dev/zero of=tfs.img bs=1024 count=1024

# cp the kernel
dd if=thunix.img of=root.img conv=notrunc


qemu-img create tfs.img 1M
$TFS_DIR/mktfs tfs.img 0

# init the fs
${TFS_DIR}/tfsh tfs.img << EOF
mkdir bin
cd bin
touch ls
touch cd
touch cat
touch mkdir
touch rmdir
touch rm
touch touch
touch echo
touch hexdump
touch date
touch help
touch hello
touch halt
touch reboot
quit
EOF

dd if=tfs.img of=root.img bs=1024 seek=416 conv=notrunc
mv root.img thunix.img
