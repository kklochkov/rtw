load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "utility",
    hdrs = ["utility.h"],
)

cc_library(
    name = "stl",
    srcs = ["contracts.cpp"],
    hdrs = [
        "circular_buffer.h",
        "contiguous_storage_iterator.h",
        "contracts.h",
        "flags.h",
        "graph.h",
        "hash_iterator.h",
        "heap_array.h",
        "id.h",
        "inplace_string.h",
        "iterator.h",
        "ostream.h",
        "packed_buffer.h",
        "source_location.h",
        "span.h",
        "static_contiguous_storage.h",
        "static_flat_unordered_map.h",
        "static_flat_unordered_set.h",
        "static_priority_queue.h",
        "static_queue.h",
        "static_stack.h",
        "static_string.h",
        "static_vector.h",
        "string_view.h",
    ],
    deps = [":utility"],
)

cc_library(
    name = "format",
    hdrs = ["format.h"],
    deps = [
        ":stl",
        "@fmt",
    ],
)
