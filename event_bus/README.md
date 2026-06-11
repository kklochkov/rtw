# rtw/event_bus -- Type-Safe Event Bus

A single-header, C++17 publish-subscribe event bus with configurable threading policy. Supports lambdas, free functions, member functions, and functors as event handlers.

## Design Principles

- **Type-safe dispatch** -- Events are dispatched via RTTI (`std::type_index`). Only handlers registered for the exact published type are invoked.
- **RAII subscriptions** -- `Subscription` handles automatically unsubscribe on destruction. Safe to outlive the bus.
- **Configurable threading** -- `EventBus` (zero-cost, single-threaded) or `ThreadSafeEventBus` (reader-writer locking via `std::shared_mutex`).
- **Registration-order dispatch** -- Handlers for the same event type are invoked in the order they were registered.
- **Exception transparency** -- If a handler throws, the exception propagates to the caller of `publish()`. Subsequent handlers are not called.

## Quick Start

```cpp
#include "event_bus/event_bus.h"

// Define an event type (must inherit from Event).
struct PlayerDied : rtw::event_bus::Event
{
  int player_id{};
};

// Create an event bus.
rtw::event_bus::EventBus bus;

// Subscribe with a lambda (RAII handle auto-unsubscribes).
auto subscription = bus.subscribe<PlayerDied>([](const PlayerDied& e) {
    // handle event
});

// Or subscribe without a handle (lives until bus destruction or clear()).
bus.add_subscription<PlayerDied>([](const PlayerDied& e) {
    // handle event
});

// Publish an event.
bus.publish(PlayerDied{42});

// Manual unsubscribe (or let subscription go out of scope).
subscription.unsubscribe();
```

## Subscription Types

| Method | Returns | Lifetime |
|--------|---------|----------|
| `subscribe<EventT>(callable, args...)` | `Subscription` (RAII) | Until Subscription destroyed/unsubscribed |
| `add_subscription<EventT>(callable, args...)` | void | Until `unsubscribe<EventT>()`, `clear()`, or bus destruction |

## Callable Types

```cpp
// Free function
void on_event(const MyEvent& e);
bus.add_subscription<MyEvent>(on_event);

// Lambda (stateful or stateless)
bus.add_subscription<MyEvent>([&state](const MyEvent& e) { /* ... */ });

// Functor
struct Handler { void operator()(const MyEvent& e) const; };
bus.add_subscription<MyEvent>(Handler{});

// Member function (const or mutable)
struct Listener { void on_event(const MyEvent& e); };
Listener listener;
bus.add_subscription<MyEvent>(&Listener::on_event, &listener);

// Static member function
struct Static { static void on_event(const MyEvent& e); };
bus.add_subscription<MyEvent>(Static::on_event);
```

## Threading

```cpp
// Single-threaded (zero synchronization overhead)
rtw::event_bus::EventBus bus;

// Thread-safe (std::shared_mutex reader-writer locking)
rtw::event_bus::ThreadSafeEventBus bus;
```

Lock semantics:
- **Shared lock** (concurrent reads): `empty()`, `get_number_of_subscribers()`, `get_total_number_of_subscribers()`, `has_subscribers()`
- **Unique lock** (exclusive writes): `subscribe()`, `add_subscription()`, `unsubscribe()`, `clear()`, `publish()`

## Subscription Lifetime Safety

Subscriptions safely outlive the event bus:

```cpp
rtw::event_bus::EventBus::Subscription sub;
{
    rtw::event_bus::EventBus bus;
    sub = bus.subscribe<MyEvent>([](const MyEvent&) {});
} // bus destroyed here
// sub.unsubscribe() and ~Subscription() are safe no-ops
```

This is implemented via a `shared_ptr<atomic<EventBusState>>` token that the bus sets to `DESTROYED` in its destructor. Subscriptions check this token before attempting to unsubscribe.

## Constraints

- Event handlers MUST NOT call `subscribe()`, `unsubscribe()`, or `clear()` on the same bus during dispatch (causes deadlock in multi-threaded mode or iterator invalidation in single-threaded mode).
- The caller is responsible for ensuring that objects bound to member-function handlers remain alive for the duration of the subscription.
- RTTI is required (`std::type_index` used for event type dispatch and handler identity).
- Events must inherit from `rtw::event_bus::Event`.

## Usage (Bazel)

```python
cc_binary(
    name = "my_target",
    deps = ["//event_bus"],
)
```

## Build & Test

```bash
bazel build //event_bus
bazel test //event_bus/...
```

Test target: `//event_bus/tests:event_bus_tests`

Test coverage includes:
- All callable types (lambda, free function, functor, const/mutable/static member functions)
- RAII subscription lifecycle (auto-unsubscribe, manual unsubscribe, double-unsubscribe, release, move construct, move assign)
- Subscription outliving bus (both single-threaded and thread-safe)
- `clear()` (removes all handlers, double-clear safe)
- `has_subscribers()` / `empty()` queries
- Publish with no subscribers (silent no-op)
- Multiple handlers for same event type (registration-order dispatch verified)
- `ThreadSafeEventBus` basic operations (subscribe, publish, unsubscribe, const queries, clear, outlive-bus)
