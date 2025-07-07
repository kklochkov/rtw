#pragma once

#include "stl/flags.h"
#include "stl/heap_array.h"
#include "stl/packed_buffer.h"
#include "stl/queue.h"

#include <array>
#include <cstdint>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

namespace rtw::ecs
{

namespace details
{
constexpr std::uint8_t log2(const std::uint64_t n) noexcept { return n > 1U ? 1U + log2(n / 2U) : 0U; }
} // namespace details

struct Id
{
  using ID_TYPE = std::uint32_t;

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr operator ID_TYPE() const noexcept { return id; }
  constexpr bool operator==(const Id& other) const noexcept { return id == other.id; }
  constexpr bool operator!=(const Id& other) const noexcept { return !(*this == other); }

  ID_TYPE id{};
};

struct EntityId : Id
{};

template <typename EnumT>
struct EntitySignature : stl::Flags<EnumT>
{};

template <typename EnumT>
struct Entity
{
  using EntitySignature = EntitySignature<EnumT>;

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  Entity(const EntityId id = {}, EntitySignature signature = {}) : signature{std::move(signature)}, id{id} {}

  constexpr bool operator==(const Entity& other) const noexcept { return id == other.id; }

  EntitySignature signature{};
  EntityId id{};
};

struct ComponentId : Id
{};

template <typename EnumT, EnumT VALUE>
struct Component
{
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;

  constexpr static ComponentType TYPE = VALUE;
  constexpr static ComponentId COMPONENT_ID{details::log2(static_cast<std::underlying_type_t<ComponentType>>(VALUE))};

  constexpr bool operator==(const Component& other) const noexcept
  {
    return TYPE == other.TYPE && COMPONENT_ID == other.COMPONENT_ID;
  }
};

template <typename EnumT>
struct SystemSignature : stl::Flags<EnumT>
{};

} // namespace rtw::ecs

template <>
struct std::hash<rtw::ecs::EntityId>
{
  std::size_t operator()(const rtw::ecs::EntityId& id) const noexcept
  {
    return std::hash<rtw::ecs::EntityId::ID_TYPE>{}(id.id);
  }
};

template <typename EnumT>
struct std::hash<rtw::ecs::Entity<EnumT>>
{
  std::size_t operator()(const rtw::ecs::Entity<EnumT>& entity) const noexcept
  {
    return std::hash<rtw::ecs::EntityId>{}(entity.id);
  }
};

template <typename EnumT, EnumT VALUE>
struct std::hash<rtw::ecs::Component<EnumT, VALUE>>
{
  std::size_t operator()(const rtw::ecs::Component<EnumT, VALUE>& component) const noexcept
  {
    const auto hash_id = std::hash<rtw::ecs::Id>{}(component.COMPONENT_ID);
    const auto hash_type =
        std::hash<std::underlying_type_t<EnumT>>{}(static_cast<std::underlying_type_t<EnumT>>(component.TYPE));
    return hash_id ^ (hash_type << 1U);
  }
};

namespace rtw::ecs
{

class IComponentStorage
{
public:
  IComponentStorage() noexcept = default;
  IComponentStorage(const IComponentStorage&) noexcept = delete;
  IComponentStorage(IComponentStorage&&) noexcept = default;
  IComponentStorage& operator=(const IComponentStorage&) noexcept = delete;
  IComponentStorage& operator=(IComponentStorage&&) noexcept = default;
  virtual ~IComponentStorage() = default;
};

template <typename EnumT, typename ComponentT>
class ComponentStorage final : public IComponentStorage
{
public:
  using ComponentType = EnumT;
  using Entity = Entity<ComponentType>;

  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");
  static_assert(std::is_same_v<ComponentType, typename ComponentT::ComponentType>,
                "ComponentT must have the same enum type as ComponentType.");

  explicit ComponentStorage(const std::size_t max_number_of_entities) noexcept : components_{max_number_of_entities} {}

  template <typename... ArgsT>
  void emplace(const Entity& entity, ArgsT&&... args) noexcept
  {
    const auto index = components_.size();
    components_.push_back(std::forward<ArgsT>(args)...);
    entity_id_to_index_[entity.id] = index;
    index_to_entity_id_[index] = entity.id;
  }

  bool empty() const noexcept { return components_.empty(); }
  std::size_t size() const noexcept { return components_.size(); }

  bool contains(const Entity& entity) const noexcept
  {
    return entity_id_to_index_.find(entity.id) != entity_id_to_index_.end();
  }

  ComponentT& get(const Entity& entity) noexcept
  {
    assert(contains(entity));
    const auto index = entity_id_to_index_[entity.id];
    return components_[index];
  }

  const ComponentT& get(const Entity& entity) const noexcept
  {
    assert(contains(entity));
    const auto index = entity_id_to_index_[entity.id];
    return components_[index];
  }

