#!/bin/bash
set -e
export ARCH=arm64
export AR=llvm-ar
export CC=clang
export LLVM=1
export CROSS_COMPILE=aarch64-linux-gnu-
cd "$(dirname "$0")"
OUT=${OUT:-out}
KCF="-Wno-error -Wno-incompatible-pointer-types-discards-qualifiers"
exec make -j3 O=$OUT "$@" KCFLAGS="$KCF"
