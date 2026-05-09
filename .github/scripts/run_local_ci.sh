#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$repo_root"

run_lint() {
  docker run --rm \
    -v "$repo_root":/workspace \
    -w /workspace \
    ubuntu:24.04 \
    bash -lc '
      set -euo pipefail
      export DEBIAN_FRONTEND=noninteractive
      apt-get update
      apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        git \
        golang-go \
        libdbd-mysql-perl \
        libdbi-perl \
        libjson-perl \
        lua5.4 \
        perl \
        python3 \
        shellcheck

      git config --global --add safe.directory /workspace

      go install github.com/rhysd/actionlint/cmd/actionlint@latest
      export PATH="$PATH:/root/go/bin"
      actionlint

      if git grep -nI -E "[[:blank:]]$" -- \
        "*.c" "*.cc" "*.cpp" "*.h" "*.hpp" \
        "*.cmake" "CMakeLists.txt" \
        "*.json" "*.md" "*.pl" "*.py" "*.sh" "*.lua" \
        "*.yaml" "*.yml"; then
        echo "Trailing whitespace found"
        exit 1
      fi

      while IFS= read -r -d "" file; do
        if [ -s "$file" ] && [ "$(tail -c 1 "$file" | wc -l)" -eq 0 ]; then
          echo "$file: missing trailing newline"
          exit 1
        fi
      done < <(git ls-files -z \
        "*.c" "*.cc" "*.cpp" "*.h" "*.hpp" \
        "*.cmake" "CMakeLists.txt" \
        "*.json" "*.md" "*.pl" "*.py" "*.sh" "*.lua" \
        "*.yaml" "*.yml")

      git ls-files -z "*.sh" | xargs -0 --no-run-if-empty shellcheck --severity=error
      git ls-files -z "*.py" ":!:utils/scripts/vcxproj_dependencies.py" | xargs -0 --no-run-if-empty python3 -m py_compile
      while IFS= read -r -d "" file; do
        perl -c "$file"
      done < <(git ls-files -z "*.pl" ":!:utils/deprecated/**")
      git ls-files -z "*.lua" | xargs -0 --no-run-if-empty luac -p
    '
}

run_unit_tests() {
  docker run --rm \
    -v "$repo_root":/workspace \
    -w /workspace \
    ubuntu:24.04 \
    bash -lc '
      set -euo pipefail
      export DEBIAN_FRONTEND=noninteractive
      apt-get update
      apt-get install -y --no-install-recommends \
        autoconf \
        automake \
        build-essential \
        ca-certificates \
        ccache \
        cmake \
        curl \
        git \
        libtool \
        make \
        nasm \
        ninja-build \
        perl \
        pkg-config \
        python3 \
        tar \
        unzip \
        uuid-dev \
        zip \
        zstd

      git config --global --add safe.directory /workspace
      git submodule update --init --recursive

      mkdir -p .cache/ccache .cache/vcpkg/archives submodules/vcpkg/downloads
      ccache --set-config=cache_dir=/workspace/.cache/ccache
      ccache --set-config=max_size=1G

      export VCPKG_FEATURE_FLAGS=binarycaching
      export VCPKG_BINARY_SOURCES="clear;files,/workspace/.cache/vcpkg/archives,readwrite"

      cmake -S . -B build/unit-tests -G Ninja \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        -DEQEMU_BUILD_SERVER=OFF \
        -DEQEMU_BUILD_LOGIN=OFF \
        -DEQEMU_BUILD_CLIENT_FILES=OFF \
        -DEQEMU_BUILD_TESTS=ON \
        -DEQEMU_BUILD_LUA=OFF \
        -DEQEMU_BUILD_PERL=OFF

      cmake --build build/unit-tests --target tests --parallel 4
      ./build/unit-tests/bin/tests
      ccache --show-stats
    '
}

run_lint
run_unit_tests
