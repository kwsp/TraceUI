# TraceUI

A fullscreen system monitoring dashboard with a Neo-Kitsch aesthetic. Built with Qt Quick/QML, targeting macOS and Linux.

## Build

```sh
cmake --preset clang
cmake --build build-clang
```

## Run

```sh
./build-clang/TraceUI.app/Contents/MacOS/TraceUI
```

## Test

```sh
cd build-clang && ctest -C debug --output-on-failure
```
