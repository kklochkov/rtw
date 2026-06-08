#include "stl/static_vector.h"

#include "stl/static_stack.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{

struct Struct
{
  Struct() = default;
  Struct(const float a, const std::int32_t b, const std::uint8_t c) : a{a}, b{b}, c{c} {}

  float a{};
  std::int32_t b{};
  std::uint8_t c{};

  bool operator==(const Struct& other) const { return std::tie(a, b, c) == std::tie(other.a, other.b, other.c); }
};

using InplaceStaticVector = rtw::stl::InplaceStaticVector<Struct, 10U>;

} // namespace

TEST(InplaceStaticVectorTest, constructor)
{
  static_assert(std::is_trivially_copyable_v<InplaceStaticVector>, "InplaceStaticVector should be trivially copyable.");
  static_assert(std::is_trivially_destructible_v<InplaceStaticVector>,
                "InplaceStaticVector should be trivially destructible.");

  InplaceStaticVector vector;
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());
  EXPECT_TRUE(vector.begin() == vector.end());
}

TEST(InplaceStaticVectorTest, push_back)
{
  InplaceStaticVector vector;
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    vector.push_back(s);
    EXPECT_EQ(vector[i], s);
  }

  EXPECT_EQ(vector.size(), vector.capacity());
  EXPECT_FALSE(vector.begin() == vector.end());

  for (std::size_t i = 0U; i < vector.size(); ++i)
  {
    EXPECT_EQ(vector[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_DEATH(vector.push_back(Struct{}), ".*");

  vector.clear();

  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());
  EXPECT_TRUE(vector.begin() == vector.end());
}

TEST(InplaceStaticVectorTest, emplace_back)
{
  InplaceStaticVector vector;
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    const auto& s = vector.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                        static_cast<std::uint8_t>(i + 1U));
    EXPECT_EQ(vector[i], s);
    EXPECT_EQ(vector[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_EQ(vector.size(), vector.capacity());

  for (std::size_t i = 0U; i < vector.size(); ++i)
  {
    EXPECT_EQ(vector[i], (Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                                 static_cast<std::uint8_t>(i + 1U)}));
  }

  EXPECT_DEATH(vector.emplace_back(), ".*");

  vector.clear();
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());
}

TEST(InplaceStaticVectorTest, pop_back)
{
  InplaceStaticVector vector;
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    vector.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                        static_cast<std::uint8_t>(i + 1U));
  }

  const auto size = vector.size() - 1U;
  for (std::size_t i = 0U; i < size; ++i)
  {
    const std::size_t index = vector.size() - 2U;
    const auto* ptr = &vector[index];

    vector.pop_back();

    EXPECT_EQ(vector.size(), vector.capacity() - i - 1U);
    EXPECT_FALSE(vector.empty());
    EXPECT_EQ(vector[index], (Struct{static_cast<float>(index + 1U), static_cast<std::int32_t>(index + 1U),
                                     static_cast<std::uint8_t>(index + 1U)}));
    EXPECT_EQ(ptr, &vector[index]);
  }

  EXPECT_EQ(vector.size(), 1U);
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(vector[0U], (Struct{1.0F, 1, 1}));

  vector.pop_back();

  EXPECT_EQ(vector.size(), 0U);
  EXPECT_TRUE(vector.empty());
  EXPECT_DEATH(vector.pop_back(), ".*");
}

TEST(InplaceStaticVectorTest, iterators)
{
  InplaceStaticVector vector;
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    vector.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                        static_cast<std::uint8_t>(i + 1U));
  }

  std::size_t index = 0U;
  for (const auto& value : vector)
  {
    EXPECT_EQ(&value, &vector[index]);
    EXPECT_EQ(value, vector[index]);
    ++index;
  }

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(vector.begin(), vector.end()));

  vector.pop_back();
  vector.pop_back();
  vector.pop_back();

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(vector.begin(), vector.end()));
}

