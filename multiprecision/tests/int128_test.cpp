#include "multiprecision/format.h" // IWYU pragma: keep
#include "multiprecision/int128.h"

#include <gtest/gtest.h>

#include <array>

// These aliases are used solely for the purpose of testing of the Int class against the built-in integer types.
// They should not be used in production code.
namespace rtw::multiprecision
{
using Int16 = Int<std::int8_t>;
using Int16U = Int<std::uint8_t>;
using Int32 = Int<std::int16_t>;
using Int32U = Int<std::uint16_t>;
using Int64 = Int<std::int32_t>;
using Int64U = Int<std::uint32_t>;
} // namespace rtw::multiprecision

// -----------------------------------------------------------------------------

namespace
{
template <typename PackedIntT, typename HiIntT, typename LoIntT>
constexpr PackedIntT pack_hi_lo(const HiIntT hi, const LoIntT lo)
{
  using PackedUIntT = std::make_unsigned_t<PackedIntT>;
  using HiUIntT = std::make_unsigned_t<HiIntT>;

  constexpr std::uint8_t WORD_SIZE = sizeof(LoIntT) * 8U;

  // Input values need to be upper casted to the unsigned type to avoid undefined behavior:
  // 1. cast signed value to unsigned type of the same size (e.g. int8_t -> uint8_t)
  // 2. store the result in a larger unsigned type (e.g. uint8_t -> uint16_t)
  // 3. shift left the value to the higher word by the size of the lower word
  // 4. OR the result with the lower word
  // 5. cast the result to the packed type
  if constexpr (std::is_same_v<PackedUIntT, std::uint16_t> && std::is_same_v<HiUIntT, std::uint8_t>)
  {
    const std::uint16_t hi_u = static_cast<LoIntT>(hi);
    const std::uint16_t result = (static_cast<std::uint32_t>(hi_u) << WORD_SIZE) | lo;
    return static_cast<PackedIntT>(result);
  }
  else if constexpr (std::is_same_v<PackedUIntT, std::uint32_t> && std::is_same_v<HiUIntT, std::uint16_t>)
  {
    const std::uint32_t hi_u = static_cast<LoIntT>(hi);
    const std::uint32_t result = (hi_u << WORD_SIZE) | lo;
    return static_cast<PackedIntT>(result);
  }
  else if constexpr (std::is_same_v<PackedUIntT, std::uint64_t> && std::is_same_v<HiUIntT, std::uint32_t>)
  {
    const std::uint64_t hi_u = static_cast<LoIntT>(hi);
    const std::uint64_t result = (hi_u << WORD_SIZE) | lo;
    return static_cast<PackedIntT>(result);
  }
  else
  {
    static_assert(sizeof(PackedIntT) == 0,
                  "Unsupported type"); // workaround before CWG2518/P2593R1
  }
  return 0;
}

template <typename PackedIntT, typename HiIntT, typename LoIntT>
void unpack_hi_lo(const PackedIntT packed, HiIntT& hi, LoIntT& lo)
{
  using PackedUIntT = std::make_unsigned_t<PackedIntT>;
  using HiUIntT = std::make_unsigned_t<HiIntT>;

  constexpr std::uint8_t WORD_SIZE = sizeof(LoIntT) * 8U;
  constexpr PackedUIntT LO_MASK = (PackedUIntT{1U} << WORD_SIZE) - PackedUIntT{1U};

  // The packed value needs to be casted to the unsigned type to avoid undefined behavior:
  // 1. cast the packed value to the unsigned type of the same size (e.g. int8_t -> uint8_t)
  // 2. store the result in a larger unsigned type (e.g. uint8_t -> uint16_t)
  // 3. shift right the result by the size of the lower word
  // 4. cast the result to the hi part
  // 5. AND the packed value with the mask to get the lower word
  // 6. cast the result to the lo part
  if constexpr (std::is_same_v<PackedUIntT, std::uint16_t> && std::is_same_v<HiUIntT, std::uint8_t>)
  {
    const std::uint32_t packed_u = static_cast<std::uint16_t>(packed);
    hi = static_cast<HiIntT>(packed_u >> WORD_SIZE);
    lo = static_cast<LoIntT>(packed_u & LO_MASK);
  }
  else if constexpr (std::is_same_v<PackedUIntT, std::uint32_t> && std::is_same_v<HiUIntT, std::uint16_t>)
  {
    const std::uint64_t packed_u = static_cast<std::uint32_t>(packed);
    hi = static_cast<HiIntT>(packed_u >> WORD_SIZE);
    lo = static_cast<LoIntT>(packed_u & LO_MASK);
  }
  else if constexpr (std::is_same_v<PackedUIntT, std::uint64_t> && std::is_same_v<HiUIntT, std::uint32_t>)
  {
    const auto packed_u = static_cast<std::uint64_t>(packed);
    hi = static_cast<HiIntT>(packed_u >> WORD_SIZE);
    lo = static_cast<LoIntT>(packed_u & LO_MASK);
  }
  else
  {
    static_assert(sizeof(PackedIntT) == 0,
                  "Unsupported type"); // workaround before CWG2518/P2593R1
  }
}

template <typename IntT, typename ResultT, typename ContainerT>
void check_conversion_ctor(const ContainerT& test_case)
{
  using CustomIntT = rtw::multiprecision::Int<IntT>;

  for (const auto& expected : test_case)
  {
    const CustomIntT result{expected};
    const auto result_t = pack_hi_lo<ResultT>(result.hi(), result.lo());
    const IntT expected_hi = -rtw::multiprecision::math::signbit(expected); // The sign is in the hi part
    const auto expected_lo =
        static_cast<typename CustomIntT::lo_type>(expected); // The lo part is the same as the input
    EXPECT_EQ(result_t, expected);
    EXPECT_EQ(result.hi(), expected_hi);
    EXPECT_EQ(result.lo(), expected_lo);
  }
}

template <typename IntT, typename ResultT, typename CustomIntT>
constexpr void check_result(const CustomIntT result, const ResultT expected)
{
  using LoIntT = std::make_unsigned_t<IntT>;
  IntT expected_hi;
  LoIntT expected_lo;
  unpack_hi_lo(expected, expected_hi, expected_lo);
  const auto result_t = pack_hi_lo<ResultT>(result.hi(), result.lo());
  EXPECT_EQ(result_t, expected);
  EXPECT_EQ(result.hi(), expected_hi);
  EXPECT_EQ(result.lo(), expected_lo);
}

enum class OperationType : std::uint8_t
{
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
};

// NOLINTBEGIN(readability-function-cognitive-complexity)
template <typename IntT, typename ResultT, OperationType OPERATION, typename CustomIntT>
void check_operation(const typename CustomIntT::hi_type a, const typename CustomIntT::hi_type b)
{
  if constexpr (OPERATION == OperationType::ADD)
  {
    check_result<IntT, ResultT>(CustomIntT{a} + CustomIntT{b}, ResultT{a} + ResultT{b});
  }
  else if constexpr (OPERATION == OperationType::SUB)
  {
    check_result<IntT, ResultT>(CustomIntT{a} - CustomIntT{b}, ResultT{a} - ResultT{b});
  }
  else if constexpr (OPERATION == OperationType::MUL)
  {
    check_result<IntT, ResultT>(CustomIntT{a} * CustomIntT{b}, ResultT{a} * ResultT{b});
  }
  else if constexpr (OPERATION == OperationType::DIV)
  {
    if (b == 0)
    {
      EXPECT_DEATH(CustomIntT{a} / CustomIntT{b}, "");
      return;
    }

    check_result<IntT, ResultT>(CustomIntT{a} / CustomIntT{b}, ResultT{a} / ResultT{b});
  }
  else if constexpr (OPERATION == OperationType::MOD)
  {
    if (b == 0)
    {
      EXPECT_DEATH(CustomIntT{a} % CustomIntT{b}, "");
      return;
    }

    check_result<IntT, ResultT>(CustomIntT{a} % CustomIntT{b}, ResultT{a} % ResultT{b});
  }
}
// NOLINTEND(readability-function-cognitive-complexity)

template <typename IntT, typename ResultT, OperationType OPERATION, typename ContainerT,
          typename CustomIntT = rtw::multiprecision::Int<IntT>>
void check(const ContainerT& test_case)
{
  for (const auto& [a, b] : test_case)
  {
    check_operation<IntT, ResultT, OPERATION, CustomIntT>(a, b);
    check_operation<IntT, ResultT, OPERATION, CustomIntT>(b, a);
  }
}
} // namespace

