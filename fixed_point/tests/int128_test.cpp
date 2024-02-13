#include "fixed_point/int128.h"

#include <array>

#include <gtest/gtest.h>

// These aliases are used solely for the purpose of testing of the Int class.against the built-in integer types.
// They should not be used in production code.
namespace rtw::fixed_point {
using Int16 = Int<std::int8_t>;
using UInt16 = Int<std::uint8_t>;
using Int32 = Int<std::int16_t>;
using UInt32 = Int<std::uint16_t>;
using Int64 = Int<std::int32_t>;
using UInt64 = Int<std::uint32_t>;
} // namespace rtw::fixed_point

// -----------------------------------------------------------------------------

namespace {
template <typename IntT, typename ResultT, typename Container>
inline void check_conversion_ctor(const Container& test_cases)
{
  using Int = rtw::fixed_point::Int<IntT>;
  constexpr ResultT WordSize = sizeof(ResultT) * ResultT{4};

  for (const auto& a : test_cases)
  {
    const Int result{a};
    const IntT hi = -rtw::fixed_point::sign_bit(a);
    const auto lo = static_cast<typename Int::LoType>(a);
    const ResultT result_t = ResultT(result.hi()) << WordSize | result.lo();
    EXPECT_EQ(result_t, a);
    EXPECT_EQ(result.hi(), hi);
    EXPECT_EQ(result.lo(), lo);
  }
}

enum class OperationType
{
  Add,
  Sub,
  Mul,
};

template <typename IntT, typename ResultT, OperationType Operation, typename Container>
inline void check(const Container& test_cases)
{
  using Int = rtw::fixed_point::Int<IntT>;
  constexpr ResultT WordSize = sizeof(ResultT) * ResultT{4};
  constexpr ResultT LoMask = (ResultT{1} << WordSize) - ResultT{1};

  constexpr auto check_operation = [](const auto& a, const auto& b) {
    if constexpr (Operation == OperationType::Add)
    {
      const Int result = Int{a} + Int{b};
      const ResultT expected = ResultT(a) + ResultT(b);
      const ResultT result_t = ResultT(result.hi()) << WordSize | result.lo();
      EXPECT_EQ(result_t, expected);
      EXPECT_EQ(result.hi(), expected >> WordSize);
      EXPECT_EQ(result.lo(), expected & LoMask);
    }
    else if constexpr (Operation == OperationType::Sub)
    {
      const Int result = Int{a} - Int{b};
      const ResultT expected = ResultT(a) - ResultT(b);
      const ResultT result_t = ResultT(result.hi()) << WordSize | result.lo();
      EXPECT_EQ(result_t, expected);
      EXPECT_EQ(result.hi(), expected >> WordSize);
      EXPECT_EQ(result.lo(), expected & LoMask);
    }
    else if constexpr (Operation == OperationType::Mul)
    {
      const Int result = Int{a} * Int{b};
      const ResultT expected = ResultT(a) * ResultT(b);
      const ResultT result_t = ResultT(result.hi()) << WordSize | result.lo();
      EXPECT_EQ(result_t, expected);
      EXPECT_EQ(result.hi(), expected >> WordSize);
      EXPECT_EQ(result.lo(), expected & LoMask);
    }
  };

  for (const auto& [a, b] : test_cases)
  {
    check_operation(a, b);
    check_operation(b, a);
  }
}
} // namespace

// -----------------------------------------------------------------------------
// These tests cases are done solely for the purpose to test base class operations against the built-in types.
// The aliases declared above should not be used in producation code.

TEST(Int16, conversion_ctor)
{
  using rtw::fixed_point::Int16;

  constexpr std::array test_cases = {
      std::int8_t{0}, std::int8_t{-128}, std::int8_t{-19}, std::int8_t{17}, std::int8_t{127},
  };

  check_conversion_ctor<std::int8_t, std::int16_t>(test_cases);
}

TEST(UInt16, conversion_ctor)
{
  using rtw::fixed_point::UInt16;

  constexpr std::array test_cases = {
      std::uint8_t{0},
      std::uint8_t{17},
      std::uint8_t{19},
      std::uint8_t{255},
  };

  check_conversion_ctor<std::uint8_t, std::uint16_t>(test_cases);
}

TEST(Int32, conversion_ctor)
{
  using rtw::fixed_point::Int32;

  constexpr std::array test_cases = {
      std::int16_t{0}, std::int16_t{-32768}, std::int16_t{-19}, std::int16_t{17}, std::int16_t{32767},
  };

  check_conversion_ctor<std::int16_t, std::int32_t>(test_cases);
}

TEST(UInt32, conversion_ctor)
{
  using rtw::fixed_point::UInt32;

  constexpr std::array test_cases = {
      std::uint16_t{0},
      std::uint16_t{17},
      std::uint16_t{19},
      std::uint16_t{65535},
  };

  check_conversion_ctor<std::uint16_t, std::uint32_t>(test_cases);
}

