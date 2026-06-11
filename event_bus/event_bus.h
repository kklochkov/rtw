#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rtw::event_bus
{

/// Polymorphic base class for all event types.
///
/// User-defined events must inherit from this class to be publishable on the event bus.
/// The virtual destructor enables safe polymorphic deletion and RTTI-based type dispatch.
struct Event
{
  Event() = default;
  Event(const Event&) = default;
  Event(Event&&) = default;
  Event& operator=(const Event&) = default;
  Event& operator=(Event&&) = default;
  virtual ~Event() = default;
};

/// Threading policy selector for the event bus.
enum class ThreadingPolicy : std::uint8_t
{
  SINGLE_THREADED, ///< No synchronization is performed. Single-threaded access only.
  MULTI_THREADED   ///< Reader-writer (shared_mutex) synchronization. Multi-threaded access is safe.
};

namespace details
{

/// Synchronization policy abstraction. Specializations provide zero-cost no-ops (single-threaded)
/// or real shared_mutex locking (multi-threaded).
/// @tparam POLICY The threading policy.
template <ThreadingPolicy POLICY>
class SynchronizationPolicy;

/// Single-threaded specialization: all lock operations are no-ops.
/// Uses a stub SharedMutex satisfying the SharedMutex named requirement with empty methods.
template <>
class SynchronizationPolicy<ThreadingPolicy::SINGLE_THREADED>
{
public:
  struct SharedMutex
  {
    void lock_shared() const noexcept {}
    void unlock_shared() const noexcept {}
    void lock() const noexcept {}
    void unlock() const noexcept {}
  };

  using SharedLockGuard = std::shared_lock<SharedMutex>;
  using UniqueLockGuard = std::unique_lock<SharedMutex>;

  SharedLockGuard make_shared_lock_guard() const noexcept { return SharedLockGuard{mutex_}; }
  UniqueLockGuard make_unique_lock_guard() const noexcept { return UniqueLockGuard{mutex_}; }

private:
  mutable SharedMutex mutex_;
};

/// Multi-threaded specialization: uses std::shared_mutex for reader-writer locking.
/// Read-only queries (empty, get_number_of_subscribers) acquire shared locks;
/// mutations (subscribe, unsubscribe, publish, clear) acquire unique locks.
template <>
class SynchronizationPolicy<ThreadingPolicy::MULTI_THREADED>
{
public:
  using SharedLockGuard = std::shared_lock<std::shared_mutex>;
  using UniqueLockGuard = std::unique_lock<std::shared_mutex>;

  SharedLockGuard make_shared_lock_guard() const noexcept { return SharedLockGuard{mutex_}; }
  UniqueLockGuard make_unique_lock_guard() const noexcept { return UniqueLockGuard{mutex_}; }

private:
  mutable std::shared_mutex mutex_;
};

enum class EventBusState : std::uint8_t
{
  ALIVE,    ///< The event bus is alive and can be subscribed to and published.
  DESTROYED ///< The event bus has been destroyed.
};

using EventBusStateToken = std::shared_ptr<std::atomic<EventBusState>>;

} // namespace details

/// An event bus implementation that supports subscribing to and publishing events of arbitrary types.
///
/// The threading policy can be configured via the template parameter, allowing for both single-threaded and
/// multi-threaded use cases.
///
/// The event bus allows for subscribing to events using various callable types, including free functions, member
/// functions, lambdas, and functors.
///
/// @note In case of member functions it is a caller's responsibility to ensure that the
/// instance on which the member function is called remains valid for the duration of the subscription. The event bus
/// does not manage the lifetime of the subscriber instances, and it is the caller's responsibility to ensure that they
/// are not destroyed while still subscribed to events.
///
/// @note The event bus does not handle exceptions thrown by event handlers. If an event handler throws an exception,
/// other event handlers won't be called, and the exception will propagate to the caller of `publish()`. It is the
/// responsibility of the event handlers to catch and handle any exceptions they may throw.
///
/// @note Event handlers MUST NOT call subscribe(), unsubscribe(), or clear() on the same event bus during dispatch.
/// Doing so will cause undefined behavior (deadlock in multi-threaded mode,
/// or iterator invalidation in single-threaded mode).
///
/// @tparam POLICY The threading policy to use for synchronization. @see ThreadingPolicy.
template <ThreadingPolicy POLICY>
class GenericEventBus
{
  struct IEventHandler
  {
    IEventHandler() = default;
    IEventHandler(const IEventHandler&) = default;
    IEventHandler(IEventHandler&&) = default;
    IEventHandler& operator=(const IEventHandler&) = default;
    IEventHandler& operator=(IEventHandler&&) = default;
    virtual ~IEventHandler() = default;

    virtual void dispatch(const Event& event) = 0;
  };

  template <typename EventT, typename CallableT, typename... ArgsT>
  struct EventHandler : public IEventHandler
  {
    template <typename OtherCallableT, typename... OtherArgsT>
    explicit EventHandler(OtherCallableT&& callable, OtherArgsT&&... args)
        : callable{std::forward<OtherCallableT>(callable)}, args{std::forward<OtherArgsT>(args)...}
    {
    }

    void dispatch(const Event& event) final
    {
      std::apply(callable, std::tuple_cat(args, std::make_tuple(static_cast<const EventT&>(event))));
    }

    CallableT callable;
    std::tuple<ArgsT...> args;
  };

  template <typename EventT>
  static std::type_index get_event_type_index() noexcept
  {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
    return typeid(EventT);
  }

public:
  /// RAII subscription handle. Automatically unsubscribes when destroyed.
  ///
  /// Move-only. Holding a Subscription keeps the handler alive; dropping it removes the handler.
  /// Safe to outlive the EventBus -- destruction/unsubscribe becomes a no-op if the bus is already gone
  /// (detected via a shared atomic state token).
  ///
  /// Use `release()` to detach without unsubscribing (the handler remains registered permanently
  /// or until the bus is destroyed).
  class Subscription
  {
    friend class GenericEventBus;

    Subscription(GenericEventBus* event_bus, const std::type_index& handler_type_index, IEventHandler* event_handler,
                 details::EventBusStateToken event_bus_state_token) noexcept
        : event_bus_{event_bus}, handler_type_index_{handler_type_index}, event_handler_{event_handler},
          event_bus_state_token_{std::move(event_bus_state_token)}
    {
    }

    static inline const std::type_index VOID_TYPE_INDEX{typeid(void)};

  public:
    Subscription() noexcept = default;

    Subscription(const Subscription&) noexcept = delete;
    Subscription& operator=(const Subscription&) noexcept = delete;
    Subscription(Subscription&& other) noexcept
        : event_bus_{std::exchange(other.event_bus_, nullptr)},
          handler_type_index_{std::exchange(other.handler_type_index_, VOID_TYPE_INDEX)},
          event_handler_{std::exchange(other.event_handler_, nullptr)},
          event_bus_state_token_{std::exchange(other.event_bus_state_token_, nullptr)}
    {
    }
    Subscription& operator=(Subscription&& other) noexcept
    {
      if (this != &other)
      {
        unsubscribe();

        event_bus_ = other.event_bus_;
        handler_type_index_ = other.handler_type_index_;
        event_handler_ = other.event_handler_;
        event_bus_state_token_ = std::move(other.event_bus_state_token_);

        other.release();
      }
      return *this;
    }
    ~Subscription() { unsubscribe(); }

    void unsubscribe() noexcept
    {
      if (event_bus_state_token_ && (event_bus_state_token_->load() == details::EventBusState::ALIVE))
      {
        event_bus_->unsubscribe(*this);
      }

      release();
    }

    void release() noexcept
    {
      event_bus_ = nullptr;
      handler_type_index_ = VOID_TYPE_INDEX;
      event_handler_ = nullptr;
      event_bus_state_token_ = nullptr;
    }

    bool operator==(const Subscription& other) const noexcept
    {
      return (handler_type_index_ == other.handler_type_index_) && (event_handler_ == other.event_handler_);
    }

    explicit operator bool() const noexcept { return event_bus_ != nullptr; }

  private:
    GenericEventBus* get_event_bus() const noexcept { return event_bus_; }

    GenericEventBus* event_bus_{nullptr};
    std::type_index handler_type_index_{VOID_TYPE_INDEX};
    IEventHandler* event_handler_{nullptr};
    details::EventBusStateToken event_bus_state_token_{nullptr};
  };

  GenericEventBus()
      : state_token_{std::make_shared<details::EventBusStateToken::element_type>(details::EventBusState::ALIVE)}
  {
  }

  GenericEventBus(const GenericEventBus&) = delete;
  GenericEventBus& operator=(const GenericEventBus&) = delete;
  GenericEventBus(GenericEventBus&& other) noexcept = delete;
  GenericEventBus& operator=(GenericEventBus&& other) noexcept = delete;
  ~GenericEventBus() { *state_token_ = details::EventBusState::DESTROYED; }

  /// Subscribes a callable to events of type EventT without returning a Subscription handle.
  /// The handler remains registered until the bus is destroyed or `unsubscribe<EventT>()` / `clear()` is called.
  /// @tparam EventT The event type to subscribe to (must derive from Event).
  /// @tparam CallableT The callable type (lambda, functor, function pointer, or member function pointer).
  /// @tparam ArgsT Additional bound arguments (e.g., object pointer for member functions).
  /// @param[in] callable The handler to invoke when EventT is published.
  /// @param[in] args Additional arguments bound to the callable (forwarded at dispatch time).
  template <typename EventT, typename CallableT, typename... ArgsT>
  void add_subscription(CallableT&& callable, ArgsT&&... args)
  {
    static_assert(std::is_invocable_v<CallableT, ArgsT..., const EventT&>,
                  "Callable must be invocable with (ArgsT..., const EventT&)");
    const auto lock_guard = synchronization_policy_.make_unique_lock_guard();
    subscribe_unsafe<EventT>(std::forward<CallableT>(callable), std::forward<ArgsT>(args)...);
  }

  /// Subscribes a callable to events of type EventT and returns an RAII Subscription handle.
  /// The handler is automatically unsubscribed when the Subscription is destroyed or `unsubscribe()` is called on it.
  /// @tparam EventT The event type to subscribe to (must derive from Event).
  /// @tparam CallableT The callable type.
  /// @tparam ArgsT Additional bound arguments.
  /// @param[in] callable The handler to invoke when EventT is published.
  /// @param[in] args Additional arguments bound to the callable.
  /// @return A move-only Subscription handle that unsubscribes on destruction.
  template <typename EventT, typename CallableT, typename... ArgsT>
  [[nodiscard]] Subscription subscribe(CallableT&& callable, ArgsT&&... args)
  {
    static_assert(std::is_invocable_v<CallableT, ArgsT..., const EventT&>,
                  "Callable must be invocable with (ArgsT..., const EventT&)");
    const auto lock_guard = synchronization_policy_.make_unique_lock_guard();
    auto& context = subscribe_unsafe<EventT>(std::forward<CallableT>(callable), std::forward<ArgsT>(args)...);
    return Subscription{this, context.handler_type_index, context.event_handler.get(), state_token_};
  }

  /// Removes a specific subscription by handle. No-op if the subscription belongs to a different bus.
  /// @param[in] subscription The subscription handle to remove.
  void unsubscribe(const Subscription& subscription) noexcept
  {
    if (subscription.get_event_bus() != this)
    {
      return;
    }

    const auto lock_guard = synchronization_policy_.make_unique_lock_guard();

    for (auto& [_, subscriptions] : event_handlers_)
    {
      for (auto sub_it = subscriptions.begin(); sub_it != subscriptions.end();)
      {
        if ((sub_it->handler_type_index == subscription.handler_type_index_)
            && (sub_it->event_handler.get() == subscription.event_handler_))
        {
          --total_subscribers_;
          sub_it = subscriptions.erase(sub_it);
        }
        else
        {
          ++sub_it;
        }
      }
    }
  }

  /// Removes all subscriptions for a specific event type.
  /// @tparam EventT The event type whose handlers should be removed.
  template <typename EventT>
  void unsubscribe() noexcept
  {
    const auto lock_guard = synchronization_policy_.make_unique_lock_guard();

    const auto type_index = get_event_type_index<EventT>();
    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      total_subscribers_ -= it->second.size();
      event_handlers_.erase(it);
    }
  }

  /// Removes all subscriptions for all event types.
  void clear() noexcept
  {
    const auto lock_guard = synchronization_policy_.make_unique_lock_guard();
    event_handlers_.clear();
    total_subscribers_ = 0U;
  }

  /// @return True if there are no subscribers for any event type.
  bool empty() const noexcept
  {
    const auto lock_guard = synchronization_policy_.make_shared_lock_guard();
    return total_subscribers_ == 0U;
  }

  template <typename EventT>
  /// Publishes an event to all registered handlers for that event type.
  /// Handlers are invoked synchronously in registration order.
  /// If a handler throws, the exception propagates immediately and subsequent handlers are not called.
  /// @tparam EventT The event type to publish (must derive from Event).
  /// @param[in] event The event instance to dispatch to handlers.
  void publish(const EventT& event)
  {
    const auto lock_guard = synchronization_policy_.make_unique_lock_guard();

    const auto type_index = get_event_type_index<EventT>();
    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      for (const auto& subscription : it->second)
      {
        subscription.event_handler->dispatch(event);
      }
    }
  }

  /// @tparam EventT The event type to query.
  /// @return The number of handlers registered for the given event type.
  template <typename EventT>
  std::size_t get_number_of_subscribers() const noexcept
  {
    const auto lock_guard = synchronization_policy_.make_shared_lock_guard();

    const auto type_index = get_event_type_index<EventT>();
    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      return it->second.size();
    }
    return 0U;
  }

  /// @return The total number of handlers registered across all event types.
  std::size_t get_total_number_of_subscribers() const noexcept
  {
    const auto lock_guard = synchronization_policy_.make_shared_lock_guard();
    return total_subscribers_;
  }

  /// @tparam EventT The event type to query.
  /// @return True if at least one handler is registered for the given event type.
  template <typename EventT>
  bool has_subscribers() const noexcept
  {
    return get_number_of_subscribers<EventT>() > 0U;
  }