// -----------------------------------------------------------------------------
// These tests cases are done solely for the purpose to test base class operations against the built-in types.
// The aliases declared above should not be used in producation code.

TEST(Int16, conversion_ctor)
{
  using rtw::multiprecision::Int16;

  constexpr std::array TEST_CASE = {
      std::int8_t{0}, std::int8_t{-128}, std::int8_t{-19}, std::int8_t{17}, std::int8_t{127},
  };

  check_conversion_ctor<std::int8_t, std::int16_t>(TEST_CASE);
}

TEST(Int16U, conversion_ctor)
{
  using rtw::multiprecision::Int16U;

  constexpr std::array TEST_CASE = {
      std::uint8_t{0},
      std::uint8_t{17},
      std::uint8_t{19},
      std::uint8_t{255},
  };

  check_conversion_ctor<std::uint8_t, std::uint16_t>(TEST_CASE);
}

TEST(Int32, conversion_ctor)
{
  using rtw::multiprecision::Int32;

  constexpr std::array TEST_CASE = {
      std::int16_t{0}, std::int16_t{-32'768}, std::int16_t{-19}, std::int16_t{17}, std::int16_t{32'767},
  };

  check_conversion_ctor<std::int16_t, std::int32_t>(TEST_CASE);
}

TEST(Int32U, conversion_ctor)
{
  using rtw::multiprecision::Int32U;

  constexpr std::array TEST_CASE = {
      std::uint16_t{0},
      std::uint16_t{17},
      std::uint16_t{19},
      std::uint16_t{65'535},
  };

  check_conversion_ctor<std::uint16_t, std::uint32_t>(TEST_CASE);
}

TEST(Int64, conversion_ctor)
{
  using rtw::multiprecision::Int64;

  constexpr std::array TEST_CASE = {
      std::int32_t{0}, std::int32_t{-2'147'483'648}, std::int32_t{-19}, std::int32_t{17}, std::int32_t{2'147'483'647},
  };

  check_conversion_ctor<std::int32_t, std::int64_t>(TEST_CASE);
}

TEST(Int64U, conversion_ctor)
{
  using rtw::multiprecision::Int64U;

  constexpr std::array TEST_CASE = {
      std::uint32_t{0},
      std::uint32_t{17},
      std::uint32_t{19},
      std::uint32_t{4'294'967'295},
  };

  check_conversion_ctor<std::uint32_t, std::uint64_t>(TEST_CASE);
}

TEST(Int16, arithmetic_operations)
{
  using rtw::multiprecision::Int16;

  constexpr std::array TEST_CASE = {
      std::pair<std::int8_t, std::int8_t>{0, 0},      std::pair<std::int8_t, std::int8_t>{-128, -128},
      std::pair<std::int8_t, std::int8_t>{-128, 127}, std::pair<std::int8_t, std::int8_t>{127, -128},
      std::pair<std::int8_t, std::int8_t>{127, 127},  std::pair<std::int8_t, std::int8_t>{19, 17},
      std::pair<std::int8_t, std::int8_t>{19, -17},   std::pair<std::int8_t, std::int8_t>{-19, 17},
      std::pair<std::int8_t, std::int8_t>{-19, -17},
  };

  check<std::int8_t, std::int16_t, OperationType::ADD>(TEST_CASE);
  check<std::int8_t, std::int16_t, OperationType::SUB>(TEST_CASE);
  check<std::int8_t, std::int16_t, OperationType::MUL>(TEST_CASE);
  check<std::int8_t, std::int16_t, OperationType::DIV>(TEST_CASE);
  check<std::int8_t, std::int16_t, OperationType::MOD>(TEST_CASE);
}

TEST(Int16U, arithmetic_operations)
{
  using rtw::multiprecision::Int16U;

  constexpr std::array TEST_CASE = {
      std::pair<std::uint8_t, std::uint8_t>{0, 0},   std::pair<std::uint8_t, std::uint8_t>{0, 255},
      std::pair<std::uint8_t, std::uint8_t>{255, 0}, std::pair<std::uint8_t, std::uint8_t>{255, 255},
      std::pair<std::uint8_t, std::uint8_t>{19, 17},
  };

  check<std::uint8_t, std::uint16_t, OperationType::ADD>(TEST_CASE);
  check<std::uint8_t, std::uint16_t, OperationType::SUB>(TEST_CASE);
  check<std::uint8_t, std::uint16_t, OperationType::MUL>(TEST_CASE);
  check<std::uint8_t, std::uint16_t, OperationType::DIV>(TEST_CASE);
  check<std::uint8_t, std::uint16_t, OperationType::MOD>(TEST_CASE);
}

TEST(Int32, arithmetic_operations)
{
  using rtw::multiprecision::Int32;

  constexpr std::array TEST_CASE = {
      std::pair<std::int16_t, std::int16_t>{0, 0},
      std::pair<std::int16_t, std::int16_t>{-32'768, -32'768},
      std::pair<std::int16_t, std::int16_t>{-32'768, 32'767},
      std::pair<std::int16_t, std::int16_t>{32'767, -32'768},
      std::pair<std::int16_t, std::int16_t>{32'767, 32'767},
      std::pair<std::int16_t, std::int16_t>{19, 17},
      std::pair<std::int16_t, std::int16_t>{19, -17},
      std::pair<std::int16_t, std::int16_t>{-19, 17},
      std::pair<std::int16_t, std::int16_t>{-19, -17},
  };

  check<std::int16_t, std::int32_t, OperationType::ADD>(TEST_CASE);
  check<std::int16_t, std::int32_t, OperationType::SUB>(TEST_CASE);
  check<std::int16_t, std::int32_t, OperationType::MUL>(TEST_CASE);
  check<std::int16_t, std::int32_t, OperationType::DIV>(TEST_CASE);
  check<std::int16_t, std::int32_t, OperationType::MOD>(TEST_CASE);
}

TEST(Int32U, arithmetic_operations)
{
  using rtw::multiprecision::Int32U;

  constexpr std::array TEST_CASE = {
      std::pair<std::uint16_t, std::uint16_t>{0, 0},      std::pair<std::uint16_t, std::uint16_t>{0, 65'535},
      std::pair<std::uint16_t, std::uint16_t>{65'535, 0}, std::pair<std::uint16_t, std::uint16_t>{65'535, 65'535},
      std::pair<std::uint16_t, std::uint16_t>{19, 17},
  };

  check<std::uint16_t, std::uint32_t, OperationType::ADD>(TEST_CASE);
  check<std::uint16_t, std::uint32_t, OperationType::SUB>(TEST_CASE);
  check<std::uint16_t, std::uint32_t, OperationType::MUL>(TEST_CASE);
  check<std::uint16_t, std::uint32_t, OperationType::DIV>(TEST_CASE);
  check<std::uint16_t, std::uint32_t, OperationType::MOD>(TEST_CASE);
}

TEST(Int64, arithmetic_operations)
{
  using rtw::multiprecision::Int64;

  constexpr std::array TEST_CASE = {
      std::pair<std::int32_t, std::int32_t>{0, 0},
      std::pair<std::int32_t, std::int32_t>{-2'147'483'648, -2'147'483'648},
      std::pair<std::int32_t, std::int32_t>{-2'147'483'648, 2'147'483'647},
      std::pair<std::int32_t, std::int32_t>{2'147'483'647, -2'147'483'648},
      std::pair<std::int32_t, std::int32_t>{2'147'483'647, 2'147'483'647},
      std::pair<std::int32_t, std::int32_t>{19, 17},
      std::pair<std::int32_t, std::int32_t>{19, -17},
      std::pair<std::int32_t, std::int32_t>{-19, 17},
      std::pair<std::int32_t, std::int32_t>{-19, -17},
  };

  check<std::int32_t, std::int64_t, OperationType::ADD>(TEST_CASE);
  check<std::int32_t, std::int64_t, OperationType::SUB>(TEST_CASE);
  check<std::int32_t, std::int64_t, OperationType::MUL>(TEST_CASE);
  check<std::int32_t, std::int64_t, OperationType::DIV>(TEST_CASE);
  check<std::int32_t, std::int64_t, OperationType::MOD>(TEST_CASE);
}

TEST(Int64U, arithmetic_operations)
{
  using rtw::multiprecision::Int64U;

  constexpr std::array TEST_CASE = {
      std::pair<std::uint32_t, std::uint32_t>{0, 0},
      std::pair<std::uint32_t, std::uint32_t>{0, 4'294'967'295},
      std::pair<std::uint32_t, std::uint32_t>{4'294'967'295, 0},
      std::pair<std::uint32_t, std::uint32_t>{4'294'967'295, 4'294'967'295},
      std::pair<std::uint32_t, std::uint32_t>{19, 17},
  };

  check<std::uint32_t, std::uint64_t, OperationType::ADD>(TEST_CASE);
  check<std::uint32_t, std::uint64_t, OperationType::SUB>(TEST_CASE);
  check<std::uint32_t, std::uint64_t, OperationType::MUL>(TEST_CASE);
  check<std::uint32_t, std::uint64_t, OperationType::DIV>(TEST_CASE);
  check<std::uint32_t, std::uint64_t, OperationType::MOD>(TEST_CASE);
}

template <typename T>
class IntTest : public ::testing::Test
{};

using IntTypes = ::testing::Types<rtw::multiprecision::Int16, rtw::multiprecision::Int16U, rtw::multiprecision::Int32,
                                  rtw::multiprecision::Int32U, rtw::multiprecision::Int64, rtw::multiprecision::Int64U,
                                  rtw::multiprecision::Int128, rtw::multiprecision::Int128U>;
TYPED_TEST_SUITE(IntTest, IntTypes, );

TYPED_TEST(IntTest, traits)
{
  static_assert(std::is_trivially_default_constructible_v<TypeParam>);
  static_assert(std::is_nothrow_default_constructible_v<TypeParam>);
  static_assert(std::is_trivially_copy_constructible_v<TypeParam>);
  static_assert(std::is_trivially_copy_assignable_v<TypeParam>);
  static_assert(std::is_trivially_destructible_v<TypeParam>);
  static_assert(std::is_nothrow_move_constructible_v<TypeParam>);
  static_assert(std::is_nothrow_move_assignable_v<TypeParam>);
  static_assert(std::is_nothrow_swappable_v<TypeParam>);
}

TEST(Int128, arithmetic_operations)
{
  using rtw::multiprecision::Int128;

  {
    // Addition
    EXPECT_EQ(Int128{0} + Int128{0}, 0);
    EXPECT_EQ(Int128{0} + Int128{-1}, -1);
    EXPECT_EQ(Int128{1} + Int128{0}, 1);
    EXPECT_EQ(Int128{42} + Int128{42}, 84);
    EXPECT_EQ(Int128{42} + Int128{-42}, 0);
    EXPECT_EQ(Int128{-42} + Int128{-42}, -84);
    EXPECT_EQ(Int128::max() + Int128::min(), -1);
    EXPECT_EQ(Int128::min() + Int128::max(), -1);
  }
  {
    // Subtraction
    EXPECT_EQ(Int128{0} - Int128{0}, 0);
    EXPECT_EQ(Int128{0} - Int128{-1}, 1);
    EXPECT_EQ(Int128{1} - Int128{0}, 1);
    EXPECT_EQ(Int128{42} - Int128{42}, 0);
    EXPECT_EQ(Int128{42} - Int128{-42}, 84);
    EXPECT_EQ(Int128{-42} - Int128{-42}, 0);
    EXPECT_EQ(Int128::max() - Int128::max(), 0);
    EXPECT_EQ(Int128::min() - Int128::min(), 0);
  }
  {
    // Multiplication
    EXPECT_EQ(Int128{0} * Int128{0}, 0);
    EXPECT_EQ(Int128{0} * Int128{-1}, 0);
    EXPECT_EQ(Int128{1} * Int128{0}, 0);
    EXPECT_EQ(Int128{42} * Int128{42}, 1'764);
    EXPECT_EQ(Int128{42} * Int128{-42}, -1'764);
    EXPECT_EQ(Int128{-42} * Int128{-42}, 1'764);
    EXPECT_EQ(Int128::max() * -1, Int128::min() + 1);
    EXPECT_EQ(Int128::min() * -1, Int128::max() + 1);
  }
  {
    // Division
    EXPECT_EQ(Int128{0} / Int128{1}, 0);
    EXPECT_EQ(Int128{1} / Int128{1}, 1);
    EXPECT_EQ(Int128{42} / Int128{42}, 1);
    EXPECT_EQ(Int128{42} / Int128{-42}, -1);
    EXPECT_EQ(Int128{-42} / Int128{-42}, 1);
    EXPECT_EQ(Int128::max() / Int128::max(), 1);
    EXPECT_EQ(Int128::min() / Int128::min(), 1);
    EXPECT_DEATH(Int128{-1} / Int128{0}, "");
  }
  {
    // Modulo
    EXPECT_EQ(Int128{0} % Int128{1}, 0);
    EXPECT_EQ(Int128{1} % Int128{1}, 0);
    EXPECT_EQ(Int128{42} % Int128{42}, 0);
    EXPECT_EQ(Int128{42} % Int128{-42}, 0);
    EXPECT_EQ(Int128{-42} % Int128{-42}, 0);
    EXPECT_EQ(Int128::max() % Int128::max(), 0);
    EXPECT_EQ(Int128::min() % Int128::min(), 0);
    EXPECT_DEATH(Int128{-1} % Int128{0}, "");
  }
}

TEST(Int128U, arithmetic_operations)
{
  using rtw::multiprecision::Int128U;

  {
    // Addition
    EXPECT_EQ(Int128U{0} + Int128U{0}, 0);
    EXPECT_EQ(Int128U{0} + Int128U{1}, 1);
    EXPECT_EQ(Int128U{1} + Int128U{0}, 1);
    EXPECT_EQ(Int128U{42} + Int128U{42}, 84);

    auto result =
        Int128U{std::numeric_limits<std::uint64_t>::max()} + Int128U{std::numeric_limits<std::uint64_t>::max()};
    EXPECT_EQ(result.hi(), 1);
    EXPECT_EQ(result.lo(), std::numeric_limits<std::uint64_t>::max() - 1);

    result = Int128U::max() + Int128U::max();
    EXPECT_EQ(result.hi(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(result.lo(), std::numeric_limits<std::uint64_t>::max() - 1);

    EXPECT_EQ(Int128U::max() + Int128U{1}, 0);
    EXPECT_EQ(Int128U{1} + Int128U::max(), 0);
  }

  {
    // Subtraction
    EXPECT_EQ(Int128U{0} - Int128U{0}, 0);
    EXPECT_EQ(Int128U{1} - Int128U{0}, 1);
    EXPECT_EQ(Int128U{1} - Int128U{1}, 0);
    EXPECT_EQ(Int128U{42} - Int128U{42}, 0);

    auto result = Int128U{std::numeric_limits<std::uint64_t>::max()} - Int128U{0};
    EXPECT_EQ(result, std::numeric_limits<std::uint64_t>::max());

    result = Int128U{0} - Int128U{std::numeric_limits<std::uint64_t>::max()};
    EXPECT_EQ(result.hi(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(result.lo(), 1);

    result = Int128U{0} - Int128U{1};
    EXPECT_EQ(result.hi(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(result.lo(), std::numeric_limits<std::uint64_t>::max());
  }

  {
    // Multiplication
    EXPECT_EQ(Int128U{0} * Int128U{0}, 0);
    EXPECT_EQ(Int128U{1} * Int128U{0}, 0);
    EXPECT_EQ(Int128U{42} * 2, 84);

    auto result = Int128U{std::numeric_limits<std::uint64_t>::max()} * Int128U{0};
    EXPECT_EQ(result, 0);

    result = Int128U{0} * Int128U{std::numeric_limits<std::uint64_t>::max()};
    EXPECT_EQ(result, 0);

    result = Int128U{std::numeric_limits<std::uint64_t>::max()} * Int128U{1};
    EXPECT_EQ(result, std::numeric_limits<std::uint64_t>::max());

    EXPECT_EQ(Int128U::max() * Int128U::max(), 1);
    EXPECT_EQ(Int128U::max() * 2, Int128U::max() - 1);
    EXPECT_EQ(Int128U::max() * 2, Int128U::max() << 1);
  }

  {
    // Division
    EXPECT_EQ(Int128U{0} / Int128U{1}, 0);
    EXPECT_EQ(Int128U{1} / Int128U{1}, 1);
    EXPECT_EQ(Int128U{42} / 2, 21);
    EXPECT_DEATH(Int128U{1} / Int128U{0}, "");

    auto result = Int128U{std::numeric_limits<std::uint64_t>::max()} / Int128U{1};
    EXPECT_EQ(result, std::numeric_limits<std::uint64_t>::max());

    result = Int128U::max() / 2;
    EXPECT_EQ(result, Int128U::max() >> 1);

    EXPECT_EQ(Int128U::max() / Int128U::max(), 1);
  }

  {
    // Modulo
    EXPECT_EQ(Int128U{0} % Int128U{1}, 0);
    EXPECT_EQ(Int128U{1} % Int128U{1}, 0);
    EXPECT_EQ(Int128U{42} % 2, 0);
    EXPECT_DEATH(Int128U{1} % Int128U{0}, "");

    auto result = Int128U{std::numeric_limits<std::uint64_t>::max()} % Int128U{1};
    EXPECT_EQ(result, 0);

    result = Int128U::max() % 2;
    EXPECT_EQ(result, 1);

    EXPECT_EQ(Int128U::max() % Int128U::max(), 0);
  }
}

TEST(Int, negate)
{
  using rtw::multiprecision::Int128;
  using rtw::multiprecision::Int16;
  using rtw::multiprecision::Int32;
  using rtw::multiprecision::Int64;

  {
    EXPECT_EQ(-Int16(36), Int16(-36));
    EXPECT_EQ(-Int16(-36), Int16(36));
  }
  {
    EXPECT_EQ(-Int32(369), Int32(-369));
    EXPECT_EQ(-Int32(-369), Int32(369));
  }
  {
    EXPECT_EQ(-Int64(369), Int64(-369));
    EXPECT_EQ(-Int64(-369), Int64(369));
  }
  {
    EXPECT_EQ(-Int128(369), Int128(-369));
    EXPECT_EQ(-Int128(-369), Int128(369));
  }

  using rtw::multiprecision::Int128U;
  using rtw::multiprecision::Int16U;
  using rtw::multiprecision::Int32U;
  using rtw::multiprecision::Int64U;

  {
    EXPECT_EQ(-Int16U(36), Int16U(-36));
    EXPECT_EQ(-Int16U(-36), Int16U(36));
  }
  {
    EXPECT_EQ(-Int32U(369), Int32U(-369));
    EXPECT_EQ(-Int32U(-369), Int32U(369));
  }
  {
    EXPECT_EQ(-Int64U(369), Int64U(-369));
    EXPECT_EQ(-Int64U(-369), Int64U(369));
  }
  {
    EXPECT_EQ(-Int128U(369), Int128U(-369));
    EXPECT_EQ(-Int128U(-369), Int128U(369));
  }
}

TEST(Int16u, bitwise_shifts)
{
  using rtw::multiprecision::Int16U;

  Int16U a = 1;
  a <<= 0;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);

  a <<= 15;
  EXPECT_EQ(a.hi(), 0x80);
  EXPECT_EQ(a.lo(), 0);

  a >>= 15;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);

  a >>= 0;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);
}

TEST(Int32U, bitwise_shifts)
{
  using rtw::multiprecision::Int32U;

  Int32U a = 1;
  a <<= 0;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);

  a <<= 31;
  EXPECT_EQ(a.hi(), 0x8000);
  EXPECT_EQ(a.lo(), 0);

  a >>= 31;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);

  a >>= 0;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);
}

TEST(Int64U, bitwise_shifts)
{
  using rtw::multiprecision::Int64U;

  Int64U a = 1;
  a <<= 0;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);

  a <<= 63;
  EXPECT_EQ(a.hi(), 0x80'00'00'00);
  EXPECT_EQ(a.lo(), 0);

  a >>= 63;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);

  a >>= 0;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);
}

TEST(Int128U, bitwise_shifts)
{
  using rtw::multiprecision::Int128U;

  Int128U a = 1;
  a <<= 127;
  EXPECT_EQ(a.hi(), 0x80'00'00'00'00'00'00'00);
  EXPECT_EQ(a.lo(), 0);

  a >>= 127;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);
}
// -----------------------------------------------------------------------------
TEST(operators, count_leading_zero_16)
{
  // Shift left
  {
    rtw::multiprecision::Int16U a = {0xFF, 0xFF};
    std::uint16_t expected = 0xFFFF;
    for (std::uint32_t i = 0; i <= rtw::multiprecision::Int16U::BITS; ++i)
    {
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
                rtw::multiprecision::math::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1U;
    }
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int16U::BITS);
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
              rtw::multiprecision::math::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::multiprecision::Int16U a = 1;
    std::uint16_t expected = 1;
    for (std::uint32_t i = 0; i <= rtw::multiprecision::Int16U::BITS; ++i)
    {
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
                rtw::multiprecision::math::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1U;
    }
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int16U::BITS); // a == 0
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
              rtw::multiprecision::math::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_32)
{
  // Shift left
  {
    rtw::multiprecision::Int32U a = {0xFFFF, 0xFFFF};
    std::uint32_t expected = 0xFF'FF'FF'FF;
    for (std::uint32_t i = 0; i <= rtw::multiprecision::Int32U::BITS; ++i)
    {
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), i);
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
                rtw::multiprecision::math::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1U;
    }
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int32U::BITS);
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
              rtw::multiprecision::math::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::multiprecision::Int32U a = 1;
    std::uint32_t expected = 1;
    for (std::uint32_t i = 0; i <= rtw::multiprecision::Int32U::BITS; ++i)
    {
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
                rtw::multiprecision::math::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1U;
    }
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int32U::BITS); // a == 0
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
              rtw::multiprecision::math::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_64)
{
  // Shift left
  {
    rtw::multiprecision::Int64U a = {0xFF'FF'FF'FF, 0xFF'FF'FF'FF};
    std::uint64_t expected = 0xFF'FF'FF'FF'FF'FF'FF'FF;
    for (std::uint32_t i = 0; i <= rtw::multiprecision::Int64U::BITS; ++i)
    {
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), i);
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
                rtw::multiprecision::math::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1U;
    }
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int64U::BITS);
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
              rtw::multiprecision::math::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::multiprecision::Int64U a = 1;
    std::uint64_t expected = 1;
    for (std::uint64_t i = 0; i <= rtw::multiprecision::Int64U::BITS; ++i)
    {
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
                rtw::multiprecision::math::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1U;
    }
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int64U::BITS); // a == 0
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a),
              rtw::multiprecision::math::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_128)
{
  // Shift left
  {
    rtw::multiprecision::Int128U a = {0xFF'FF'FF'FF'FF'FF'FF'FF, 0xFF'FF'FF'FF'FF'FF'FF'FF};
    for (std::uint32_t i = 0; i <= rtw::multiprecision::Int128U::BITS; ++i)
    {
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), i);
      a >>= 1;
    }
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int128U::BITS);
  }
  // Shift right
  {
    rtw::multiprecision::Int128U a = 1;
    for (std::uint64_t i = 0; i < rtw::multiprecision::Int128U::BITS; ++i)
    {
      EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int128U::BITS - i - 1);
      a <<= 1;
    }
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int128U::BITS); // a == 0
    a <<= 1;
    EXPECT_EQ(rtw::multiprecision::math::count_leading_zero(a), rtw::multiprecision::Int128U::BITS); // a == 0
  }
}

