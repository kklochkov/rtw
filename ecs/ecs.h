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
  static_assert(std::is_enum_v<EnumT>, "EnumT must be an enum type.");

  constexpr static EnumT TYPE = VALUE;

  constexpr static Id COMPONENT_ID =
      static_cast<Id>((details::log2(static_cast<std::underlying_type_t<EnumT>>(VALUE))));
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

template <typename T, Id MAX_NUMBER_OF_ENTITIES>
class ComponentStorage final : public IComponentStorage
{
public:
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
  stl::PackedBuffer<T> components_{MAX_NUMBER_OF_ENTITIES};
  std::unordered_map<Id, std::size_t> entity_id_to_index_;
  std::unordered_map<std::size_t, Id> index_to_entity_id_;
};

template <typename EnumT, Id MAX_NUMBER_OF_ENTITIES>
class ComponentManager
{
public:
  static_assert(std::is_enum_v<EnumT>, "EnumT must be an enum type.");

  constexpr static Id MAX_NUMBER_OF_COMPONENTS = sizeof(EnumT) * 8U;

  template <typename T>
  using ComponentStorage = ComponentStorage<T, MAX_NUMBER_OF_ENTITIES>;

  template <typename T>
  void allocate_component_storage() noexcept
  {
    auto& component_storage = components_storage_[get_component_id<T>()];
    if (!component_storage)
    {
      component_storage = std::make_unique<ComponentStorage<T>>();
    }
  }

  template <typename T>
  bool has_component_storage() const noexcept
  {
    return components_storage_[get_component_id<T>()] != nullptr;
  }

  template <typename T, typename... ArgsT>
  void add_component(const Entity entity, ArgsT&&... args) noexcept
  {
    auto& storage = get_component_storage<T>();
    storage.emplace(entity, std::forward<ArgsT>(args)...);
  }

  std::size_t get_number_of_components() const noexcept { return components_storage_.size(); }

  template <typename T>
  std::size_t get_number_of_entities() const noexcept
  {
    const auto& storage = get_component_storage<T>();
    return storage.size();
  }

  template <typename T>
  bool has_component(const Entity entity) const noexcept
  {
    const auto& storage = get_component_storage<T>();
    return storage.contains(entity);
  }

  template <typename T>
  T& get_component(const Entity entity) noexcept
  {
    auto& storage = get_component_storage<T>();
    return storage.get(entity);
  }

  template <typename T>
  const T& get_component(const Entity entity) const noexcept
  {
    return get_component<T>(entity);
  }

  template <typename T>
  void remove_component(const Entity entity) noexcept
  {
    auto& storage = get_component_storage<T>();
    storage.remove(entity);
  }

  template <typename... ComponentsT>
  void remove_components(const Entity entity) noexcept
  {
    (remove_component<ComponentsT>(entity), ...);
  }

private:
  template <typename T>
  constexpr static Id get_component_id() noexcept
  {
    static_assert(T::COMPONENT_ID < MAX_NUMBER_OF_COMPONENTS, "Component ID exceeds maximum number of components.");
    return T::COMPONENT_ID;
  }

  template <typename T>
  ComponentStorage<T>& get_component_storage() noexcept
  {
    auto& component_storage = components_storage_[get_component_id<T>()];
    assert(component_storage != nullptr);
    return static_cast<ComponentStorage<T>&>(*component_storage.get());
  }

  template <typename T>
  const ComponentStorage<T>& get_component_storage() const noexcept
  {
    const auto& component_storage = components_storage_[get_component_id<T>()];
    assert(component_storage != nullptr);
    return static_cast<const ComponentStorage<T>&>(*component_storage.get());
  }

  using ComponentsStorage = std::array<std::unique_ptr<IComponentStorage>, MAX_NUMBER_OF_COMPONENTS>;
  ComponentsStorage components_storage_{};
};

template <typename EnumT, Id MAX_NUMBER_OF_ENTITIES>
class EntityManger
{
public:
  static_assert(std::is_enum_v<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;

  EntityManger() noexcept
  {
    for (Id id = 0U; id < MAX_NUMBER_OF_ENTITIES; ++id)
    {
      free_entities_.emplace(id);
    }
  }

  Entity create_entity() noexcept
  {
    assert(!free_entities_.empty());
    Entity entity;
    free_entities_.pop(entity);
    entity_signatures_[entity.id] = stl::Flags<EnumT>();
    return entity;
  }

  void destroy_entity(const Entity entity) noexcept
  {
    assert(entity.id < MAX_NUMBER_OF_ENTITIES);
    entity_signatures_[entity.id] = stl::Flags<EnumT>();
    free_entities_.push(entity);
  }

  stl::Flags<EnumT>& get_entity_signature(const Entity entity) noexcept
  {
    assert(entity.id < MAX_NUMBER_OF_ENTITIES);
    return entity_signatures_[entity.id];
  }

  const stl::Flags<EnumT>& get_entity_signature(const Entity entity) const noexcept
  {
    assert(entity.id < MAX_NUMBER_OF_ENTITIES);
    return entity_signatures_[entity.id];
  }

  void set_entity_signature(const Entity entity, const ComponentType signature) noexcept
  {
    assert(entity.id < MAX_NUMBER_OF_ENTITIES);
    entity_signatures_[entity.id].set(signature);
  }

  bool test_entity_signature(const Entity entity, const ComponentType signature) const noexcept
  {
    assert(entity.id < MAX_NUMBER_OF_ENTITIES);
    return entity_signatures_[entity.id].test(signature);
  }

  std::size_t get_number_of_entities() const noexcept { return MAX_NUMBER_OF_ENTITIES - free_entities_.size(); }

private:
  stl::HeapArray<stl::Flags<EnumT>> entity_signatures_{MAX_NUMBER_OF_ENTITIES};
  stl::Queue<Entity> free_entities_{MAX_NUMBER_OF_ENTITIES};
};

} // namespace rtw::ecs
