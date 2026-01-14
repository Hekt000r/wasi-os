#!/bin/bash



set -e

HOST_APPS="$(pwd)/apps"


# Paths
SRC_DIR=src
APPS_SRC_DIR=apps-src
APPS_DIR=apps
BUILD_DIR=build

# Ensure build directory exists
mkdir -p $BUILD_DIR
mkdir -p $APPS_DIR

echo "Building Core OS..."
clang --target=wasm32-wasi --sysroot=/opt/wasi-sdk/share/wasi-sysroot -O2 \
  $SRC_DIR/main.c $SRC_DIR/vfs.c -o $BUILD_DIR/os.wasm

echo "Building apps..."
for src_file in $APPS_SRC_DIR/*.c; do
  app_name=$(basename "$src_file" .c)
  clang --target=wasm32-wasi --sysroot=/opt/wasi-sdk/share/wasi-sysroot -O2 \
    "$src_file" -o "$APPS_DIR/$app_name.wasm"
done

echo "Listing apps directory:"
ls -l $APPS_DIR

echo "Running OS with wasmtime..."
wasmtime run --dir "$HOST_APPS"::/apps ./build/os.wasm
