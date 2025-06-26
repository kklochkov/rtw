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

constexpr std::size_t MAX_NUMBER_OF_ENTITIES = 1'000;
using ComponentManager =
    rtw::ecs::ComponentManager<ComponentType, Transform, Rigidbody, Collider, Sprite, Mesh, Debug, Health, Damage>;
using EntityManger = rtw::ecs::EntityManger<ComponentType>;

static_assert(ComponentManager::NUMBER_OF_REGISTERED_COMPONENTS == 8U,
              "ComponentManager must have 8 registered components.");
static_assert(std::is_same_v<ComponentManager::ComponentType, ComponentType>,
              "ComponentManager's ComponentType must match the enum type.");
static_assert(std::is_same_v<EntityManger::ComponentType, ComponentType>,
              "EntityManger's ComponentType must match the enum type.");
static_assert(std::is_same_v<ComponentManager::ComponentType, EntityManger::ComponentType>,
              "ComponentManager's ComponentType must match EntityManger's ComponentType.");

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

TEST(EcsTest, component_manager_add_component)
{
  ComponentManager component_manager{MAX_NUMBER_OF_ENTITIES};
  EntityManger entity_manager{MAX_NUMBER_OF_ENTITIES};

  for (std::uint32_t i = 0U; i < 10U; ++i)
  {
    auto entity = entity_manager.create();

    EXPECT_FALSE(component_manager.has<Transform>(entity));
    EXPECT_FALSE(component_manager.has<Rigidbody>(entity));
    EXPECT_FALSE(component_manager.has<Collider>(entity));
    EXPECT_FALSE(component_manager.has<Sprite>(entity));
    EXPECT_FALSE(component_manager.has<Mesh>(entity));
    EXPECT_FALSE(component_manager.has<Debug>(entity));
    EXPECT_FALSE(component_manager.has<Health>(entity));
    EXPECT_FALSE(component_manager.has<Damage>(entity));

    component_manager.emplace<Transform>(entity, 42U + i);
    entity_manager.set_signature(entity, ComponentType::TRANSFORM);
    EXPECT_EQ(component_manager.get<Transform>(entity).data, 42U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::TRANSFORM));
    EXPECT_TRUE(component_manager.has<Transform>(entity));

    component_manager.emplace<Rigidbody>(entity, 43U + i);
    entity_manager.set_signature(entity, ComponentType::RIGID_BODY);
    EXPECT_EQ(component_manager.get<Rigidbody>(entity).data, 43U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::RIGID_BODY));
    EXPECT_TRUE(component_manager.has<Rigidbody>(entity));

    component_manager.emplace<Collider>(entity, 44U + i);
    entity_manager.set_signature(entity, ComponentType::COLLIDER);
    EXPECT_EQ(component_manager.get<Collider>(entity).data, 44U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::COLLIDER));
    EXPECT_TRUE(component_manager.has<Collider>(entity));

    component_manager.emplace<Sprite>(entity, 45U + i);
    entity_manager.set_signature(entity, ComponentType::SPRITE);
    EXPECT_EQ(component_manager.get<Sprite>(entity).data, 45U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::SPRITE));
    EXPECT_TRUE(component_manager.has<Sprite>(entity));

    component_manager.emplace<Mesh>(entity, 46U + i);
    entity_manager.set_signature(entity, ComponentType::MESH);
    EXPECT_EQ(component_manager.get<Mesh>(entity).data, 46U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::MESH));
    EXPECT_TRUE(component_manager.has<Mesh>(entity));

    component_manager.emplace<Debug>(entity, 47U + i);
    entity_manager.set_signature(entity, ComponentType::DEBUG);
    EXPECT_EQ(component_manager.get<Debug>(entity).data, 47U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::DEBUG));
    EXPECT_TRUE(component_manager.has<Debug>(entity));

    component_manager.emplace<Health>(entity, 48U + i);
    entity_manager.set_signature(entity, ComponentType::HEALTH);
    EXPECT_EQ(component_manager.get<Health>(entity).data, 48U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::HEALTH));
    EXPECT_TRUE(component_manager.has<Health>(entity));

    component_manager.emplace<Damage>(entity, 49U + i);
    entity_manager.set_signature(entity, ComponentType::DAMAGE);
    EXPECT_EQ(component_manager.get<Damage>(entity).data, 49U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::DAMAGE));
    EXPECT_TRUE(component_manager.has<Damage>(entity));
  }

  EXPECT_EQ(entity_manager.size(), 10U);
  EXPECT_EQ(component_manager.size<Transform>(), 10U);
  EXPECT_EQ(component_manager.size<Rigidbody>(), 10U);
  EXPECT_EQ(component_manager.size<Collider>(), 10U);
  EXPECT_EQ(component_manager.size<Sprite>(), 10U);
  EXPECT_EQ(component_manager.size<Mesh>(), 10U);
  EXPECT_EQ(component_manager.size<Debug>(), 10U);
  EXPECT_EQ(component_manager.size<Health>(), 10U);
  EXPECT_EQ(component_manager.size<Damage>(), 10U);
}

