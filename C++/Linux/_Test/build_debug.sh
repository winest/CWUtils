#!/bin/sh

CURR_DIR=$PWD
BUILD_TYPE=Debug
BUILD_DIR=Build$BUILD_TYPE
SOURCE_DIR=Src
OUTPUT_DIR=Output/$BUILD_TYPE
CMAKE_PATH=cmake



#Build HFT
echo "Building TestCWEvent"
cd "$CURR_DIR"
if [ ! -d "UnitTest/TestCWEvent" ]; then
    echo "Cannot find $PWD/UnitTest/TestCWEvent"
    exit 1
fi

if [ -d "$OUTPUT_DIR" ]; then
    rm -rf "$OUTPUT_DIR";
fi

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR";
fi
mkdir "$BUILD_DIR"

cd "$BUILD_DIR"
"$CMAKE_PATH" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_STATIC_LIBS=ON ".."
make
make install
