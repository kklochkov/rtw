#pragma once

#include "sw_renderer/flags.h"
#include "sw_renderer/packed_buffer.h"

#include <array>
#include <cstdint>
#include <type_traits>
#include <unordered_map>

namespace rtw::ecs
{

namespace details
{
constexpr std::uint8_t log2(const std::uint64_t n) { return n > 1U ? 1U + log2(n / 2U) : 0U; }
} // namespace details

struct Entity
{
  using Id = std::uint32_t;

  Id id;
};

template <typename EnumT, EnumT VALUE>
struct Component
{
  static_assert(std::is_enum_v<EnumT>, "EnumT must be an enum type.");

  constexpr static EnumT TYPE = VALUE;

  using Id = std::uint32_t;

  constexpr static Id COMPONENT_ID =
      static_cast<Id>((details::log2(static_cast<std::underlying_type_t<EnumT>>(VALUE))));
};

struct IComponentStorage
{
  IComponentStorage() = default;
  IComponentStorage(const IComponentStorage&) = delete;
  IComponentStorage(IComponentStorage&&) = default;
  IComponentStorage& operator=(const IComponentStorage&) = delete;
  IComponentStorage& operator=(IComponentStorage&&) = default;
  virtual ~IComponentStorage() = default;
};

template <typename T>
struct ComponentStorage : IComponentStorage
{
  constexpr static std::size_t MAX_ENTITIES = 1'000U;

  explicit ComponentStorage() : components{MAX_ENTITIES} {}

  rtw::stl::PackedBuffer<T> components;
};

template <typename EnumT>
struct ComponentManager
{
  static_assert(std::is_enum_v<EnumT>, "EnumT must be an enum type.");

  constexpr static std::size_t NUM_COMPONENTS = sizeof(EnumT) * 8U;
  std::array<std::unique_ptr<IComponentStorage>, NUM_COMPONENTS> component_storages{};

  std::unordered_map<Entity::Id, std::size_t> entity_id_to_index;
  std::unordered_map<std::size_t, Entity::Id> index_to_entity_id;

  template <typename T>
  void allocate_component_storage()
  {
    const auto component_id = T::COMPONENT_ID;
    auto& component_storage = component_storages.at(component_id);
    assert(component_storage == nullptr);
    component_storage = std::make_unique<ComponentStorage<T>>();
  }

  template <typename T>
  ComponentStorage<T>* get_component_storage()
  {
    const auto component_id = T::COMPONENT_ID;
    auto& component_storage = component_storages.at(component_id);
    assert(component_storage != nullptr);
    return static_cast<ComponentStorage<T>*>(component_storage.get());
  }

  template <typename T, typename... ArgsT>
  void add_component(const Entity entity, ArgsT&&... args)
  {
    auto* storage = get_component_storage<T>();
    const auto index_in_storage = storage->components.size();
    entity_id_to_index[entity.id] = index_in_storage;
    index_to_entity_id[index_in_storage] = entity.id;
    storage->components.emplace_back(std::forward<ArgsT>(args)...);
  }

  template <typename T>
  bool has_component(const Entity entity)
  {
    return entity_id_to_index.find(entity.id) != entity_id_to_index.end();
  }

  template <typename T>
  T& get_component(const Entity entity)
  {
    auto* storage = get_component_storage<T>();

    assert(!storage->components.empty());
    assert(entity_id_to_index.find(entity.id) != entity_id_to_index.end());

    const auto index_in_storage = entity_id_to_index[entity.id];
    return storage->components[index_in_storage];
  }

  template <typename T>
  void remove_component(const Entity entity)
  {
    auto* storage = get_component_storage<T>();

    const auto index_to_remove = entity_id_to_index[entity.id];
    auto entity_to_remove_it = entity_id_to_index.find(entity.id);
    assert(entity_to_remove_it != entity_id_to_index.end());

    assert(!storage->components.empty());
    const auto index_to_replace = storage->components.size() - 1U;
    auto entity_to_replace_it = index_to_entity_id.find(index_to_replace);
    assert(entity_to_replace_it != index_to_entity_id.end());

    storage->components->remove(index_to_remove);

    entity_id_to_index[entity_to_replace_it->second] = index_to_remove;
    index_to_entity_id[index_to_remove] = entity_to_replace_it->second;

    // TODO: think of fixed size hashmap
    entity_id_to_index.erase(entity_to_remove_it);
    index_to_entity_id.erase(entity_to_replace_it);
  }
};

template <typename EnumT>
struct EntityManger
{
  static_assert(std::is_enum_v<EnumT>, "EnumT must be an enum type.");

  using ComponentType = EnumT;

  std::vector<rtw::sw_renderer::Flags<EnumT>> entity_signatures;

  Entity create_entity()
  {
    const auto entity_id = static_cast<Entity::Id>(entity_signatures.size());
    entity_signatures.emplace_back();
    return {entity_id};
  }
};

} // namespace rtw::ecs
