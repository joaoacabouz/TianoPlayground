#!/bin/bash
cd "$(dirname "$0")"

# Initialize EDK2 if directory is missing
if [ ! -d "edk2" ]; then
    echo "EDK2 directory not found. Initializing repository..."
    git clone https://github.com/tianocore/edk2.git || exit 1
    cd edk2
    git submodule update --init || exit 1
    make -C BaseTools || exit 1
    cd ..
fi

# Validate required build tools in PATH
for tool in python3 nasm iasl; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        if [ "$tool" = "python3" ] && command -v python >/dev/null 2>&1; then
            continue
        fi
        echo "Error: '$tool' is required but not found in PATH."
        exit 1
    fi
done

PYTHON_BIN=$(command -v python3 || command -v python)

# Interactive build menu loop
while true; do
    echo
    echo "==================================================="
    echo "    EDK2 Builder for PlaygroundPkg (OVMF X64)"
    echo "==================================================="
    echo

    # 1. Select Target
    echo "[1] DEBUG (default)"
    echo "[2] RELEASE"
    read -r -p "Select Target [1-2]: " TARGET_CHOICE
    TARGET="DEBUG"
    [ "$TARGET_CHOICE" = "2" ] && TARGET="RELEASE"

    # 2. Select Build Type
    echo
    echo "[1] Incremental (default)"
    echo "[2] Clean"
    read -r -p "Select Build Type [1-2]: " BUILD_CHOICE
    CLEAN_FLAG=""
    [ "$BUILD_CHOICE" = "2" ] && CLEAN_FLAG="--clean"

    echo
    echo "Building: X64 | $TARGET | GCC $CLEAN_FLAG"
    echo

    "$PYTHON_BIN" build.py --arch X64 --target "$TARGET" --toolchain GCC $CLEAN_FLAG "$@"

    echo
    read -r -p "Build again? [Y/N] (default: Y): " BUILD_AGAIN
    if [ "${BUILD_AGAIN^^}" = "N" ]; then
        break
    fi
done
