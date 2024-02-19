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
  Div,
  Mod,
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
    else if constexpr (Operation == OperationType::Div)
    {
      if (b == 0)
      {
        EXPECT_DEATH(Int{a} / Int{b}, "");
        return;
      }

      const Int result = Int{a} / Int{b};
      const ResultT expected = ResultT(a) / ResultT(b);
      const ResultT result_t = ResultT(result.hi()) << WordSize | result.lo();
      EXPECT_EQ(result_t, expected);
      EXPECT_EQ(result.hi(), expected >> WordSize);
      EXPECT_EQ(result.lo(), expected & LoMask);
    }
    else if constexpr (Operation == OperationType::Mod)
    {
      if (b == 0)
      {
        EXPECT_DEATH(Int{a} % Int{b}, "");
        return;
      }

      const Int result = Int{a} % Int{b};
      const ResultT expected = ResultT(a) % ResultT(b);
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

TEST(Int16, arithmetic_operations)
{
  using rtw::fixed_point::Int16;

  constexpr std::array test_cases = {
      std::pair<std::int8_t, std::int8_t>{0, 0},      std::pair<std::int8_t, std::int8_t>{-128, -128},
      std::pair<std::int8_t, std::int8_t>{-128, 127}, std::pair<std::int8_t, std::int8_t>{127, -128},
      std::pair<std::int8_t, std::int8_t>{127, 127},  std::pair<std::int8_t, std::int8_t>{19, 17},
      std::pair<std::int8_t, std::int8_t>{19, -17},   std::pair<std::int8_t, std::int8_t>{-19, 17},
      std::pair<std::int8_t, std::int8_t>{-19, -17},
  };

  check<std::int8_t, std::int16_t, OperationType::Add>(test_cases);
  check<std::int8_t, std::int16_t, OperationType::Sub>(test_cases);
  check<std::int8_t, std::int16_t, OperationType::Mul>(test_cases);
  check<std::int8_t, std::int16_t, OperationType::Div>(test_cases);
  check<std::int8_t, std::int16_t, OperationType::Mod>(test_cases);
}

TEST(UInt16, arithmetic_operations)
{
  using rtw::fixed_point::UInt16;

  constexpr std::array test_cases = {
      std::pair<std::uint8_t, std::uint8_t>{0, 0},   std::pair<std::uint8_t, std::uint8_t>{0, 255},
      std::pair<std::uint8_t, std::uint8_t>{255, 0}, std::pair<std::uint8_t, std::uint8_t>{255, 255},
      std::pair<std::uint8_t, std::uint8_t>{19, 17},
  };

  check<std::uint8_t, std::uint16_t, OperationType::Add>(test_cases);
  check<std::uint8_t, std::uint16_t, OperationType::Sub>(test_cases);
  check<std::uint8_t, std::uint16_t, OperationType::Mul>(test_cases);
  check<std::uint8_t, std::uint16_t, OperationType::Div>(test_cases);
  check<std::uint8_t, std::uint16_t, OperationType::Mod>(test_cases);
}

TEST(Int32, arithmetic_operations)
{
  using rtw::fixed_point::Int32;

  constexpr std::array test_cases = {
      std::pair<std::int16_t, std::int16_t>{0, 0},          std::pair<std::int16_t, std::int16_t>{-32768, -32768},
      std::pair<std::int16_t, std::int16_t>{-32768, 32767}, std::pair<std::int16_t, std::int16_t>{32767, -32768},
      std::pair<std::int16_t, std::int16_t>{32767, 32767},  std::pair<std::int16_t, std::int16_t>{19, 17},
      std::pair<std::int16_t, std::int16_t>{19, -17},       std::pair<std::int16_t, std::int16_t>{-19, 17},
      std::pair<std::int16_t, std::int16_t>{-19, -17},
  };

  check<std::int16_t, std::int32_t, OperationType::Add>(test_cases);
  check<std::int16_t, std::int32_t, OperationType::Sub>(test_cases);
  check<std::int16_t, std::int32_t, OperationType::Mul>(test_cases);
  check<std::int16_t, std::int32_t, OperationType::Div>(test_cases);
  check<std::int16_t, std::int32_t, OperationType::Mod>(test_cases);
}

TEST(UInt32, arithmetic_operations)
{
  using rtw::fixed_point::UInt32;

  constexpr std::array test_cases = {
      std::pair<std::uint16_t, std::uint16_t>{0, 0},     std::pair<std::uint16_t, std::uint16_t>{0, 65535},
      std::pair<std::uint16_t, std::uint16_t>{65535, 0}, std::pair<std::uint16_t, std::uint16_t>{65535, 65535},
      std::pair<std::uint16_t, std::uint16_t>{19, 17},
  };

  check<std::uint16_t, std::uint32_t, OperationType::Add>(test_cases);
  check<std::uint16_t, std::uint32_t, OperationType::Sub>(test_cases);
  check<std::uint16_t, std::uint32_t, OperationType::Mul>(test_cases);
  check<std::uint16_t, std::uint32_t, OperationType::Div>(test_cases);
  check<std::uint16_t, std::uint32_t, OperationType::Mod>(test_cases);
}

TEST(Int64, arithmetic_operations)
{
  using rtw::fixed_point::Int64;

  constexpr std::array test_cases = {
      std::pair<std::int32_t, std::int32_t>{0, 0},
      std::pair<std::int32_t, std::int32_t>{-2147483648, -2147483648},
      std::pair<std::int32_t, std::int32_t>{-2147483648, 2147483647},
      std::pair<std::int32_t, std::int32_t>{2147483647, -2147483648},
      std::pair<std::int32_t, std::int32_t>{2147483647, 2147483647},
      std::pair<std::int32_t, std::int32_t>{19, 17},
      std::pair<std::int32_t, std::int32_t>{19, -17},
      std::pair<std::int32_t, std::int32_t>{-19, 17},
      std::pair<std::int32_t, std::int32_t>{-19, -17},
  };

  check<std::int32_t, std::int64_t, OperationType::Add>(test_cases);
  check<std::int32_t, std::int64_t, OperationType::Sub>(test_cases);
  check<std::int32_t, std::int64_t, OperationType::Mul>(test_cases);
  check<std::int32_t, std::int64_t, OperationType::Div>(test_cases);
  check<std::int32_t, std::int64_t, OperationType::Mod>(test_cases);
}

TEST(UInt64, arithmetic_operations)
{
  using rtw::fixed_point::UInt64;

  constexpr std::array test_cases = {
      std::pair<std::uint32_t, std::uint32_t>{0, 0},
      std::pair<std::uint32_t, std::uint32_t>{0, 4294967295},
      std::pair<std::uint32_t, std::uint32_t>{4294967295, 0},
      std::pair<std::uint32_t, std::uint32_t>{4294967295, 4294967295},
      std::pair<std::uint32_t, std::uint32_t>{19, 17},
  };

  check<std::uint32_t, std::uint64_t, OperationType::Add>(test_cases);
  check<std::uint32_t, std::uint64_t, OperationType::Sub>(test_cases);
  check<std::uint32_t, std::uint64_t, OperationType::Mul>(test_cases);
  check<std::uint32_t, std::uint64_t, OperationType::Div>(test_cases);
  check<std::uint32_t, std::uint64_t, OperationType::Mod>(test_cases);
}

TEST(Int, negate)
{
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

  using rtw::fixed_point::UInt16;
  using rtw::fixed_point::UInt32;
  using rtw::fixed_point::UInt64;

  {
    EXPECT_EQ(-UInt16(36), UInt16(-36));
    EXPECT_EQ(-UInt16(-36), UInt16(36));
  }
  {
    EXPECT_EQ(-UInt32(369), UInt32(-369));
    EXPECT_EQ(-UInt32(-369), UInt32(369));
  }
  {
    EXPECT_EQ(-UInt64(369), UInt64(-369));
    EXPECT_EQ(-UInt64(-369), UInt64(369));
  }
}

TEST(UInt16, bitwise_shifts)
{
  using rtw::fixed_point::UInt16;

  UInt16 a = 1;
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

TEST(UInt32, bitwise_shifts)
{
  using rtw::fixed_point::UInt32;

  UInt32 a = 1;
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

TEST(UInt64, bitwise_shifts)
{
  using rtw::fixed_point::UInt64;

  UInt64 a = 1;
  a <<= 0;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);

  a <<= 63;
  EXPECT_EQ(a.hi(), 0x8000'0000);
  EXPECT_EQ(a.lo(), 0);

  a >>= 63;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);

  a >>= 0;
  EXPECT_EQ(a.hi(), 0);
  EXPECT_EQ(a.lo(), 1);
}

TEST(UInt128, bitwise_shifts)
{
  using rtw::fixed_point::UInt128;

  UInt128 a = 1;
  a <<= 127;
  EXPECT_EQ(a.hi(), 0x8000'0000'0000'0000);
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
    rtw::fixed_point::UInt16 a = {0xFF, 0xFF};
    std::uint16_t expected = 0xFFFF;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::UInt16::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt16::BITS);
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::fixed_point::UInt16 a = 1;
    std::uint16_t expected = 1;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::UInt16::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt16::BITS); // a == 0
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_32)
{
  // Shift left
  {
    rtw::fixed_point::UInt32 a = {0xFFFF, 0xFFFF};
    std::uint32_t expected = 0xFFFF'FFFF;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::UInt32::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt32::BITS);
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::fixed_point::UInt32 a = 1;
    std::uint32_t expected = 1;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::UInt32::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt32::BITS); // a == 0
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_64)
{
  // Shift left
  {
    rtw::fixed_point::UInt64 a = {0xFFFF'FFFF, 0xFFFF'FFFF};
    std::uint64_t expected = 0xFFFF'FFFF'FFFF'FFFF;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::UInt64::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt64::BITS);
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::fixed_point::UInt64 a = 1;
    std::uint64_t expected = 1;
    for (std::uint64_t i = 0; i <= rtw::fixed_point::UInt64::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt64::BITS); // a == 0
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_128)
{
  // Shift left
  {
    rtw::fixed_point::UInt128 a = {0xFFFF'FFFF'FFFF'FFFF, 0xFFFF'FFFF'FFFF'FFFF};
    for (std::uint32_t i = 0; i <= rtw::fixed_point::UInt128::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
      a >>= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt128::BITS);
  }
  // Shift right
  {
    rtw::fixed_point::UInt128 a = 1;
    for (std::uint64_t i = 0; i < rtw::fixed_point::UInt128::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt128::BITS - i - 1);
      a <<= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt128::BITS); // a == 0
    a <<= 1;
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::UInt128::BITS); // a == 0
  }
}
