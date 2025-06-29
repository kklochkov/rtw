#pragma once

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>

// demangle
#include <cstdlib>
#include <cxxabi.h>

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

namespace details
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

template <typename CallableT, typename EventT>
struct FunctionEventHandler final : IEventHandler
{
  explicit FunctionEventHandler(CallableT function) : callable{std::move(function)} {}

  void dispatch(const Event& event) final { std::invoke(callable, static_cast<const EventT&>(event)); }

  CallableT callable; // Can be a function/lambda or a function pointer
};

template <typename ClassT, typename MemberFunctionT, typename EventT>
struct MemberFunctionEventHandler final : IEventHandler
{
  explicit MemberFunctionEventHandler(ClassT& object, MemberFunctionT function)
      : instance{&object}, member_function{std::move(function)}
  {
  }

  void dispatch(const Event& event) final { std::invoke(member_function, instance, static_cast<const EventT&>(event)); }

  ClassT* instance;
  MemberFunctionT member_function;
};

template <typename ClassT, typename MemberFunctionT, typename EventT>
struct ConstMemberFunctionEventHandler final : IEventHandler
{
  explicit ConstMemberFunctionEventHandler(const ClassT& object, MemberFunctionT function)
      : instance{&object}, member_function{std::move(function)}
  {
  }

  void dispatch(const Event& event) final { std::invoke(member_function, instance, static_cast<const EventT&>(event)); }

  const ClassT* instance;
  MemberFunctionT member_function;
};

std::string demangle(const std::type_index& type_index)
{
  char* name = abi::__cxa_demangle(type_index.name(), nullptr, nullptr, nullptr);
  std::string demangled_name{name};

  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,hicpp-no-malloc, cppcoreguidelines-owning-memory)
  std::free(name);

  return demangled_name;
}

} // namespace details

class EventBus
{
public:
  template <typename EventT, typename EventHandlerT>
  std::type_index subscribe(EventHandlerT&& event_handler)
  {
    using HandlerWrapper = details::FunctionEventHandler<EventHandlerT, EventT>;
    const std::type_index handler_type_index{typeid(HandlerWrapper)};
    return emplace_event_handler<EventT>(handler_type_index,
                                         std::make_unique<HandlerWrapper>(std::forward<EventHandlerT>(event_handler)));
  }

  template <typename EventT, typename ClassT, typename EventHandlerT>
  std::type_index subscribe(const ClassT& instance, EventHandlerT&& event_handler)
  {
    using HandlerWrapper = details::ConstMemberFunctionEventHandler<ClassT, EventHandlerT, EventT>;
    const std::type_index handler_type_index{typeid(HandlerWrapper)};
    return emplace_event_handler<EventT>(
        handler_type_index, std::make_unique<HandlerWrapper>(instance, std::forward<EventHandlerT>(event_handler)));
  }

  template <typename EventT, typename ClassT, typename EventHandlerT>
  std::type_index subscribe(ClassT& instance, EventHandlerT&& event_handler)
  {
    using HandlerWrapper = details::MemberFunctionEventHandler<ClassT, EventHandlerT, EventT>;
    const std::type_index handler_type_index{typeid(HandlerWrapper)};
    return emplace_event_handler<EventT>(
        handler_type_index, std::make_unique<HandlerWrapper>(instance, std::forward<EventHandlerT>(event_handler)));
  }

  void unsubscribe(const std::type_index& handler_type_index)
  {
    for (auto& [_, subscriptions] : event_handlers_)
    {
      for (auto sub_it = subscriptions.begin(); sub_it != subscriptions.end();)
      {
        if (sub_it->handler_type_index == handler_type_index)
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
  void unsubscribe()
  {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
    const std::type_index type_index{typeid(EventT)};
    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      total_subscribers_ -= it->second.size();
      event_handlers_.erase(it);
    }
  }

  template <typename EventT>
  void publish(const EventT& event)
  {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
    const std::type_index type_index{typeid(EventT)};

    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      for (const auto& subscription : it->second)
      {
        subscription.event_handler->dispatch(event);
      }
    }
  }

  template <typename EventT>
  std::size_t get_number_of_subscribers() const
  {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
    const std::type_index type_index{typeid(EventT)};
    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      return it->second.size();
    }
    return 0U;
  }

  std::size_t get_total_number_of_subscribers() const { return total_subscribers_; }

private:
  struct Subscription
  {
    Subscription(const std::type_index& handler_type_index, std::unique_ptr<details::IEventHandler> event_handler)
        : handler_type_index{handler_type_index}, event_handler{std::move(event_handler)}
    {
    }

    std::type_index handler_type_index;
    std::unique_ptr<details::IEventHandler> event_handler;
  };

  template <typename EventT>
  std::type_index emplace_event_handler(const std::type_index& handler_type_index,
                                        std::unique_ptr<details::IEventHandler> event_handler)
  {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
    const std::type_index type_index{typeid(EventT)};

    event_handlers_[type_index].emplace_back(handler_type_index, std::move(event_handler));
    ++total_subscribers_;

    return handler_type_index;
  }

  using Subscriptions = std::vector<Subscription>;
  using EventHandlers = std::unordered_map<std::type_index, Subscriptions>;
  EventHandlers event_handlers_;

  std::size_t total_subscribers_{0U};
};

} // namespace rtw::event_bus
