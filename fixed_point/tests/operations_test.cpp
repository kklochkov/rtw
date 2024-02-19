#include "fixed_point/operations.h"

#include <gtest/gtest.h>

TEST(operations, sign_bit)
{
  {
    constexpr std::int8_t a = 0;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 0);
  }
  {
    constexpr std::int8_t a = -1;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 1);
  }
  {
    constexpr std::int16_t a = 0;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 0);
  }
  {
    constexpr std::int16_t a = -1;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 1);
  }
  {
    constexpr std::int32_t a = 0;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 0);
  }
  {
    constexpr std::int32_t a = -1;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 1);
  }
  {
    constexpr std::int64_t a = 0;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 0);
  }
  {
    constexpr std::int64_t a = -1;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 1);
  }
  {
    constexpr std::uint8_t a = 0;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 0);
  }
  {
    constexpr std::uint16_t a = 0;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 0);
  }
  {
    constexpr std::uint32_t a = 0;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 0);
  }
  {
    constexpr std::uint64_t a = 0;
    EXPECT_EQ(rtw::fixed_point::sign_bit(a), 0);
  }
}

TEST(operators, count_leading_zero)
{
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint8_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint8_t a = 0xFF >> i;
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
    }
  }
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint16_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint16_t a = 0xFFFF >> i;
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
    }
  }
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint32_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint32_t a = 0xFFFFFFFF >> i;
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
    }
  }
  {
    constexpr std::uint32_t SIZE = sizeof(std::uint64_t) * 8U;
    for (std::uint32_t i = 0; i < SIZE; ++i)
    {
      const std::uint64_t a = 0xFFFFFFFFFFFFFFFFULL >> i;
      EXPECT_EQ(rtw::fixed_point::count_leading_zero(a), i);
    }
  }
}
