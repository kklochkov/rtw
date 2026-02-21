#pragma once

#include "stl/flags.h"
#include "stl/heap_array.h"
#include "stl/id.h"
#include "stl/packed_buffer.h"
#include "stl/static_flat_unordered_map.h"
#include "stl/static_flat_unordered_set.h"
#include "stl/static_queue.h"
#include "stl/static_string.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <type_traits>
#include <typeindex>

namespace rtw::ecs
{

namespace details
{
constexpr std::uint8_t log2(const std::uint64_t n) noexcept { return n > 1U ? 1U + log2(n / 2U) : 0U; }
} // namespace details

/// Entity identifier consisting of an ID and a generation to prevent dangling references.
///
/// The ID is a unique identifier for the entity, while the generation is incremented each time an entity with the same
/// ID is destroyed and recreated.
struct EntityId
{
  using INDEX_TYPE = std::uint32_t;
  using GENERATION_TYPE = std::uint32_t;

  constexpr bool operator==(const EntityId& other) const noexcept
  {
    return (index == other.index) && (generation == other.generation);
  }

  constexpr bool operator!=(const EntityId& other) const noexcept { return !(*this == other); }

  INDEX_TYPE index{};
  GENERATION_TYPE generation{};
};

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

struct ComponentId : stl::Id
{};

template <typename EnumT, EnumT VALUE>
struct Component
{
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;

  constexpr static ComponentType TYPE = VALUE;
  constexpr static ComponentId COMPONENT_ID{details::log2(static_cast<std::underlying_type_t<ComponentType>>(VALUE))};

  constexpr ComponentType get_type() const noexcept { return TYPE; }
  constexpr ComponentId get_component_id() const noexcept { return COMPONENT_ID; }

  constexpr bool operator==(const Component& other) const noexcept
  {
    return TYPE == other.get_type() && COMPONENT_ID == other.get_component_id();
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
    const auto index_hash = std::hash<rtw::ecs::EntityId::INDEX_TYPE>{}(id.index);
    const auto generation_hash = std::hash<rtw::ecs::EntityId::GENERATION_TYPE>{}(id.generation);
    return index_hash ^ (generation_hash << 1U);
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
    const auto hash_id = std::hash<rtw::stl::Id>{}(component.COMPONENT_ID);
    const auto hash_type =
        std::hash<std::underlying_type_t<EnumT>>{}(static_cast<std::underlying_type_t<EnumT>>(component.TYPE));
    return hash_id ^ (hash_type << 1U);
  }
};

namespace rtw::ecs
{

/// Abstract base class for type-erased component storage.
/// Enables storing different component types in a homogeneous container.
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

/// Stores components of a specific type in a packed array for cache efficiency.
/// Maps EntityIds to component indices for O(1) lookup.
/// Uses swap-and-pop removal to maintain contiguous storage.
template <typename EnumT, typename ComponentT>
class ComponentStorage final : public IComponentStorage
{
public:
  using ComponentType = EnumT;
  using Entity = Entity<ComponentType>;

  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");
  static_assert(std::is_same_v<ComponentType, typename ComponentT::ComponentType>,
                "ComponentT must have the same enum type as ComponentType.");

  explicit ComponentStorage(const std::size_t max_number_of_entities) noexcept
      : components_{max_number_of_entities}, entity_id_to_index_{max_number_of_entities},
        index_to_entity_id_{max_number_of_entities}
  {
  }

  template <typename... ArgsT>
  void emplace(const Entity& entity, ArgsT&&... args) noexcept
  {
    if (contains(entity))
    {
      return;
    }

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
  stl::StaticFlatUnorderedMap<EntityId, std::size_t> entity_id_to_index_;
  stl::StaticFlatUnorderedMap<std::size_t, EntityId> index_to_entity_id_;
};

/// Manages all component storage for registered component types.
/// Provides type-safe access to components via template methods.
/// Enforces a maximum of 64 component types (bitmask-based signatures).
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
  static_assert(sizeof...(ComponentsT) <= 64U, "Number of registered components must be less than or equal to 64.");

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

