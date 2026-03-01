load("//:cc_defs.bzl", "cc_test_with_llvm_symbolizer")

cc_test_with_llvm_symbolizer(
    name = "stl_tests",
    size = "small",
    srcs = [
        "circular_buffer_test.cpp",
        "contracts_test.cpp",
        "flags_test.cpp",
        "graph_test.cpp",
        "heap_array_test.cpp",
        "inplace_circular_buffer_test.cpp",
        "inplace_graph_test.cpp",
        "inplace_packed_buffer_test.cpp",
        "inplace_static_contiguous_storage_test.cpp",
        "inplace_static_flat_unordered_map_test.cpp",
        "inplace_static_flat_unordered_set_test.cpp",
        "inplace_static_priority_queue_test.cpp",
        "inplace_static_queue_test.cpp",
        "inplace_static_stack_test.cpp",
        "inplace_static_vector_test.cpp",
        "inplace_string_test.cpp",
        "packed_buffer_test.cpp",
        "source_location_test.cpp",
        "span_test.cpp",
        "static_contiguous_storage_test.cpp",
        "static_flat_unordered_map_test.cpp",
        "static_flat_unordered_set_test.cpp",
        "static_priority_queue_test.cpp",
        "static_queue_test.cpp",
        "static_stack_test.cpp",
        "static_string_test.cpp",
        "static_vector_test.cpp",
        "string_view_test.cpp",
    ],
    tags = ["no-clang-tidy"],
    deps = [
        "//stl",
        "//stl:format",
        "@googletest//:gtest_main",
    ],
)

cc_test_with_llvm_symbolizer(
    name = "stl_contracts_ignore_tests",
    size = "small",
    srcs = ["contracts_ignore_test.cpp"],
    defines = ["RTW_CONTRACTS_IGNORE"],
    tags = ["no-clang-tidy"],
    deps = [
        "//stl",
        "//stl:format",
        "@googletest//:gtest_main",
    ],
)

cc_test_with_llvm_symbolizer(
    name = "stl_contracts_observe_tests",
    size = "small",
    srcs = ["contracts_observe_test.cpp"],
    defines = ["RTW_CONTRACTS_OBSERVE"],
    tags = ["no-clang-tidy"],
    deps = [
        "//stl",
        "//stl:format",
        "@googletest//:gtest_main",
    ],
)

cc_test_with_llvm_symbolizer(
    name = "stl_contracts_quick_enforce_tests",
    size = "small",
    srcs = ["contracts_quick_enforce_test.cpp"],
    defines = ["RTW_CONTRACTS_QUICK_ENFORCE"],
    tags = ["no-clang-tidy"],
    deps = [
        "//stl",
        "//stl:format",
        "@googletest//:gtest_main",
    ],
)