// =============================================================================
// Constexpr static_assert tests
// =============================================================================
namespace
{

// --- Int128 arithmetic at compile time ---
static_assert(rtw::multiprecision::Int128{0}.hi() == 0, "Int128 zero hi");
static_assert(rtw::multiprecision::Int128{0}.lo() == 0, "Int128 zero lo");
static_assert(rtw::multiprecision::Int128{42}.lo() == 42, "Int128 from int");
static_assert(rtw::multiprecision::Int128{-1}.hi() == -1, "Int128 -1 hi is all-ones");
static_assert(rtw::multiprecision::Int128{-1}.lo() == ~std::uint64_t{0}, "Int128 -1 lo is all-ones");

// Addition
static_assert((rtw::multiprecision::Int128{10} + rtw::multiprecision::Int128{20}).lo() == 30, "Int128 add");
static_assert((rtw::multiprecision::Int128{-5} + rtw::multiprecision::Int128{5}) == rtw::multiprecision::Int128{0},
              "Int128 add to zero");

// Subtraction
static_assert((rtw::multiprecision::Int128{100} - rtw::multiprecision::Int128{42}).lo() == 58, "Int128 sub");

// Multiplication
static_assert((rtw::multiprecision::Int128{7} * rtw::multiprecision::Int128{6}).lo() == 42, "Int128 mul");
static_assert((rtw::multiprecision::Int128{-3} * rtw::multiprecision::Int128{4}) == rtw::multiprecision::Int128{-12},
              "Int128 mul negative");

// Division
static_assert((rtw::multiprecision::Int128{42} / rtw::multiprecision::Int128{7}).lo() == 6, "Int128 div");
static_assert((rtw::multiprecision::Int128{-42} / rtw::multiprecision::Int128{7}) == rtw::multiprecision::Int128{-6},
              "Int128 div negative");

// Modulo
static_assert((rtw::multiprecision::Int128{10} % rtw::multiprecision::Int128{3}).lo() == 1, "Int128 mod");

// Shifts
static_assert((rtw::multiprecision::Int128{1} << 64U).hi() == 1, "Int128 shift left 64");
static_assert((rtw::multiprecision::Int128{1} << 64U).lo() == 0, "Int128 shift left 64 lo");
static_assert((rtw::multiprecision::Int128{0, 1} >> 1U).lo() == 0, "Int128 shift right");

// Bitwise
static_assert((rtw::multiprecision::Int128{0xFF} & rtw::multiprecision::Int128{0x0F}).lo() == 0x0F, "Int128 AND");
static_assert((rtw::multiprecision::Int128{0xF0} | rtw::multiprecision::Int128{0x0F}).lo() == 0xFF, "Int128 OR");
static_assert((rtw::multiprecision::Int128{0xFF} ^ rtw::multiprecision::Int128{0x0F}).lo() == 0xF0, "Int128 XOR");
static_assert((~rtw::multiprecision::Int128{0}).hi() == -1, "Int128 NOT hi");
static_assert((~rtw::multiprecision::Int128{0}).lo() == ~std::uint64_t{0}, "Int128 NOT lo");

// Comparisons
static_assert(rtw::multiprecision::Int128{1} > rtw::multiprecision::Int128{0}, "Int128 gt");
static_assert(rtw::multiprecision::Int128{-1} < rtw::multiprecision::Int128{0}, "Int128 lt");
static_assert(rtw::multiprecision::Int128{42} == rtw::multiprecision::Int128{42}, "Int128 eq");
static_assert(rtw::multiprecision::Int128{42} != rtw::multiprecision::Int128{43}, "Int128 ne");
static_assert(rtw::multiprecision::Int128::min() < rtw::multiprecision::Int128::max(), "Int128 min < max");

// Unsigned
static_assert((rtw::multiprecision::Int128U{100} + rtw::multiprecision::Int128U{200}).lo() == 300, "Int128U add");
static_assert((rtw::multiprecision::Int128U{50} * rtw::multiprecision::Int128U{3}).lo() == 150, "Int128U mul");

// --- math::abs at compile time ---
static_assert(rtw::multiprecision::math::abs(rtw::multiprecision::Int128{-42}) == rtw::multiprecision::Int128{42},
              "Int128 abs(-42)");
static_assert(rtw::multiprecision::math::abs(rtw::multiprecision::Int128{0}) == rtw::multiprecision::Int128{0},
              "Int128 abs(0)");
static_assert(rtw::multiprecision::math::abs(rtw::multiprecision::Int128{42}) == rtw::multiprecision::Int128{42},
              "Int128 abs(42)");
static_assert(rtw::multiprecision::math::abs(rtw::multiprecision::Int128::min()) == rtw::multiprecision::Int128::max(),
              "Int128 abs(min) saturates to max");
static_assert(rtw::multiprecision::math::abs(rtw::multiprecision::Int128::max()) == rtw::multiprecision::Int128::max(),
              "Int128 abs(max) == max");

// Unary negation of min(): wraps to min() (well-defined via unsigned arithmetic, no UB)
static_assert(-rtw::multiprecision::Int128::min() == rtw::multiprecision::Int128::min(),
              "Int128 -min() wraps to min() (no UB)");
static_assert((-rtw::multiprecision::Int128{42}) == rtw::multiprecision::Int128{-42}, "Int128 -42 negates correctly");
static_assert((-(-rtw::multiprecision::Int128{7})) == rtw::multiprecision::Int128{7},
              "Int128 double negation is identity");
static_assert((-rtw::multiprecision::Int128{0}) == rtw::multiprecision::Int128{0}, "Int128 -0 == 0");

} // namespace

