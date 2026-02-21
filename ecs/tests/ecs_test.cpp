#include "ecs/ecs.h"

#include <gtest/gtest.h>

namespace
{

// NOLINTNEXTLINE(performance-enum-size): Just to illustrate that ECS supports up to 64 components.
enum class ComponentType : std::uint64_t
{
  NONE = 0U,
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

constexpr std::size_t MAX_NUMBER_OF_ENTITIES = 1'000U;
constexpr std::size_t MAX_NUMBER_OF_ENTITIES_PER_GROUP = 100U;
constexpr std::size_t MAX_NUMBER_OF_ENTITIES_PER_SYSTEM = 100U;
constexpr std::size_t MAX_NUMBER_OF_SYSTEMS = 10U;

using ComponentManager =
    rtw::ecs::ComponentManager<ComponentType, Transform, Rigidbody, Collider, Sprite, Mesh, Debug, Health, Damage>;
using EntityManager = rtw::ecs::EntityManager<ComponentType, MAX_NUMBER_OF_ENTITIES_PER_GROUP>;
using SystemManager = rtw::ecs::SystemManager<ComponentType>;
using System = rtw::ecs::System<ComponentType>;
using Entity = rtw::ecs::Entity<ComponentType>;

constexpr inline rtw::ecs::EntitySignature<ComponentType> DEFAULT_ENTITY_SIGNATURE{
    ComponentType::TRANSFORM | ComponentType::RIGID_BODY | ComponentType::COLLIDER | ComponentType::SPRITE
    | ComponentType::MESH | ComponentType::DEBUG | ComponentType::HEALTH | ComponentType::DAMAGE};
constexpr inline rtw::ecs::SystemSignature<ComponentType> DEFAULT_SYSTEM_SIGNATURE{
    ComponentType::TRANSFORM | ComponentType::RIGID_BODY | ComponentType::COLLIDER | ComponentType::SPRITE
    | ComponentType::MESH | ComponentType::DEBUG | ComponentType::HEALTH | ComponentType::DAMAGE};

static_assert(ComponentManager::NUMBER_OF_REGISTERED_COMPONENTS == 8U,
              "ComponentManager must have 8 registered components.");
static_assert(std::is_same_v<ComponentManager::ComponentType, ComponentType>,
              "ComponentManager's ComponentType must match the enum type.");
static_assert(std::is_same_v<EntityManager::ComponentType, ComponentType>,
              "EntityManager's ComponentType must match the enum type.");
static_assert(std::is_same_v<ComponentManager::ComponentType, EntityManager::ComponentType>,
              "ComponentManager's ComponentType must match EntityManager's ComponentType.");
static_assert(std::is_same_v<SystemManager ::ComponentType, EntityManager::ComponentType>,
              "SystemManager's ComponentType must match EntityManager's ComponentType.");

using ECSManager = rtw::ecs::ECSManager<ComponentType, MAX_NUMBER_OF_ENTITIES_PER_GROUP, Transform, Rigidbody, Collider,
                                        Sprite, Mesh, Debug, Health, Damage>;

struct DefaultSystem : public System
{
  DefaultSystem() noexcept : System{DEFAULT_SYSTEM_SIGNATURE, MAX_NUMBER_OF_ENTITIES_PER_SYSTEM} {}

  void update(ECSManager& ecs_manager)
  {
    for (const auto& entity : get_entities())
    {
      auto& transform = ecs_manager.get_component<Transform>(entity);
      auto& rigidbody = ecs_manager.get_component<Rigidbody>(entity);

      transform.data = 42U;
      rigidbody.data = 24U;
    }
  }
};

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
  EntityManager entity_manager{MAX_NUMBER_OF_ENTITIES};