TEST(InplaceStaticVectorTest, reverse_iterator)
{
  InplaceStaticVector vector;
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_EQ(vector.capacity(), 10U);
  EXPECT_TRUE(vector.empty());

  std::vector<Struct> expected;
  for (std::size_t i = 0U; i < vector.capacity(); ++i)
  {
    vector.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                        static_cast<std::uint8_t>(i + 1U));
    expected.emplace_back(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                          static_cast<std::uint8_t>(i + 1U));
  }

  std::reverse(expected.begin(), expected.end());
  std::reverse(vector.begin(), vector.end());

  EXPECT_THAT(expected, ::testing::ElementsAreArray(vector));

  std::reverse(expected.rbegin(), expected.rend());
  std::reverse(vector.rbegin(), vector.rend());

  EXPECT_THAT(expected, ::testing::ElementsAreArray(vector));
}

TEST(InplaceStaticVectorTest, resize)
{
  InplaceStaticVector vector;

  vector.resize(5U);
  EXPECT_EQ(vector.size(), 5U);
  for (std::size_t i = 0U; i < 5U; ++i)
  {
    EXPECT_EQ(vector[i], (Struct{}));
  }

  // Resize down.
  vector.resize(2U);
  EXPECT_EQ(vector.size(), 2U);

  // Resize back up.
  vector.resize(4U);
  EXPECT_EQ(vector.size(), 4U);

  // Resize to zero.
  vector.resize(0U);
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_TRUE(vector.empty());
}

TEST(InplaceStaticVectorTest, insert)
{
  InplaceStaticVector vector;

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});

  // Insert at the beginning.
  vector.insert(0U, Struct{0.0F, 0, 0});
  EXPECT_EQ(vector.size(), 4U);
  EXPECT_EQ(vector[0U], (Struct{0.0F, 0, 0}));
  EXPECT_EQ(vector[1U], (Struct{1.0F, 1, 1}));
  EXPECT_EQ(vector[2U], (Struct{2.0F, 2, 2}));
  EXPECT_EQ(vector[3U], (Struct{3.0F, 3, 3}));

  // Insert in the middle.
  vector.insert(2U, Struct{1.5F, 15, 15});
  EXPECT_EQ(vector.size(), 5U);
  EXPECT_EQ(vector[2U], (Struct{1.5F, 15, 15}));
  EXPECT_EQ(vector[3U], (Struct{2.0F, 2, 2}));

  // Insert at the end.
  vector.insert(vector.size(), Struct{9.0F, 9, 9});
  EXPECT_EQ(vector.size(), 6U);
  EXPECT_EQ(vector[5U], (Struct{9.0F, 9, 9}));

  // Insert with move.
  Struct moved{7.0F, 7, 7};
  vector.insert(1U, std::move(moved));
  EXPECT_EQ(vector.size(), 7U);
  EXPECT_EQ(vector[1U], (Struct{7.0F, 7, 7}));
}

TEST(InplaceStaticVectorTest, erase)
{
  InplaceStaticVector vector;

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});
  vector.push_back(Struct{4.0F, 4, 4});
  vector.push_back(Struct{5.0F, 5, 5});

  // Erase from the middle.
  vector.erase(2U);
  EXPECT_EQ(vector.size(), 4U);
  EXPECT_EQ(vector[0U], (Struct{1.0F, 1, 1}));
  EXPECT_EQ(vector[1U], (Struct{2.0F, 2, 2}));
  EXPECT_EQ(vector[2U], (Struct{4.0F, 4, 4}));
  EXPECT_EQ(vector[3U], (Struct{5.0F, 5, 5}));

  // Erase from the beginning.
  vector.erase(0U);
  EXPECT_EQ(vector.size(), 3U);
  EXPECT_EQ(vector[0U], (Struct{2.0F, 2, 2}));

  // Erase from the end.
  vector.erase(vector.size() - 1U);
  EXPECT_EQ(vector.size(), 2U);
  EXPECT_EQ(vector[0U], (Struct{2.0F, 2, 2}));
  EXPECT_EQ(vector[1U], (Struct{4.0F, 4, 4}));

  // Erase all.
  vector.erase(0U);
  vector.erase(0U);
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_TRUE(vector.empty());
}

