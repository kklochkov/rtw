#include "stl/static_contiguous_storage.h"

#include <gtest/gtest.h>

namespace
{

struct Struct
{
  Struct() = default;
  Struct(const float a, const std::int32_t b, const std::uint8_t c) : a{a}, b{b}, c{c} {}
  Struct(const Struct&) = default;
  Struct(Struct&&) = default;
  Struct& operator=(const Struct&) = default;
  Struct& operator=(Struct&&) = default;
  ~Struct() {} // NOLINT: intentionally make class non-trivial

  float a{};
  std::int32_t b{};
  std::uint8_t c{};

  bool operator==(const Struct& other) const { return std::tie(a, b, c) == std::tie(other.a, other.b, other.c); }
};

using StaticContiguousStorage = rtw::stl::StaticContiguousStorage<Struct>;
using AlignedObjectStorage = StaticContiguousStorage::storage_type;

} // namespace

TEST(StaticContiguousStorageTest, constructor)
{
  static_assert(!AlignedObjectStorage::is_trivial::value, "AlignedObjectStorage should not trival.");
  static_assert(!std::is_trivially_copyable_v<AlignedObjectStorage>,
                "AlignedObjectStorage should not be trivially copyable.");
  static_assert(sizeof(AlignedObjectStorage) == sizeof(Struct) + alignof(Struct),
                "AlignedObjectStorage should have the same size as Struct plus alignment.");

  StaticContiguousStorage storage{10U};
  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());
  EXPECT_TRUE(storage.cbegin() == storage.cend());

  EXPECT_DEATH(StaticContiguousStorage{0U}, ".*");
}

TEST(StaticContiguousStorageTest, construct)
{
  StaticContiguousStorage storage{10U};
  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    EXPECT_FALSE(storage.is_constructed(i));
  }

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    storage.construct_at(i, static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                         static_cast<std::uint8_t>(i + 1U));
    EXPECT_EQ(storage[i], s);
  }

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    EXPECT_TRUE(storage.is_constructed(i));
  }

  EXPECT_EQ(storage.used_slots(), storage.capacity());
  EXPECT_FALSE(storage.empty());
  EXPECT_FALSE(storage.cbegin() == storage.cend());

  storage.clear();

  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());
  EXPECT_TRUE(storage.cbegin() == storage.cend());

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    EXPECT_FALSE(storage.is_constructed(i));
  }

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    auto& s = storage.construct_for_overwrite_at(i);
    s = Struct{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    EXPECT_EQ(storage[i], s);
  }

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    EXPECT_TRUE(storage.is_constructed(i));
  }

  EXPECT_EQ(storage.used_slots(), storage.capacity());
  EXPECT_FALSE(storage.empty());
}

TEST(StaticContiguousStorageTest, destruct)
{
  StaticContiguousStorage storage{10U};
  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    const Struct s{static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U), static_cast<std::uint8_t>(i + 1U)};
    storage.construct_at(i, static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                         static_cast<std::uint8_t>(i + 1U));
    EXPECT_EQ(storage[i], s);
  }

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    EXPECT_TRUE(storage.is_constructed(i));
  }

  EXPECT_EQ(storage.used_slots(), storage.capacity());
  EXPECT_FALSE(storage.empty());

  for (std::size_t i = 0U; i < storage.capacity(); i += 2U)
  {
    storage.destruct_at(i);
  }

  EXPECT_EQ(storage.used_slots(), storage.capacity() / 2U);

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    if (i % 2U)
    {
      EXPECT_TRUE(storage.is_constructed(i));
    }
    else
    {
      EXPECT_FALSE(storage.is_constructed(i));
    }
  }

  storage.clear();

  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    EXPECT_FALSE(storage.is_constructed(i));
  }
}

TEST(StaticContiguousStorageTest, iterators)
{
  StaticContiguousStorage storage{10U};
  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());

  std::vector<AlignedObjectStorage> expected{10U};
  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    storage.construct_at(i, static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                         static_cast<std::uint8_t>(i + 1U));
    expected[i].construct(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                          static_cast<std::uint8_t>(i + 1U));
  }

  std::size_t index = 0U;
  for (const auto& value : storage)
  {
    EXPECT_EQ(&value, &storage[index]);
    EXPECT_EQ(value, storage[index]);
    ++index;
  }

  EXPECT_EQ(*(storage.begin() + 1U), storage[1U]);
  EXPECT_EQ(&*(storage.begin() + 1U), &storage[1U]);
  EXPECT_EQ(*(storage.begin() + 1U), *expected[1U]);

  EXPECT_TRUE(rtw::stl::is_memory_contiguous(expected.begin(), expected.end()));
  EXPECT_TRUE(rtw::stl::is_memory_contiguous(storage.begin(), storage.end()));
}

TEST(StaticContiguousStorageTest, reverse_iterator)
{
  StaticContiguousStorage storage{10U};
  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());

  std::vector<AlignedObjectStorage> expected{10U};
  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    storage.construct_at(i, static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                         static_cast<std::uint8_t>(i + 1U));
    expected[i].construct(static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                          static_cast<std::uint8_t>(i + 1U));
  }

  std::reverse(expected.begin(), expected.end());
  std::reverse(storage.begin(), storage.end());

  for (std::size_t i = 0U; i < storage.used_slots(); ++i)
  {
    EXPECT_EQ(storage[i], *expected[i]);
  }

  std::reverse(expected.rbegin(), expected.rend());
  std::reverse(storage.rbegin(), storage.rend());

  for (std::size_t i = 0U; i < storage.used_slots(); ++i)
  {
    EXPECT_EQ(storage[i], *expected[i]);
  }
}
