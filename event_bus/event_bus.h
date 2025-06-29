#pragma once

#include <functional>
#include <memory>
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

} // namespace details

class EventBus
{
public:
  template <typename EventT, typename EventHandlerT>
  void subscribe(EventHandlerT&& event_handler)
  {
    emplace_event_handler<EventT>(std::make_unique<details::FunctionEventHandler<EventHandlerT, EventT>>(
        std::forward<EventHandlerT>(event_handler)));
  }

  template <typename EventT, typename ClassT, typename EventHandlerT>
  void subscribe(const ClassT& instance, EventHandlerT&& event_handler)
  {
    emplace_event_handler<EventT>(
        std::make_unique<details::ConstMemberFunctionEventHandler<ClassT, EventHandlerT, EventT>>(
            instance, std::forward<EventHandlerT>(event_handler)));
  }

  template <typename EventT, typename ClassT, typename EventHandlerT>
  void subscribe(ClassT& instance, EventHandlerT&& event_handler)
  {
    emplace_event_handler<EventT>(std::make_unique<details::MemberFunctionEventHandler<ClassT, EventHandlerT, EventT>>(
        instance, std::forward<EventHandlerT>(event_handler)));
  }

  template <typename EventT>
  void publish(const EventT& event)
  {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
    const std::type_index type_index{typeid(EventT)};

    if (auto it = event_handlers_.find(type_index); it != event_handlers_.end())
    {
      for (const auto& handler : it->second)
      {
        handler->dispatch(event);
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
  template <typename EventT>
  void emplace_event_handler(std::unique_ptr<details::IEventHandler> event_handler)
  {
    static_assert(std::is_base_of_v<Event, EventT>, "EventT must derive from Event");
    const std::type_index type_index{typeid(EventT)};
    event_handlers_[type_index].emplace_back(std::move(event_handler));
    ++total_subscribers_;
  }

  using EventHandlers = std::unordered_map<std::type_index, std::vector<std::unique_ptr<details::IEventHandler>>>;
  EventHandlers event_handlers_;

  std::size_t total_subscribers_{0U};
};

} // namespace rtw::event_bus
