#pragma once

#include "stl/flags.h"
#include "stl/heap_array.h"
#include "stl/packed_buffer.h"
#include "stl/queue.h"

#include <array>
#include <cstdint>
#include <type_traits>
#include <unordered_map>

namespace rtw::ecs
{

namespace details
{
constexpr std::uint8_t log2(const std::uint64_t n) noexcept { return n > 1U ? 1U + log2(n / 2U) : 0U; }
} // namespace details

using Id = std::uint32_t;

struct Entity
{
  Id id{};
};

template <typename EnumT, EnumT VALUE>
struct Component
{
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;

  constexpr static ComponentType TYPE = VALUE;

  constexpr static Id COMPONENT_ID =
      static_cast<Id>(details::log2(static_cast<std::underlying_type_t<ComponentType>>(VALUE)));
};

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

template <typename T>
class ComponentStorage final : public IComponentStorage
{
public:
  explicit ComponentStorage(const std::size_t max_number_of_entities) noexcept : components_{max_number_of_entities} {}

  template <typename... ArgsT>
  void emplace(const Entity entity, ArgsT&&... args) noexcept
  {
    const auto index = components_.size();
    components_.push_back(std::forward<ArgsT>(args)...);
    entity_id_to_index_[entity.id] = index;
    index_to_entity_id_[index] = entity.id;
  }

  bool empty() const noexcept { return components_.empty(); }
  std::size_t size() const noexcept { return components_.size(); }

  bool contains(const Entity entity) const noexcept
  {
    return entity_id_to_index_.find(entity.id) != entity_id_to_index_.end();
  }

  T& get(const Entity entity) noexcept
  {
    assert(contains(entity));
    const auto index = entity_id_to_index_[entity.id];
    return components_[index];
  }
  const T& get(const Entity entity) const noexcept { return get(entity); }
  T& operator[](const Entity entity) noexcept { return get(entity); }
  const T& operator[](const Entity entity) const noexcept { return get(entity); }

  void remove(const Entity entity) noexcept
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
  stl::PackedBuffer<T> components_;
  std::unordered_map<Id, std::size_t> entity_id_to_index_;
  std::unordered_map<std::size_t, Id> index_to_entity_id_;
};

template <typename EnumT, typename... ComponentsT>
class ComponentManager
{
public:
  using ComponentType = EnumT;

  constexpr static std::size_t NUMBER_OF_REGISTERED_COMPONENTS = sizeof...(ComponentsT);

  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");
  static_assert((std::is_same_v<ComponentType, typename ComponentsT::ComponentType> && ...),
                "All components must have the same enum type.");

  explicit ComponentManager(const std::size_t max_number_of_entities) noexcept
  {
    (allocate_storage<ComponentsT>(max_number_of_entities), ...);
  }

  template <typename ComponentT, typename... ArgsT>
  void emplace(const Entity entity, ArgsT&&... args) noexcept
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
  bool has(const Entity entity) const noexcept
  {
    return get_storage<ComponentT>().contains(entity);
  }

  template <typename ComponentT>
  ComponentT& get(const Entity entity) noexcept
  {
    return get_storage<ComponentT>()[entity];
  }

  template <typename ComponentT>
  const ComponentT& get(const Entity entity) const noexcept
  {
    return get_storage<ComponentT>()[entity];
  }

  template <typename ComponentT>
  void remove(const Entity entity) noexcept
  {
    get_storage<ComponentT>().remove(entity);
  }

  void remove_all(const Entity entity) noexcept { (remove<ComponentsT>(entity), ...); }

private:
  template <typename ComponentT>
  using ComponentStorage = ComponentStorage<ComponentT>;

  template <typename ComponentT>
  void allocate_storage(const std::size_t max_number_of_entities) noexcept
  {
    components_storage_[get_component_id<ComponentT>()] =
        std::make_unique<ComponentStorage<ComponentT>>(max_number_of_entities);
  }

  template <typename ComponentT>
  constexpr static Id get_component_id() noexcept
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

  using ComponentsStorage = std::array<std::unique_ptr<IComponentStorage>, NUMBER_OF_REGISTERED_COMPONENTS>;
  ComponentsStorage components_storage_{};
};

template <typename EnumT>
class EntityManger
{
public:
  static_assert(stl::details::IS_SCOPED_ENUM_V<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;

  explicit EntityManger(const std::size_t max_number_of_entities) noexcept
      : entity_signatures_{max_number_of_entities}, free_entities_{max_number_of_entities}
  {
    for (Id id = 0U; id < max_number_of_entities; ++id)
    {
      free_entities_.emplace(id);
    }
  }

  Entity create() noexcept
  {
    assert(!free_entities_.empty());
    Entity entity;
    free_entities_.pop(entity);
    entity_signatures_[entity.id] = stl::Flags<EnumT>();
    return entity;
  }

  void destroy(const Entity entity) noexcept
  {
    assert(entity.id < entity_signatures_.size());
    entity_signatures_[entity.id] = stl::Flags<EnumT>();
    free_entities_.push(entity);
  }

  stl::Flags<EnumT>& get_signature(const Entity entity) noexcept
  {
    assert(entity.id < entity_signatures_.size());
    return entity_signatures_[entity.id];
  }

  const stl::Flags<EnumT>& get_signature(const Entity entity) const noexcept
  {
    assert(entity.id < entity_signatures_.size());
    return entity_signatures_[entity.id];
  }

  void set_signature(const Entity entity, const ComponentType signature) noexcept
  {
    assert(entity.id < entity_signatures_.size());
    entity_signatures_[entity.id].set(signature);
  }

  bool test_signature(const Entity entity, const ComponentType signature) const noexcept
  {
    assert(entity.id < entity_signatures_.size());
    return entity_signatures_[entity.id].test(signature);
  }

  std::size_t size() const noexcept { return entity_signatures_.size() - free_entities_.size(); }

private:
  stl::HeapArray<stl::Flags<EnumT>> entity_signatures_;
  stl::Queue<Entity> free_entities_;
};

} // namespace rtw::ecs
