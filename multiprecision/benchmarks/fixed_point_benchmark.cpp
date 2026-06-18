#include "multiprecision/fixed_point.h"
#include "multiprecision/int128.h"

#include <benchmark/benchmark.h>

#include <array>
#include <cstddef>
#include <cstdint>

// Micro-benchmarks for the multiprecision scalar types. The renderer's `double_precision` is
// `FixedPoint32`, whose saturation type is the software `Int128`; these benchmarks isolate the per-operation
// cost of `FixedPoint32` add / subtract / multiply (the operations on the per-pixel rasteriser hot path) and
// divide (which the rasteriser uses only per triangle, never per pixel), and contrast them with the
// native-`int64`-backed `FixedPoint16`.
//
// To back the "all operations are at the bit-identical optimization limit" audit with measurements rather
// than reasoning, every arithmetic operation of both types is covered: add / subtract / multiply / divide /
// modulo for `FixedPoint16` and `FixedPoint32`, and add / subtract / multiply / divide / modulo for the raw
// `Int128`. The `Int128` operands are sized to expose each primitive's real cost: full 128-bit values for
// the O(1) add / subtract (to exercise carry / borrow) and for division dividends (to drive the
// shift-and-subtract loop), with bounded non-zero divisors (magnitude >= 2, mixed sign) so division is
// always defined and never hits the `min() / -1` overflow. `bm_int128_mul` keeps 64-bit-ranged operands
// because that is exactly how `FixedPoint32` multiply uses it (both factors are sign-extended `int64`).

