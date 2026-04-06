cmake_minimum_required(VERSION 3.25)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(TraceUI
  LANGUAGES C CXX
  VERSION 0.1.0
)

enable_testing()

add_subdirectory(external/encom-globe)

# libvterm doesn't have its own CMakeLists.txt, so we define it here
add_library(vterm STATIC
  external/libvterm/src/encoding.c
  external/libvterm/src/keyboard.c
  external/libvterm/src/mouse.c
  external/libvterm/src/parser.c
  external/libvterm/src/pen.c
  external/libvterm/src/screen.c
  external/libvterm/src/state.c
  external/libvterm/src/unicode.c
  external/libvterm/src/vterm.c
)
target_include_directories(vterm PUBLIC external/libvterm/include)
target_include_directories(vterm PRIVATE external/libvterm/src)
if(MSVC)
  target_compile_definitions(vterm PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(vterm PRIVATE -Wall)
endif()

add_subdirectory(src)
add_subdirectory(tests)
