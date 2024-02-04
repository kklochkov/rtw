#include "sw_renderer/flags.h"

#include <gtest/gtest.h>

enum class TestEnum : std::uint8_t
{
  A = 1U << 0U,
  B = 1U << 1U,
  C = 1U << 2U,
};

using TestFlags = rtw::sw_renderer::Flags<TestEnum>;

TEST(Flags, default_constructor)
{
  using namespace rtw::sw_renderer;

  constexpr TestFlags flags;
  EXPECT_EQ(static_cast<TestEnum>(0), flags);
}

TEST(Flags, constructor)
{
  using namespace rtw::sw_renderer;

  constexpr TestFlags flags{TestEnum::A};
  EXPECT_EQ(TestEnum::A, flags);
}

TEST(Flags, basic)
{
  using namespace rtw::sw_renderer;

  constexpr TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  EXPECT_TRUE(flags.test(TestEnum::A));
  EXPECT_TRUE(flags.test(TestEnum::B));
  EXPECT_FALSE(flags.test(TestEnum::C));
  EXPECT_TRUE(flags);
  EXPECT_EQ(TestEnum::A | TestEnum::B, static_cast<TestEnum>(flags));
  EXPECT_TRUE((flags & TestEnum::A) == TestEnum::A);
  EXPECT_TRUE((flags & TestEnum::B) == TestEnum::B);
  EXPECT_FALSE((flags & TestEnum::C));
}

TEST(Flags, set)
{
  using namespace rtw::sw_renderer;

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
  EXPECT_EQ(static_cast<TestEnum>(0), flags);
}

TEST(Flags, operator_bitwise_or)
{
  using namespace rtw::sw_renderer;

  constexpr TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, flags | TestEnum::C);
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, TestEnum::C | flags);
}

TEST(Flags, operator_bitwise_and)
{
  using namespace rtw::sw_renderer;

  constexpr TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  EXPECT_EQ(TestEnum::A, flags & TestEnum::A);
  EXPECT_EQ(TestEnum::B, flags & TestEnum::B);
  EXPECT_EQ(static_cast<TestEnum>(0), flags & TestEnum::C);
}

TEST(Flags, operator_bitwise_xor)
{
  using namespace rtw::sw_renderer;

  constexpr TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, flags ^ TestEnum::C);
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, TestEnum::C ^ flags);
}

TEST(Flags, operator_bitwise_or_equal)
{
  using namespace rtw::sw_renderer;

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
  using namespace rtw::sw_renderer;

  TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  flags &= TestEnum::A;
  EXPECT_EQ(TestEnum::A, flags);
  flags &= TestEnum::B;
  EXPECT_EQ(static_cast<TestEnum>(0), flags);
}

TEST(Flags, operator_bitwise_xor_equal)
{
  using namespace rtw::sw_renderer;

  TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  flags ^= TestEnum::C;
  EXPECT_EQ(TestEnum::A | TestEnum::B | TestEnum::C, flags);
  flags ^= TestEnum::C;
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
}

TEST(Flags, operator_equal)
{
  using namespace rtw::sw_renderer;

  constexpr TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  EXPECT_TRUE(flags == flags);
  EXPECT_TRUE(flags == (TestEnum::A | TestEnum::B));
  EXPECT_TRUE((TestEnum::A | TestEnum::B) == flags);
  EXPECT_FALSE(flags == (TestEnum::A | TestEnum::C));
  EXPECT_FALSE((TestEnum::A | TestEnum::C) == flags);
}

TEST(Flags, operator_not_equal)
{
  using namespace rtw::sw_renderer;

  constexpr TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  EXPECT_FALSE(flags != flags);
  EXPECT_FALSE(flags != (TestEnum::A | TestEnum::B));
  EXPECT_FALSE((TestEnum::A | TestEnum::B) != flags);
  EXPECT_TRUE(flags != (TestEnum::A | TestEnum::C));
  EXPECT_TRUE((TestEnum::A | TestEnum::C) != flags);
}

TEST(Flags, operator_bool)
{
  using namespace rtw::sw_renderer;

  constexpr TestFlags flags{TestEnum::A | TestEnum::B};
  EXPECT_EQ(TestEnum::A | TestEnum::B, flags);
  EXPECT_TRUE(flags);
  EXPECT_TRUE(static_cast<bool>(flags));
  EXPECT_FALSE(!flags);
  EXPECT_FALSE(static_cast<bool>(!flags));
  EXPECT_FALSE(TestFlags{});
  EXPECT_FALSE(static_cast<bool>(TestFlags{}));
  EXPECT_TRUE(!TestFlags{});
  EXPECT_TRUE(static_cast<bool>(!TestFlags{}));
}