namespace
{

using rtw::multiprecision::FixedPoint16;
using rtw::multiprecision::FixedPoint32;
using rtw::multiprecision::Int128;

constexpr std::size_t COUNT = 256U;

/// Deterministic spread of values in roughly [-8, 8), generated with a small LCG so the benchmark is
/// reproducible and independent of <random>.
template <typename Fp>
std::array<Fp, COUNT> make_values(const std::uint64_t seed)
{
  std::array<Fp, COUNT> values{};
  std::uint64_t s = seed;
  for (auto& value : values)
  {
    s = (s * 6'364'136'223'846'793'005ULL) + 1'442'695'040'888'963'407ULL;
    const auto unit = static_cast<double>(s >> 11U) / static_cast<double>(std::uint64_t{1} << 53U); // [0, 1)
    value = Fp{(unit - 0.5) * 16.0};
  }
  return values;
}

/// Deterministic divisors with magnitude in roughly [1, 9) and mixed sign, so division is always defined
/// (no zero divisor) while still exercising both the same-sign and mixed-sign rounding paths.
template <typename Fp>
std::array<Fp, COUNT> make_divisors(const std::uint64_t seed)
{
  std::array<Fp, COUNT> values{};
  std::uint64_t s = seed;
  for (auto& value : values)
  {
    s = (s * 6'364'136'223'846'793'005ULL) + 1'442'695'040'888'963'407ULL;
    const auto unit = static_cast<double>(s >> 11U) / static_cast<double>(std::uint64_t{1} << 53U); // [0, 1)
    const double magnitude = 1.0 + (unit * 8.0);                                                    // [1, 9)
    const double sign = ((s >> 10U) & 1U) != 0U ? -1.0 : 1.0;
    value = Fp{magnitude * sign};
  }
  return values;
}

/// Deterministic full-width `Int128` values (independent high and low halves) so the O(1) add / subtract
/// exercise carry / borrow across the half boundary and division dividends drive the shift-and-subtract loop.
std::array<Int128, COUNT> make_int128_values(const std::uint64_t seed)
{
  std::array<Int128, COUNT> values{};
  std::uint64_t s = seed;
  for (auto& value : values)
  {
    s = (s * 6'364'136'223'846'793'005ULL) + 1'442'695'040'888'963'407ULL;
    const auto hi = static_cast<std::int64_t>(s);
    s = (s * 6'364'136'223'846'793'005ULL) + 1'442'695'040'888'963'407ULL;
    const auto lo = s;
    value = Int128{hi, lo};
  }
  return values;
}

/// Deterministic `Int128` divisors with magnitude in roughly [2, 2^32) and mixed sign: never zero and never
/// +/-1, so division is always defined and can never reach the `min() / -1` overflow precondition.
std::array<Int128, COUNT> make_int128_divisors(const std::uint64_t seed)
{
  std::array<Int128, COUNT> values{};
  std::uint64_t s = seed;
  for (auto& value : values)
  {
    s = (s * 6'364'136'223'846'793'005ULL) + 1'442'695'040'888'963'407ULL;
    const auto magnitude = static_cast<std::int64_t>((s >> 32U) + 2U); // [2, 2^32 + 1]
    const auto sign = ((s >> 10U) & 1U) != 0U ? std::int64_t{-1} : std::int64_t{1};
    value = Int128{magnitude * sign};
  }
  return values;
}

template <typename Fp>
void bm_add(benchmark::State& state)
{
  const auto lhs = make_values<Fp>(0x12'34'56'78'9a'bc'de'f0ULL);
  const auto rhs = make_values<Fp>(0x0f'ed'cb'a9'87'65'43'21ULL);
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] + rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

template <typename Fp>
void bm_sub(benchmark::State& state)
{
  const auto lhs = make_values<Fp>(0x12'34'56'78'9a'bc'de'f0ULL);
  const auto rhs = make_values<Fp>(0x0f'ed'cb'a9'87'65'43'21ULL);
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] - rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

template <typename Fp>
void bm_mul(benchmark::State& state)
{
  const auto lhs = make_values<Fp>(0x12'34'56'78'9a'bc'de'f0ULL);
  const auto rhs = make_values<Fp>(0x0f'ed'cb'a9'87'65'43'21ULL);
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] * rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

template <typename Fp>
void bm_div(benchmark::State& state)
{
  const auto lhs = make_values<Fp>(0x12'34'56'78'9a'bc'de'f0ULL);
  const auto rhs = make_divisors<Fp>(0x0f'ed'cb'a9'87'65'43'21ULL);
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] / rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

template <typename Fp>
void bm_mod(benchmark::State& state)
{
  const auto lhs = make_values<Fp>(0x12'34'56'78'9a'bc'de'f0ULL);
  const auto rhs = make_divisors<Fp>(0x0f'ed'cb'a9'87'65'43'21ULL);
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] % rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

void bm_int128_mul(benchmark::State& state)
{
  std::array<Int128, COUNT> lhs{};
  std::array<Int128, COUNT> rhs{};
  std::uint64_t s = 0x12'34'56'78'9a'bc'de'f0ULL;
  for (std::size_t i = 0U; i < COUNT; ++i)
  {
    s = (s * 6'364'136'223'846'793'005ULL) + 1'442'695'040'888'963'407ULL;
    lhs[i] = Int128{static_cast<std::int64_t>(s)};
    s = (s * 6'364'136'223'846'793'005ULL) + 1'442'695'040'888'963'407ULL;
    rhs[i] = Int128{static_cast<std::int64_t>(s)};
  }
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] * rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

void bm_int128_add(benchmark::State& state)
{
  const auto lhs = make_int128_values(0x12'34'56'78'9a'bc'de'f0ULL);
  const auto rhs = make_int128_values(0x0f'ed'cb'a9'87'65'43'21ULL);
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] + rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

void bm_int128_sub(benchmark::State& state)
{
  const auto lhs = make_int128_values(0x12'34'56'78'9a'bc'de'f0ULL);
  const auto rhs = make_int128_values(0x0f'ed'cb'a9'87'65'43'21ULL);
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] - rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

void bm_int128_div(benchmark::State& state)
{
  const auto lhs = make_int128_values(0x12'34'56'78'9a'bc'de'f0ULL);
  const auto rhs = make_int128_divisors(0x0f'ed'cb'a9'87'65'43'21ULL);
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] / rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

void bm_int128_mod(benchmark::State& state)
{
  const auto lhs = make_int128_values(0x12'34'56'78'9a'bc'de'f0ULL);
  const auto rhs = make_int128_divisors(0x0f'ed'cb'a9'87'65'43'21ULL);
  for (auto _ : state)
  {
    for (std::size_t i = 0U; i < COUNT; ++i)
    {
      auto result = lhs[i] % rhs[i];
      benchmark::DoNotOptimize(result);
    }
  }
}

} // namespace

BENCHMARK_TEMPLATE(bm_add, FixedPoint16);
BENCHMARK_TEMPLATE(bm_add, FixedPoint32);
BENCHMARK_TEMPLATE(bm_sub, FixedPoint16);
BENCHMARK_TEMPLATE(bm_sub, FixedPoint32);
BENCHMARK_TEMPLATE(bm_mul, FixedPoint16);
BENCHMARK_TEMPLATE(bm_mul, FixedPoint32);
BENCHMARK_TEMPLATE(bm_div, FixedPoint16);
BENCHMARK_TEMPLATE(bm_div, FixedPoint32);
BENCHMARK_TEMPLATE(bm_mod, FixedPoint16);
BENCHMARK_TEMPLATE(bm_mod, FixedPoint32);
BENCHMARK(bm_int128_add);
BENCHMARK(bm_int128_sub);
BENCHMARK(bm_int128_mul);
BENCHMARK(bm_int128_div);
BENCHMARK(bm_int128_mod);

BENCHMARK_MAIN();