  template <typename ComponentT>
  void remove(const Entity& entity) noexcept
  {
    get_storage<ComponentT>().remove(entity);
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
                  "Component ID exceeds the number of registered components.");
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

  using ComponentsStorage = std::array<std::unique_ptr<IComponentStorage>, NUMBER_OF_REGISTERED_COMPONENTS>;
  ComponentsStorage components_storage_{};
};

/// Manages entity lifecycle, tags, and groups.
/// Uses generational indices to detect stale entity references.
/// Tags provide unique 1:1 entity naming; groups provide 1:N categorization.
template <typename EnumT, std::size_t MAX_NUMBER_OF_ENTITIES_PER_GROUP>
class EntityManager
{
public:
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;
  using Entity = Entity<ComponentType>;
  using EntitySignature = typename Entity::EntitySignature;

  explicit EntityManager(const std::size_t max_number_of_entities) noexcept
      : entities_{max_number_of_entities}, free_ids_{max_number_of_entities}, tag_to_entity_id_{max_number_of_entities},
        entity_id_to_tag_{max_number_of_entities}, group_to_entity_ids_{max_number_of_entities},
        entity_id_to_group_{max_number_of_entities}
  {
    for (EntityId::INDEX_TYPE id = 0U; id < max_number_of_entities; ++id)
    {
      free_ids_.emplace(id, 0U);
    }
  }

  Entity create(EntitySignature signature) noexcept
  {
    assert(!free_ids_.empty());

    auto& entity = entities_[free_ids_.front().index];
    entity.id = free_ids_.front();
    entity.signature = std::move(signature);

    free_ids_.pop();

    return entity;
  }

  bool is_valid(const Entity& entity) const noexcept
  {
    const auto index = entity.id.index;
    return (index < entities_.size()) && (entities_[index].id.generation == entity.id.generation);
  }

  void destroy(const Entity& entity) noexcept
  {
    assert(entity.id.index < entities_.size());

    if (!is_valid(entity))
    {
      return;
    }

    untag(entity);
    remove_from_group(entity);

    {
      auto& e = entities_[entity.id.index];
      ++e.id.generation; // Increment generation to invalidate existing references
      e.signature = {};  // Clear signature to remove component associations

      free_ids_.push(e.id);
    }
  }

  std::size_t size() const noexcept { return entities_.size() - free_ids_.size(); }

  void tag(const Entity& entity, const stl::InplaceStringSmall& tag) noexcept
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

  bool is_tagged(const Entity& entity, const stl::InplaceStringSmall& tag) const noexcept
  {
    const auto it = tag_to_entity_id_.find(tag);
    return (it != tag_to_entity_id_.end()) && (it->second == entity.id);
  }

  std::optional<Entity> get_entity_by_tag(const stl::InplaceStringSmall& tag) const noexcept
  {
    const auto it = tag_to_entity_id_.find(tag);
    if (it == tag_to_entity_id_.end())
    {
      return std::nullopt;
    }
    return entities_[it->second.index];
  }

  template <typename FuncT>
  void for_each_entity_with_tag(const stl::InplaceStringSmall& tag, FuncT&& func) const noexcept
  {
    if (auto it = tag_to_entity_id_.find(tag); it != tag_to_entity_id_.end())
    {
      std::invoke(std::forward<FuncT>(func), entities_[it->second.index]);
    }
  }

  void add_to_group(const Entity& entity, const stl::InplaceStringSmall& group) noexcept
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

  bool is_in_group(const Entity& entity, const stl::InplaceStringSmall& group) const noexcept
  {
    const auto it = entity_id_to_group_.find(entity.id);
    return (it != entity_id_to_group_.end()) && (it->second == group);
  }

