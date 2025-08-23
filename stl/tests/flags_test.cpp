#include "stl/flags.h"
#include "stl/format.h"

#include <gtest/gtest.h>

enum class TestEnum : std::uint8_t
{
  A = 1U << 0U,
  B = 1U << 1U,
  C = 1U << 2U,
  D = 1U << 3U,
  E = 1U << 4U,
  F = 1U << 5U,
  G = 1U << 6U,
  H = 1U << 7U
};

using TestFlags = rtw::stl::Flags<TestEnum>;

TEST(Flags, default_constructor)
{
  using namespace rtw::stl;

  constexpr TestFlags FLAGS;
  EXPECT_TRUE(FLAGS.none());
}

TEST(Flags, constructor)
{
  using namespace rtw::stl;

  constexpr TestFlags FLAGS{TestEnum::A};
  EXPECT_EQ(TestEnum::A, FLAGS);
}

TEST(Flags, basic)
{
  using namespace rtw::stl;

  constexpr TestFlags FLAGS{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, FLAGS);
  EXPECT_TRUE(FLAGS.test(TestEnum::A));
  EXPECT_TRUE(FLAGS.test(TestEnum::B));
  EXPECT_FALSE(FLAGS.test(TestEnum::C));
  EXPECT_TRUE(FLAGS);
  EXPECT_TRUE((FLAGS & TestEnum::A) == TestEnum::A);
  EXPECT_TRUE((FLAGS & TestEnum::B) == TestEnum::B);
  EXPECT_FALSE((FLAGS & TestEnum::C));
}

TEST(Flags, set)
{
  using namespace rtw::stl;

  TestFlags flags;
  flags.set(TestEnum::A);
  EXPECT_EQ(TestEnum::A, flags);
  flags.set(TestEnum::B);
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  flags.set(TestEnum::C);
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, flags);
  flags.set(TestEnum::A, false);
  EXPECT_EQ(TestEnum::B | TestEnum::C, flags);
  flags.set(TestEnum::B, false);
  EXPECT_EQ(TestEnum::C, flags);
  flags.set(TestEnum::C, false);
  EXPECT_TRUE(flags.none());
}

TEST(Flags, operator_bitwise_or)
{
  using namespace rtw::stl;

  constexpr TestFlags FLAGS{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, FLAGS);
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, FLAGS | TestEnum::C);
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, TestEnum::C | FLAGS);
}

TEST(Flags, operator_bitwise_and)
{
  using namespace rtw::stl;

  constexpr TestFlags FLAGS{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, FLAGS);
  EXPECT_EQ(TestEnum::A, FLAGS & TestEnum::A);
  EXPECT_EQ(TestEnum::B, FLAGS & TestEnum::B);
  EXPECT_FALSE(FLAGS.test(TestEnum::C));
}

TEST(Flags, operator_bitwise_xor)
{
  using namespace rtw::stl;

  constexpr TestFlags FLAGS{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, FLAGS);
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, FLAGS ^ TestEnum::C);
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, TestEnum::C ^ FLAGS);
}

TEST(Flags, operator_bitwise_or_equal)
{
  using namespace rtw::stl;

  TestFlags flags;
  flags |= TestEnum::A;
  EXPECT_EQ(TestEnum::A, flags);
  flags |= TestEnum::B;
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  flags |= TestEnum::C;
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, flags);
}

TEST(Flags, operator_bitwise_and_equal)
{
  using namespace rtw::stl;

  TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  flags &= TestEnum::A;
  EXPECT_EQ(TestEnum::A, flags);
  flags &= TestEnum::B;
  EXPECT_TRUE(flags.none());
}

TEST(Flags, operator_bitwise_xor_equal)
{
  using namespace rtw::stl;

  TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  flags ^= TestEnum::C;
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, flags);
  flags ^= TestEnum::C;
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
}

TEST(Flags, operator_equal)
{
  using namespace rtw::stl;

  constexpr TestFlags FLAGS{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, FLAGS);
  EXPECT_TRUE(FLAGS == FLAGS);
  EXPECT_TRUE(FLAGS == (TestEnum::A | TestEnum::B));
  EXPECT_TRUE((TestEnum::A | TestEnum::B) == FLAGS);
  EXPECT_FALSE(FLAGS == (TestEnum::A | TestEnum::C));
  EXPECT_FALSE((TestEnum::A | TestEnum::C) == FLAGS);
}

TEST(Flags, operator_not_equal)
{
  using namespace rtw::stl;

  constexpr TestFlags FLAGS{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, FLAGS);
  EXPECT_FALSE(FLAGS != FLAGS);
  EXPECT_FALSE(FLAGS != (TestEnum::A | TestEnum::B));
  EXPECT_FALSE((TestEnum::A | TestEnum::B) != FLAGS);
  EXPECT_TRUE(FLAGS != (TestEnum::A | TestEnum::C));
  EXPECT_TRUE((TestEnum::A | TestEnum::C) != FLAGS);
}

TEST(Flags, operator_bool)
{
  using namespace rtw::stl;

  constexpr TestFlags FLAGS{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, FLAGS);
  EXPECT_TRUE(FLAGS);
  EXPECT_TRUE(static_cast<bool>(FLAGS));
  EXPECT_FALSE(!FLAGS);
  EXPECT_FALSE(static_cast<bool>(!FLAGS));
  EXPECT_FALSE(TestFlags{});
  EXPECT_FALSE(static_cast<bool>(TestFlags{}));
  EXPECT_TRUE(!TestFlags{});
  EXPECT_TRUE(static_cast<bool>(!TestFlags{}));
}

TEST(Flags, operator_stream)
{
  using namespace rtw::stl;

  {
    const TestFlags flags{};
    std::ostringstream oss;
    oss << flags;
    EXPECT_EQ("Flags(00000000)", oss.str());
  }
  {
    const TestFlags flags{TestEnum::A | TestEnum::B | TestEnum::C | TestEnum::D | TestEnum::E | TestEnum::F
                          | TestEnum::G | TestEnum::H};
    std::ostringstream oss;
    oss << flags;
    EXPECT_EQ("Flags(11111111)", oss.str());
  }
  {
    const TestFlags flags{TestEnum::A | TestEnum::C | TestEnum::E | TestEnum::G};
    std::ostringstream oss;
    oss << flags;
    EXPECT_EQ("Flags(01010101)", oss.str());
  }
  {
    const TestFlags flags{TestEnum::B | TestEnum::D | TestEnum::F | TestEnum::H};
    std::ostringstream oss;
    oss << flags;
    EXPECT_EQ("Flags(10101010)", oss.str());
  }
  {
    const auto bits_count = sizeof(TestEnum) * 8U;
    for (std::size_t i = 0U; i < bits_count; ++i)
    {
      const TestFlags flags{static_cast<TestEnum>(1U << i)};
      std::ostringstream oss;
      oss << flags;
      std::string bits_str{"00000000"};
      bits_str[bits_count - 1U - i] = '1';
      const std::string expected = "Flags(" + bits_str + ")";
      EXPECT_EQ(expected, oss.str());
    }
  }
}

TEST(Flags, reset)
{
  using namespace rtw::stl;

  TestFlags flags{TestEnum::A | TestEnum::B | TestEnum::C};
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, flags);
  flags.reset();
  EXPECT_TRUE(flags.none());
}
