#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
CMAKE_BIN=${CMAKE:-cmake}
CTEST_BIN=${CTEST:-ctest}

BUILD_DIR=""
BUILD_TYPE=""
ENABLE_ASAN="OFF"
CLEAN=0

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

CMake 3.11+ compatible build and test runner for docopt.cpp03.

Options:
  --asan          Enable AddressSanitizer/UBSan (defaults: Debug, build_asan/)
  --debug         Set CMAKE_BUILD_TYPE=Debug
  --release       Set CMAKE_BUILD_TYPE=Release (default)
  --clean         Remove the target build directory before configuring
  --dir <path>    Specify custom build directory (default: build/ or build_asan/)
  -h, --help      Show this help message

Environment variables:
  CMAKE           CMake executable to use (default: cmake)
  CTEST           CTest executable to use (default: ctest)
  CC / CXX        C / C++ compiler executables
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --asan)
            ENABLE_ASAN="ON"
            ;;
        --debug)
            BUILD_TYPE="Debug"
            ;;
        --release)
            BUILD_TYPE="Release"
            ;;
        --clean)
            CLEAN=1
            ;;
        --dir)
            if [ $# -lt 2 ]; then
                echo "Error: --dir requires a path argument" >&2
                exit 1
            fi
            BUILD_DIR="$2"
            shift
            ;;
        --dir=*)
            BUILD_DIR="${1#--dir=}"
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Error: Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
    shift
done

if [ -z "$BUILD_TYPE" ]; then
    if [ "$ENABLE_ASAN" = "ON" ]; then
        BUILD_TYPE="Debug"
    else
        BUILD_TYPE="Release"
    fi
fi

if [ -z "$BUILD_DIR" ]; then
    if [ "$ENABLE_ASAN" = "ON" ]; then
        BUILD_DIR="build_asan"
    else
        BUILD_DIR="build"
    fi
fi

if [ "$CLEAN" -eq 1 ]; then
    case "$BUILD_DIR" in
        ""|"/"|"."|"..")
            echo "Error: Refusing to remove unsafe build directory: '$BUILD_DIR'" >&2
            exit 1
            ;;
    esac
    echo "Cleaning build directory: $BUILD_DIR"
    rm -rf -- "$BUILD_DIR"
fi

echo "=== Configuring ($BUILD_TYPE, ENABLE_ASAN=$ENABLE_ASAN) in $BUILD_DIR ==="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
"$CMAKE_BIN" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DENABLE_ASAN="$ENABLE_ASAN" "$ROOT_DIR"

echo "=== Building ==="
"$CMAKE_BIN" --build .

echo "=== Running CTest ==="
"$CTEST_BIN" --output-on-failure

echo "=== Running test_docopt ==="
./test_docopt "$ROOT_DIR/docopt.python/testcases.docopt"

echo "=== Running unit_tests ==="
./unit_tests

echo "=== All tests passed ==="
