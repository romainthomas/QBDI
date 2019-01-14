#!/bin/sh

VERSION="1.7.0"
SOURCE_URL="https://github.com/google/googletest/archive/release-${VERSION}.tar.gz"

case "$1" in

    prepare)
        rm -f "release-${VERSION}.tar.gz"
        wget $SOURCE_URL
        tar xf "release-${VERSION}.tar.gz"
    ;;
    build)
        mkdir -p build
        cd build
        cmake "../googletest-release-${VERSION}" \
              -DANDROID_PLATFORM=android-24 \
              -DANDROID_ABI=armeabi-v7a \
              -DCMAKE_TOOLCHAIN_FILE=/home/romain/android/ndk/build/cmake/android.toolchain.cmake
        make -j4
    ;;
    package)
        mkdir -p lib
        cp build/libgtest.a build/libgtest_main.a lib/
        cp -r "googletest-release-${VERSION}/include" .
    ;;
    clean)
        rm -rf "googletest-release-${VERSION}" build "release-${VERSION}.tar.gz"
    ;;

esac

exit 0