  for (std::uint32_t i = 0U; i < 10U; ++i)
  {
    const auto entity = entity_manager.create(DEFAULT_ENTITY_SIGNATURE);
    EXPECT_EQ(entity.signature, DEFAULT_ENTITY_SIGNATURE);

    EXPECT_FALSE(component_manager.has<Transform>(entity));
    EXPECT_FALSE(component_manager.has<Rigidbody>(entity));
    EXPECT_FALSE(component_manager.has<Collider>(entity));
    EXPECT_FALSE(component_manager.has<Sprite>(entity));
    EXPECT_FALSE(component_manager.has<Mesh>(entity));
    EXPECT_FALSE(component_manager.has<Debug>(entity));
    EXPECT_FALSE(component_manager.has<Health>(entity));
    EXPECT_FALSE(component_manager.has<Damage>(entity));

    component_manager.emplace<Transform>(entity, 42U + i);
    component_manager.emplace<Rigidbody>(entity, 43U + i);
    component_manager.emplace<Collider>(entity, 44U + i);
    component_manager.emplace<Sprite>(entity, 45U + i);
    component_manager.emplace<Mesh>(entity, 46U + i);
    component_manager.emplace<Debug>(entity, 47U + i);
    component_manager.emplace<Health>(entity, 48U + i);
    component_manager.emplace<Damage>(entity, 49U + i);

    EXPECT_TRUE(component_manager.has<Transform>(entity));
    EXPECT_TRUE(component_manager.has<Rigidbody>(entity));
    EXPECT_TRUE(component_manager.has<Collider>(entity));
    EXPECT_TRUE(component_manager.has<Sprite>(entity));
    EXPECT_TRUE(component_manager.has<Mesh>(entity));
    EXPECT_TRUE(component_manager.has<Debug>(entity));
    EXPECT_TRUE(component_manager.has<Health>(entity));
    EXPECT_TRUE(component_manager.has<Damage>(entity));

    EXPECT_EQ(component_manager.get<Transform>(entity).data, 42U + i);
    EXPECT_EQ(component_manager.get<Rigidbody>(entity).data, 43U + i);
    EXPECT_EQ(component_manager.get<Collider>(entity).data, 44U + i);
    EXPECT_EQ(component_manager.get<Sprite>(entity).data, 45U + i);
    EXPECT_EQ(component_manager.get<Mesh>(entity).data, 46U + i);
    EXPECT_EQ(component_manager.get<Debug>(entity).data, 47U + i);
    EXPECT_EQ(component_manager.get<Health>(entity).data, 48U + i);
    EXPECT_EQ(component_manager.get<Damage>(entity).data, 49U + i);
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
  EntityManager entity_manager{MAX_NUMBER_OF_ENTITIES};

  std::vector<Entity> entities;
  for (std::uint32_t i = 0U; i < 10U; ++i)
  {
    const auto entity = entity_manager.create(DEFAULT_ENTITY_SIGNATURE);
    entities.push_back(entity);

    EXPECT_EQ(entity.signature, DEFAULT_ENTITY_SIGNATURE);

    EXPECT_FALSE(component_manager.has<Transform>(entity));
    EXPECT_FALSE(component_manager.has<Rigidbody>(entity));
    EXPECT_FALSE(component_manager.has<Collider>(entity));
    EXPECT_FALSE(component_manager.has<Sprite>(entity));
    EXPECT_FALSE(component_manager.has<Mesh>(entity));
    EXPECT_FALSE(component_manager.has<Debug>(entity));
    EXPECT_FALSE(component_manager.has<Health>(entity));
    EXPECT_FALSE(component_manager.has<Damage>(entity));

    component_manager.emplace<Transform>(entity, 42U + i);
    component_manager.emplace<Rigidbody>(entity, 43U + i);
    component_manager.emplace<Collider>(entity, 44U + i);
    component_manager.emplace<Sprite>(entity, 45U + i);
    component_manager.emplace<Mesh>(entity, 46U + i);
    component_manager.emplace<Debug>(entity, 47U + i);
    component_manager.emplace<Health>(entity, 48U + i);
    component_manager.emplace<Damage>(entity, 49U + i);

    EXPECT_TRUE(component_manager.has<Transform>(entity));
    EXPECT_TRUE(component_manager.has<Rigidbody>(entity));
    EXPECT_TRUE(component_manager.has<Collider>(entity));
    EXPECT_TRUE(component_manager.has<Sprite>(entity));
    EXPECT_TRUE(component_manager.has<Mesh>(entity));
    EXPECT_TRUE(component_manager.has<Debug>(entity));
    EXPECT_TRUE(component_manager.has<Health>(entity));
    EXPECT_TRUE(component_manager.has<Damage>(entity));

    EXPECT_EQ(component_manager.get<Transform>(entity).data, 42U + i);
    EXPECT_EQ(component_manager.get<Rigidbody>(entity).data, 43U + i);
    EXPECT_EQ(component_manager.get<Collider>(entity).data, 44U + i);
    EXPECT_EQ(component_manager.get<Sprite>(entity).data, 45U + i);
    EXPECT_EQ(component_manager.get<Mesh>(entity).data, 46U + i);
    EXPECT_EQ(component_manager.get<Debug>(entity).data, 47U + i);
    EXPECT_EQ(component_manager.get<Health>(entity).data, 48U + i);
    EXPECT_EQ(component_manager.get<Damage>(entity).data, 49U + i);
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
    component_manager.remove(entity);

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
    const auto entity = entity_manager.create(DEFAULT_ENTITY_SIGNATURE);
    entities.push_back(entity);

    EXPECT_EQ(entity.signature, DEFAULT_ENTITY_SIGNATURE);

    EXPECT_FALSE(component_manager.has<Transform>(entity));
    EXPECT_FALSE(component_manager.has<Rigidbody>(entity));
    EXPECT_FALSE(component_manager.has<Collider>(entity));
    EXPECT_FALSE(component_manager.has<Sprite>(entity));
    EXPECT_FALSE(component_manager.has<Mesh>(entity));
    EXPECT_FALSE(component_manager.has<Debug>(entity));
    EXPECT_FALSE(component_manager.has<Health>(entity));
    EXPECT_FALSE(component_manager.has<Damage>(entity));

    component_manager.emplace<Transform>(entity, 442U + i);
    component_manager.emplace<Rigidbody>(entity, 443U + i);
    component_manager.emplace<Collider>(entity, 444U + i);
    component_manager.emplace<Sprite>(entity, 445U + i);
    component_manager.emplace<Mesh>(entity, 446U + i);
    component_manager.emplace<Debug>(entity, 447U + i);
    component_manager.emplace<Health>(entity, 448U + i);
    component_manager.emplace<Damage>(entity, 449U + i);

    EXPECT_TRUE(component_manager.has<Transform>(entity));
    EXPECT_TRUE(component_manager.has<Rigidbody>(entity));
    EXPECT_TRUE(component_manager.has<Collider>(entity));
    EXPECT_TRUE(component_manager.has<Sprite>(entity));
    EXPECT_TRUE(component_manager.has<Mesh>(entity));
    EXPECT_TRUE(component_manager.has<Debug>(entity));
    EXPECT_TRUE(component_manager.has<Health>(entity));
    EXPECT_TRUE(component_manager.has<Damage>(entity));

    EXPECT_EQ(component_manager.get<Transform>(entity).data, 442U + i);
    EXPECT_EQ(component_manager.get<Rigidbody>(entity).data, 443U + i);
    EXPECT_EQ(component_manager.get<Collider>(entity).data, 444U + i);
    EXPECT_EQ(component_manager.get<Sprite>(entity).data, 445U + i);
    EXPECT_EQ(component_manager.get<Mesh>(entity).data, 446U + i);
    EXPECT_EQ(component_manager.get<Debug>(entity).data, 447U + i);
    EXPECT_EQ(component_manager.get<Health>(entity).data, 448U + i);
    EXPECT_EQ(component_manager.get<Damage>(entity).data, 449U + i);
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

TEST(EcsTest, system_basic)
{
  ComponentManager component_manager{MAX_NUMBER_OF_ENTITIES};
  EntityManager entity_manager{MAX_NUMBER_OF_ENTITIES};
  SystemManager system_manager{MAX_NUMBER_OF_SYSTEMS};

  auto& system = system_manager.create<DefaultSystem>();
  EXPECT_EQ(system.get_signature(), DEFAULT_SYSTEM_SIGNATURE);

  const auto entity = entity_manager.create(DEFAULT_ENTITY_SIGNATURE);
  system.add_entity(entity);
  EXPECT_EQ(system.size(), 1U);

  component_manager.emplace<Transform>(entity, 42U);
  component_manager.emplace<Rigidbody>(entity, 43U);
  component_manager.emplace<Collider>(entity, 44U);
  component_manager.emplace<Sprite>(entity, 45U);
  component_manager.emplace<Mesh>(entity, 46U);
  component_manager.emplace<Debug>(entity, 47U);
  component_manager.emplace<Health>(entity, 48U);
  component_manager.emplace<Damage>(entity, 49U);
}

TEST(EcsTest, ecs_basic)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};
  ecs_manager.create_system<DefaultSystem>();

  std::vector<Entity> entities;
  for (std::uint32_t i = 0U; i < 10U; ++i)
  {
    const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
    entities.push_back(entity);

    EXPECT_EQ(entity.signature, DEFAULT_ENTITY_SIGNATURE);

    ecs_manager.emplace_component<Transform>(entity, 42U + i);
    ecs_manager.emplace_component<Rigidbody>(entity, 43U + i);
    ecs_manager.emplace_component<Collider>(entity, 44U + i);
    ecs_manager.emplace_component<Sprite>(entity, 45U + i);
    ecs_manager.emplace_component<Mesh>(entity, 46U + i);
    ecs_manager.emplace_component<Debug>(entity, 47U + i);
    ecs_manager.emplace_component<Health>(entity, 48U + i);
    ecs_manager.emplace_component<Damage>(entity, 49U + i);

    EXPECT_EQ(ecs_manager.get_component<Transform>(entity).data, 42U + i);
    EXPECT_EQ(ecs_manager.get_component<Rigidbody>(entity).data, 43U + i);
    EXPECT_EQ(ecs_manager.get_component<Collider>(entity).data, 44U + i);
    EXPECT_EQ(ecs_manager.get_component<Sprite>(entity).data, 45U + i);
    EXPECT_EQ(ecs_manager.get_component<Mesh>(entity).data, 46U + i);
    EXPECT_EQ(ecs_manager.get_component<Debug>(entity).data, 47U + i);
    EXPECT_EQ(ecs_manager.get_component<Health>(entity).data, 48U + i);
    EXPECT_EQ(ecs_manager.get_component<Damage>(entity).data, 49U + i);
  }

  EXPECT_EQ(ecs_manager.get_number_of_entities(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Transform>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Rigidbody>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Collider>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Sprite>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Mesh>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Debug>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Health>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Damage>(), 10U);
  EXPECT_EQ(ecs_manager.get_total_number_of_components(), 80U);
  EXPECT_EQ(ecs_manager.get_system<DefaultSystem>().size(), 10U);
  EXPECT_EQ(entities.size(), 10U);

  for (const auto entity : entities)
  {
    ecs_manager.destroy_entity(entity);
  }
  entities.clear();

  EXPECT_EQ(ecs_manager.get_number_of_entities(), 0U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Transform>(), 0U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Rigidbody>(), 0U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Collider>(), 0U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Sprite>(), 0U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Mesh>(), 0U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Debug>(), 0U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Health>(), 0U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Damage>(), 0U);
  EXPECT_EQ(ecs_manager.get_total_number_of_components(), 0U);
  EXPECT_EQ(ecs_manager.get_system<DefaultSystem>().size(), 0U);
  EXPECT_EQ(entities.size(), 0U);

  for (std::uint32_t i = 0U; i < 10U; ++i)
  {
    const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
    entities.push_back(entity);

    EXPECT_EQ(entity.signature, DEFAULT_ENTITY_SIGNATURE);

    ecs_manager.emplace_component<Transform>(entity, 442U + i);
    ecs_manager.emplace_component<Rigidbody>(entity, 443U + i);
    ecs_manager.emplace_component<Collider>(entity, 444U + i);
    ecs_manager.emplace_component<Sprite>(entity, 445U + i);
    ecs_manager.emplace_component<Mesh>(entity, 446U + i);
    ecs_manager.emplace_component<Debug>(entity, 447U + i);
    ecs_manager.emplace_component<Health>(entity, 448U + i);
    ecs_manager.emplace_component<Damage>(entity, 449U + i);

    EXPECT_EQ(ecs_manager.get_component<Transform>(entity).data, 442U + i);
    EXPECT_EQ(ecs_manager.get_component<Rigidbody>(entity).data, 443U + i);
    EXPECT_EQ(ecs_manager.get_component<Collider>(entity).data, 444U + i);
    EXPECT_EQ(ecs_manager.get_component<Sprite>(entity).data, 445U + i);
    EXPECT_EQ(ecs_manager.get_component<Mesh>(entity).data, 446U + i);
    EXPECT_EQ(ecs_manager.get_component<Debug>(entity).data, 447U + i);
    EXPECT_EQ(ecs_manager.get_component<Health>(entity).data, 448U + i);
    EXPECT_EQ(ecs_manager.get_component<Damage>(entity).data, 449U + i);
  }

  EXPECT_EQ(ecs_manager.get_number_of_entities(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Transform>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Rigidbody>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Collider>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Sprite>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Mesh>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Debug>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Health>(), 10U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Damage>(), 10U);
  EXPECT_EQ(ecs_manager.get_total_number_of_components(), 80U);
  EXPECT_EQ(ecs_manager.get_system<DefaultSystem>().size(), 10U);
  EXPECT_EQ(entities.size(), 10U);

  auto& default_system = ecs_manager.get_system<DefaultSystem>();
  default_system.update(ecs_manager);

  for (const auto entity : default_system.get_entities())
  {
    const auto& transform = ecs_manager.get_component<Transform>(entity);
    const auto& rigidbody = ecs_manager.get_component<Rigidbody>(entity);

    EXPECT_EQ(transform.data, 42U);
    EXPECT_EQ(rigidbody.data, 24U);
  }
}

TEST(EcsTest, tagging_and_groupping)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};

  const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);

