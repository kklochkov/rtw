#pragma once

#include <memory>
#include <mutex>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace rtw::event_bus
{

struct Event
{
  Event() = default;
  Event(const Event&) = default;
  Event(Event&&) = default;
  Event& operator=(const Event&) = default;
  Event& operator=(Event&&) = default;
  virtual ~Event() = default;
};

enum class ThreadingPolicy : std::uint8_t
{
  SINGLE_THREADED, ///< No synchronization is performed. Single-threaded access only.
  MULTI_THREADED   ///< Mutex-based synchronization. Multi-threaded access is safe.
};

namespace details
{

template <ThreadingPolicy POLICY>
class SynchronizationPolicy;

template <>
class SynchronizationPolicy<ThreadingPolicy::SINGLE_THREADED>
{
public:
  struct LockGuard
  {};

  static LockGuard make_lock_guard() noexcept { return LockGuard{}; }
};

template <>
class SynchronizationPolicy<ThreadingPolicy::MULTI_THREADED>
{
public:
  using LockGuard = std::lock_guard<std::mutex>;

  LockGuard make_lock_guard() noexcept { return LockGuard{mutex_}; }

private:
  std::mutex mutex_;
};

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
  class Subscription
  {
    friend class GenericEventBus;

    Subscription(GenericEventBus* event_bus, const std::type_index& handler_type_index) noexcept
        : event_bus_{event_bus}, handler_type_index_{handler_type_index}
    {
    }

    static inline const std::type_index VOID_TYPE_INDEX{typeid(void)};

  public:
    Subscription() noexcept = default;

    Subscription(const Subscription&) noexcept = delete;
    Subscription& operator=(const Subscription&) noexcept = delete;
    Subscription(Subscription&& other) noexcept
        : event_bus_{std::exchange(other.event_bus_, nullptr)},
          handler_type_index_{std::exchange(other.handler_type_index_, VOID_TYPE_INDEX)}
    {
    }
    Subscription& operator=(Subscription&& other) noexcept
    {
      if (this != &other)
      {
        unsubscribe();

        event_bus_ = other.event_bus_;
        handler_type_index_ = other.handler_type_index_;

        other.release();
      }
      return *this;
    }
    ~Subscription() { unsubscribe(); }

    void unsubscribe() noexcept
    {
      if (event_bus_)
      {
        event_bus_->unsubscribe(*this);
        release();
      }
    }

    void release() noexcept
    {
      event_bus_ = nullptr;
      handler_type_index_ = VOID_TYPE_INDEX;
    }

    bool operator==(const Subscription& other) const noexcept
    {
      return handler_type_index_ == other.handler_type_index_;
    }

    explicit operator bool() const noexcept { return event_bus_ != nullptr; }

    std::type_index get_handler_type_index() const noexcept { return handler_type_index_; }

  private:
    GenericEventBus* get_event_bus() const noexcept { return event_bus_; }

    GenericEventBus* event_bus_{nullptr};
    std::type_index handler_type_index_{VOID_TYPE_INDEX};
  };

  template <typename EventT, typename CallableT, typename... ArgsT>
  void add_subscription(CallableT&& callable, ArgsT&&... args) noexcept
  {
    [[maybe_unused]] const auto lock_guard = synchronization_policy_.make_lock_guard();
    subscribe_unsafe<EventT>(std::forward<CallableT>(callable), std::forward<ArgsT>(args)...);
  }

  template <typename EventT, typename CallableT, typename... ArgsT>
  [[nodiscard]] Subscription subscribe(CallableT&& callable, ArgsT&&... args) noexcept
  {
    [[maybe_unused]] const auto lock_guard = synchronization_policy_.make_lock_guard();
    auto& context = subscribe_unsafe<EventT>(std::forward<CallableT>(callable), std::forward<ArgsT>(args)...);
    return std::exchange(context.subscription, Subscription{});
  }

  void unsubscribe(const Subscription& subscription) noexcept
  {
    if (subscription.get_event_bus() != this)
    {
      return;
    }

    [[maybe_unused]] const auto lock_guard = synchronization_policy_.make_lock_guard();

    for (auto& [_, subscriptions] : event_handlers_)
    {
      for (auto sub_it = subscriptions.begin(); sub_it != subscriptions.end();)
      {
        if (*sub_it == subscription)
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

  template <typename EventT>
  void unsubscribe() noexcept
  {
    [[maybe_unused]] const auto lock_guard = synchronization_policy_.make_lock_guard();

    const auto type_index = get_event_type_index<EventT>();
    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      total_subscribers_ -= it->second.size();
      event_handlers_.erase(it);
    }
  }

  void clear() noexcept
  {
    [[maybe_unused]] const auto lock_guard = synchronization_policy_.make_lock_guard();
    event_handlers_.clear();
    total_subscribers_ = 0U;
  }

  bool empty() const noexcept
  {
    [[maybe_unused]] const auto lock_guard = synchronization_policy_.make_lock_guard();
    return total_subscribers_ == 0U;
  }

  template <typename EventT>
  void publish(const EventT& event) noexcept
  {
    [[maybe_unused]] const auto lock_guard = synchronization_policy_.make_lock_guard();

    const auto type_index = get_event_type_index<EventT>();
    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      for (const auto& subscription : it->second)
      {
        subscription.event_handler->dispatch(event);
      }
    }
  }

  template <typename EventT>
  std::size_t get_number_of_subscribers() const noexcept
  {
    [[maybe_unused]] const auto lock_guard = synchronization_policy_.make_lock_guard();

    const auto type_index = get_event_type_index<EventT>();
    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      return it->second.size();
    }
    return 0U;
  }

  std::size_t get_total_number_of_subscribers() const noexcept
  {
    [[maybe_unused]] const auto lock_guard = synchronization_policy_.make_lock_guard();
    return total_subscribers_;
  }

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

    bool operator==(const Subscription& other) const noexcept
    {
      return handler_type_index == other.get_handler_type_index();
    }

    std::type_index handler_type_index;
    std::unique_ptr<IEventHandler> event_handler;
    Subscription subscription;
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
    context.subscription = Subscription{this, handler_type_index};

    ++total_subscribers_;

    return context;
  }

  using EventHandlers = std::unordered_map<std::type_index, std::vector<SubscriptionContext>>;
  EventHandlers event_handlers_;

  details::SynchronizationPolicy<POLICY> synchronization_policy_;

  std::size_t total_subscribers_{0U};
};

using EventBus = GenericEventBus<ThreadingPolicy::SINGLE_THREADED>;
using ThreadSafeEventBus = GenericEventBus<ThreadingPolicy::MULTI_THREADED>;

} // namespace rtw::event_bus
