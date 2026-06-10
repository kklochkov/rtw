# rtw/multiprecision -- Multiprecision Arithmetic Library

A C++17 header-only library providing fixed-point, 128-bit integer, and rational number arithmetic. All operations are `constexpr`, `noexcept`, and free of undefined behavior. Designed for embedded, real-time, and safety-critical systems where floating-point is unavailable or undesirable.

## Design Principles

- **Zero UB in C++17** -- Every edge case (MIN negation, overflow, signed shift) is handled without invoking undefined behavior.
- **Constexpr throughout** -- All arithmetic, math functions, and type conversions are `constexpr`. Correctness is proven via 100+ `static_assert` tests.
- **Saturation semantics** -- FixedPoint arithmetic saturates to min/max on overflow rather than wrapping or trapping.
- **Assert-guarded preconditions** -- Every function with a precondition (`log(x > 0)`, `fmod(d != 0)`, `shift < BITS`) uses `assert` with a descriptive message.
- **No exceptions, no heap** -- All operations are `noexcept` with zero dynamic allocation.

## Components

### Core Types

| Header | Type | Description |
|--------|------|-------------|
| `int128.h` | `Int<T>` | Double-width integer (e.g., `Int<int64_t>` = signed 128-bit). Full arithmetic, shifts, bitwise, comparisons. |
| `fixed_point.h` | `FixedPoint<T, FRAC_BITS, SaturationT>` | QM.N fixed-point with saturating arithmetic. Configurable precision and saturation type. |
| `rational.h` | `Rational<T>` | Exact rational number (numerator/denominator in lowest terms, positive denominator). |

### Type Aliases

```cpp
// Fixed-point (signed)
using FixedPoint8  = FixedPoint<int16_t,  8, int32_t>;   // Q7.8,  range: -128 to ~127.996
using FixedPoint16 = FixedPoint<int32_t, 16, int64_t>;   // Q15.16, range: -32768 to ~32767.99998
using FixedPoint32 = FixedPoint<int64_t, 32, Int128>;    // Q31.32

// Fixed-point (unsigned)
using FixedPoint8U  = FixedPoint<uint16_t,  8, uint32_t>; // Q8.8,  range: 0 to ~255.996
using FixedPoint16U = FixedPoint<uint32_t, 16, uint64_t>; // Q16.16
using FixedPoint32U = FixedPoint<uint64_t, 32, Int128U>;  // Q32.32

// Integer
using Int128  = Int<int64_t>;    // Signed 128-bit integer
using Int128U = Int<uint64_t>;   // Unsigned 128-bit integer

// Rational
using Rational32 = Rational<int32_t>;  // Exact fractions with 32-bit storage
using Rational64 = Rational<int64_t>;  // Exact fractions with 64-bit storage
```

### Math Functions

| Header | Functions |
|--------|-----------|
| `math.h` | `signbit`, `sign`, `abs`, `count_leading_zero`, `round_to_nearest_integer`, `fmod` |
| `int128_math.h` | `abs` (for Int128, saturates MIN to MAX) |
| `fixed_point_math.h` | `floor`, `ceil`, `round`, `trunc`, `abs`, `sqrt`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `exp`, `log`, `log2`, `log10`, `pow`, `hypot`, `fmod`, `fdim`, `copysign`, `isfinite`, `isinf`, `isnan`, `isnormal` |
| `rational_math.h` | `floor`, `ceil`, `round`, `trunc`, `abs`, `pow` |

### Utilities

| Header | Description |
|--------|-------------|
| `format.h` | `fmt`-based formatting (separate `:format` Bazel target) |
| `ostream.h` | `operator<<` overloads for all types |

## Usage (Bazel)

```python
cc_binary(
    name = "my_target",
    deps = ["//multiprecision"],
)
```

```cpp
#include "multiprecision/fixed_point.h"
#include "multiprecision/fixed_point_math.h"

using FP = rtw::multiprecision::FixedPoint16;

constexpr FP angle(1.5);
constexpr FP result = rtw::multiprecision::math::sin(angle);
static_assert(result > FP(0.99) && result < FP(1.0));
```

For `fmt` integration, depend on `//multiprecision:format`.

## Constexpr Support

All types and functions are fully `constexpr` in C++17 (except `Rational` construction from floating-point, which uses `std::frexp`/`std::ldexp`).

```cpp
#include "multiprecision/int128.h"
#include "multiprecision/rational.h"

using rtw::multiprecision::Int128;
using rtw::multiprecision::Rational32;

// Int128 arithmetic at compile time
static_assert((Int128{7} * Int128{6}).lo() == 42);
static_assert(-Int128::min() == Int128::min());  // wraps safely (no UB)

// Rational arithmetic at compile time
constexpr Rational32 half(1, 2);
constexpr Rational32 third(1, 3);
static_assert(half + third == Rational32(5, 6));
```

## Overflow Handling

| Type | Overflow Strategy |
|------|-------------------|
| `FixedPoint` | **Saturates** to min/max. All operators use a wider `SaturationT` intermediate. |
| `Int128` | **Wraps** (two's complement). Matches native integer semantics. |
| `Rational` | **Asserts** if normalized result exceeds T's range. All arithmetic uses a double-width intermediate (int64/Int128) for cross-products; comparisons also use widened cross-multiplication. |

## Edge Case Handling

- `FixedPoint::operator-()` -- saturates `-MIN` to `MAX`
- `Int128::operator-()` -- uses unsigned arithmetic internally (no signed overflow UB on `min()`)
- `Rational::operator-()` -- negates denominator (not numerator) to avoid `-MIN` UB
- `Rational::normalize()` -- binary factor-of-2 reduction for `MIN` edge cases
- `ceil()`/`round()` -- uses `SaturationT` for near-MAX values; saturates when result exceeds range
- `exp()` -- casts to unsigned before left-shift (avoids C++17 signed shift UB)
- `div_signed(MIN, -1)` -- asserts (result unrepresentable)
- `abs(MIN)` -- saturates to `MAX` (all types)

## Build & Test

```bash
# Build the library
bazel build //multiprecision

# Run all tests (includes death tests, static_asserts, typed tests, property tests)
bazel test //multiprecision/...
```

Test target: `//multiprecision/tests:multiprecision_test`

Test coverage includes:
- Typed tests across all 6 FixedPoint, 8 Int, and 2 Rational aliases
- Death tests for every `assert`-guarded precondition
- Property-based algebraic tests (commutativity, identity, inverse, distributivity)
- Precision sweeps (sqrt over full FixedPoint8 range, sin/cos over 720 degrees)
- 100+ `static_assert` compile-time correctness proofs