TEST(InplaceStaticVectorTest, insert_iterator)
{
  InplaceStaticVector vector;

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});

  // Insert at the beginning via iterator.
  auto it = vector.insert(vector.cbegin(), Struct{0.0F, 0, 0});
  EXPECT_EQ(vector.size(), 4U);
  EXPECT_EQ(*it, (Struct{0.0F, 0, 0}));
  EXPECT_EQ(vector[0U], (Struct{0.0F, 0, 0}));
  EXPECT_EQ(vector[1U], (Struct{1.0F, 1, 1}));

  // Insert in the middle via iterator.
  it = vector.insert(vector.cbegin() + 2, Struct{1.5F, 15, 15});
  EXPECT_EQ(vector.size(), 5U);
  EXPECT_EQ(*it, (Struct{1.5F, 15, 15}));
  EXPECT_EQ(vector[2U], (Struct{1.5F, 15, 15}));
  EXPECT_EQ(vector[3U], (Struct{2.0F, 2, 2}));

  // Insert at the end via iterator.
  it = vector.insert(vector.cend(), Struct{9.0F, 9, 9});
  EXPECT_EQ(vector.size(), 6U);
  EXPECT_EQ(*it, (Struct{9.0F, 9, 9}));
  EXPECT_EQ(vector[5U], (Struct{9.0F, 9, 9}));

  // Insert with move via iterator.
  Struct moved{7.0F, 7, 7};
  it = vector.insert(vector.cbegin() + 1, std::move(moved));
  EXPECT_EQ(vector.size(), 7U);
  EXPECT_EQ(*it, (Struct{7.0F, 7, 7}));
  EXPECT_EQ(vector[1U], (Struct{7.0F, 7, 7}));
}

TEST(InplaceStaticVectorTest, erase_iterator)
{
  InplaceStaticVector vector;

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});
  vector.push_back(Struct{4.0F, 4, 4});
  vector.push_back(Struct{5.0F, 5, 5});

  // Erase from the middle via iterator.
  auto it = vector.erase(vector.cbegin() + 2);
  EXPECT_EQ(vector.size(), 4U);
  EXPECT_EQ(*it, (Struct{4.0F, 4, 4}));
  EXPECT_EQ(vector[2U], (Struct{4.0F, 4, 4}));

  // Erase from the beginning via iterator.
  it = vector.erase(vector.cbegin());
  EXPECT_EQ(vector.size(), 3U);
  EXPECT_EQ(*it, (Struct{2.0F, 2, 2}));

  // Erase last element via iterator — returned iterator should equal end().
  it = vector.erase(vector.cbegin() + 2);
  EXPECT_EQ(vector.size(), 2U);
  EXPECT_EQ(it, vector.end());

  // Erase all via iterator.
  vector.erase(vector.cbegin());
  vector.erase(vector.cbegin());
  EXPECT_EQ(vector.size(), 0U);
  EXPECT_TRUE(vector.empty());
}

TEST(InplaceStaticVectorTest, front_back_data)
{
  InplaceStaticVector vector;

  vector.push_back(Struct{1.0F, 1, 1});
  vector.push_back(Struct{2.0F, 2, 2});
  vector.push_back(Struct{3.0F, 3, 3});

  EXPECT_EQ(vector.front(), (Struct{1.0F, 1, 1}));
  EXPECT_EQ(vector.back(), (Struct{3.0F, 3, 3}));

  // Modify via front/back.
  vector.front() = Struct{10.0F, 10, 10};
  vector.back() = Struct{30.0F, 30, 30};
  EXPECT_EQ(vector[0U], (Struct{10.0F, 10, 10}));
  EXPECT_EQ(vector[2U], (Struct{30.0F, 30, 30}));
}

