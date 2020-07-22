# This script file is part of Junkyard repository.
# Licensed under the terms of the GNU General Public License v3 or later.
# Copyright (C) Biswapriyo Nath

#!/bin/sh

set -e

## Required packages
# ArchLinux: base-devel cmake ninja
# MSYS2/MINGW-W64: git mingw-w64-x86_64-gcc mingw-w64-x86_64-ninja \
# mingw-w64-x86_64-cmake mingw-w64-x86_64-python3

# Set global environment variables
export DEFAULT_CRT="ucrt"
export PREFIX="install"
export TOOLCHAIN_ARCHS="aarch64"

# Clone and enter into the llvm-mingw repository folder
git clone --depth=1 https://github.com/mstorsjo/llvm-mingw.git
cd llvm-mingw

## Download llvm version 10 snapshot from GitHub
# This folder name is checked in build-llvm.sh script
LINK=https://github.com/llvm/llvm-project/archive/llvmorg-10.0.0.tar.gz
FILE=llvm-project.tar.gz
FOLDER=llvm-project

# Build llvm
wget $LINK --output-document=$FILE
tar -xf $FILE
mv llvm-project-* $FOLDER
./build-llvm.sh $PREFIX
./strip-llvm.sh $PREFIX
./install-wrappers.sh $PREFIX

## Clone the latest mingw-w64 git repository master branch
# This folder name is checked in build-mingw-w64.sh script
git clone --depth=1 https://git.code.sf.net/p/mingw-w64/mingw-w64.git

./build-mingw-w64.sh $PREFIX
./build-compiler-rt.sh $PREFIX
./build-mingw-w64-libraries.sh $PREFIX
./build-libcxx.sh $PREFIX
./build-compiler-rt.sh $PREFIX --build-sanitizers

## Download gcc version 10.1.0 snapshot from GitHub mirror
# This folder name is checked in build-libssp.sh script
LINK=https://github.com/gcc-mirror/gcc/archive/releases/gcc-10.1.0.tar.gz
FILE=gcc.tar.gz
FOLDER=gcc

wget $LINK --output-document=$FILE
tar -xf $FILE
mv gcc-* $FOLDER
cp -r $FOLDER/libssp ./

./build-libssp.sh $PREFIX
