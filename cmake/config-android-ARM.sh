#!/bin/sh
cmake  .. \
      -DANDROID_ABI="armeabi-v7a" \
      -DANDROID_PLATFORM=android-24 \
      -DPLATFORM=android-ARM \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=/home/romain/android/ndk/build/cmake/android.toolchain.cmake \
      -DCMAKE_INSTALL_PREFIX=./builded