  ComponentT& operator[](const Entity& entity) noexcept { return get(entity); }
  const ComponentT& operator[](const Entity& entity) const noexcept { return get(entity); }

  void remove(const Entity& entity) noexcept
  {
    if (components_.empty())
    {
      return;
    }

    auto entity_to_remove_it = entity_id_to_index_.find(entity.id);
    if (entity_to_remove_it == entity_id_to_index_.end())
    {
      return;
    }

    const auto index_to_remove = entity_to_remove_it->second;

    const auto index_to_replace = components_.size() - 1U;
    auto entity_to_replace_it = index_to_entity_id_.find(index_to_replace);
    assert(entity_to_replace_it != index_to_entity_id_.end());

    components_.remove(index_to_remove);

    entity_id_to_index_[entity_to_replace_it->second] = index_to_remove;
    index_to_entity_id_[index_to_remove] = entity_to_replace_it->second;

    entity_id_to_index_.erase(entity_to_remove_it);
    index_to_entity_id_.erase(entity_to_replace_it);
  }

private:
  stl::PackedBuffer<ComponentT> components_;
  std::unordered_map<EntityId, std::size_t> entity_id_to_index_;
  std::unordered_map<std::size_t, EntityId> index_to_entity_id_;
};

template <typename EnumT, typename... ComponentsT>
class ComponentManager
{
public:
  using ComponentType = EnumT;
  using Entity = Entity<ComponentType>;

  constexpr static std::size_t NUMBER_OF_REGISTERED_COMPONENTS = sizeof...(ComponentsT);

  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");
  static_assert((std::is_same_v<ComponentType, typename ComponentsT::ComponentType> && ...),
                "All components must have the same enum type.");

  explicit ComponentManager(const std::size_t max_number_of_entities) noexcept
  {
    (allocate_storage<ComponentsT>(max_number_of_entities), ...);
  }

  template <typename ComponentT, typename... ArgsT>
  void emplace(const Entity& entity, ArgsT&&... args) noexcept
  {
    get_storage<ComponentT>().emplace(entity, std::forward<ArgsT>(args)...);
  }

  template <typename ComponentT>
  std::size_t size() const noexcept
  {
    return get_storage<ComponentT>().size();
  }

  std::size_t total_size() const noexcept { return (size<ComponentsT>() + ...); }

  template <typename ComponentT>
  bool has(const Entity& entity) const noexcept
  {
    return get_storage<ComponentT>().contains(entity);
  }

  template <typename ComponentT>
  ComponentT& get(const Entity& entity) noexcept
  {
    return get_storage<ComponentT>()[entity];
  }

  template <typename ComponentT>
  const ComponentT& get(const Entity& entity) const noexcept
  {
    return get_storage<ComponentT>()[entity];
  }

  void remove(const Entity& entity) noexcept { (remove<ComponentsT>(entity), ...); }

private:
  template <typename ComponentT>
  using ComponentStorage = ComponentStorage<ComponentType, ComponentT>;

  template <typename ComponentT>
  void allocate_storage(const std::size_t max_number_of_entities) noexcept
  {
    components_storage_[get_component_id<ComponentT>()] =
        std::make_unique<ComponentStorage<ComponentT>>(max_number_of_entities);
  }

  template <typename ComponentT>
  constexpr static ComponentId get_component_id() noexcept
  {
    static_assert((std::is_same_v<ComponentT, ComponentsT> || ...),
                  "ComponentT must be one of the component types defined in ComponentsT.");
    static_assert(ComponentT::COMPONENT_ID < NUMBER_OF_REGISTERED_COMPONENTS,
                  "Component ID exceeds  number of registered components.");
    return ComponentT::COMPONENT_ID;
  }

  template <typename ComponentT>
  ComponentStorage<ComponentT>& get_storage() noexcept
  {
    static_assert((std::is_same_v<ComponentT, ComponentsT> || ...),
                  "ComponentT must be one of the component types defined in ComponentsT.");
    auto& component_storage = components_storage_[get_component_id<ComponentT>()];
    return static_cast<ComponentStorage<ComponentT>&>(*component_storage.get());
  }

  template <typename ComponentT>
  const ComponentStorage<ComponentT>& get_storage() const noexcept
  {
    static_assert((std::is_same_v<ComponentT, ComponentsT> || ...),
                  "ComponentT must be one of the component types defined in ComponentsT.");
    const auto& component_storage = components_storage_[get_component_id<ComponentT>()];
    return static_cast<const ComponentStorage<ComponentT>&>(*component_storage.get());
  }

  template <typename ComponentT>
  void remove(const Entity& entity) noexcept
  {
    get_storage<ComponentT>().remove(entity);
  }