// =============================================================================
// Additional runtime tests
// =============================================================================

TEST(Int128, float_roundtrip)
{
  using rtw::multiprecision::Int128;

  // Small values should roundtrip exactly through double
  for (std::int64_t v : {0L, 1L, -1L, 42L, -42L, 1'000'000L, -1'000'000L})
  {
    const Int128 original{v};
    const double as_double = static_cast<double>(original);
    const Int128 back{as_double};
    EXPECT_EQ(back, original) << "Roundtrip failed for " << v;
  }

  // Large values lose precision through float but should be approximately correct
  const Int128 large{std::int64_t{1} << 50, 0};
  const double as_double = static_cast<double>(large);
  const Int128 back{as_double};
  // Within floating-point precision (double has 53 bits of mantissa)
  EXPECT_EQ(back, large);
}

TEST(Int128, bitwise_not)
{
  using rtw::multiprecision::Int128;
  using rtw::multiprecision::Int128U;

  EXPECT_EQ(~Int128{0}, Int128{-1});
  EXPECT_EQ(~Int128{-1}, Int128{0});
  EXPECT_EQ(~Int128U{0}, Int128U::max());
  EXPECT_EQ(~~Int128{42}, Int128{42}); // Double complement is identity
}

TEST(Int128, modulo_operations)
{
  using rtw::multiprecision::Int128;

  EXPECT_EQ(Int128{10} % Int128{3}, Int128{1});
  EXPECT_EQ(Int128{10} % Int128{5}, Int128{0});
  EXPECT_EQ(Int128{-10} % Int128{3}, Int128{-1});
  EXPECT_EQ(Int128{10} % Int128{-3}, Int128{1});
  EXPECT_EQ(Int128{7} % Int128{7}, Int128{0});
  EXPECT_EQ(Int128{0} % Int128{42}, Int128{0});
}

