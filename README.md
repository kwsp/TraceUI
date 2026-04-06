# TraceUI

A fullscreen system monitoring dashboard with a Neo-Kitsch aesthetic. Built with Qt Quick/QML, targeting macOS and Linux.

## Build

```sh
git clone --recursive https://github.com/kwsp/TraceUI.git
cd TraceUI
cmake --preset clang
cmake --build build-clang
```

Note: If you already cloned the repository without `--recursive`, run `git submodule update --init --recursive` before building.

## Run

```sh
./build-clang/TraceUI.app/Contents/MacOS/TraceUI
```

## Test

```sh
cd build-clang && ctest -C debug --output-on-failure
```
