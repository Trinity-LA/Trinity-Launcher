# armhf support libs for mcpelauncher

This doesn't create a real `libc.so`, instead it contains only a small subset of symbols it would normally include.
Android armv7 is softfloat based, while most linux distributions are using hardfloat. This would lead to a lot of wrapped floating point functions avoided by these support libraries.

Based on android ndk and modified source code of https://android.googlesource.com/platform/bionic/+/refs/tags/platform-tools-29.0.6/libc/arch-arm/bionic/

## Build steps
- `PATH_TO_NDK=<path to android-ndk-r20>`
- `"$PATH_TO_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi21-clang" -shared -o libc.so libcrt_compat.c __aeabi.c exidx_dynamic.c -Wl,-soname=libc.so -ldl -nostdlib "$(echo "$PATH_TO_NDK/toolchains/llvm/prebuilt/linux-x86_64/lib64/clang/"*)/lib/linux/libclang_rt.builtins-arm-android.a"`
- `"$PATH_TO_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi21-clang" -shared -o libm.so -Wl,-soname=libm.so -Wl,--whole-archive "$PATH_TO_NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/arm-linux-androideabi/libm.a" -Wl,--no-whole-archive`