  using ComponentsStorage = std::array<std::unique_ptr<IComponentStorage>, NUMBER_OF_REGISTERED_COMPONENTS>;
  ComponentsStorage components_storage_{};
};

template <typename EnumT>
class EntityManager
{
public:
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;
  using Entity = Entity<ComponentType>;
  using EntitySignature = typename Entity::EntitySignature;

  explicit EntityManager(const std::size_t max_number_of_entities) noexcept
      : entities_{max_number_of_entities}, free_ids_{max_number_of_entities}
  {
    for (EntityId::ID_TYPE id = 0U; id < max_number_of_entities; ++id)
    {
      free_ids_.emplace(id);
    }
  }

  Entity create(EntitySignature signature) noexcept
  {
    assert(!free_ids_.empty());

    auto& entity = entities_[free_ids_.front()];
    entity.id = free_ids_.front();
    entity.signature = std::move(signature);

    free_ids_.pop();

    return entity;
  }

  void destroy(const Entity& entity) noexcept
  {
    assert(entity.id < entities_.size());
    entities_[entity.id] = Entity{};
    free_ids_.push(entity.id);
  }

  std::size_t size() const noexcept { return entities_.size() - free_ids_.size(); }

  void tag(const Entity& entity, const std::string& tag) noexcept
  {
    tag_to_entity_id_[tag] = entity.id;
    entity_id_to_tag_[entity.id] = tag;
  }

  void untag(const Entity& entity) noexcept
  {
    const auto it = entity_id_to_tag_.find(entity.id);
    if (it != entity_id_to_tag_.end())
    {
      tag_to_entity_id_.erase(it->second);
      entity_id_to_tag_.erase(it);
    }
  }

  bool is_tagged(const Entity& entity, const std::string& tag) const noexcept
  {
    const auto it = tag_to_entity_id_.find(tag);
    return (it != tag_to_entity_id_.end()) && (it->second == entity.id);
  }

  void add_to_group(const Entity& entity, const std::string& group) noexcept
  {
    remove_from_group(entity);

    group_to_entity_ids_[group].insert(entity.id);
    entity_id_to_group_[entity.id] = group;
  }

  void remove_from_group(const Entity& entity) noexcept
  {
    const auto it = entity_id_to_group_.find(entity.id);
    if (it != entity_id_to_group_.end())
    {
      group_to_entity_ids_[it->second].erase(entity.id);
      entity_id_to_group_.erase(it);
    }
  }

  bool is_in_group(const Entity& entity, const std::string& group) const noexcept
  {
    const auto it = entity_id_to_group_.find(entity.id);
    return (it != entity_id_to_group_.end()) && (it->second == group);
  }

private:
  stl::HeapArray<Entity> entities_;
  stl::Queue<EntityId> free_ids_;
  std::unordered_map<std::string, EntityId> tag_to_entity_id_;
  std::unordered_map<EntityId, std::string> entity_id_to_tag_;
  std::unordered_map<std::string, std::unordered_set<EntityId>> group_to_entity_ids_;
  std::unordered_map<EntityId, std::string> entity_id_to_group_;
};

template <typename EnumT>
class System;

class ISystem
{
public:
  ISystem() noexcept = default;
  ISystem(const ISystem&) noexcept = delete;
  ISystem(ISystem&&) noexcept = default;
  ISystem& operator=(const ISystem&) noexcept = delete;
  ISystem& operator=(ISystem&&) noexcept = default;
  virtual ~ISystem() = default;

  // Method template with CRTP touch: casting to derived class System<EnumT> to access get_signature().
  // I haven't seen such usage before, just regular CRTP on classes, but not on methods. Have someone done this already?
  template <typename EnumT>
  void add_entity(const Entity<EnumT>& entity) noexcept
  {
    const auto system_signature = static_cast<const System<EnumT>*>(this)->get_signature();
    if ((entity.signature & system_signature) == system_signature)
    {
      entities_.insert(entity.id);
    }
  }

  template <typename EnumT>
  void remove_entity(const Entity<EnumT>& entity) noexcept
  {
    entities_.erase(entity.id);
  }

  std::size_t size() const noexcept { return entities_.size(); }

  const std::unordered_set<EntityId>& get_entities() const noexcept { return entities_; }

private:
  std::unordered_set<EntityId> entities_;
};

template <typename EnumT>
class System : public ISystem
{
public:
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;
  using SystemSignature = SystemSignature<ComponentType>;

  explicit System(SystemSignature signature) noexcept : signature_{std::move(signature)} {}

  const SystemSignature& get_signature() const noexcept { return signature_; }

private:
  SystemSignature signature_;
};

template <typename EnumT>
class SystemManager
{
public:
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;
  using Entity = Entity<ComponentType>;

