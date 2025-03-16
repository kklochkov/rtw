#include "fixed_point/int128.h"
#include "fixed_point/format.h"

#include <gtest/gtest.h>

#include <array>

// These aliases are used solely for the purpose of testing of the Int class against the built-in integer types.
// They should not be used in production code.
namespace rtw::fixed_point
{
using Int16 = Int<std::int8_t>;
using Int16U = Int<std::uint8_t>;
using Int32 = Int<std::int16_t>;
using Int32U = Int<std::uint16_t>;
using Int64 = Int<std::int32_t>;
using Int64U = Int<std::uint32_t>;
} // namespace rtw::fixed_point

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
  using CustomIntT = rtw::fixed_point::Int<IntT>;

  for (const auto& expected : test_case)
  {
    const CustomIntT result{expected};
    const auto result_t = pack_hi_lo<ResultT>(result.hi(), result.lo());
    const IntT expected_hi = -rtw::fixed_point::sign_bit(expected); // The sign is in the hi part
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
          typename CustomIntT = rtw::fixed_point::Int<IntT>>
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
  using rtw::fixed_point::Int16;

  constexpr std::array TEST_CASE = {
      std::int8_t{0}, std::int8_t{-128}, std::int8_t{-19}, std::int8_t{17}, std::int8_t{127},
  };

  check_conversion_ctor<std::int8_t, std::int16_t>(TEST_CASE);
}

TEST(Int16U, conversion_ctor)
{
  using rtw::fixed_point::Int16U;

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
  using rtw::fixed_point::Int32;

  constexpr std::array TEST_CASE = {
      std::int16_t{0}, std::int16_t{-32'768}, std::int16_t{-19}, std::int16_t{17}, std::int16_t{32'767},
  };

  check_conversion_ctor<std::int16_t, std::int32_t>(TEST_CASE);
}

TEST(Int32U, conversion_ctor)
{
  using rtw::fixed_point::Int32U;

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
  using rtw::fixed_point::Int64;

  constexpr std::array TEST_CASE = {
      std::int32_t{0}, std::int32_t{-2'147'483'648}, std::int32_t{-19}, std::int32_t{17}, std::int32_t{2'147'483'647},
  };

  check_conversion_ctor<std::int32_t, std::int64_t>(TEST_CASE);
}

TEST(Int64U, conversion_ctor)
{
  using rtw::fixed_point::Int64U;

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
  using rtw::fixed_point::Int16;

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
  using rtw::fixed_point::Int16U;

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
  using rtw::fixed_point::Int32;

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
  using rtw::fixed_point::Int32U;

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
  using rtw::fixed_point::Int64;

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
  using rtw::fixed_point::Int64U;

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

using IntTypes = ::testing::Types<rtw::fixed_point::Int16, rtw::fixed_point::Int16U, rtw::fixed_point::Int32,
                                  rtw::fixed_point::Int32U, rtw::fixed_point::Int64, rtw::fixed_point::Int64U,
                                  rtw::fixed_point::Int128, rtw::fixed_point::Int128U>;
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
  using rtw::fixed_point::Int128;

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
  using rtw::fixed_point::Int128U;

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
  using rtw::fixed_point::Int128;
  using rtw::fixed_point::Int16;
  using rtw::fixed_point::Int32;
  using rtw::fixed_point::Int64;

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

  using rtw::fixed_point::Int128U;
  using rtw::fixed_point::Int16U;
  using rtw::fixed_point::Int32U;
  using rtw::fixed_point::Int64U;

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
  using rtw::fixed_point::Int16U;

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
  using rtw::fixed_point::Int32U;

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
  using rtw::fixed_point::Int64U;

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
  using rtw::fixed_point::Int128U;

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
    rtw::fixed_point::Int16U a = {0xFF, 0xFF};
    std::uint16_t expected = 0xFFFF;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int16U::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int16U::BITS);
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::fixed_point::Int16U a = 1;
    std::uint16_t expected = 1;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int16U::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int16U::BITS); // a == 0
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_32)
{
  // Shift left
  {
    rtw::fixed_point::Int32U a = {0xFFFF, 0xFFFF};
    std::uint32_t expected = 0xFF'FF'FF'FF;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int32U::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int32U::BITS);
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::fixed_point::Int32U a = 1;
    std::uint32_t expected = 1;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int32U::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int32U::BITS); // a == 0
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_64)
{
  // Shift left
  {
    rtw::fixed_point::Int64U a = {0xFF'FF'FF'FF, 0xFF'FF'FF'FF};
    std::uint64_t expected = 0xFF'FF'FF'FF'FF'FF'FF'FF;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int64U::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int64U::BITS);
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::fixed_point::Int64U a = 1;
    std::uint64_t expected = 1;
    for (std::uint64_t i = 0; i <= rtw::fixed_point::Int64U::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int64U::BITS); // a == 0
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_128)
{
  // Shift left
  {
    rtw::fixed_point::Int128U a = {0xFF'FF'FF'FF'FF'FF'FF'FF, 0xFF'FF'FF'FF'FF'FF'FF'FF};
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int128U::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
      a >>= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int128U::BITS);
  }
  // Shift right
  {
    rtw::fixed_point::Int128U a = 1;
    for (std::uint64_t i = 0; i < rtw::fixed_point::Int128U::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int128U::BITS - i - 1);
      a <<= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int128U::BITS); // a == 0
    a <<= 1;
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int128U::BITS); // a == 0
  }
}
