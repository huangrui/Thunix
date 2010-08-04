#!/bin/bash

[ -z $TFS_DIR ] && TFS_DIR=${HOME}/project/fstk/fs/tfs

# make a 1.44M disk
dd if=/dev/zero of=root.img bs=1024 count=1440 2>/tmp/dd-out

dd if=/dev/zero of=tfs.img bs=1024 count=1024 2>/tmp/dd-out

# cp the kernel
dd if=thunix.img of=root.img conv=notrunc 2>/tmp/dd-out


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
cd ..
cp_in gen-test.sh
cp_in README
cp_in user-test
quit
EOF

dd if=tfs.img of=root.img bs=1024 seek=416 conv=notrunc 2>/tmp/dd-out
mv root.img thunix.img
