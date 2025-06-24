load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "stl_tests",
    size = "small",
    srcs = [
        "circular_buffer_test.cpp",
        "contiguous_storage_test.cpp",
        "flags_test.cpp",
        "heap_array_test.cpp",
        "packed_buffer_test.cpp",
        "queue_test.cpp",
        "static_vector_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//stl",
        "//stl:format",
        "@googletest//:gtest_main",
    ],
)
