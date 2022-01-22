#!/bin/bash

SYSROOT=/opt/nautilus/sysroot
rm -rf $SYSROOT/usr/include/*
rm -rf $SYSROOT/usr/lib/*
mv $SYSROOT/usr/x86_64-nautilus/lib/* $SYSROOT/usr/lib/
mv $SYSROOT/usr/x86_64-nautilus/include/* $SYSROOT/usr/include/
