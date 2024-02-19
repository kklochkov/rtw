#include "fixed_point/int128.h"

#include <gtest/gtest.h>

#include <array>

// These aliases are used solely for the purpose of testing of the Int class.against the built-in integer types.
// They should not be used in production code.
namespace rtw::fixed_point
{
using Int16 = Int<std::int8_t>;
using Int16u = Int<std::uint8_t>;
using Int32 = Int<std::int16_t>;
using Int32u = Int<std::uint16_t>;
using Int64 = Int<std::int32_t>;
using Int64u = Int<std::uint32_t>;
} // namespace rtw::fixed_point

// -----------------------------------------------------------------------------

namespace
{
template <typename IntT, typename ResultT, typename Container>
inline void check_conversion_ctor(const Container& test_case)
{
  using Int = rtw::fixed_point::Int<IntT>;
  constexpr ResultT WORD_SIZE = sizeof(ResultT) * ResultT{4};

  for (const auto& a : test_case)
  {
    const Int result{a};
    const IntT hi = -rtw::fixed_point::sign_bit(a);
    const auto lo = static_cast<typename Int::LoType>(a);
    const ResultT result_t = ResultT(result.hi()) << WORD_SIZE | result.lo();
    EXPECT_EQ(result_t, a);
    EXPECT_EQ(result.hi(), hi);
    EXPECT_EQ(result.lo(), lo);
  }
}

enum class OperationType
{
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
};

template <typename IntT, typename ResultT, OperationType Operation, typename Container>
inline void check(const Container& test_case)
{
  using Int = rtw::fixed_point::Int<IntT>;
  constexpr ResultT WORD_SIZE = sizeof(ResultT) * ResultT{4};
  constexpr ResultT LoMask = (ResultT{1} << WORD_SIZE) - ResultT{1};

  constexpr auto CHECK_OPERATION = [](const auto& a, const auto& b)
  {
    if constexpr (Operation == OperationType::ADD)
    {
      const Int result = Int{a} + Int{b};
      const ResultT expected = ResultT(a) + ResultT(b);
      const ResultT result_t = ResultT(result.hi()) << WORD_SIZE | result.lo();
      EXPECT_EQ(result_t, expected);
      EXPECT_EQ(result.hi(), expected >> WORD_SIZE);
      EXPECT_EQ(result.lo(), expected & LoMask);
    }
    else if constexpr (Operation == OperationType::SUB)
    {
      const Int result = Int{a} - Int{b};
      const ResultT expected = ResultT(a) - ResultT(b);
      const ResultT result_t = ResultT(result.hi()) << WORD_SIZE | result.lo();
      EXPECT_EQ(result_t, expected);
      EXPECT_EQ(result.hi(), expected >> WORD_SIZE);
      EXPECT_EQ(result.lo(), expected & LoMask);
    }
    else if constexpr (Operation == OperationType::MUL)
    {
      const Int result = Int{a} * Int{b};
      const ResultT expected = ResultT(a) * ResultT(b);
      const ResultT result_t = ResultT(result.hi()) << WORD_SIZE | result.lo();
      EXPECT_EQ(result_t, expected);
      EXPECT_EQ(result.hi(), expected >> WORD_SIZE);
      EXPECT_EQ(result.lo(), expected & LoMask);
    }
    else if constexpr (Operation == OperationType::DIV)
    {
      if (b == 0)
      {
        EXPECT_DEATH(Int{a} / Int{b}, "");
        return;
      }

      const Int result = Int{a} / Int{b};
      const ResultT expected = ResultT(a) / ResultT(b);
      const ResultT result_t = ResultT(result.hi()) << WORD_SIZE | result.lo();
      EXPECT_EQ(result_t, expected);
      EXPECT_EQ(result.hi(), expected >> WORD_SIZE);
      EXPECT_EQ(result.lo(), expected & LoMask);
    }
    else if constexpr (Operation == OperationType::MOD)
    {
      if (b == 0)
      {
        EXPECT_DEATH(Int{a} % Int{b}, "");
        return;
      }

      const Int result = Int{a} % Int{b};
      const ResultT expected = ResultT(a) % ResultT(b);
      const ResultT result_t = ResultT(result.hi()) << WORD_SIZE | result.lo();
      EXPECT_EQ(result_t, expected);
      EXPECT_EQ(result.hi(), expected >> WORD_SIZE);
      EXPECT_EQ(result.lo(), expected & LoMask);
    }
  };

  for (const auto& [a, b] : test_case)
  {
    CHECK_OPERATION(a, b);
    CHECK_OPERATION(b, a);
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

TEST(Int16u, conversion_ctor)
{
  using rtw::fixed_point::Int16u;

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

TEST(Int32u, conversion_ctor)
{
  using rtw::fixed_point::Int32u;

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

TEST(Int64u, conversion_ctor)
{
  using rtw::fixed_point::Int64u;

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

TEST(Int16u, arithmetic_operations)
{
  using rtw::fixed_point::Int16u;

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

TEST(Int32u, arithmetic_operations)
{
  using rtw::fixed_point::Int32u;

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

TEST(Int64u, arithmetic_operations)
{
  using rtw::fixed_point::Int64u;

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

  using rtw::fixed_point::Int16u;
  using rtw::fixed_point::Int32u;
  using rtw::fixed_point::Int64u;

  {
    EXPECT_EQ(-Int16u(36), Int16u(-36));
    EXPECT_EQ(-Int16u(-36), Int16u(36));
  }
  {
    EXPECT_EQ(-Int32u(369), Int32u(-369));
    EXPECT_EQ(-Int32u(-369), Int32u(369));
  }
  {
    EXPECT_EQ(-Int64u(369), Int64u(-369));
    EXPECT_EQ(-Int64u(-369), Int64u(369));
  }
}

TEST(Int16u, bitwise_shifts)
{
  using rtw::fixed_point::Int16u;

  Int16u a = 1;
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

TEST(Int32u, bitwise_shifts)
{
  using rtw::fixed_point::Int32u;

  Int32u a = 1;
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

TEST(Int64u, bitwise_shifts)
{
  using rtw::fixed_point::Int64u;

  Int64u a = 1;
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

TEST(Int128u, bitwise_shifts)
{
  using rtw::fixed_point::Int128u;

  Int128u a = 1;
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
    rtw::fixed_point::Int16u a = {0xFF, 0xFF};
    std::uint16_t expected = 0xFFFF;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int16u::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int16u::BITS);
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::fixed_point::Int16u a = 1;
    std::uint16_t expected = 1;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int16u::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int16u::BITS); // a == 0
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_32)
{
  // Shift left
  {
    rtw::fixed_point::Int32u a = {0xFFFF, 0xFFFF};
    std::uint32_t expected = 0xFF'FF'FF'FF;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int32u::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int32u::BITS);
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::fixed_point::Int32u a = 1;
    std::uint32_t expected = 1;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int32u::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int32u::BITS); // a == 0
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_64)
{
  // Shift left
  {
    rtw::fixed_point::Int64u a = {0xFF'FF'FF'FF, 0xFF'FF'FF'FF};
    std::uint64_t expected = 0xFF'FF'FF'FF'FF'FF'FF'FF;
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int64u::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a >>= 1;
      expected >>= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int64u::BITS);
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
  // Shift right
  {
    rtw::fixed_point::Int64u a = 1;
    std::uint64_t expected = 1;
    for (std::uint64_t i = 0; i <= rtw::fixed_point::Int64u::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
      a <<= 1;
      expected <<= 1U;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int64u::BITS); // a == 0
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::count_leading_zero(expected));
  }
}

TEST(operators, count_leading_zero_128)
{
  // Shift left
  {
    rtw::fixed_point::Int128u a = {0xFF'FF'FF'FF'FF'FF'FF'FF, 0xFF'FF'FF'FF'FF'FF'FF'FF};
    for (std::uint32_t i = 0; i <= rtw::fixed_point::Int128u::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
      a >>= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int128u::BITS);
  }
  // Shift right
  {
    rtw::fixed_point::Int128u a = 1;
    for (std::uint64_t i = 0; i < rtw::fixed_point::Int128u::BITS; ++i)
    {
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int128u::BITS - i - 1);
      a <<= 1;
    }
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int128u::BITS); // a == 0
    a <<= 1;
    EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), rtw::fixed_point::Int128u::BITS); // a == 0
  }
}
