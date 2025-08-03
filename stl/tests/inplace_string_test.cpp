#include "stl/format.h" // IWYU pragma: keep
#include "stl/inplace_string.h"

#include <gtest/gtest.h>

TEST(InplaceStringTest, constructor)
{
  {
    // Default constructor
    rtw::stl::InplaceString<10U> string{};
    EXPECT_EQ(string.size(), 0U);
    EXPECT_EQ(string.capacity(), 10U);
    EXPECT_TRUE(string.empty());
  }
  {
    // Pointer constructor
    const char* str = "Hello, World!";
    rtw::stl::InplaceString<13U> string{str};
    EXPECT_EQ(string.size(), 13U);
    EXPECT_STREQ(string.data(), str);
    EXPECT_EQ(string[0], 'H');
    EXPECT_EQ(string[12], '!');
    EXPECT_TRUE(string.starts_with("Hello"));
    EXPECT_TRUE(string.ends_with("World!"));
  }
  {
    // C-string constructor
    rtw::stl::InplaceString string{"Hello, World!"};
    EXPECT_EQ(string.size(), 13U);
    EXPECT_STREQ(string.data(), "Hello, World!");
    EXPECT_EQ(string[0], 'H');
    EXPECT_EQ(string[12], '!');
    EXPECT_TRUE(string.starts_with("Hello"));
    EXPECT_TRUE(string.ends_with("World!"));
  }
}

TEST(InplaceStringTest, size_and_empty)
{
  rtw::stl::InplaceString string{"Hello, World!"};
  EXPECT_EQ(string.size(), 13U);
  EXPECT_FALSE(string.empty());

  rtw::stl::InplaceString<10U> empty_string{};
  EXPECT_EQ(empty_string.size(), 0U);
  EXPECT_TRUE(empty_string.empty());
}

TEST(InplaceStringTest, clear)
{
  rtw::stl::InplaceString string{"Hello, World!"};
  EXPECT_EQ(string.size(), 13U);
  string.clear();
  EXPECT_EQ(string.size(), 0U);
  EXPECT_TRUE(string.empty());
}

TEST(InplaceStringTest, push_back)
{
  rtw::stl::InplaceString<10U> string{};
  EXPECT_EQ(string.size(), 0U);
  string.push_back('H');
  EXPECT_EQ(string.size(), 1U);
  EXPECT_EQ(string[0], 'H');
  string.push_back('e');
  EXPECT_EQ(string.size(), 2U);
  EXPECT_EQ(string[1], 'e');
  string.push_back('l');
  string.push_back('l');
  string.push_back('o');
  EXPECT_EQ(string.size(), 5U);
  EXPECT_STREQ(string.data(), "Hello");
}

TEST(InplaceStringTest, operator_brackets)
{
  rtw::stl::InplaceString string{"Hello, World!"};
  EXPECT_EQ(string[0], 'H');
  EXPECT_EQ(string[12], '!');
  EXPECT_DEATH(string[13], ".*");
}

TEST(InplaceStringTest, operator_plus_equal)
{
  {
    rtw::stl::InplaceString<15U> string{"Hello"};
    string += ", World!";
    EXPECT_EQ(string.size(), 13U);
    EXPECT_STREQ(string.data(), "Hello, World!");

    string += '!';
    EXPECT_EQ(string.size(), 14U);
    EXPECT_EQ(string.back(), '!');
  }
  {
    // Concatenating two InplaceStrings with different sizes leading to truncated result
    rtw::stl::InplaceString<5U> string{"123"};
    string += "456";

    EXPECT_EQ(string.size(), 5U);
    EXPECT_STREQ(string.data(), "12345");
  }
}

TEST(InplaceStringTest, data_and_c_str)
{
  const char* str = "Hello, World!";
  rtw::stl::InplaceString<13U> string{str};
  EXPECT_STREQ(string.data(), str);
  EXPECT_STREQ(string.c_str(), str);
}

TEST(InplaceStringTest, front_and_back)
{
  rtw::stl::InplaceString string{"Hello, World!"};
  EXPECT_EQ(string.front(), 'H');
  EXPECT_EQ(string.back(), '!');
}

TEST(InplaceStringTest, begin_and_end)
{
  rtw::stl::InplaceString string{"Hello, World!"};
  const auto* it = string.begin();
  EXPECT_EQ(*it, 'H');
  ++it; // NOLINT
  EXPECT_EQ(*it, 'e');
  it = string.end() - 1;
  EXPECT_EQ(*it, '!');
}

TEST(InplaceStringTest, substr)
{
  rtw::stl::InplaceString string{"Hello, World!"};
  rtw::stl::InplaceString<5U> substr = string.substr(7U, 5U);
  EXPECT_EQ(substr.size(), 5U);
  EXPECT_EQ(substr, "World");
}

TEST(InplaceStringTest, compare)
{
  rtw::stl::InplaceString string1{"Hello"};
  rtw::stl::InplaceString string2{"Hello"};
  rtw::stl::InplaceString string3{"World"};

  EXPECT_EQ(string1.compare(string2), 0);
  EXPECT_LT(string1.compare(string3), 0);
  EXPECT_GT(string3.compare(string1), 0);
}

TEST(InplaceStringTest, starts_with_and_ends_with)
{
  rtw::stl::InplaceString string{"Hello, World!"};

  EXPECT_TRUE(string.starts_with("Hello"));
  EXPECT_FALSE(string.starts_with("World"));
  EXPECT_TRUE(string.ends_with("World!"));
  EXPECT_FALSE(string.ends_with("Hello"));
}

TEST(InplaceStringTest, find)
{
  rtw::stl::InplaceString string{"Hello, World!"};

  EXPECT_EQ(string.find("World"), 7U);
  EXPECT_EQ(string.find("Hello"), 0U);
  EXPECT_EQ(string.find("!"), 12U);
  EXPECT_EQ(string.find("NotFound"), std::string::npos);
  EXPECT_EQ(string.find('o'), 4U);
  EXPECT_EQ(string.find('z'), std::string::npos);
}

TEST(InplaceStringTest, contains)
{
  rtw::stl::InplaceString string{"Hello, World!"};

  EXPECT_TRUE(string.contains("World"));
  EXPECT_TRUE(string.contains("Hello"));
  EXPECT_TRUE(string.contains('H'));
  EXPECT_FALSE(string.contains("NotFound"));
  EXPECT_FALSE(string.contains('z'));
}

TEST(InplaceStringTest, equality_operators)
{
  rtw::stl::InplaceString string1{"Hello"};
  rtw::stl::InplaceString string2{"Hello"};
  EXPECT_TRUE(string1 == string2);
  EXPECT_FALSE(string1 != string2);
  EXPECT_FALSE(string1 < string2);
  EXPECT_TRUE(string1 <= string2);
  EXPECT_FALSE(string1 > string2);
  EXPECT_TRUE(string1 >= string2);

  EXPECT_TRUE(rtw::stl::InplaceString{"Hello"} == rtw::stl::make_string("Hello"));

  rtw::stl::InplaceString string3{"World"};
  EXPECT_FALSE(string1 == string3);
  EXPECT_TRUE(string1 != string3);
  EXPECT_TRUE(string1 < string3);
  EXPECT_TRUE(string1 <= string3);
  EXPECT_FALSE(string1 > string3);
  EXPECT_FALSE(string1 >= string3);
  EXPECT_TRUE(string3 > string1);
}

TEST(InplaceStringTest, ostream_operator)
{
  rtw::stl::InplaceString string{"Hello, World!"};
  std::ostringstream oss;
  oss << string;
  EXPECT_EQ(oss.str(), "Hello, World!");
}