  ecs_manager.tag_entity(entity, "Player");
  EXPECT_TRUE(ecs_manager.is_entity_tagged(entity, "Player"));

  ecs_manager.add_entity_to_group(entity, "Group1");
  EXPECT_TRUE(ecs_manager.is_entity_in_group(entity, "Group1"));

  ecs_manager.add_entity_to_group(entity, "Group2");
  EXPECT_TRUE(ecs_manager.is_entity_in_group(entity, "Group2"));
  EXPECT_FALSE(ecs_manager.is_entity_in_group(entity, "Group1"));

  ecs_manager.remove_entity_from_group(entity);
  EXPECT_FALSE(ecs_manager.is_entity_in_group(entity, "Group1"));
  EXPECT_FALSE(ecs_manager.is_entity_in_group(entity, "Group2"));

  ecs_manager.untag_entity(entity);
  EXPECT_FALSE(ecs_manager.is_entity_tagged(entity, "Player"));
}

TEST(EcsTest, generation_counter_validity)
{
  // Use a small entity pool to ensure index reuse
  constexpr std::size_t SMALL_POOL = 2U;
  rtw::ecs::ECSManager<ComponentType, MAX_NUMBER_OF_ENTITIES_PER_GROUP, Transform, Rigidbody, Collider, Sprite, Mesh,
                       Debug, Health, Damage>
      ecs_manager{SMALL_POOL, MAX_NUMBER_OF_SYSTEMS};

  // Create first entity (uses index 0 from free list)
  const auto entity1 = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  ecs_manager.emplace_component<Transform>(entity1, 100U);
  EXPECT_TRUE(ecs_manager.is_entity_valid(entity1));
  EXPECT_EQ(entity1.id.index, 0U);
  EXPECT_EQ(entity1.id.generation, 0U);

  // Create second entity (uses index 1 from free list, exhausting the pool)
  const auto entity2 = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  EXPECT_EQ(entity2.id.index, 1U);
  EXPECT_EQ(entity2.id.generation, 0U);

  // Destroy first entity - its index goes to back of free list with incremented generation
  ecs_manager.destroy_entity(entity1);
  EXPECT_FALSE(ecs_manager.is_entity_valid(entity1));

  // Create third entity with DIFFERENT signature - should reuse index 0 with generation 1
  constexpr rtw::ecs::EntitySignature<ComponentType> MINIMAL_SIGNATURE{ComponentType::TRANSFORM | ComponentType::NONE};
  const auto entity3 = ecs_manager.create_entity(MINIMAL_SIGNATURE);
  EXPECT_EQ(entity3.id.index, 0U);
  EXPECT_EQ(entity3.id.generation, 1U);

  // Original entity1 handle should still be invalid (different generation)
  EXPECT_FALSE(ecs_manager.is_entity_valid(entity1));
  EXPECT_TRUE(ecs_manager.is_entity_valid(entity3));

  // They have the same index but different generations
  EXPECT_EQ(entity1.id.index, entity3.id.index);
  EXPECT_NE(entity1.id.generation, entity3.id.generation);

  // Verify signature was properly reset - entity3 should have minimal signature, not entity1's old signature
  EXPECT_EQ(entity3.signature, MINIMAL_SIGNATURE);
  EXPECT_NE(entity3.signature, DEFAULT_ENTITY_SIGNATURE);
}

TEST(EcsTest, edge_cases_double_destroy_and_double_add)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};

  // Create entity with a component
  const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  ecs_manager.emplace_component<Transform>(entity, 42U);
  EXPECT_EQ(ecs_manager.get_component<Transform>(entity).data, 42U);

  // Double add should preserve original value (silent no-op)
  ecs_manager.emplace_component<Transform>(entity, 999U);
  EXPECT_EQ(ecs_manager.get_component<Transform>(entity).data, 42U);

  // Destroy the entity
  ecs_manager.destroy_entity(entity);
  EXPECT_FALSE(ecs_manager.is_entity_valid(entity));
  EXPECT_EQ(ecs_manager.get_number_of_entities(), 0U);

  // Double destroy should be safe (no-op, no crash)
  ecs_manager.destroy_entity(entity);
  EXPECT_FALSE(ecs_manager.is_entity_valid(entity));
  EXPECT_EQ(ecs_manager.get_number_of_entities(), 0U);
}

