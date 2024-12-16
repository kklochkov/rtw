load("@rules_cc//cc:defs.bzl", "cc_test")

def cc_test_with_llvm_symbolizer(name, srcs, deps, tags = [], **kwargs):
    """Allows to use the llvm-symbolizer to symbolize stack traces from sanitizers using the LLVM toolchain.

    The sanitizer process requres the full path to the llvm-symbolizer binary,
    therefore the path gets resolved and also the llvm-symbolizer is added to the data attribute.
    """
    data = kwargs.pop("data", [])
    env = kwargs.pop("env", {})

    data.append("@llvm_toolchain//:llvm-symbolizer")
    env["ASAN_SYMBOLIZER_PATH"] = "$(rootpath @llvm_toolchain//:llvm-symbolizer)"
    env["TSAN_SYMBOLIZER_PATH"] = "$(rootpath @llvm_toolchain//:llvm-symbolizer)"
    env["MSAN_SYMBOLIZER_PATH"] = "$(rootpath @llvm_toolchain//:llvm-symbolizer)"
    env["UBSAN_SYMBOLIZER_PATH"] = "$(rootpath @llvm_toolchain//:llvm-symbolizer)"
    env["SSSAN_SYMBOLIZER_PATH"] = "$(rootpath @llvm_toolchain//:llvm-symbolizer)"

    cc_test(
        name = name,
        srcs = srcs,
        deps = deps,
        tags = tags,
        data = data,
        env = env,
        **kwargs
    )
