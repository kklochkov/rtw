# rtw/ecs -- Entity Component System

A single-header, C++17 ECS framework built on custom containers from `rtw/stl`. Designed for real-time applications where deterministic memory layout, zero heap churn at runtime, and compile-time type safety are essential.

## Design Principles

- **Fixed-capacity, zero runtime allocation** -- All storage (entities, components, systems, groups) is pre-allocated at construction. No `new`/`delete` during operation.
- **Bitmask signatures** -- Entity-to-system matching uses power-of-2 enum bitmasks. An entity joins a system when its signature is a superset of the system's required signature.
- **Packed component storage** -- Components are stored contiguously in `PackedBuffer` with O(1) swap-and-pop removal. Cache-friendly iteration.
- **Generational entity IDs** -- Stale handles are detected via a generation counter that increments on each destroy, preventing dangling-reference bugs.
- **Assert-guarded invariants** -- 25 runtime assertions protect pool exhaustion, component lookup misses, and system registration violations.
- **Single-header** -- One `#include "ecs/ecs.h"` gives you everything.

## Architecture

```
ECSManager (facade)
  |-- ComponentManager (type-erased storage per component type)
  |     |-- ComponentStorage<T> (packed array + bidirectional entity<->index maps)
  |-- EntityManager (lifecycle, tags, groups)
  |     |-- StaticQueue<EntityId> (free list)
  |     |-- HeapArray<Entity> (slot array with generational IDs)
  |-- SystemManager (RTTI-based registry)
        |-- StaticFlatUnorderedMap<type_index, unique_ptr<ISystem>>
```

## Component Type Enum

Component types must be a scoped enum with power-of-2 values (one bit per type, max 64):

```cpp
enum class ComponentType : std::uint64_t
{
  NONE       = 0U,
  TRANSFORM  = 1U << 0U,
  RIGID_BODY = 1U << 1U,
  COLLIDER   = 1U << 2U,
  SPRITE     = 1U << 3U,
  HEALTH     = 1U << 6U,
  DAMAGE     = 1U << 7U,
};
```

## Defining Components

```cpp
struct Transform : rtw::ecs::Component<ComponentType, ComponentType::TRANSFORM>
{
  float x{}, y{}, z{};
};

struct Health : rtw::ecs::Component<ComponentType, ComponentType::HEALTH>
{
  int hp{100};
};
```

The `Component` CRTP base provides compile-time `TYPE` and `COMPONENT_ID` (derived via `log2` of the enum value).

## Defining Systems

```cpp
using System = rtw::ecs::System<ComponentType>;

constexpr rtw::ecs::SystemSignature<ComponentType> PHYSICS_SIGNATURE{
    ComponentType::TRANSFORM | ComponentType::RIGID_BODY};

struct PhysicsSystem : public System
{
  PhysicsSystem() noexcept : System{PHYSICS_SIGNATURE, MAX_ENTITIES} {}

  void update(float dt) { /* iterate get_entities() */ }
};
```

An entity is added to a system if `(entity.signature & system.signature) == system.signature`.

## Usage

```cpp
#include "ecs/ecs.h"

using ECSManager = rtw::ecs::ECSManager<ComponentType, 128, Transform, Health>;

ECSManager ecs{/*max_entities=*/1024, /*max_systems=*/16};

// Create a system
auto& physics = ecs.create_system<PhysicsSystem>();

// Create an entity
rtw::ecs::EntitySignature<ComponentType> sig{ComponentType::TRANSFORM | ComponentType::HEALTH};
auto entity = ecs.create_entity(sig);

// Attach components
ecs.emplace_component<Transform>(entity, 0.0F, 0.0F, 0.0F);
ecs.emplace_component<Health>(entity, 100);

// Query
bool alive = ecs.is_entity_valid(entity);
auto& hp = ecs.get_component<Health>(entity);

// Tags (1:1 naming)
ecs.tag_entity(entity, "player");
auto player = ecs.get_entity_by_tag("player");

// Groups (1:N categorization, single-membership)
ecs.add_entity_to_group(entity, "enemies");
ecs.for_each_entity_in_group("enemies", [](const auto& e) { /* ... */ });

// Destroy (increments generation, returns slot to free list)
ecs.destroy_entity(entity);
```

## Usage (Bazel)

```python
cc_binary(
    name = "my_target",
    deps = ["//ecs"],
)
```

## Design Decisions

| Decision | Rationale |
|----------|-----------|
| Single group membership | Simplifies ownership semantics; use tags for multi-label needs |
| `std::type_index` in SystemManager | Enables type-safe system lookup without manual ID assignment; requires RTTI |
| `MAX_NUMBER_OF_ENTITIES_PER_GROUP` as template param | Enables `InplaceStaticFlatUnorderedSet` (no heap); entity/system counts are runtime for flexibility |
| Power-of-2 enum constraint | Enables O(1) bitmask matching and `log2`-based component ID derivation |
| `ISystem` downcast in `add_entity` | Safe by construction (SystemManager only stores `System<EnumT>`); documented |
| `find()` + `assert` in `get()` | Prevents silent map insertion (unlike `operator[]`) even if asserts are compiled out |

## Limitations

- **RTTI required** -- `SystemManager` uses `std::type_index`. Not compatible with `-fno-rtti` builds.
- **Max 64 component types** -- Limited by underlying integer bitmask width.
- **No entity iteration by signature** -- Must iterate via systems or groups. No direct "give me all entities with component X" query outside of a system.
- **Single group membership** -- An entity can belong to at most one group at a time.

## Build & Test

```bash
# Build the library
bazel build //ecs

# Run all tests (includes death tests, signature matching, multi-system tests)
bazel test //ecs/...
```

Test target: `//ecs/tests:ecs_tests`

Test coverage includes:
- Entity lifecycle (create, destroy, generation reuse, pool exhaustion death test)
- Component storage (emplace, get, remove, swap-and-pop ordering, missing component death test)
- System registration (create, signature matching, duplicate creation death test)
- Partial signature matching (superset matches, subset does not)
- Multiple systems with different signatures (entity routed to correct systems)
- Tags (assign, lookup, remove, overwrite)
- Groups (add, remove, single-membership enforcement, iteration)