private:
  struct SubscriptionContext
  {
    SubscriptionContext(const std::type_index& handler_type_index,
                        std::unique_ptr<IEventHandler> event_handler) noexcept
        : handler_type_index{handler_type_index}, event_handler{std::move(event_handler)}
    {
    }

    std::type_index handler_type_index;
    std::unique_ptr<IEventHandler> event_handler;
  };

  template <typename EventT, typename CallableT, typename... ArgsT>
  SubscriptionContext& subscribe_unsafe(CallableT&& callable, ArgsT&&... args)
  {
    using HandlerWrapper = EventHandler<EventT, CallableT, ArgsT...>;
    const std::type_index handler_type_index{typeid(HandlerWrapper)};

    auto event_handler =
        std::make_unique<HandlerWrapper>(std::forward<CallableT>(callable), std::forward<ArgsT>(args)...);

    const auto type_index = get_event_type_index<EventT>();
    auto& context = event_handlers_[type_index].emplace_back(handler_type_index, std::move(event_handler));

    ++total_subscribers_;

    return context;
  }

  using EventHandlers = std::unordered_map<std::type_index, std::vector<SubscriptionContext>>;
  EventHandlers event_handlers_;

  details::EventBusStateToken state_token_;
  details::SynchronizationPolicy<POLICY> synchronization_policy_;

  std::size_t total_subscribers_{0U};
};

/// Single-threaded event bus (no synchronization overhead).
using EventBus = GenericEventBus<ThreadingPolicy::SINGLE_THREADED>;
/// Thread-safe event bus with reader-writer locking via std::shared_mutex.
using ThreadSafeEventBus = GenericEventBus<ThreadingPolicy::MULTI_THREADED>;

} // namespace rtw::event_bus