TEST(EcsTest, remove_component)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};

  const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  ecs_manager.emplace_component<Transform>(entity, 42U);
  ecs_manager.emplace_component<Rigidbody>(entity, 43U);

  EXPECT_TRUE(ecs_manager.has_component<Transform>(entity));
  EXPECT_TRUE(ecs_manager.has_component<Rigidbody>(entity));
  EXPECT_EQ(ecs_manager.get_number_of_components<Transform>(), 1U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Rigidbody>(), 1U);

  // Remove one component
  ecs_manager.remove_component<Transform>(entity);
  EXPECT_FALSE(ecs_manager.has_component<Transform>(entity));
  EXPECT_TRUE(ecs_manager.has_component<Rigidbody>(entity));
  EXPECT_EQ(ecs_manager.get_number_of_components<Transform>(), 0U);
  EXPECT_EQ(ecs_manager.get_number_of_components<Rigidbody>(), 1U);

  // Remove should be idempotent (removing again shouldn't crash)
  ecs_manager.remove_component<Transform>(entity);
  EXPECT_FALSE(ecs_manager.has_component<Transform>(entity));
}

TEST(EcsTest, has_component)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};

  const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);

  // Initially no components
  EXPECT_FALSE(ecs_manager.has_component<Transform>(entity));
  EXPECT_FALSE(ecs_manager.has_component<Rigidbody>(entity));

  // Add one component
  ecs_manager.emplace_component<Transform>(entity, 42U);
  EXPECT_TRUE(ecs_manager.has_component<Transform>(entity));
  EXPECT_FALSE(ecs_manager.has_component<Rigidbody>(entity));

  // Add another component
  ecs_manager.emplace_component<Rigidbody>(entity, 43U);
  EXPECT_TRUE(ecs_manager.has_component<Transform>(entity));
  EXPECT_TRUE(ecs_manager.has_component<Rigidbody>(entity));
}

