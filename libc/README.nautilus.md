
# Adding Nautilus as a newlib target

## Prerequisites
* Install autoconf (I used 2.69, newest on Ubuntu Focal)
* Install `automake1.11` (this is not the default anymore, but newer versions are broken for newlib, you'll get automake errors)


## How it came together
1. Added `nautilus*` entry to `config.sub` in top-level dir
2. Modified `newlib/configure.host` as per docs to add a `nautilus` target and create a new `sys_dir` path.  Note that I had
   to add `-D_FORTIFY_SOURCE=0` to the `newlib_cflags` var here since `-O2` will automatically set this to 2 on newer
   versions of gcc, thus breaking the newlib build.
3. Modified `newlib/libc/sys/configure.in` as per the docs to add a subdir entry, then ran `autoconf`
4. Created `newlib/libc/sys/nautilus` dir
5. Added stub implementations for syscalls (`*.c`) in that dir based on `sys/rdos` and `sys/linux` which 
   will link against Nautilus `sys_*` syscall (not really syscall) stubs.
6. Created a `configure.in` and `Makefile.am` in `sys/nautilus` based on linux/rdos entries.
7. Ran `aclocal`, `autoconf`, and `automake` as per docs. Encountered issues here because of newer automake version. Worked
   after forced downgrade to automake 1.11.
8. Set up a fake cross-compiler toolchain with symlinks to please newlib build toolchain (see below).
9. Configured and built newlib as below, then copied over the install files to Nautilus to link directly.

## Configure
First I set up a dummy cross toolchain for binutils on the first run:

```
$ for i in ar as cc g++ gcc ld nm objcopy objdump ranlib readelf strip; do ln -s /usr/bin/$i /usr/bin/x86_64--nautilus-$i; done;
```

If you've already got a cross-compiler built, just make sure the binaries (in my case in `/opt/nautilus/cross/bin/`)
are on your `PATH`.


Then you can configure, from the top-level newlib dir:

```
$ mkdir bld-nautilus
$ cd bld-nautilus
$ ../configure --target=x86_64-nautilus --with-newlib --disable-multilib --prefix=/usr
```


## Build
```
$ make all-target-newlib -j
$ make -j
$ make DESTDIR=$SYSROOT install
```

Where `SYSROOT` for me is `/opt/nautilus/sysroot`. This was also passed to the gcc/binutils build.

newlib does not install in quite the right place. You have to move things over to `sysroot/usr/{include,lib}`
rather than `sysroot/usr/TARGET/{include,lib}`.
See `naut-scripts/fixup.sh`

Note that Nautilus must point directly to these libraries and the cross-compiler toolchain binaries.

## References
These StackOverflow questions were helpful in finding out what was going on with automake
* Official newlib [documentation](https://www.embecosm.com/appnotes/ean9/html/ch09.html) for adding a new target
* [SO post 1](https://stackoverflow.com/questions/16188335/automake-error-no-proper-invocation-of-am-init-automake-was-found)
* [SO post 2](https://stackoverflow.com/questions/23976423/porting-newlib-with-current-autotools)

