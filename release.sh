#!/bin/sh

#
# make release package
#

repo_dir=`pwd`
tmp_dir=`mktemp -d`

if ! (cd $repo_dir; git diff >/dev/null 2>&1); then
	echo "Please run this script inside the thunix git repo" 1>&2
	exit 1
fi

version=`cat version`

function build_clean()
{
	make && make clean
}


(
	cd $tmp_dir
	git clone $repo_dir
	echo -n "build for release... "
	(cd ${repo_dir##*/}; build_clean >/dev/null 2>&1 && mv *.img image/)
	echo "[ DONE ]"
	echo -n "creating release tar package... "
	tar --exclude-vcs -cjf thunix-$version.tar.bz2 thunix
	tar --exclude-vcs -cf  thunix-$version.tar.gz  thunix
	echo "[ DONE ]"
)

rm -rf release && mkdir release 2>/dev/null
cp ${tmp_dir}/{*.bz2,*.gz} release/
rm -rf $tmp_dir