TEST(EcsTest, create_system_returns_reference)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};

  // create_system should return a reference to the created system
  auto& system = ecs_manager.create_system<DefaultSystem>();
  EXPECT_EQ(system.get_signature(), DEFAULT_SYSTEM_SIGNATURE);
  EXPECT_EQ(system.size(), 0U);

  // Add an entity and verify through the returned reference
  const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  ecs_manager.emplace_component<Transform>(entity, 42U);
  ecs_manager.emplace_component<Rigidbody>(entity, 43U);
  ecs_manager.emplace_component<Collider>(entity, 44U);
  ecs_manager.emplace_component<Sprite>(entity, 45U);
  ecs_manager.emplace_component<Mesh>(entity, 46U);
  ecs_manager.emplace_component<Debug>(entity, 47U);
  ecs_manager.emplace_component<Health>(entity, 48U);
  ecs_manager.emplace_component<Damage>(entity, 49U);

  EXPECT_EQ(system.size(), 1U);

  // Verify it's the same system as get_system returns
  auto& retrieved_system = ecs_manager.get_system<DefaultSystem>();
  EXPECT_EQ(&system, &retrieved_system);
}

TEST(EcsTest, destroy_cleans_up_tags_and_groups)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};

  const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);

  // Tag and group the entity
  ecs_manager.tag_entity(entity, "Player");
  ecs_manager.add_entity_to_group(entity, "Enemies");

  EXPECT_TRUE(ecs_manager.is_entity_tagged(entity, "Player"));
  EXPECT_TRUE(ecs_manager.is_entity_in_group(entity, "Enemies"));

  // Destroy should clean up both tag and group
  ecs_manager.destroy_entity(entity);

  // Verify the entity is no longer tagged or in a group
  // (we can't directly check after destroy, but we can verify no crash occurs
  // and that creating a new entity with the same tag/group works)
  const auto new_entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  ecs_manager.tag_entity(new_entity, "Player");
  ecs_manager.add_entity_to_group(new_entity, "Enemies");

  EXPECT_TRUE(ecs_manager.is_entity_tagged(new_entity, "Player"));
  EXPECT_TRUE(ecs_manager.is_entity_in_group(new_entity, "Enemies"));
}