TEST(Int128, comparison_operators)
{
  using rtw::multiprecision::Int128;

  EXPECT_TRUE(Int128{0} == Int128{0});
  EXPECT_TRUE(Int128{1} != Int128{0});
  EXPECT_TRUE(Int128{1} > Int128{0});
  EXPECT_TRUE(Int128{0} < Int128{1});
  EXPECT_TRUE(Int128{1} >= Int128{1});
  EXPECT_TRUE(Int128{1} <= Int128{1});
  EXPECT_TRUE(Int128{-1} < Int128{0});
  EXPECT_TRUE(Int128{-1} < Int128{1});
  EXPECT_TRUE(Int128::min() < Int128::max());
  EXPECT_TRUE(Int128::max() > Int128::min());
  EXPECT_TRUE(Int128::min() <= Int128::min());
  EXPECT_TRUE(Int128::max() >= Int128::max());
}

TEST(Int128, property_based_arithmetic)
{
  using rtw::multiprecision::Int128;

  // Commutativity of addition
  const Int128 a{123'456'789};
  const Int128 b{987'654'321};
  EXPECT_EQ(a + b, b + a);

  // Commutativity of multiplication
  EXPECT_EQ(a * b, b * a);

  // Additive identity
  EXPECT_EQ(a + Int128{0}, a);
  EXPECT_EQ(Int128{0} + a, a);

  // Multiplicative identity
  EXPECT_EQ(a * Int128{1}, a);
  EXPECT_EQ(Int128{1} * a, a);

  // Additive inverse
  EXPECT_EQ(a + (-a), Int128{0});

  // Subtraction as inverse of addition
  EXPECT_EQ((a + b) - b, a);
  EXPECT_EQ((a + b) - a, b);

  // Division/modulo relationship: a == (a/b)*b + (a%b)
  const Int128 q = a / b;
  const Int128 r = a % b;
  EXPECT_EQ(q * b + r, a);

  // Distributivity: a * (b + c) == a*b + a*c
  const Int128 c{42};
  EXPECT_EQ(a * (b + c), a * b + a * c);
}

