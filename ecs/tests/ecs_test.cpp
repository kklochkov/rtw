#include "ecs/ecs.h"
#include "stl/format.h" // IWYU pragma: keep

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
using SystemManger = rtw::ecs::SystemManger<ComponentType>;
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
static_assert(std::is_same_v<EntityManger::ComponentType, ComponentType>,
              "EntityManger's ComponentType must match the enum type.");
static_assert(std::is_same_v<ComponentManager::ComponentType, EntityManger::ComponentType>,
              "ComponentManager's ComponentType must match EntityManger's ComponentType.");
static_assert(std::is_same_v<SystemManger ::ComponentType, EntityManger::ComponentType>,
              "SystemManger's ComponentType must match EntityManger's ComponentType.");

using ECSManager =
    rtw::ecs::ECSManager<ComponentType, Transform, Rigidbody, Collider, Sprite, Mesh, Debug, Health, Damage>;

struct DefaultSystem : public System
{
  explicit DefaultSystem() noexcept : System{DEFAULT_SYSTEM_SIGNATURE} {}
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
  EntityManger entity_manager{MAX_NUMBER_OF_ENTITIES};

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
   EntityManger entity_manager{MAX_NUMBER_OF_ENTITIES};

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
   EntityManger entity_manager{MAX_NUMBER_OF_ENTITIES};
   SystemManger system_manager{};

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
   ECSManager ecs_manager{MAX_NUMBER_OF_ENTITIES};
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

   EXPECT_EQ(ecs_manager.get_number_of_entites(), 10U);
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

   EXPECT_EQ(ecs_manager.get_number_of_entites(), 0U);
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

   EXPECT_EQ(ecs_manager.get_number_of_entites(), 10U);
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
 }