  template <typename FuncT>
  void for_each_entity_in_group(const stl::InplaceStringSmall& group, FuncT&& func) const noexcept
  {
    if (auto it = group_to_entity_ids_.find(group); it != group_to_entity_ids_.end())
    {
      for (const auto& entity_id : it->second)
      {
        std::invoke(std::forward<FuncT>(func), entities_[entity_id.index]);
      }
    }
  }

private:
  stl::HeapArray<Entity> entities_;
  stl::StaticQueue<EntityId> free_ids_;
  stl::StaticFlatUnorderedMap<stl::InplaceStringSmall, EntityId> tag_to_entity_id_;
  stl::StaticFlatUnorderedMap<EntityId, stl::InplaceStringSmall> entity_id_to_tag_;
  stl::StaticFlatUnorderedMap<stl::InplaceStringSmall,
                              stl::InplaceStaticFlatUnorderedSet<EntityId, MAX_NUMBER_OF_ENTITIES_PER_GROUP>>
      group_to_entity_ids_;
  stl::StaticFlatUnorderedMap<EntityId, stl::InplaceStringSmall> entity_id_to_group_;
};

template <typename EnumT>
class System;

/// Abstract base class for systems that process entities.
/// Tracks entities whose signatures match the system's requirements.
class ISystem
{
public:
  explicit ISystem(const std::size_t max_number_of_entities) noexcept : entities_{max_number_of_entities} {}
  ISystem(const ISystem&) noexcept = delete;
  ISystem(ISystem&&) noexcept = default;
  ISystem& operator=(const ISystem&) noexcept = delete;
  ISystem& operator=(ISystem&&) noexcept = default;
  virtual ~ISystem() = default;

  /// Adds the entity to the system if its signature matches the system's signature.
  ///
  /// @note The method uses static_cast to downcast to System<EnumT> to access the system's signature.
  /// This is type-safe because:
  /// - The SystemManager ensures that only systems of type System<EnumT> are created and stored.
  /// - EnumT is bounded by the Entity<EnumT> param and ensures the system's signature matches the entity's signature.
  /// - Mismatched types cause compile-time errors (missing get_signature() method or incorrect return type).
  ///
  /// Direct ISystem* usage with wrong EnumT is undefined behavior,
  /// but this is prevented by the design of SystemManager.
  template <typename EnumT>
  void add_entity(const Entity<EnumT>& entity) noexcept
  {
    const auto system_signature = static_cast<const System<EnumT>*>(this)->get_signature();
    using SystemSignatureType = std::remove_cv_t<std::remove_reference_t<decltype(system_signature)>>;
    static_assert(std::is_same_v<SystemSignatureType, SystemSignature<EnumT>>,
                  "System<EnumT>::get_signature() must return SystemSignature<EnumT>");
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

  const stl::StaticFlatUnorderedSet<EntityId>& get_entities() const noexcept { return entities_; }

private:
  stl::StaticFlatUnorderedSet<EntityId> entities_;
};

/// Base class for user-defined systems.
/// Derive from this class and implement update logic to process entities.
template <typename EnumT>
class System : public ISystem
{
public:
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;
  using SystemSignature = SystemSignature<ComponentType>;

  System(SystemSignature signature, const std::size_t max_number_of_entities) noexcept
      : ISystem{max_number_of_entities}, signature_{std::move(signature)}
  {
  }

  const SystemSignature& get_signature() const noexcept { return signature_; }

private:
  SystemSignature signature_;
};

/// Manages system registration and entity distribution.
/// Automatically notifies all systems when entities are added or removed.
template <typename EnumT>
class SystemManager
{
public:
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;
  using Entity = Entity<ComponentType>;

  explicit SystemManager(const std::size_t max_number_of_systems) noexcept : systems_{max_number_of_systems} {}

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
    for (auto it = systems_.begin(); it != systems_.end(); ++it)
    {
      auto& system = it->second;
      system->add_entity(entity);
    }
  }

  void remove_entity(const Entity& entity) noexcept
  {
    for (auto it = systems_.begin(); it != systems_.end(); ++it)
    {
      auto& system = it->second;
      system->remove_entity(entity);
    }
  }

