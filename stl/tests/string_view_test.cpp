#include "stl/format.h" // IWYU pragma: keep
#include "stl/string_view.h"

#include <gtest/gtest.h>

TEST(StringViewTest, constructor)
{
  {
    // Default constructor
    rtw::stl::StringView view;
    EXPECT_EQ(view.size(), 0U);
    EXPECT_EQ(view.data(), nullptr);
    EXPECT_TRUE(view.empty());
  }
  {
    // Pointer and size constructor
    const char* str = "Hello, World!";
    rtw::stl::StringView view{str, 13U};
    EXPECT_EQ(view.size(), 13U);
    EXPECT_EQ(view.data(), str);
    EXPECT_EQ(view[0], 'H');
    EXPECT_EQ(view[12], '!');
    EXPECT_TRUE(view.starts_with("Hello"));
    EXPECT_TRUE(view.ends_with("World!"));
  }
  {
    // Pointer constructor
    const char* str = "Hello, World!";
    rtw::stl::StringView view{str};
    EXPECT_EQ(view.size(), 13U);
    EXPECT_EQ(view.data(), str);
    EXPECT_EQ(view[0], 'H');
    EXPECT_EQ(view[12], '!');
    EXPECT_TRUE(view.starts_with("Hello"));
    EXPECT_TRUE(view.ends_with("World!"));
  }
  {
    // C-string constructor
    rtw::stl::StringView view{"Hello, World!"};
    EXPECT_EQ(view.size(), 13U);
    EXPECT_EQ(view.data(), "Hello, World!");
    EXPECT_EQ(view[0], 'H');
    EXPECT_EQ(view[12], '!');
    EXPECT_TRUE(view.starts_with("Hello"));
    EXPECT_TRUE(view.ends_with("World!"));
  }
}

TEST(StringViewTest, size_and_empty)
{
  rtw::stl::StringView view{"Hello, World!"};
  EXPECT_EQ(view.size(), 13U);
  EXPECT_FALSE(view.empty());

  rtw::stl::StringView empty_view;
  EXPECT_EQ(empty_view.size(), 0U);
  EXPECT_TRUE(empty_view.empty());
}

TEST(StringViewTest, data_and_c_str)
{
  const char* str = "Hello, World!";
  rtw::stl::StringView view{str};
  EXPECT_EQ(view.data(), str);
  EXPECT_EQ(view.c_str(), str);
}

TEST(StringViewTest, front_and_back)
{
  rtw::stl::StringView view{"Hello, World!"};
  EXPECT_EQ(view.front(), 'H');
  EXPECT_EQ(view.back(), '!');
}

TEST(StringViewTest, begin_and_end)
{
  rtw::stl::StringView view{"Hello, World!"};
  const auto* it = view.begin();
  EXPECT_EQ(*it, 'H');
  ++it; // NOLINT
  EXPECT_EQ(*it, 'e');
  it = view.end() - 1;
  EXPECT_EQ(*it, '!');
}

TEST(StringViewTest, remove_prefix_and_suffix)
{
  rtw::stl::StringView view{"Hello, World!"};
  view.remove_prefix(7U);
  EXPECT_EQ(view.size(), 6U);
  EXPECT_EQ(view, "World!");

  view.remove_suffix(1U);
  EXPECT_EQ(view.size(), 5U);
  EXPECT_EQ(view, "World");
}

TEST(StringViewTest, copy_and_substr)
{
  rtw::stl::StringView view{"Hello, World!"};
  char buffer[6] = {};
  view.copy(buffer, 5U);         // NOLINT
  EXPECT_STREQ(buffer, "Hello"); // NOLINT

  rtw::stl::StringView substr = view.substr(7U, 5U);
  EXPECT_EQ(substr.size(), 5U);
  EXPECT_EQ(substr, "World");
}

TEST(StringViewTest, compare)
{
  rtw::stl::StringView view1{"Hello"};
  rtw::stl::StringView view2{"Hello"};
  rtw::stl::StringView view3{"World"};

  EXPECT_EQ(view1.compare(view2), 0);
  EXPECT_LT(view1.compare(view3), 0);
  EXPECT_GT(view3.compare(view1), 0);
}

TEST(StringViewTest, starts_with_and_ends_with)
{
  rtw::stl::StringView view{"Hello, World!"};

  EXPECT_TRUE(view.starts_with("Hello"));
  EXPECT_FALSE(view.starts_with("World"));
  EXPECT_TRUE(view.ends_with("World!"));
  EXPECT_FALSE(view.ends_with("Hello"));
}

TEST(StringViewTest, find)
{
  rtw::stl::StringView view{"Hello, World!"};

  EXPECT_EQ(view.find("World"), 7U);
  EXPECT_EQ(view.find("Hello"), 0U);
  EXPECT_EQ(view.find("!"), 12U);
  EXPECT_EQ(view.find("NotFound"),  std::string::npos);
  EXPECT_EQ(view.find('o'), 4U);
  EXPECT_EQ(view.find('z'),  std::string::npos);
}

TEST(StringViewTest, contains)
{
  rtw::stl::StringView view{"Hello, World!"};

  EXPECT_TRUE(view.contains("World"));
  EXPECT_TRUE(view.contains("Hello"));
  EXPECT_TRUE(view.contains('H'));
  EXPECT_FALSE(view.contains("NotFound"));
  EXPECT_FALSE(view.contains('z'));
}

TEST(StringViewTest, equality_operators)
{
  rtw::stl::StringView view1{"Hello"};
  rtw::stl::StringView view2{"Hello"};
  EXPECT_TRUE(view1 == view2);
  EXPECT_FALSE(view1 != view2);
  EXPECT_FALSE(view1 < view2);
  EXPECT_TRUE(view1 <= view2);
  EXPECT_FALSE(view1 > view2);
  EXPECT_TRUE(view1 >= view2);

  rtw::stl::StringView view3{"World"};
  EXPECT_FALSE(view1 == view3);
  EXPECT_TRUE(view1 != view3);
  EXPECT_TRUE(view1 < view3);
  EXPECT_TRUE(view1 <= view3);
  EXPECT_FALSE(view1 > view3);
  EXPECT_FALSE(view1 >= view3);
  EXPECT_TRUE(view3 > view1);
}

TEST(StringViewTest, ostream_operator)
{
  rtw::stl::StringView view{"Hello, World!"};
  std::ostringstream oss;
  oss << view;
  EXPECT_EQ(oss.str(), "Hello, World!");
}