TEST(EcsTest, for_each_entity_in_group)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};

  // Create entities and add to groups
  const auto entity1 = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  const auto entity2 = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  const auto entity3 = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);

  ecs_manager.emplace_component<Transform>(entity1, 1U);
  ecs_manager.emplace_component<Transform>(entity2, 2U);
  ecs_manager.emplace_component<Transform>(entity3, 3U);

  ecs_manager.add_entity_to_group(entity1, "GroupA");
  ecs_manager.add_entity_to_group(entity2, "GroupA");
  ecs_manager.add_entity_to_group(entity3, "GroupB");

  // Count entities in GroupA
  std::size_t count_a = 0U;
  std::uint32_t sum_a = 0U;
  ecs_manager.for_each_entity_in_group("GroupA",
                                       [&](const Entity& entity)
                                       {
                                         ++count_a;
                                         sum_a += ecs_manager.get_component<Transform>(entity).data;
                                       });

  EXPECT_EQ(count_a, 2U);
  EXPECT_EQ(sum_a, 3U); // 1 + 2

  // Count entities in GroupB
  std::size_t count_b = 0U;
  ecs_manager.for_each_entity_in_group("GroupB",
                                       [&](const Entity& entity)
                                       {
                                         ++count_b;
                                         (void)entity;
                                       });

  EXPECT_EQ(count_b, 1U);

  // Non-existent group should iterate zero times
  std::size_t count_none = 0U;
  ecs_manager.for_each_entity_in_group("NonExistent",
                                       [&](const Entity& entity)
                                       {
                                         ++count_none;
                                         (void)entity;
                                       });

  EXPECT_EQ(count_none, 0U);
}

