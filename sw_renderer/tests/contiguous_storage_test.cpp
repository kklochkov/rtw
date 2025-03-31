#include "sw_renderer/contiguous_storage.h"

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

using AlignedObjectStorage = rtw::stl::AlignedObjectStorage<Struct>;
using ContiguousStorage = rtw::stl::ContiguousStorage<Struct>;

} // namespace

TEST(AlignedObjectStorageTest, basic)
{
  AlignedObjectStorage storage{};
  EXPECT_FALSE(storage.is_constructed());
  EXPECT_DEATH(storage.get_pointer(), ".*");
  EXPECT_DEATH(storage.get_reference(), ".*");

  storage.construct(1.0F, 2, std::uint8_t{3});
  EXPECT_TRUE(storage.is_constructed());
  EXPECT_EQ(*storage.get_pointer(), (Struct{1.0F, 2, 3}));
  EXPECT_EQ(storage.get_reference(), (Struct{1.0F, 2, 3}));

  storage.destruct();
  EXPECT_FALSE(storage.is_constructed());
  EXPECT_DEATH(storage.get_pointer(), ".*");
  EXPECT_DEATH(storage.get_reference(), ".*");

  auto& value = storage.construct_for_overwrite_at();
  value = Struct{4.0F, 5, 6};
  EXPECT_TRUE(storage.is_constructed());
  EXPECT_EQ(*storage.get_pointer(), (Struct{4.0F, 5, 6}));
  EXPECT_EQ(storage.get_reference(), (Struct{4.0F, 5, 6}));
}

TEST(ContiguousStorageTest, constructor)
{
  ContiguousStorage storage{10U};
  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());

  EXPECT_DEATH(ContiguousStorage{0U}, ".*");
}

TEST(ContiguousStorageTest, construct)
{
  ContiguousStorage storage{10U};
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

  storage.clear();

  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());

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

TEST(ContiguousStorageTest, destruct)
{
  ContiguousStorage storage{10U};
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

TEST(ContiguousStorageTest, iterators)
{
  ContiguousStorage storage{10U};
  EXPECT_EQ(storage.used_slots(), 0U);
  EXPECT_EQ(storage.capacity(), 10U);
  EXPECT_TRUE(storage.empty());

  for (std::size_t i = 0U; i < storage.capacity(); ++i)
  {
    storage.construct_at(i, static_cast<float>(i + 1U), static_cast<std::int32_t>(i + 1U),
                         static_cast<std::uint8_t>(i + 1U));
  }

  std::size_t index = 0U;
  for (const auto& value : storage)
  {
    EXPECT_EQ(&value, &storage[index]);
    EXPECT_EQ(&value, storage.get_pointer(index));
    EXPECT_EQ(value, storage[index]);
    ++index;
  }

  for (std::size_t i = 0U; i < storage.used_slots() - 1U; ++i)
  {
    const auto* curr = storage.get_pointer(i);
    const auto* next = storage.get_pointer(i + 1U);
    EXPECT_EQ(next - curr, 1U);
  }
}