TEST(Int128, division_by_zero_death)
{
  using rtw::multiprecision::Int128;
  using rtw::multiprecision::Int128U;

  EXPECT_DEATH(Int128{42} / Int128{0}, ".*");
  EXPECT_DEATH(Int128{42} % Int128{0}, ".*");
  EXPECT_DEATH(Int128U{42} / Int128U{0}, ".*");
  EXPECT_DEATH(Int128U{42} % Int128U{0}, ".*");
}

TEST(Int128, shift_overflow_death)
{
  using rtw::multiprecision::Int128;
  using rtw::multiprecision::Int128U;

  // Shift by BITS (128) or more is undefined — assert fires
  EXPECT_DEATH(Int128{1} << 128U, ".*");
  EXPECT_DEATH(Int128{1} >> 128U, ".*");
  EXPECT_DEATH(Int128U{1} << 128U, ".*");
  EXPECT_DEATH(Int128U{1} >> 128U, ".*");
  EXPECT_DEATH(Int128{1} << 200U, ".*");
}

TEST(Int128, div_signed_min_by_neg1_death)
{
  using rtw::multiprecision::Int128;

  // min() / -1 overflows: -min() exceeds max(). Assert fires.
  EXPECT_DEATH(Int128::min() / Int128{-1}, ".*");
  EXPECT_DEATH(Int128::min() % Int128{-1}, ".*");
}

