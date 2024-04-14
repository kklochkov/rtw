# Remove these lines once this is resolved: https://github.com/bazelbuild/bazel/issues/18285 it should be part of Bazel 7.2.0.
# brew install sdl2 sdl2_image sfml
new_local_repository(
    name = "sysroot_macos",
    build_file = "//third_party/sysroot:sysroot_macos.BUILD",
    path = "/opt/homebrew",
)

# sudo apt install '^libsdl2.*'
new_local_repository(
    name = "sysroot_linux",
    build_file = "//third_party/sysroot:sysroot_linux.BUILD",
    path = "/usr",
)
