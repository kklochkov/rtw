# Reimplementing the wheel

The goal of this project is nothing, but to have fun and to understand how things work by implementing them from scratch.
Code is written in C++ (C++17) and is not intended to be used in production.

## The structure of the project

The top-level folders contain stand alone libraries. Each library has a set of unit tests and (sometimes) benchmarks.
The [third_party](third_party) folder contains third party libraries that are used in the project.
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

## Currently implemented:
- [x] [software renderer](sw_renderer) and [math](math) libraries. To run the software renderer demo application use the following command:
```bash
bazel run //sandbox/sw_renderer -c opt
```
Note the `-c opt` flag, which is used to run the application in release mode to get better performance.

## Third party libraries
- [SDL2](https://www.libsdl.org/) - Simple DirectMedia Layer is a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D
- [fmtlib](https://github.com/fmtlib) - A modern formatting library
- [googletest](https://github.com/google/googletest) - Google Testing and Mocking Framework
- [benchmark](https://github.com/google/benchmark) - A microbenchmark support library
- [imgui](https://github.com/ocornut/imgui) - Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies
- [implot](https://github.com/epezent/implot) - Advanced 2D Plotting for Dear ImGui
- [CLI11](https://github.com/CLIUtils/CLI11) - CLI11 is a command line parser for C++11 and beyond that provides a rich feature set with a simple and intuitive interface