TEST(Int64, conversion_ctor)
{
  using rtw::fixed_point::Int64;

  constexpr std::array test_cases = {
      std::int32_t{0}, std::int32_t{-2147483648}, std::int32_t{-19}, std::int32_t{17}, std::int32_t{2147483647},
  };

  check_conversion_ctor<std::int32_t, std::int64_t>(test_cases);
}

TEST(UInt64, conversion_ctor)
{
  using rtw::fixed_point::UInt64;

  constexpr std::array test_cases = {
      std::uint32_t{0},
      std::uint32_t{17},
      std::uint32_t{19},
      std::uint32_t{4294967295},
  };

  check_conversion_ctor<std::uint32_t, std::uint64_t>(test_cases);
}

TEST(Int16, mul16s)
{
  using rtw::fixed_point::Int16;

  constexpr std::array test_cases = {
      std::pair<std::int8_t, std::int8_t>{0, 0},      std::pair<std::int8_t, std::int8_t>{-128, -128},
      std::pair<std::int8_t, std::int8_t>{-128, 127}, std::pair<std::int8_t, std::int8_t>{127, -128},
      std::pair<std::int8_t, std::int8_t>{127, 127},
  };

  check<std::int8_t, std::int16_t, OperationType::Add>(test_cases);
  check<std::int8_t, std::int16_t, OperationType::Sub>(test_cases);
  check<std::int8_t, std::int16_t, OperationType::Mul>(test_cases);
}

TEST(UInt128, mul16u)
{
  using rtw::fixed_point::UInt16;

  constexpr std::array test_cases = {
      std::pair<std::uint8_t, std::uint8_t>{0, 0},
      std::pair<std::uint8_t, std::uint8_t>{0, 255},
      std::pair<std::uint8_t, std::uint8_t>{255, 0},
      std::pair<std::uint8_t, std::uint8_t>{255, 255},
  };

  check<std::uint8_t, std::uint16_t, OperationType::Add>(test_cases);
  check<std::uint8_t, std::uint16_t, OperationType::Sub>(test_cases);
  check<std::uint8_t, std::uint16_t, OperationType::Mul>(test_cases);
}

TEST(Int32, mul32s)
{
  using rtw::fixed_point::Int32;

  constexpr std::array test_cases = {
      std::pair<std::int16_t, std::int16_t>{0, 0},          std::pair<std::int16_t, std::int16_t>{-32768, -32768},
      std::pair<std::int16_t, std::int16_t>{-32768, 32767}, std::pair<std::int16_t, std::int16_t>{32767, -32768},
      std::pair<std::int16_t, std::int16_t>{32767, 32767},
  };

  check<std::int16_t, std::int32_t, OperationType::Add>(test_cases);
  check<std::int16_t, std::int32_t, OperationType::Sub>(test_cases);
  check<std::int16_t, std::int32_t, OperationType::Mul>(test_cases);
}

TEST(UInt32, mul32u)
{
  using rtw::fixed_point::UInt32;

  constexpr std::array test_cases = {
      std::pair<std::uint16_t, std::uint16_t>{0, 0},
      std::pair<std::uint16_t, std::uint16_t>{0, 65535},
      std::pair<std::uint16_t, std::uint16_t>{65535, 0},
      std::pair<std::uint16_t, std::uint16_t>{65535, 65535},
  };

  check<std::uint16_t, std::uint32_t, OperationType::Add>(test_cases);
  check<std::uint16_t, std::uint32_t, OperationType::Sub>(test_cases);
  check<std::uint16_t, std::uint32_t, OperationType::Mul>(test_cases);
}

TEST(Int64, mul64s)
{
  using rtw::fixed_point::Int64;

  constexpr std::array test_cases = {
      std::pair<std::int32_t, std::int32_t>{0, 0},
      std::pair<std::int32_t, std::int32_t>{-2147483648, -2147483648},
      std::pair<std::int32_t, std::int32_t>{-2147483648, 2147483647},
      std::pair<std::int32_t, std::int32_t>{2147483647, -2147483648},
      std::pair<std::int32_t, std::int32_t>{2147483647, 2147483647},
  };

  check<std::int32_t, std::int64_t, OperationType::Add>(test_cases);
  check<std::int32_t, std::int64_t, OperationType::Sub>(test_cases);
  check<std::int32_t, std::int64_t, OperationType::Mul>(test_cases);
}

TEST(UInt64, mul64u)
{
  using rtw::fixed_point::UInt64;

  constexpr std::array test_cases = {
      std::pair<std::uint32_t, std::uint32_t>{0, 0},
      std::pair<std::uint32_t, std::uint32_t>{0, 4294967295},
      std::pair<std::uint32_t, std::uint32_t>{4294967295, 0},
      std::pair<std::uint32_t, std::uint32_t>{4294967295, 4294967295},
  };

  check<std::uint32_t, std::uint64_t, OperationType::Add>(test_cases);
  check<std::uint32_t, std::uint64_t, OperationType::Sub>(test_cases);
  check<std::uint32_t, std::uint64_t, OperationType::Mul>(test_cases);
}
