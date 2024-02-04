load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "cli11",
    srcs = glob(
        [
            "include/CLI/*.hpp",
            "include/CLI/impl/*.hpp",
        ],
        exclude = ["CLI/CLI.hpp"],
    ),
    hdrs = ["include/CLI/CLI.hpp"],
    includes = [
        "include",
        "include/CLI",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
