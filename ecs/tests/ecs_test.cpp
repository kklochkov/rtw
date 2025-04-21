#include "ecs/ecs.h"

#include <gtest/gtest.h>

namespace
{

enum class ComponentType : std::uint8_t
{
  NONE,
  TRANSFORM = 1U << 0U,
  RIGID_BODY = 1U << 1U,
  COLLIDER = 1U << 2U,
  SPRITE = 1U << 3U,
  MESH = 1U << 4U,
  DEBUG = 1U << 5U,
  HEALTH = 1U << 6U,
  DAMAGE = 1U << 7U,
};
struct Transform : rtw::ecs::Component<ComponentType, ComponentType::TRANSFORM>
{
  explicit Transform(const std::uint32_t data) : data{data} {}
  std::uint32_t data;
};

struct Rigidbody : rtw::ecs::Component<ComponentType, ComponentType::RIGID_BODY>
{
  explicit Rigidbody(const std::uint32_t data) : data{data} {}
  std::uint32_t data;
};

struct Collider : rtw::ecs::Component<ComponentType, ComponentType::COLLIDER>
{
  explicit Collider(const std::uint32_t data) : data{data} {}
  std::uint32_t data;
};

struct Sprite : rtw::ecs::Component<ComponentType, ComponentType::SPRITE>
{
  explicit Sprite(const std::uint32_t data) : data{data} {}
  std::uint32_t data;
};

struct Mesh : rtw::ecs::Component<ComponentType, ComponentType::MESH>
{
  explicit Mesh(const std::uint32_t data) : data{data} {}
  std::uint32_t data;
};

struct Debug : rtw::ecs::Component<ComponentType, ComponentType::DEBUG>
{
  explicit Debug(const std::uint32_t data) : data{data} {}
  std::uint32_t data;
};

struct Health : rtw::ecs::Component<ComponentType, ComponentType::HEALTH>
{
  explicit Health(const std::uint32_t data) : data{data} {}
  std::uint32_t data;
};

struct Damage : rtw::ecs::Component<ComponentType, ComponentType::DAMAGE>
{
  explicit Damage(const std::uint32_t data) : data{data} {}
  std::uint32_t data;
};

using ComponentManager = rtw::ecs::ComponentManager<ComponentType>;

} // namespace

TEST(EcsTest, log2)
{
  for (std::uint8_t i = 0U; i < 64U; ++i)
  {
    EXPECT_EQ(rtw::ecs::details::log2(1U << i), std::log2(1U << i));
  }
}

TEST(EcsTest, component_basic)
{
  static_assert(std::is_standard_layout_v<Transform>);
  static_assert(std::is_trivially_copyable_v<Transform>);
  static_assert(std::is_trivially_destructible_v<Transform>);

  EXPECT_EQ(Transform::TYPE, ComponentType::TRANSFORM);
  EXPECT_EQ(Transform::COMPONENT_ID, 0U);

  EXPECT_EQ(Rigidbody::TYPE, ComponentType::RIGID_BODY);
  EXPECT_EQ(Rigidbody::COMPONENT_ID, 1U);

  EXPECT_EQ(Collider::TYPE, ComponentType::COLLIDER);
  EXPECT_EQ(Collider::COMPONENT_ID, 2U);

  EXPECT_EQ(Sprite::TYPE, ComponentType::SPRITE);
  EXPECT_EQ(Sprite::COMPONENT_ID, 3U);

  EXPECT_EQ(Mesh::TYPE, ComponentType::MESH);
  EXPECT_EQ(Mesh::COMPONENT_ID, 4U);

  EXPECT_EQ(Debug::TYPE, ComponentType::DEBUG);
  EXPECT_EQ(Debug::COMPONENT_ID, 5U);

  EXPECT_EQ(Health::TYPE, ComponentType::HEALTH);
  EXPECT_EQ(Health::COMPONENT_ID, 6U);

  EXPECT_EQ(Damage::TYPE, ComponentType::DAMAGE);
  EXPECT_EQ(Damage::COMPONENT_ID, 7U);
}