  std::size_t size() const noexcept { return systems_.size(); }

private:
  stl::StaticFlatUnorderedMap<std::type_index, std::unique_ptr<ISystem>> systems_;
};

/// High-level facade coordinating entities, components, and systems.
///
/// Provides a unified API for all ECS operations, including entity lifecycle management,
/// component attachment/removal, system registration, and entity tagging/grouping. This is
/// the primary interface users should interact with rather than using the underlying managers
/// directly.
///
/// @tparam EnumT                        Power-of-2 enum for component type bitmask (max 64 types).
/// @tparam MAX_NUMBER_OF_ENTITIES_PER_GROUP  Maximum entities allowed per group.
/// @tparam ComponentsT                  List of component types to register.
template <typename EnumT, std::size_t MAX_NUMBER_OF_ENTITIES_PER_GROUP, typename... ComponentsT>
class ECSManager
{
public:
  using ComponentType = EnumT;
  using SystemSignature = SystemSignature<ComponentType>;
  using Entity = Entity<ComponentType>;
  using EntitySignature = typename Entity::EntitySignature;

  ECSManager(const std::size_t max_number_of_entities, const std::size_t max_number_of_systems) noexcept
      : component_manager_{max_number_of_entities}, entity_manager_{max_number_of_entities},
        system_manager_{max_number_of_systems}
  {
  }

  template <typename SystemT, typename... ArgsT>
  SystemT& create_system(ArgsT&&... args) noexcept
  {
    return system_manager_.template create<SystemT>(std::forward<ArgsT>(args)...);
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

  bool is_entity_valid(const Entity& entity) const noexcept { return entity_manager_.is_valid(entity); }

  void destroy_entity(const Entity& entity) noexcept
  {
    entity_manager_.destroy(entity);
    system_manager_.remove_entity(entity);
    component_manager_.remove(entity);
  }

  std::size_t get_number_of_entities() const noexcept { return entity_manager_.size(); }

  void tag_entity(const Entity& entity, const stl::InplaceStringSmall& tag) noexcept
  {
    entity_manager_.tag(entity, tag);
  }

  void untag_entity(const Entity& entity) noexcept { entity_manager_.untag(entity); }

  bool is_entity_tagged(const Entity& entity, const stl::InplaceStringSmall& tag) const noexcept
  {
    return entity_manager_.is_tagged(entity, tag);
  }

  std::optional<Entity> get_entity_by_tag(const stl::InplaceStringSmall& tag) const noexcept
  {
    return entity_manager_.get_entity_by_tag(tag);
  }

  template <typename FuncT>
  void for_each_entity_with_tag(const stl::InplaceStringSmall& tag, FuncT&& func) const noexcept
  {
    entity_manager_.for_each_entity_with_tag(tag, std::forward<FuncT>(func));
  }

  void add_entity_to_group(const Entity& entity, const stl::InplaceStringSmall& group) noexcept
  {
    entity_manager_.add_to_group(entity, group);
  }

  void remove_entity_from_group(const Entity& entity) noexcept { entity_manager_.remove_from_group(entity); }

  bool is_entity_in_group(const Entity& entity, const stl::InplaceStringSmall& group) const noexcept
  {
    return entity_manager_.is_in_group(entity, group);
  }

  template <typename FuncT>
  void for_each_entity_in_group(const stl::InplaceStringSmall& group, FuncT&& func) const noexcept
  {
    entity_manager_.for_each_entity_in_group(group, std::forward<FuncT>(func));
  }

  template <typename ComponentT, typename... ArgsT>
  void emplace_component(const Entity& entity, ArgsT&&... args) noexcept
  {
    component_manager_.template emplace<ComponentT>(entity, std::forward<ArgsT>(args)...);
  }

  template <typename ComponentT>
  bool has_component(const Entity& entity) const noexcept
  {
    return component_manager_.template has<ComponentT>(entity);
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
  void remove_component(const Entity& entity) noexcept
  {
    component_manager_.template remove<ComponentT>(entity);
  }

  template <typename ComponentT>
  std::size_t get_number_of_components() const noexcept
  {
    return component_manager_.template size<ComponentT>();
  }

  std::size_t get_total_number_of_components() const noexcept { return component_manager_.total_size(); }

private:
  ComponentManager<ComponentType, ComponentsT...> component_manager_;
  EntityManager<ComponentType, MAX_NUMBER_OF_ENTITIES_PER_GROUP> entity_manager_;
  SystemManager<ComponentType> system_manager_;
};

} // namespace rtw::ecs
