load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_test")
load("@with_cfg.bzl", "with_cfg")

def _cc_rule_with_llvm_symbolizer(cc_rule, name, srcs, deps, tags, llvm_symbolizer = "@llvm_toolchain//:llvm-symbolizer", **kwargs):
    """Allows to use the llvm-symbolizer to symbolize stack traces from sanitizers using the LLVM toolchain.

    The sanitizer process requres the full path to the llvm-symbolizer binary,
    therefore the path gets resolved and also the llvm-symbolizer is added to the data attribute.
    """
    data = kwargs.pop("data", [])
    env = kwargs.pop("env", {})

    data.append(llvm_symbolizer)

    llvm_symbolizer_location = "$(rootpath {symbolizer})".format(symbolizer = llvm_symbolizer)
    common_options = "color=always:strip_path_prefix=/proc/self/cwd/"

    env["LLVM_SYMBOLIZER_PATH"] = llvm_symbolizer_location
    env["ASAN_SYMBOLIZER_PATH"] = llvm_symbolizer_location
    env["ASAN_OPTIONS"] = common_options
    env["TSAN_SYMBOLIZER_PATH"] = llvm_symbolizer_location
    env["TSAN_OPTIONS"] = common_options
    env["MSAN_SYMBOLIZER_PATH"] = llvm_symbolizer_location
    env["MSAN_OPTIONS"] = "{common_options}:poison_in_dtor=0".format(common_options = common_options)
    env["UBSAN_SYMBOLIZER_PATH"] = llvm_symbolizer_location
    env["UBSAN_OPTIONS"] = common_options
    env["SSSAN_SYMBOLIZER_PATH"] = llvm_symbolizer_location
    env["SSSAN_OPTIONS"] = common_options

    cc_rule(
        name = name,
        srcs = srcs,
        deps = deps,
        tags = tags,
        data = data,
        env = env,
        **kwargs
    )

def cc_test_with_llvm_symbolizer(name, srcs, deps, tags = [], **kwargs):
    _cc_rule_with_llvm_symbolizer(cc_test, name, srcs, deps, tags, **kwargs)

def cc_binary_with_llvm_symbolizer(name, srcs, deps, tags = [], **kwargs):
    _cc_rule_with_llvm_symbolizer(cc_binary, name, srcs, deps, tags, **kwargs)

# Fixed-point variants of cc_test/cc_binary.
#
# These apply a configuration transition that appends -DRTW_USE_FIXED_POINT to
# --copt, so the target AND its entire (transitive) dependency graph are
# recompiled in fixed-point mode. This is ODR-correct: it also recompiles the
# //sw_renderer library (e.g. pipeline.cpp), unlike a per-target local_defines
# which would only define the macro for the target's own translation units while
# linking against the default float-compiled library.
#
# with_cfg returns (macro, internal_rule); the internal rule must be assigned to
# a global to satisfy Bazel and is otherwise unused directly.
_cc_test_fixed_point, _cc_test_fixed_point_internal = (
    with_cfg(cc_test).extend("copt", ["-DRTW_USE_FIXED_POINT"]).build()
)
_cc_binary_fixed_point, _cc_binary_fixed_point_internal = (
    with_cfg(cc_binary).extend("copt", ["-DRTW_USE_FIXED_POINT"]).build()
)

def cc_test_with_fixed_point(name, srcs, deps, tags = [], **kwargs):
    _cc_rule_with_llvm_symbolizer(_cc_test_fixed_point, name, srcs, deps, tags, **kwargs)

def cc_binary_with_fixed_point(name, srcs, deps, tags = [], **kwargs):
    _cc_rule_with_llvm_symbolizer(_cc_binary_fixed_point, name, srcs, deps, tags, **kwargs)
