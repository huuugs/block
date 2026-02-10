#!/bin/bash
PROJECT="/root/block"
ROOTFS="/tmp/x86_64-root"
NDK="/android-ndk"
proot -q qemu-x86_64 -r $ROOTFS -b $PROJECT/android-ndk:$NDK /bin/sh -c "LD_LIBRARY_PATH=$NDK/toolchains/llvm/prebuilt/linux-x86_64/lib64 $NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/clang-14.orig --version"
