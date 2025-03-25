# Compile Firefox with MinGW-w64 toolchain

The procedure will explain how to cross compile Firefox with MinGW-w64 toolchain
in Linux desktop OS. The following steps are based on [this Google group message](https://groups.google.com/a/mozilla.org/g/dev-platform/c/w8F43mKLmj0/m/zmZ0RUOcBwAJ)
which may change in future.

## Procedure:

Create a folder to store all files. For example, `/home/user/mozilla` is used here.

0. Install some packages using system package manager. The following command
uses ArchLinux package name.

```sh
pacman -S --needed base-devel cbindgen direnv git git-cinnabar nasm nodejs \
    python rust sccache unzip wine zstd
```

1. Clone mozilla-central repository with [this official guide](https://firefox-source-docs.mozilla.org/contributing/contribution_quickref.html).

2. Download and extract prebuilt toolchain from [this link](https://firefox-ci-tc.services.mozilla.com/tasks/index/gecko.cache.level-3.toolchains.v3).

    * [linux64-clang-19-mingw-x64](https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/gecko.cache.level-3.toolchains.v3.linux64-clang-19-mingw-x64.latest/artifacts/public%2Fbuild%2Fclangmingw.tar.zst)
    * [mingw32-rust-1/85](https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/gecko.cache.level-3.toolchains.v3.mingw32-rust-1.85.latest/artifacts/public%2Fbuild%2Frustc.tar.zst)
    * [sysroot-wasm32-wasi-clang-20](https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/gecko.cache.level-3.toolchains.v3.sysroot-wasm32-wasi-clang-20.latest/artifacts/public%2Fbuild%2Fsysroot-wasm32-wasi.tar.zst)
    * [linux64-mingw-fxc2-x86](https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/gecko.cache.level-3.toolchains.v3.linux64-mingw-fxc2-x86.latest/artifacts/public%2Fbuild%2Ffxc2.tar.zst)

3. Optionally, create `.envrc` file with the following commands to separate cache
   files from default path.

```sh
# envrc for mozilla-central
export CARGO_HOME="$PWD/cargo"
export MOZBUILD_STATE_PATH="$PWD/mozbuild"
export PIP_CACHE_DIR="$PWD/pipcache"
export RUSTC_WRAPPER="/usr/bin/sccache"
export SCCACHE_DIR="$PWD/sccache"
export WINEPREFIX="$PWD/wineprefix"
```

4. Download the windows rust crate source.

```sh
cargo install cargo-download
cargo download -x windows=0.58.0
```

5. Create `.mozconfig` file with the following lines in mozilla-unified folder.
More info about mozconfig can be found in [this page](https://firefox-source-docs.mozilla.org/setup/configuring_build_options.html).

```sh
# x64 Builds
ac_add_options --target=x86_64-pc-windows-gnu
ac_add_options --with-toolchain-prefix=x86_64-w64-mingw32-

ac_add_options --disable-bootstrap # Bug 1920741

ac_add_options --disable-warnings-as-errors
mk_add_options "export WIDL_TIME_OVERRIDE=0"

# This replicates Tor's configuration
ac_add_options --enable-proxy-bypass-protection
ac_add_options --enable-disk-remnant-avoidance

# These aren't supported on mingw at this time
ac_add_options --disable-webrtc # Bug 1393901
ac_add_options --disable-geckodriver # Bug 1489320
ac_add_options --disable-update-agent # Bug 1561797
ac_add_options --disable-default-browser-agent # WinToast does not build on mingw
ac_add_options --disable-notification-server

# Find our toolchain
HOST_CC="/home/user/mozilla/clang/bin/clang"
HOST_CXX="/home/user/mozilla/clang/bin/clang++"

# x64 Builds
CC="/home/user/mozilla/clang/bin/x86_64-w64-mingw32-clang"
CXX="/home/user/mozilla/clang/bin/x86_64-w64-mingw32-clang++"

CXXFLAGS="-fms-extensions"
CFLAGS="$CFLAGS -fcolor-diagnostics -include _mingw.h"
CXXFLAGS="$CXXFLAGS -fcolor-diagnostics -include _mingw.h"

RUSTC="/home/user/mozilla/rustc/bin/rustc"

mk_add_options "export PATH=/home/user/mozilla/clang/bin:/home/user/mozilla/fxc2/bin:/home/user/mozilla/cargo/bin:$PATH"
mk_add_options "export MOZ_WINDOWS_RS_DIR=/home/user/mozilla/windows-0.58.0"

ac_add_options --with-wasi-sysroot=/home/user/mozilla/sysroot-wasm32-wasi

ac_add_options --with-ccache=sccache
```

6. Configure and build.

```
cd mozilla-unified
./mach configure
./mach build
```
