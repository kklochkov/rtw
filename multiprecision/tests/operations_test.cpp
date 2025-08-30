#include "multiprecision/operations.h"

#include <gtest/gtest.h>

TEST(operations, sign_bit)
{
  {
    constexpr std::int8_t A = 0;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 0);
  }
  {
    constexpr std::int8_t A = -1;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 1);
  }
  {
    constexpr std::int16_t A = 0;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 0);
  }
  {
    constexpr std::int16_t A = -1;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 1);
  }
  {
    constexpr std::int32_t A = 0;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 0);
  }
  {
    constexpr std::int32_t A = -1;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 1);
  }
  {
    constexpr std::int64_t A = 0;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 0);
  }
  {
    constexpr std::int64_t A = -1;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 1);
  }
  {
    constexpr std::uint8_t A = 0;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 0);
  }
  {
    constexpr std::uint16_t A = 0;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 0);
  }
  {
    constexpr std::uint32_t A = 0;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 0);
  }
  {
    constexpr std::uint64_t A = 0;
    EXPECT_EQ(rtw::multiprecision::sign_bit(A), 0);
  }
}

TEST(operators, count_leading_zero)
{
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint8_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint8_t a = 0xFFU >> i;
      EXPECT_EQ(rtw::multiprecision::count_leading_zero(a), i);
    }
  }
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint16_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint16_t a = 0xFFFFU >> i;
      EXPECT_EQ(rtw::multiprecision::count_leading_zero(a), i);
    }
  }
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint32_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint32_t a = 0xFF'FF'FF'FF >> i;
      EXPECT_EQ(rtw::multiprecision::count_leading_zero(a), i);
    }
  }
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint64_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint64_t a = 0xFF'FF'FF'FF'FF'FF'FF'FFULL >> i;
      EXPECT_EQ(rtw::multiprecision::count_leading_zero(a), i);
    }
  }
}