TEST(EcsTest, get_entity_by_tag)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};

  // Initially no entity with tag
  EXPECT_FALSE(ecs_manager.get_entity_by_tag("Player").has_value());

  // Create and tag an entity
  const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  ecs_manager.emplace_component<Transform>(entity, 42U);
  ecs_manager.tag_entity(entity, "Player");

  // Should find the entity
  auto found = ecs_manager.get_entity_by_tag("Player");
  EXPECT_TRUE(found.has_value());
  EXPECT_EQ(found->id, entity.id);
  EXPECT_EQ(ecs_manager.get_component<Transform>(*found).data, 42U);

  // Different tag should not find it
  EXPECT_FALSE(ecs_manager.get_entity_by_tag("Enemy").has_value());

  // After untagging, should not find it
  ecs_manager.untag_entity(entity);
  EXPECT_FALSE(ecs_manager.get_entity_by_tag("Player").has_value());
}

TEST(EcsTest, for_each_entity_with_tag)
{
  ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES, MAX_NUMBER_OF_SYSTEMS};

  const auto entity = ecs_manager.create_entity(DEFAULT_ENTITY_SIGNATURE);
  ecs_manager.emplace_component<Transform>(entity, 100U);
  ecs_manager.tag_entity(entity, "Player");

  // Should iterate exactly once for the tagged entity
  std::size_t count = 0U;
  std::uint32_t data = 0U;
  ecs_manager.for_each_entity_with_tag("Player",
                                       [&](const Entity& e)
                                       {
                                         ++count;
                                         data = ecs_manager.get_component<Transform>(e).data;
                                       });

  EXPECT_EQ(count, 1U);
  EXPECT_EQ(data, 100U);

  // Non-existent tag should iterate zero times
  std::size_t count_none = 0U;
  ecs_manager.for_each_entity_with_tag("NonExistent", [&](const Entity&) { ++count_none; });
  EXPECT_EQ(count_none, 0U);
}