  template <typename SystemT, typename... ArgsT>
  SystemT& create(ArgsT&&... args) noexcept
  {
    static_assert(std::is_base_of_v<System<EnumT>, SystemT>, "System must derive from ISystem.");

    const std::type_index type_index{typeid(SystemT)};

    assert(systems_.find(type_index) == systems_.end());

    auto system = std::make_unique<SystemT>(std::forward<ArgsT>(args)...);
    auto& system_ref = *system;
    systems_[type_index] = std::move(system);
    return system_ref;
  }

  template <typename SystemT>
  SystemT& get() noexcept
  {
    static_assert(std::is_base_of_v<ISystem, SystemT>, "SystemT must derive from ISystem.");

    const std::type_index type_index{typeid(SystemT)};

    auto it = systems_.find(type_index);
    assert(it != systems_.end());
    return static_cast<SystemT&>(*it->second);
  }

  template <typename SystemT>
  const SystemT& get() const noexcept
  {
    static_assert(std::is_base_of_v<ISystem, SystemT>, "SystemT must derive from ISystem.");

    const std::type_index type_index{typeid(SystemT)};

    auto it = systems_.find(type_index);
    assert(it != systems_.end());
    return static_cast<const SystemT&>(*it->second);
  }

  void add_entity(const Entity& entity) noexcept
  {
    for (auto& [_, system] : systems_)
    {
      system->add_entity(entity);
    }
  }

  void remove_entity(const Entity& entity) noexcept
  {
    for (auto& [_, system] : systems_)
    {
      system->remove_entity(entity);
    }
  }

  std::size_t size() const noexcept { return systems_.size(); }

private:
  std::unordered_map<std::type_index, std::unique_ptr<ISystem>> systems_;
};

template <typename EnumT, typename... ComponentsT>
class ECSManager
{
public:
  using ComponentType = EnumT;
  using SystemSignature = SystemSignature<ComponentType>;
  using Entity = Entity<ComponentType>;
  using EntitySignature = typename Entity::EntitySignature;

  explicit ECSManager(const std::size_t max_number_of_entities) noexcept
      : component_manager_{max_number_of_entities}, entity_manager_{max_number_of_entities}
  {
  }

  template <typename SystemT, typename... ArgsT>
  void create_system(ArgsT&&... args) noexcept
  {
    system_manager_.template create<SystemT>(std::forward<ArgsT>(args)...);
  }

  template <typename SystemT>
  SystemT& get_system() noexcept
  {
    return system_manager_.template get<SystemT>();
  }

  template <typename SystemT>
  const SystemT& get_system() const noexcept
  {
    return system_manager_.template get<SystemT>();
  }

  Entity create_entity(EntitySignature signature) noexcept
  {
    const auto entity = entity_manager_.create(std::move(signature));
    system_manager_.add_entity(entity);
    return entity;
  }

  void destroy_entity(const Entity& entity) noexcept
  {
    entity_manager_.destroy(entity);
    system_manager_.remove_entity(entity);
    component_manager_.remove(entity);
  }

  std::size_t get_number_of_entities() const noexcept { return entity_manager_.size(); }

  void tag_entity(const Entity& entity, const std::string& tag) noexcept { entity_manager_.tag(entity, tag); }

  void untag_entity(const Entity& entity) noexcept { entity_manager_.untag(entity); }

  bool is_entity_tagged(const Entity& entity, const std::string& tag) const noexcept
  {
    return entity_manager_.is_tagged(entity, tag);
  }

  void add_entity_to_group(const Entity& entity, const std::string& group) noexcept
  {
    entity_manager_.add_to_group(entity, group);
  }

  void remove_entity_from_group(const Entity& entity) noexcept { entity_manager_.remove_from_group(entity); }

  bool is_entity_in_group(const Entity& entity, const std::string& group) const noexcept
  {
    return entity_manager_.is_in_group(entity, group);
  }

  template <typename ComponentT, typename... ArgsT>
  void emplace_component(const Entity& entity, ArgsT&&... args) noexcept
  {
    component_manager_.template emplace<ComponentT>(entity, std::forward<ArgsT>(args)...);
  }

  template <typename ComponentT>
  ComponentT& get_component(const Entity& entity) noexcept
  {
    return component_manager_.template get<ComponentT>(entity);
  }

  template <typename ComponentT>
  const ComponentT& get_component(const Entity& entity) const noexcept
  {
    return component_manager_.template get<ComponentT>(entity);
  }

  template <typename ComponentT>
  std::size_t get_number_of_components() const noexcept
  {
    return component_manager_.template size<ComponentT>();
  }

  std::size_t get_total_number_of_components() const noexcept { return component_manager_.total_size(); }

private:
  ComponentManager<ComponentType, ComponentsT...> component_manager_;
  EntityManager<ComponentType> entity_manager_;
  SystemManager<ComponentType> system_manager_{};
};

} // namespace rtw::ecs