TEST(Int128, abs_min_value_saturates)
{
  using rtw::multiprecision::Int128;

  // abs(min()) should saturate to max() (cannot represent -min in signed)
  EXPECT_EQ(rtw::multiprecision::math::abs(Int128::min()), Int128::max());

  // abs of normal values should work correctly
  EXPECT_EQ(rtw::multiprecision::math::abs(Int128{-42}), Int128{42});
  EXPECT_EQ(rtw::multiprecision::math::abs(Int128{42}), Int128{42});
  EXPECT_EQ(rtw::multiprecision::math::abs(Int128{0}), Int128{0});
  EXPECT_EQ(rtw::multiprecision::math::abs(Int128{-1}), Int128{1});
  EXPECT_EQ(rtw::multiprecision::math::abs(Int128::max()), Int128::max());
}

TEST(Int128, shift_boundary_cases)
{
  using rtw::multiprecision::Int128;
  using rtw::multiprecision::Int128U;

  // Shift by 0 — identity
  EXPECT_EQ(Int128{42} << 0U, Int128{42});
  EXPECT_EQ(Int128{42} >> 0U, Int128{42});
  EXPECT_EQ(Int128U{42} << 0U, Int128U{42});
  EXPECT_EQ(Int128U{42} >> 0U, Int128U{42});

  // Shift by 1
  EXPECT_EQ(Int128{1} << 1U, Int128{2});
  EXPECT_EQ(Int128{4} >> 1U, Int128{2});

  // Shift by exactly LO_BITS (64)
  EXPECT_EQ(Int128{1} << 64U, (Int128{1, 0}));
  EXPECT_EQ((Int128{1, 0}) >> 64U, Int128{1});

  // Shift by 127 (max useful for 128-bit)
  EXPECT_EQ(Int128U{1} << 127U, (Int128U{std::uint64_t{1} << 63U, 0U}));
  EXPECT_EQ((Int128U{std::uint64_t{1} << 63U, 0U}) >> 127U, Int128U{1});

  // Signed right shift — arithmetic (fills with sign bit)
  EXPECT_EQ(Int128{-1} >> 1U, Int128{-1});  // Sign extension
  EXPECT_EQ(Int128{-1} >> 64U, Int128{-1}); // Sign extension over full lo
  EXPECT_EQ(Int128{-2} >> 1U, Int128{-1});  // -2 >> 1 == -1

  // Shift by LO_BITS - 1 (63)
  EXPECT_EQ(Int128U{1} << 63U, (Int128U{0, std::uint64_t{1} << 63U}));
}

TEST(Int128, float_conversion_negative_edge_cases)
{
  using rtw::multiprecision::Int128;

  // min() should convert to double correctly (large negative)
  const double min_as_double = static_cast<double>(Int128::min());
  EXPECT_LT(min_as_double, 0.0);

  // max() should convert to double correctly (large positive)
  const double max_as_double = static_cast<double>(Int128::max());
  EXPECT_GT(max_as_double, 0.0);

  // Verify min < max in double space
  EXPECT_LT(min_as_double, max_as_double);

  // Verify symmetry: |min| approximately equals |max| (off by 1 in integer, negligible in float)
  EXPECT_NEAR(-min_as_double, max_as_double, max_as_double * 1e-15);

  // Negative values that fit in double mantissa should roundtrip exactly
  const Int128 neg{-1'000'000};
  const double neg_dbl = static_cast<double>(neg);
  EXPECT_DOUBLE_EQ(neg_dbl, -1000000.0);

  // Values near zero should be exact
  EXPECT_DOUBLE_EQ(static_cast<double>(Int128{-1}), -1.0);
  EXPECT_DOUBLE_EQ(static_cast<double>(Int128{-42}), -42.0);
}
