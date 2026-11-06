# Reimplementing the wheel

The goal of this project is nothing, but to have fun and to understand how things work by implementing them from scratch.
Code is written in C++ (C++17) and is not intended to be used in production.

## The structure of the project

The top-level folders contain standalone libraries. Each library has a set of unit tests and (sometimes) benchmarks.
The [sandbox](sandbox) folder contains simple applications that use the libraries.

## Building the project

The project uses [Bazel](https://bazel.build/) as a build system.

For instance, to build a math library use the following command:
```bash
bazel build //math/...
```

To run tests for the math library use the following command:
```bash
bazel test //math/...
```

### Editor support (clangd)

Generate or refresh the compilation database after changing Bazel dependencies
or build settings:

```bash
bazel run @hedron_compile_commands//:refresh_all
```

This writes the ignored `compile_commands.json`, including entries for headers.
Hedron is patched in `third_party/hedron_compile_commands/bazel9.patch` to use
explicit Python/C++ rules on Bazel 9 and support header discovery with LLVM 23.

## Currently implemented:
- [x] [multiprecision](multiprecision) - A set of classes to work with big ints, fixed-point and rationals. Includes arithmetic library and a set of math functions including trigonometric functions.
- [x] [math](math) - A math library, that is compatible with fixed-point arithmetic library.
- [x] [software renderer](sw_renderer) - A simple software renderer that can optionally built to use fixed-point arithmetic library.
- [x] [ecs](ecs) - Entity Component System that has type safety and efficient data management.
- [x] [stl](stl) - A set of custom containers which are relevant for the ECS implementation.
- [x] [event_bus](event_bus) - A message passing library implementing Event Bus pattern.

To run the software renderer demo application use the following command:
```bash
bazel run //sandbox/sw_renderer -c opt
```
Note the `-c opt` flag, which is used to run the application in release mode to get better performance.

It's possible to run a demo that uses fixed-point arithmetic via the dedicated target, which recompiles the renderer with `FixedPoint16` precision:
```bash
bazel run //sandbox/sw_renderer:sw_renderer_fixed_point -c opt
```

## Third party libraries

ImGui (docking) and ImPlot come from the Bazel Central Registry. SDL2 and
SDL2_image use system shared libraries via the sysroot extension: install
`sdl2` and `sdl2_image` with Homebrew on macOS, or `libsdl2-dev` and
`libsdl2-image-dev` on Debian/Ubuntu.

The small patch in `third_party/imgui/sysroot_sdl2.patch` exposes ImGui's SDL2
backend sources and headers. `//third_party/imgui:sdl2_backend` compiles them
against sysroot SDL2; use this target instead of BCR's compiled SDL2 backends
to avoid mixing source-built SDL2 with the SDL2 used by system SDL2_image.
BCR ImGui also enables FreeType/LunaSVG, 32-bit character types and 32-bit indices.

- [SDL2](https://www.libsdl.org/) - Simple DirectMedia Layer is a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D
- [fmtlib](https://github.com/fmtlib) - A modern formatting library
- [googletest](https://github.com/google/googletest) - Google Testing and Mocking Framework
- [benchmark](https://github.com/google/benchmark) - A microbenchmark support library
- [imgui](https://github.com/ocornut/imgui) - Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies
- [implot](https://github.com/epezent/implot) - Advanced 2D Plotting for Dear ImGui
- [CLI11](https://github.com/CLIUtils/CLI11) - CLI11 is a command line parser for C++11 and beyond that provides a rich feature set with a simple and intuitive interface