TEST(EcsTest, component_manager_destroy_component)
{
  ComponentManager component_manager{MAX_NUMBER_OF_ENTITIES};
  EntityManger entity_manager{MAX_NUMBER_OF_ENTITIES};

  std::vector<rtw::ecs::Entity> entities;
  for (std::uint32_t i = 0U; i < 10U; ++i)
  {
    auto entity = entity_manager.create();

    EXPECT_FALSE(component_manager.has<Transform>(entity));
    EXPECT_FALSE(component_manager.has<Rigidbody>(entity));
    EXPECT_FALSE(component_manager.has<Collider>(entity));
    EXPECT_FALSE(component_manager.has<Sprite>(entity));
    EXPECT_FALSE(component_manager.has<Mesh>(entity));
    EXPECT_FALSE(component_manager.has<Debug>(entity));
    EXPECT_FALSE(component_manager.has<Health>(entity));
    EXPECT_FALSE(component_manager.has<Damage>(entity));

    component_manager.emplace<Transform>(entity, 42U + i);
    entity_manager.set_signature(entity, ComponentType::TRANSFORM);
    EXPECT_EQ(component_manager.get<Transform>(entity).data, 42U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::TRANSFORM));
    EXPECT_TRUE(component_manager.has<Transform>(entity));

    component_manager.emplace<Rigidbody>(entity, 43U + i);
    entity_manager.set_signature(entity, ComponentType::RIGID_BODY);
    EXPECT_EQ(component_manager.get<Rigidbody>(entity).data, 43U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::RIGID_BODY));
    EXPECT_TRUE(component_manager.has<Rigidbody>(entity));

    component_manager.emplace<Collider>(entity, 44U + i);
    entity_manager.set_signature(entity, ComponentType::COLLIDER);
    EXPECT_EQ(component_manager.get<Collider>(entity).data, 44U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::COLLIDER));
    EXPECT_TRUE(component_manager.has<Collider>(entity));

    component_manager.emplace<Sprite>(entity, 45U + i);
    entity_manager.set_signature(entity, ComponentType::SPRITE);
    EXPECT_EQ(component_manager.get<Sprite>(entity).data, 45U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::SPRITE));
    EXPECT_TRUE(component_manager.has<Sprite>(entity));

    component_manager.emplace<Mesh>(entity, 46U + i);
    entity_manager.set_signature(entity, ComponentType::MESH);
    EXPECT_EQ(component_manager.get<Mesh>(entity).data, 46U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::MESH));
    EXPECT_TRUE(component_manager.has<Mesh>(entity));

    component_manager.emplace<Debug>(entity, 47U + i);
    entity_manager.set_signature(entity, ComponentType::DEBUG);
    EXPECT_EQ(component_manager.get<Debug>(entity).data, 47U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::DEBUG));
    EXPECT_TRUE(component_manager.has<Debug>(entity));

    component_manager.emplace<Health>(entity, 48U + i);
    entity_manager.set_signature(entity, ComponentType::HEALTH);
    EXPECT_EQ(component_manager.get<Health>(entity).data, 48U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::HEALTH));
    EXPECT_TRUE(component_manager.has<Health>(entity));

    component_manager.emplace<Damage>(entity, 49U + i);
    entity_manager.set_signature(entity, ComponentType::DAMAGE);
    EXPECT_EQ(component_manager.get<Damage>(entity).data, 49U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::DAMAGE));
    EXPECT_TRUE(component_manager.has<Damage>(entity));

    entities.push_back(entity);
  }

  EXPECT_EQ(entity_manager.size(), 10U);
  EXPECT_EQ(component_manager.size<Transform>(), 10U);
  EXPECT_EQ(component_manager.size<Rigidbody>(), 10U);
  EXPECT_EQ(component_manager.size<Collider>(), 10U);
  EXPECT_EQ(component_manager.size<Sprite>(), 10U);
  EXPECT_EQ(component_manager.size<Mesh>(), 10U);
  EXPECT_EQ(component_manager.size<Debug>(), 10U);
  EXPECT_EQ(component_manager.size<Health>(), 10U);
  EXPECT_EQ(component_manager.size<Damage>(), 10U);
  EXPECT_EQ(component_manager.total_size(), 80U);
  EXPECT_EQ(entities.size(), 10U);

  for (const auto entity : entities)
  {
    component_manager.remove<Transform>(entity);
    component_manager.remove<Rigidbody>(entity);
    component_manager.remove<Collider>(entity);
    component_manager.remove<Sprite>(entity);
    component_manager.remove<Mesh>(entity);
    component_manager.remove<Debug>(entity);
    component_manager.remove<Health>(entity);
    component_manager.remove<Damage>(entity);
    component_manager.remove_all(entity);

    EXPECT_FALSE(component_manager.has<Transform>(entity));
    EXPECT_FALSE(component_manager.has<Rigidbody>(entity));
    EXPECT_FALSE(component_manager.has<Collider>(entity));
    EXPECT_FALSE(component_manager.has<Sprite>(entity));
    EXPECT_FALSE(component_manager.has<Mesh>(entity));
    EXPECT_FALSE(component_manager.has<Debug>(entity));
    EXPECT_FALSE(component_manager.has<Health>(entity));
    EXPECT_FALSE(component_manager.has<Damage>(entity));

    entity_manager.destroy(entity);
  }
  entities.clear();

  EXPECT_EQ(entity_manager.size(), 0U);
  EXPECT_EQ(component_manager.size<Transform>(), 0U);
  EXPECT_EQ(component_manager.size<Rigidbody>(), 0U);
  EXPECT_EQ(component_manager.size<Collider>(), 0U);
  EXPECT_EQ(component_manager.size<Sprite>(), 0U);
  EXPECT_EQ(component_manager.size<Mesh>(), 0U);
  EXPECT_EQ(component_manager.size<Debug>(), 0U);
  EXPECT_EQ(component_manager.size<Health>(), 0U);
  EXPECT_EQ(component_manager.size<Damage>(), 0U);
  EXPECT_EQ(component_manager.total_size(), 0U);
  EXPECT_EQ(entities.size(), 0U);

  for (std::uint32_t i = 0U; i < 10U; ++i)
  {
    auto entity = entity_manager.create();
    component_manager.emplace<Transform>(entity, 442U + i);
    entity_manager.set_signature(entity, ComponentType::TRANSFORM);
    EXPECT_EQ(component_manager.get<Transform>(entity).data, 442U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::TRANSFORM));
    EXPECT_TRUE(component_manager.has<Transform>(entity));

    component_manager.emplace<Rigidbody>(entity, 443U + i);
    entity_manager.set_signature(entity, ComponentType::RIGID_BODY);
    EXPECT_EQ(component_manager.get<Rigidbody>(entity).data, 443U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::RIGID_BODY));
    EXPECT_TRUE(component_manager.has<Rigidbody>(entity));

    component_manager.emplace<Collider>(entity, 444U + i);
    entity_manager.set_signature(entity, ComponentType::COLLIDER);
    EXPECT_EQ(component_manager.get<Collider>(entity).data, 444U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::COLLIDER));
    EXPECT_TRUE(component_manager.has<Collider>(entity));

    component_manager.emplace<Sprite>(entity, 445U + i);
    entity_manager.set_signature(entity, ComponentType::SPRITE);
    EXPECT_EQ(component_manager.get<Sprite>(entity).data, 445U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::SPRITE));
    EXPECT_TRUE(component_manager.has<Sprite>(entity));

    component_manager.emplace<Mesh>(entity, 446U + i);
    entity_manager.set_signature(entity, ComponentType::MESH);
    EXPECT_EQ(component_manager.get<Mesh>(entity).data, 446U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::MESH));
    EXPECT_TRUE(component_manager.has<Mesh>(entity));

    component_manager.emplace<Debug>(entity, 447U + i);
    entity_manager.set_signature(entity, ComponentType::DEBUG);
    EXPECT_EQ(component_manager.get<Debug>(entity).data, 447U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::DEBUG));
    EXPECT_TRUE(component_manager.has<Debug>(entity));

    component_manager.emplace<Health>(entity, 448U + i);
    entity_manager.set_signature(entity, ComponentType::HEALTH);
    EXPECT_EQ(component_manager.get<Health>(entity).data, 448U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::HEALTH));
    EXPECT_TRUE(component_manager.has<Health>(entity));

    component_manager.emplace<Damage>(entity, 449U + i);
    entity_manager.set_signature(entity, ComponentType::DAMAGE);
    EXPECT_EQ(component_manager.get<Damage>(entity).data, 449U + i);
    EXPECT_TRUE(entity_manager.test_signature(entity, ComponentType::DAMAGE));
    EXPECT_TRUE(component_manager.has<Damage>(entity));

    entities.push_back(entity);
  }

  EXPECT_EQ(entity_manager.size(), 10U);
  EXPECT_EQ(component_manager.size<Transform>(), 10U);
  EXPECT_EQ(component_manager.size<Rigidbody>(), 10U);
  EXPECT_EQ(component_manager.size<Collider>(), 10U);
  EXPECT_EQ(component_manager.size<Sprite>(), 10U);
  EXPECT_EQ(component_manager.size<Mesh>(), 10U);
  EXPECT_EQ(component_manager.size<Debug>(), 10U);
  EXPECT_EQ(component_manager.size<Health>(), 10U);
  EXPECT_EQ(component_manager.size<Damage>(), 10U);
  EXPECT_EQ(component_manager.total_size(), 80U);
  EXPECT_EQ(entities.size(), 10U);
}