// =============================================================================
// Constexpr evaluation tests (static_assert)
// =============================================================================
// Use a non-standard-layout type to ensure AlignedObjectStorage<T, false> (assignment-based, C++17 constexpr).
namespace
{

struct ConstexprBase
{
  int tag{};
};

struct ConstexprVal : ConstexprBase
{
  constexpr ConstexprVal() noexcept = default;
  constexpr explicit ConstexprVal(int v) noexcept : ConstexprBase{0}, val{v} {}
  int val{};
  constexpr bool operator==(const ConstexprVal& other) const noexcept { return val == other.val; }
};

// Verify the type hits the non-trivial (constexpr-compatible) storage path.
static_assert(!rtw::stl::details::IS_TRIVIAL_V<ConstexprVal>,
              "ConstexprVal must be non-trivial to use the constexpr storage path.");

// --- InplaceStaticVector ---
constexpr auto make_constexpr_vector()
{
  rtw::stl::InplaceStaticVector<ConstexprVal, 8U> vec;
  vec.emplace_back(1);
  vec.emplace_back(2);
  vec.emplace_back(3);
  vec.push_back(ConstexprVal{4});
  vec.pop_back();
  return vec;
}

static_assert(make_constexpr_vector().size() == 3U, "constexpr vector size");
static_assert(make_constexpr_vector()[0U] == ConstexprVal{1}, "constexpr vector[0]");
static_assert(make_constexpr_vector()[2U] == ConstexprVal{3}, "constexpr vector[2]");
static_assert(!make_constexpr_vector().empty(), "constexpr vector not empty");
static_assert(make_constexpr_vector().capacity() == 8U, "constexpr vector capacity");

constexpr auto make_constexpr_vector_with_resize()
{
  rtw::stl::InplaceStaticVector<ConstexprVal, 8U> vec;
  vec.emplace_back(10);
  vec.emplace_back(20);
  vec.resize(4U);
  return vec;
}

static_assert(make_constexpr_vector_with_resize().size() == 4U, "constexpr vector resize grows");
static_assert(make_constexpr_vector_with_resize()[0U] == ConstexprVal{10}, "constexpr vector resize preserves");

constexpr auto make_constexpr_vector_cleared()
{
  rtw::stl::InplaceStaticVector<ConstexprVal, 4U> vec;
  vec.emplace_back(1);
  vec.emplace_back(2);
  vec.clear();
  return vec;
}

static_assert(make_constexpr_vector_cleared().size() == 0U, "constexpr vector clear");
static_assert(make_constexpr_vector_cleared().empty(), "constexpr vector empty after clear");

// --- InplaceStaticStack ---
constexpr auto make_constexpr_stack()
{
  rtw::stl::InplaceStaticStack<ConstexprVal, 8U> stack;
  stack.emplace(1);
  stack.emplace(2);
  stack.emplace(3);
  stack.pop();
  return stack;
}

static_assert(make_constexpr_stack().size() == 2U, "constexpr stack size");
static_assert(make_constexpr_stack().top() == ConstexprVal{2}, "constexpr stack top");
static_assert(make_constexpr_stack().bottom() == ConstexprVal{1}, "constexpr stack bottom");

// --- InplaceStaticVector insert/erase ---
constexpr auto make_constexpr_vector_with_insert()
{
  rtw::stl::InplaceStaticVector<ConstexprVal, 8U> vec;
  vec.emplace_back(1);
  vec.emplace_back(3);
  vec.emplace_back(4);
  vec.insert(1U, ConstexprVal{2}); // insert at index 1
  return vec;
}

static_assert(make_constexpr_vector_with_insert().size() == 4U, "constexpr vector insert size");
static_assert(make_constexpr_vector_with_insert()[0U] == ConstexprVal{1}, "constexpr vector insert [0]");
static_assert(make_constexpr_vector_with_insert()[1U] == ConstexprVal{2}, "constexpr vector insert [1]");
static_assert(make_constexpr_vector_with_insert()[2U] == ConstexprVal{3}, "constexpr vector insert [2]");
static_assert(make_constexpr_vector_with_insert()[3U] == ConstexprVal{4}, "constexpr vector insert [3]");

constexpr auto make_constexpr_vector_with_insert_at_begin()
{
  rtw::stl::InplaceStaticVector<ConstexprVal, 8U> vec;
  vec.emplace_back(2);
  vec.emplace_back(3);
  vec.insert(0U, ConstexprVal{1}); // insert at beginning
  return vec;
}

static_assert(make_constexpr_vector_with_insert_at_begin().size() == 3U, "constexpr vector insert begin size");
static_assert(make_constexpr_vector_with_insert_at_begin()[0U] == ConstexprVal{1}, "constexpr vector insert begin [0]");
static_assert(make_constexpr_vector_with_insert_at_begin()[1U] == ConstexprVal{2}, "constexpr vector insert begin [1]");

constexpr auto make_constexpr_vector_with_insert_at_end()
{
  rtw::stl::InplaceStaticVector<ConstexprVal, 8U> vec;
  vec.emplace_back(1);
  vec.emplace_back(2);
  vec.insert(2U, ConstexprVal{3}); // insert at end (append)
  return vec;
}

static_assert(make_constexpr_vector_with_insert_at_end().size() == 3U, "constexpr vector insert end size");
static_assert(make_constexpr_vector_with_insert_at_end()[2U] == ConstexprVal{3}, "constexpr vector insert end [2]");

constexpr auto make_constexpr_vector_with_erase()
{
  rtw::stl::InplaceStaticVector<ConstexprVal, 8U> vec;
  vec.emplace_back(1);
  vec.emplace_back(2);
  vec.emplace_back(3);
  vec.emplace_back(4);
  vec.erase(1U); // erase element at index 1 (value 2)
  return vec;
}

static_assert(make_constexpr_vector_with_erase().size() == 3U, "constexpr vector erase size");
static_assert(make_constexpr_vector_with_erase()[0U] == ConstexprVal{1}, "constexpr vector erase [0]");
static_assert(make_constexpr_vector_with_erase()[1U] == ConstexprVal{3}, "constexpr vector erase [1]");
static_assert(make_constexpr_vector_with_erase()[2U] == ConstexprVal{4}, "constexpr vector erase [2]");

constexpr auto make_constexpr_vector_with_erase_first()
{
  rtw::stl::InplaceStaticVector<ConstexprVal, 8U> vec;
  vec.emplace_back(1);
  vec.emplace_back(2);
  vec.emplace_back(3);
  vec.erase(0U); // erase first
  return vec;
}

static_assert(make_constexpr_vector_with_erase_first().size() == 2U, "constexpr vector erase first size");
static_assert(make_constexpr_vector_with_erase_first()[0U] == ConstexprVal{2}, "constexpr vector erase first [0]");
static_assert(make_constexpr_vector_with_erase_first()[1U] == ConstexprVal{3}, "constexpr vector erase first [1]");

constexpr auto make_constexpr_vector_with_erase_last()
{
  rtw::stl::InplaceStaticVector<ConstexprVal, 8U> vec;
  vec.emplace_back(1);
  vec.emplace_back(2);
  vec.emplace_back(3);
  vec.erase(2U); // erase last
  return vec;
}

static_assert(make_constexpr_vector_with_erase_last().size() == 2U, "constexpr vector erase last size");
static_assert(make_constexpr_vector_with_erase_last()[0U] == ConstexprVal{1}, "constexpr vector erase last [0]");
static_assert(make_constexpr_vector_with_erase_last()[1U] == ConstexprVal{2}, "constexpr vector erase last [1]");

} // namespace
