<!--
SPDX-FileCopyrightText: 2026 The Naja verilog authors <https://github.com/keplertech/naja-verilog/blob/main/AUTHORS>

SPDX-License-Identifier: Apache-2.0
-->

# Repository guidance

## Build systems: CMake (primary) + Bazel (validated smoke test)

CMake is naja-verilog's primary build system. It is used by the main
Ubuntu and macOS CI matrices, coverage, valgrind, CodeQL, Emscripten,
installation, and normal development, and it is the build system to use
by default:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Bazel (`MODULE.bazel`, `BUILD.bazel`, and `bazel/`) is kept in parallel
as a validated smoke test. The additive `ubuntu-bazel.yml` and
`macos-bazel.yml` workflows run:

```sh
bazel build //...
bazel test //...
```

Those workflows do not initialize git submodules: bzlmod fetches
googletest, Bison, Flex, M4 and scanner headers from Bazel modules. Bazel
is not CI's primary gate and does not need to reproduce CMake-specific
coverage, valgrind, sanitizer, CodeQL, Emscripten, or installation
behavior. Its role is to prove that the native Bazel module still
compiles and passes its tests, including when consumed by naja.

## Keep the googletest pins synchronized

CMake uses the `thirdparty/googletest` git submodule, while Bazel uses
the `googletest` version in `MODULE.bazel`. Both must name the same
upstream release. `ci/check_submodule_bazel_sync.py`, run by
`dependency-sync-check.yml`, reads the CMake gitlink and verifies it
against the upstream tag corresponding to Bazel's module version.

After changing either pin, update the other in the same change and run:

```sh
python3 ci/check_submodule_bazel_sync.py
```
