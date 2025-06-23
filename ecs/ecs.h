#pragma once

#include "sw_renderer/flags.h"
#include "sw_renderer/packed_buffer.h"
#include "sw_renderer/queue.h"

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
class ComponentStorage : public IComponentStorage
{
public:
  template <typename... ArgsT>
  constexpr std::size_t emplace(ArgsT&&... args) noexcept
  {
    const auto index = components_.size();
    components_.emplace_back(std::forward<ArgsT>(args)...);
    return index;
  }

  constexpr bool empty() const noexcept { return components_.empty(); }

  constexpr T& operator[](const std::size_t index) noexcept { return components_[index]; }
  constexpr const T& operator[](const std::size_t index) const noexcept { return components_[index]; }
  constexpr T& get(const std::size_t index) noexcept { return components_[index]; }
  constexpr const T& get(const std::size_t index) const noexcept { return components_[index]; }

private:
  rtw::stl::PackedBuffer<T> components_{MAX_NUMBER_OF_ENTITIES};
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
    const auto component_id = T::COMPONENT_ID;
    auto& component_storage = component_storages_.at(component_id);
    assert(component_storage == nullptr);
    component_storage = std::make_unique<ComponentStorage<T>>();
  }

  template <typename T>
  ComponentStorage<T>* get_component_storage() noexcept
  {
    const auto component_id = T::COMPONENT_ID;
    auto& component_storage = component_storages_.at(component_id);
    assert(component_storage != nullptr);
    return static_cast<ComponentStorage<T>*>(component_storage.get());
  }

  template <typename T, typename... ArgsT>
  void add_component(const Entity entity, ArgsT&&... args) noexcept
  {
    auto* storage = get_component_storage<T>();
    const auto index_in_storage = storage->emplace(std::forward<ArgsT>(args)...);
    entity_id_to_index_[entity.id] = index_in_storage;
    index_to_entity_id_[index_in_storage] = entity.id;
  }

  std::size_t get_number_of_components() const noexcept { return component_storages_.size(); }
  std::size_t get_number_of_entities() const noexcept { return entity_id_to_index_.size(); }

  template <typename T>
  bool has_component(const Entity entity) const noexcept
  {
    return entity_id_to_index_.find(entity.id) != entity_id_to_index_.end();
  }

  template <typename T>
  T& get_component(const Entity entity) noexcept
  {
    return get_component_helper<T>(entity);
  }

  template <typename T>
  const T& get_component(const Entity entity) const noexcept
  {
    return get_component_helper<T>(entity);
  }

  template <typename T>
  void remove_component(const Entity entity) noexcept
  {
    auto* storage = get_component_storage<T>();

    const auto index_to_remove = entity_id_to_index_[entity.id];
    auto entity_to_remove_it = entity_id_to_index_.find(entity.id);
    assert(entity_to_remove_it != entity_id_to_index_.end());

    assert(!storage->components.empty());
    const auto index_to_replace = storage->components.size() - 1U;
    auto entity_to_replace_it = index_to_entity_id_.find(index_to_replace);
    assert(entity_to_replace_it != index_to_entity_id_.end());

    storage->components->remove(index_to_remove);

    entity_id_to_index_[entity_to_replace_it->second] = index_to_remove;
    index_to_entity_id_[index_to_remove] = entity_to_replace_it->second;

    entity_id_to_index_.erase(entity_to_remove_it);
    index_to_entity_id_.erase(entity_to_replace_it);
  }

private:
  template <typename T>
  T& get_component_helper(const Entity entity) noexcept
  {
    auto* storage = get_component_storage<T>();

    assert(!storage->empty());
    assert(entity_id_to_index_.find(entity.id) != entity_id_to_index_.end());

    const auto index_in_storage = entity_id_to_index_[entity.id];
    return storage->get(index_in_storage);
  }

  std::array<std::unique_ptr<IComponentStorage>, MAX_NUMBER_OF_COMPONENTS> component_storages_{};

  std::unordered_map<Id, std::size_t> entity_id_to_index_;
  std::unordered_map<std::size_t, Id> index_to_entity_id_;
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
    entity_signatures_[entity.id] = rtw::sw_renderer::Flags<EnumT>();
    return entity;
  }

  void destroy_entity(const Entity entity) noexcept
  {
    assert(entity.id < MAX_NUMBER_OF_ENTITIES);
    entity_signatures_[entity.id] = rtw::sw_renderer::Flags<EnumT>();
    free_entities_.push(entity);
  }

  rtw::sw_renderer::Flags<EnumT>& get_entity_signature(const Entity entity) noexcept
  {
    return get_entity_signature_helper(entity);
  }

  const rtw::sw_renderer::Flags<EnumT>& get_entity_signature(const Entity entity) const noexcept
  {
    return get_entity_signature_helper(entity);
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
  rtw::sw_renderer::Flags<EnumT>& get_entity_signature_helper(const Entity entity) noexcept
  {
    assert(entity.id < MAX_NUMBER_OF_ENTITIES);
    return entity_signatures_[entity.id];
  }

  std::array<rtw::sw_renderer::Flags<EnumT>, MAX_NUMBER_OF_ENTITIES> entity_signatures_{};
  rtw::stl::Queue<Entity> free_entities_{MAX_NUMBER_OF_ENTITIES};
};

} // namespace rtw::ecs
