#!/bin/sh

VERSION="5.0.2"
SOURCE_URL="http://llvm.org/releases/${VERSION}/llvm-${VERSION}.src.tar.xz"
TARGET="ARM"
LIBRAIRIES="libLLVMSelectionDAG.a libLLVMAsmPrinter.a libLLVMBinaryFormat.a libLLVMCodeGen.a libLLVMScalarOpts.a libLLVMProfileData.a libLLVMInstCombine.a libLLVMTransformUtils.a libLLVMAnalysis.a libLLVMTarget.a libLLVMObject.a libLLVMMCParser.a libLLVMBitReader.a libLLVMMCDisassembler.a libLLVMMC.a libLLVMCore.a libLLVMSupport.a"

case "$1" in

    prepare)
        rm -rf "llvm-"$VERSION".src"
        rm -f "llvm-"$VERSION".src.tar.xz"
        wget $SOURCE_URL
        tar xf "llvm-"$VERSION".src.tar.xz"
        patch -p0 < nolibfuzzer.patch
    ;;
    build)
        rm -rf build
        mkdir build
        cd build
        cmake  -G Ninja "../llvm-"$VERSION".src" \
              -DCMAKE_BUILD_TYPE=Release \
              -DANDROID_ABI="armeabi-v7a" \
              -DANDROID_PLATFORM=android-24 \
              -DPLATFORM=android-ARM \
              -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_TOOLCHAIN_FILE=/home/romain/android/ndk/build/cmake/android.toolchain.cmake \
              -DCMAKE_INSTALL_PREFIX=./builded \
              -DLLVM_TABLEGEN=/usr/bin/llvm-tblgen \
              -DLLVM_DEFAULT_TARGET_TRIPLE=arm-linux-gnueabihf \
              -DLLVM_TARGET_ARCH=${TARGET} \
              -DLLVM_TARGETS_TO_BUILD=${TARGET} \
              -DLLVM_BUILD_TOOLS=Off \
              -DLLVM_BUILD_TESTS=Off \
              -DLLVM_INCLUDE_UTILS=Off \
              -DLLVM_INCLUDE_TESTS=Off
        ninja
    ;;
    package)
        rm -rf lib
        rm -rf include
        for f in $(find llvm-${VERSION}.src/include/ -iregex '.*\.\(h\|def\)'); do
            nf=${f#*/}
            mkdir -p ${nf%/*}
            cp $f $nf
        done
        for f in $(find build/include/ -iregex '.*\.\(h\|def\|gen\)'); do
            nf=${f#*/}
            mkdir -p ${nf%/*}
            cp $f $nf
        done
        mkdir -p lib/Target/$TARGET
        for l in $LIBRAIRIES; do
            cp build/lib/$l lib/
        done
        cp build/lib/libLLVM${TARGET}* lib/
        cp build/lib/Target/${TARGET}/*.inc lib/Target/${TARGET}/
        for f in $(find llvm-${VERSION}.src/lib/Target/${TARGET}/ -iregex '.*\.\(h\|def\)'); do
            nf=${f#*/}
            mkdir -p ${nf%/*}
            cp $f $nf
        done
    ;;
    clean)
        rm -rf "llvm-"$VERSION".src" build "llvm-"$VERSION".src.tar.xz"
    ;;

esac

exit 0
