# rtw/stl — Static/Embedded STL Library


A C++17 header-only library of fixed-capacity containers and utilities designed for embedded and real-time systems. All inplace variants are trivially copyable, trivially destructible, and require zero heap allocation.


## Design Principles


- **Fixed capacity** — All containers have compile-time or construction-time capacity. No dynamic reallocation.
- **Constexpr-friendly** — Non-trivial types use an assignment-based storage path that is fully `constexpr` in C++17. Trivial types use placement-new (constexpr in C++20+).
- **Assertion-guarded** — Every precondition (`!empty()`, `size() < capacity()`, `index < size()`) is checked via `assert` and documented with `@pre` Doxygen tags.
- **Trivially copyable** — Inplace variants (`InplaceStatic*`) are trivially copyable and trivially destructible, making them safe for `memcpy`, DMA transfers, and shared memory.
- **No exceptions** — All operations are `noexcept`.


## Containers


| Header | Class | Description |
|--------|-------|-------------|
| `static_vector.h` | `InplaceStaticVector<T, N>` | Fixed-capacity dynamic array with insert/erase, random-access iterators |
| `static_stack.h` | `InplaceStaticStack<T, N>` | LIFO stack with top/bottom access |
| `static_queue.h` | `InplaceStaticQueue<T, N>` | FIFO queue with wraparound (ring buffer internally) |
| `static_priority_queue.h` | `InplaceStaticPriorityQueue<T, N>` | Binary heap (max-heap by default, configurable comparator) |
| `circular_buffer.h` | `InplaceCircularBuffer<T, N>` | Ring buffer that overwrites oldest/newest on full |
| `packed_buffer.h` | `InplacePackedBuffer<T, N>` | Contiguous buffer with O(1) swap-and-pop removal |
| `static_flat_unordered_set.h` | `InplaceStaticFlatUnorderedSet<T, N>` | Open-addressing hash set with tombstone-aware probing |
| `static_flat_unordered_map.h` | `InplaceStaticFlatUnorderedMap<K, V, N>` | Open-addressing hash map with tombstone-aware probing |
| `inplace_string.h` | `InplaceString<N>` | Fixed-capacity null-terminated string (no heap, no `<memory>`) |
| `static_string.h` | `StaticString` | Heap-allocated fixed-capacity string |


All containers expose a uniform interface: `size()`, `empty()`, `full()`, `capacity()`, `clear()`.


## Utilities


| Header | Description |
|--------|-------------|
| `span.h` | Non-owning view over contiguous memory (C++17 `std::span` equivalent) |
| `string_view.h` | Non-owning string view with find/substr/starts_with/ends_with |
| `heap_array.h` | Heap-allocated fixed-size array (for runtime-sized buffers) |
| `graph.h` | Directed graph with DFS, BFS, topological sort, cycle detection (O(V+E)) |
| `id.h` | Lightweight strongly-typed integer ID for graph vertices |
| `contracts.h` | Minimal C++26 contracts implementation (pre/post/assert with configurable semantics) |
| `source_location.h` | `std::source_location` polyfill for C++17 |
| `format.h` | `fmt`-based formatting for library types (separate `:format` Bazel target) |
| `ostream.h` | `operator<<` overloads for library types |


## Iterator Support


| Container | Iterator Category |
|-----------|------------------|
| Vector, PackedBuffer, Span | Random-access (`contiguous_storage_iterator.h`) |
| Hash Set, Hash Map | Forward (`hash_iterator.h`) |
| Circular Buffer | Random-access (index-based) |


`iterator.h` is a convenience header that includes both iterator headers.


## Usage (Bazel)


```python
cc_binary(
    name = "my_target",
    deps = ["//stl"],
)
```


```cpp
#include "stl/static_vector.h"


rtw::stl::InplaceStaticVector<int, 64> vec;
vec.push_back(42);
```


For `fmt` integration, depend on `//stl:format` instead.


## Constexpr Support


Containers are fully `constexpr` when `T` is a non-standard-layout type (inheritance, virtual functions, etc.), which routes through an assignment-based storage path. Standard-layout/trivial types use placement-new internally, which is `constexpr` only from C++20.


```cpp
// Constexpr usage (non-trivial type)
struct MyVal : SomeBase { int x; };


constexpr auto make_vec() {
  rtw::stl::InplaceStaticVector<MyVal, 8> v;
  v.emplace_back(1);
  v.emplace_back(2);
  return v;
}
static_assert(make_vec().size() == 2);
```


## Hash Container Notes


- Capacity should ideally be a **prime number** for optimal distribution with quadratic probing.
- Erased slots become tombstones; `find` probes past them, `emplace` reuses them.
- Iterator invalidation: `emplace`/`insert`/`erase` invalidate all iterators. `clear` invalidates all iterators.


## Build & Test


```bash
# Build the library
bazel build //stl


# Run all tests (includes death tests, constexpr static_asserts, tombstone-reuse tests)
bazel test //stl/...
```


Test targets:
- `//stl/tests:stl_tests` — Full test suite
- `//stl/tests:stl_contracts_quick_enforce_tests` — Contract violation tests (quick_enforce mode)
- `//stl/tests:stl_contracts_observe_tests` — Contract violation tests (observe mode)
- `//stl/tests:stl_contracts_ignore_tests` — Contract violation tests (ignore mode)