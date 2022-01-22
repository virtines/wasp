#!/bin/bash


ARCH=x86_64


DIR=build/libc

if [ ! -f $DIR/libc.a ]; then
	mkdir -p $DIR
	pushd $DIR

		mkdir cross-bin
		for i in ar as cc g++ gcc ld nm objcopy objdump ranlib readelf strip; do
			ln -s /usr/bin/$i cross-bin/$ARCH-pc-virtine-$i;
		done;


		PATH=$PATH:$(pwd)/cross-bin/

		# configure newlib
		../../libc/configure --with-newlib --disable-multilib --target=$ARCH-pc-virtine

		PATH=$PATH:$(pwd)/cross-bin/
		make all -j --no-print-directory || exit
		cp x86_64-pc-virtine/newlib/libc.a ../libc.a
		cp x86_64-pc-virtine/newlib/libm.a ../libm.a

	popd

fi