TEST(EcsTest, component_manager)
{
  ComponentManager component_manager;

  EXPECT_DEATH(component_manager.get_component_storage<Transform>(), "");
  EXPECT_DEATH(component_manager.get_component_storage<Rigidbody>(), "");
  EXPECT_DEATH(component_manager.get_component_storage<Collider>(), "");
  EXPECT_DEATH(component_manager.get_component_storage<Sprite>(), "");
  EXPECT_DEATH(component_manager.get_component_storage<Mesh>(), "");
  EXPECT_DEATH(component_manager.get_component_storage<Debug>(), "");
  EXPECT_DEATH(component_manager.get_component_storage<Health>(), "");
  EXPECT_DEATH(component_manager.get_component_storage<Damage>(), "");

  component_manager.allocate_component_storage<Transform>();
  component_manager.allocate_component_storage<Rigidbody>();
  component_manager.allocate_component_storage<Collider>();
  component_manager.allocate_component_storage<Sprite>();
  component_manager.allocate_component_storage<Mesh>();
  component_manager.allocate_component_storage<Debug>();
  component_manager.allocate_component_storage<Health>();
  component_manager.allocate_component_storage<Damage>();

  EXPECT_NE(component_manager.get_component_storage<Transform>(), nullptr);
  EXPECT_NE(component_manager.get_component_storage<Rigidbody>(), nullptr);
  EXPECT_NE(component_manager.get_component_storage<Collider>(), nullptr);
  EXPECT_NE(component_manager.get_component_storage<Sprite>(), nullptr);
  EXPECT_NE(component_manager.get_component_storage<Mesh>(), nullptr);
  EXPECT_NE(component_manager.get_component_storage<Debug>(), nullptr);
  EXPECT_NE(component_manager.get_component_storage<Health>(), nullptr);
  EXPECT_NE(component_manager.get_component_storage<Damage>(), nullptr);
}

TEST(EcsTest, component_manager_add_component)
{
  ComponentManager component_manager;

  component_manager.allocate_component_storage<Transform>();
  component_manager.allocate_component_storage<Rigidbody>();
  component_manager.allocate_component_storage<Collider>();
  component_manager.allocate_component_storage<Sprite>();
  component_manager.allocate_component_storage<Mesh>();
  component_manager.allocate_component_storage<Debug>();
  component_manager.allocate_component_storage<Health>();
  component_manager.allocate_component_storage<Damage>();

  rtw::ecs::EntityManger<ComponentType> entity_manager;

  for (std::uint32_t i = 0U; i < 10U; ++i)
  {
    auto entity0 = entity_manager.create_entity();
    component_manager.add_component<Transform>(entity0, 42U + i);
    entity_manager.entity_signatures[entity0.id].set(ComponentType::TRANSFORM);
    EXPECT_EQ(component_manager.get_component<Transform>(entity0).data, 42U + i);
    EXPECT_TRUE(entity_manager.entity_signatures[entity0.id].test(ComponentType::TRANSFORM));

    component_manager.add_component<Rigidbody>(entity0, 43U + i);
    entity_manager.entity_signatures[entity0.id].set(ComponentType::RIGID_BODY);
    EXPECT_EQ(component_manager.get_component<Rigidbody>(entity0).data, 43U + i);
    EXPECT_TRUE(entity_manager.entity_signatures[entity0.id].test(ComponentType::RIGID_BODY));

    component_manager.add_component<Collider>(entity0, 44U + i);
    entity_manager.entity_signatures[entity0.id].set(ComponentType::COLLIDER);
    EXPECT_EQ(component_manager.get_component<Collider>(entity0).data, 44U + i);
    EXPECT_TRUE(entity_manager.entity_signatures[entity0.id].test(ComponentType::COLLIDER));

    component_manager.add_component<Sprite>(entity0, 45U + i);
    entity_manager.entity_signatures[entity0.id].set(ComponentType::SPRITE);
    EXPECT_EQ(component_manager.get_component<Sprite>(entity0).data, 45U + i);
    EXPECT_TRUE(entity_manager.entity_signatures[entity0.id].test(ComponentType::SPRITE));

    component_manager.add_component<Mesh>(entity0, 46U + i);
    entity_manager.entity_signatures[entity0.id].set(ComponentType::MESH);
    EXPECT_EQ(component_manager.get_component<Mesh>(entity0).data, 46U + i);
    EXPECT_TRUE(entity_manager.entity_signatures[entity0.id].test(ComponentType::MESH));

    component_manager.add_component<Debug>(entity0, 47U + i);
    entity_manager.entity_signatures[entity0.id].set(ComponentType::DEBUG);
    EXPECT_EQ(component_manager.get_component<Debug>(entity0).data, 47U + i);
    EXPECT_TRUE(entity_manager.entity_signatures[entity0.id].test(ComponentType::DEBUG));

    component_manager.add_component<Health>(entity0, 48U + i);
    entity_manager.entity_signatures[entity0.id].set(ComponentType::HEALTH);
    EXPECT_EQ(component_manager.get_component<Health>(entity0).data, 48U + i);
    EXPECT_TRUE(entity_manager.entity_signatures[entity0.id].test(ComponentType::HEALTH));

    component_manager.add_component<Damage>(entity0, 49U + i);
    entity_manager.entity_signatures[entity0.id].set(ComponentType::DAMAGE);
    EXPECT_EQ(component_manager.get_component<Damage>(entity0).data, 49U + i);
    EXPECT_TRUE(entity_manager.entity_signatures[entity0.id].test(ComponentType::DAMAGE));
  }

  EXPECT_EQ(entity_manager.entity_signatures.size(), 10U);
  EXPECT_EQ(component_manager.component_storages.size(), 8U);
  EXPECT_EQ(component_manager.entity_id_to_index.size(), 10U);
  EXPECT_EQ(component_manager.index_to_entity_id.size(), 10U);
}
