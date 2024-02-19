workspace(name = "rtw")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Hedron's Compile Commands Extractor for Bazel
# https://github.com/hedronvision/bazel-compile-commands-extractor
http_archive(
    name = "hedron_compile_commands",
    strip_prefix = "bazel-compile-commands-extractor-ed994039a951b736091776d677f324b3903ef939",

    # Replace the commit hash in both places (below) with the latest, rather than using the stale one here.
    # Even better, set up Renovate and let it do the work for you (see "Suggestion: Updates" in the README).
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/ed994039a951b736091776d677f324b3903ef939.tar.gz",
    # When you first run this tool, it'll recommend a sha256 hash to put here with a message like: "DEBUG: Rule 'hedron_compile_commands' indicated that a canonical reproducible form can be obtained by modifying arguments sha256 = ..."
)

# Toolchain
http_archive(
    name = "toolchains_llvm",
    canonical_id = "0.10.3",
    sha256 = "b7cd301ef7b0ece28d20d3e778697a5e3b81828393150bed04838c0c52963a01",
    strip_prefix = "toolchains_llvm-0.10.3",
    url = "https://github.com/grailbio/bazel-toolchain/releases/download/0.10.3/toolchains_llvm-0.10.3.tar.gz",
)

load("@toolchains_llvm//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@toolchains_llvm//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain",
    llvm_version = "16.0.4",
)

load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()

# clang-tidy
load(
    "@bazel_tools//tools/build_defs/repo:git.bzl",
    "git_repository",
)

git_repository(
    name = "bazel_clang_tidy",
    commit = "43bef6852a433f3b2a6b001daecc8bc91d791b92",
    remote = "https://github.com/erenon/bazel_clang_tidy.git",
)

# Third-party dependencies
http_archive(
    name = "fmt",
    build_file = "//third_party/fmt:fmt.BUILD",
    sha256 = "ede1b6b42188163a3f2e0f25ad5c0637eca564bd8df74d02e31a311dd6b37ad8",
    strip_prefix = "fmt-10.0.0",
    url = "https://github.com/fmtlib/fmt/archive/refs/tags/10.0.0.tar.gz",
)

http_archive(
    name = "googletest",
    sha256 = "8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7",
    strip_prefix = "googletest-1.14.0",
    url = "https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz",
)

http_archive(
    name = "google_benchmark",
    sha256 = "6bc180a57d23d4d9515519f92b0c83d61b05b5bab188961f36ac7b06b0d9e9ce",
    strip_prefix = "benchmark-1.8.3",
    url = "https://github.com/google/benchmark/archive/refs/tags/v1.8.3.tar.gz",
)

http_archive(
    name = "cli11",
    build_file = "//third_party/cli11:cli11.BUILD",
    sha256 = "aac0ab42108131ac5d3344a9db0fdf25c4db652296641955720a4fbe52334e22",
    strip_prefix = "CLI11-2.3.2",
    url = "https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.3.2.tar.gz",
)

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

http_archive(
    name = "imgui",
    build_file = "//third_party/imgui:imgui.BUILD",
    sha256 = "e2a361e61e3c8c683fb5a11932bbb0f4432480947f5329eeef613bbd85a0adc9",
    strip_prefix = "imgui-54c1ac3e38a5b6df4c2c016cd5859666f3472e4d",
    url = "https://github.com/ocornut/imgui/archive/54c1ac3e38a5b6df4c2c016cd5859666f3472e4d.zip",  # docking branch
)

http_archive(
    name = "implot",
    build_file = "//third_party/implot:implot.BUILD",
    sha256 = "0330569e5d3d44cc7aa56dda7f3ff980e93f5bf6da4aca011f74c332eedda609",
    strip_prefix = "implot-0.14",
    url = "https://github.com/epezent/implot/archive/refs/tags/v0.14.zip",
)
