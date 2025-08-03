#include "stl/static_string.h"

#include <gtest/gtest.h>

TEST(StaticStringTest, constructor)
{
  {
    // Default constructor
    rtw::stl::StaticString string{10U};
    EXPECT_EQ(string.size(), 0U);
    EXPECT_EQ(string.capacity(), 10U);
    EXPECT_TRUE(string.empty());
  }
  {
    // Pointer and size constructor
    const char* str = "Hello, World!";
    rtw::stl::StaticString string{str, 13U};
    EXPECT_EQ(string.size(), 13U);
    EXPECT_STREQ(string.data(), str);
    EXPECT_EQ(string[0], 'H');
    EXPECT_EQ(string[12], '!');
    EXPECT_TRUE(string.starts_with("Hello"));
    EXPECT_TRUE(string.ends_with("World!"));
  }
  {
    // Pointer constructor
    const char* str = "Hello, World!";
    rtw::stl::StaticString string{str};
    EXPECT_EQ(string.size(), 13U);
    EXPECT_STREQ(string.data(), str);
    EXPECT_EQ(string[0], 'H');
    EXPECT_EQ(string[12], '!');
    EXPECT_TRUE(string.starts_with("Hello"));
    EXPECT_TRUE(string.ends_with("World!"));
  }
  {
    // C-string constructor
    rtw::stl::StaticString string{"Hello, World!"};
    EXPECT_EQ(string.size(), 13U);
    EXPECT_STREQ(string.data(), "Hello, World!");
    EXPECT_EQ(string[0], 'H');
    EXPECT_EQ(string[12], '!');
    EXPECT_TRUE(string.starts_with("Hello"));
    EXPECT_TRUE(string.ends_with("World!"));
  }
}

TEST(StaticStringTest, size_and_empty)
{
  rtw::stl::StaticString string{"Hello, World!"};
  EXPECT_EQ(string.size(), 13U);
  EXPECT_FALSE(string.empty());

  rtw::stl::StaticString empty_string{10U};
  EXPECT_EQ(empty_string.size(), 0U);
  EXPECT_TRUE(empty_string.empty());
}

TEST(StaticStringTest, clear)
{
  rtw::stl::StaticString string{"Hello, World!"};
  EXPECT_EQ(string.size(), 13U);
  string.clear();
  EXPECT_EQ(string.size(), 0U);
  EXPECT_TRUE(string.empty());
}

TEST(StaticStringTest, push_back)
{
  rtw::stl::StaticString string{10U};
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

TEST(StaticStringTest, operator_brackets)
{
  rtw::stl::StaticString string{"Hello, World!"};
  EXPECT_EQ(string[0], 'H');
  EXPECT_EQ(string[12], '!');
  EXPECT_DEATH(string[13], ".*");
}

TEST(StaticStringTest, operator_plus_equal)
{
  {
    rtw::stl::StaticString string{"Hello", 40U};
    string += ", World!";
    EXPECT_EQ(string.size(), 13U);
    EXPECT_STREQ(string.data(), "Hello, World!");

    string += '!';
    EXPECT_EQ(string.size(), 14U);
    EXPECT_EQ(string.back(), '!');
  }
  {
    // Concatenating two StaticStrings with different sizes leading to truncated result
    rtw::stl::StaticString string{"123", 5U};
    string += rtw::stl::StaticString{"456"};

    EXPECT_EQ(string.size(), 5U);
    EXPECT_STREQ(string.data(), "12345");
  }
}

TEST(StaticStringTest, data_and_c_str)
{
  const char* str = "Hello, World!";
  rtw::stl::StaticString string{str};
  EXPECT_STREQ(string.data(), str);
  EXPECT_STREQ(string.c_str(), str);
}

TEST(StaticStringTest, front_and_back)
{
  rtw::stl::StaticString string{"Hello, World!"};
  EXPECT_EQ(string.front(), 'H');
  EXPECT_EQ(string.back(), '!');
}

TEST(StaticStringTest, begin_and_end)
{
  rtw::stl::StaticString string{"Hello, World!"};
  const auto* it = string.begin();
  EXPECT_EQ(*it, 'H');
  ++it; // NOLINT
  EXPECT_EQ(*it, 'e');
  it = string.end() - 1;
  EXPECT_EQ(*it, '!');
}

TEST(StaticStringTest, substr)
{
  rtw::stl::StaticString string{"Hello, World!"};
  rtw::stl::StaticString substr = string.substr(7U, 5U);
  EXPECT_EQ(substr.size(), 5U);
  EXPECT_EQ(substr, "World");
}

TEST(StaticStringTest, compare)
{
  rtw::stl::StaticString string1{"Hello"};
  rtw::stl::StaticString string2{"Hello"};
  rtw::stl::StaticString string3{"World"};

  EXPECT_EQ(string1.compare(string2), 0);
  EXPECT_LT(string1.compare(string3), 0);
  EXPECT_GT(string3.compare(string1), 0);
}

TEST(StaticStringTest, starts_with_and_ends_with)
{
  rtw::stl::StaticString string{"Hello, World!"};

  EXPECT_TRUE(string.starts_with("Hello"));
  EXPECT_FALSE(string.starts_with("World"));
  EXPECT_TRUE(string.ends_with("World!"));
  EXPECT_FALSE(string.ends_with("Hello"));
}

TEST(StaticStringTest, find)
{
  rtw::stl::StaticString string{"Hello, World!"};

  EXPECT_EQ(string.find("World"), 7U);
  EXPECT_EQ(string.find("Hello"), 0U);
  EXPECT_EQ(string.find("!"), 12U);
  EXPECT_EQ(string.find("NotFound"), rtw::stl::StaticString::NPOS);
  EXPECT_EQ(string.find('o'), 4U);
  EXPECT_EQ(string.find('z'), rtw::stl::StaticString::NPOS);
}

TEST(StaticStringTest, contains)
{
  rtw::stl::StaticString string{"Hello, World!"};

  EXPECT_TRUE(string.contains("World"));
  EXPECT_TRUE(string.contains("Hello"));
  EXPECT_TRUE(string.contains('H'));
  EXPECT_FALSE(string.contains("NotFound"));
  EXPECT_FALSE(string.contains('z'));
}

TEST(StaticStringTest, equality_operators)
{
  rtw::stl::StaticString string1{"Hello"};
  rtw::stl::StaticString string2{"Hello"};
  EXPECT_TRUE(string1 == string2);
  EXPECT_FALSE(string1 != string2);
  EXPECT_FALSE(string1 < string2);
  EXPECT_TRUE(string1 <= string2);
  EXPECT_FALSE(string1 > string2);
  EXPECT_TRUE(string1 >= string2);

  rtw::stl::StaticString string3{"World"};
  EXPECT_FALSE(string1 == string3);
  EXPECT_TRUE(string1 != string3);
  EXPECT_TRUE(string1 < string3);
  EXPECT_TRUE(string1 <= string3);
  EXPECT_FALSE(string1 > string3);
  EXPECT_FALSE(string1 >= string3);
  EXPECT_TRUE(string3 > string1);
}

TEST(StaticStringTest, ostream_operator)
{
  rtw::stl::StaticString string{"Hello, World!"};
  std::ostringstream oss;
  oss << string;
  EXPECT_EQ(oss.str(), "Hello, World!");
}
