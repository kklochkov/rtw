#include "event_bus/event_bus.h"

#include <gtest/gtest.h>

namespace rtw::event_bus
{

struct TestEvent42 : Event
{
  std::string name{"TestEvent42"};
  std::int32_t value{42};
};

struct TestEvent43 : Event
{
  std::string name{"TestEvent43"};
  std::int32_t value{43};
};

struct TestEvent44 : Event
{
  std::string name{"TestEvent44"};
  std::int32_t value{44};
};

struct TestEvent45 : Event
{
  std::string name{"TestEvent45"};
  std::int32_t value{45};
};

struct TestEvent46 : Event
{
  std::string name{"TestEvent46"};
  std::int32_t value{46};
};

struct TestEvent47 : Event
{
  std::string name{"TestEvent47"};
  std::int32_t value{47};
};

namespace
{

void test_event_handler(const TestEvent44& event)
{
  EXPECT_EQ(event.name, "TestEvent44");
  EXPECT_EQ(event.value, 44);
}

struct TestEventHandler
{
  void operator()(const TestEvent45& event) const
  {
    EXPECT_EQ(event.name, "TestEvent45");
    EXPECT_EQ(event.value, 45);
  }
};

struct TestEventHandler46
{
  void handler_mutable(const TestEvent46& event)
  {
    EXPECT_EQ(event.name, "TestEvent46");
    EXPECT_EQ(event.value, 46);
    event_handled = true;
    ++non_const_invocation_count;
  }

  void handler_non_mutable(const TestEvent46& event) const
  {
    EXPECT_EQ(event.name, "TestEvent46");
    EXPECT_EQ(event.value, 46);
    event_handled = true;
    ++const_invocation_count;
  }

  mutable bool event_handled{false};
  static inline std::size_t const_invocation_count{0};
  static inline std::size_t non_const_invocation_count{0};
};

struct TestEventHandler47
{
  static void handler(const TestEvent47& event)
  {
    EXPECT_EQ(event.name, "TestEvent47");
    EXPECT_EQ(event.value, 47);
  }
};

} // namespace

TEST(EventBusTest, event_dispatch)
{
  EventBus event_bus;

  // A stateful lambda
  {
    bool event_handled = false;
    event_bus.add_subscription<TestEvent42>(
        [&event_handled](const TestEvent42& event)
        {
          EXPECT_EQ(event.name, "TestEvent42");
          EXPECT_EQ(event.value, 42);
          event_handled = true;
        });

    event_bus.publish(TestEvent42{});

    EXPECT_TRUE(event_handled);
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent42>(), 1U);
  }

  // A stateless lambda
  {
    event_bus.add_subscription<TestEvent43>(
        [](const TestEvent43& event)
        {
          EXPECT_EQ(event.name, "TestEvent43");
          EXPECT_EQ(event.value, 43);
        });

    event_bus.publish(TestEvent43{});
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent43>(), 1U);
  }

  // A free function
  {
    event_bus.add_subscription<TestEvent44>(test_event_handler);
    event_bus.publish(TestEvent44{});
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent44>(), 1U);
  }

  // A functor
  {
    event_bus.add_subscription<TestEvent45>(TestEventHandler{});
    event_bus.publish(TestEvent45{});
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent45>(), 1U);
  }

  std::vector<EventBus::Subscription> subscribers;
  // A const member function of a class, on a const instance
  {
    const TestEventHandler46 handler46;
    auto subscription = event_bus.subscribe<TestEvent46>(&TestEventHandler46::handler_non_mutable, &handler46);
    event_bus.publish(TestEvent46{});
    EXPECT_TRUE(handler46.event_handled);
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent46>(), 1U);
    EXPECT_EQ(TestEventHandler46::const_invocation_count, 1U);
    EXPECT_EQ(TestEventHandler46::non_const_invocation_count, 0U);
    subscribers.push_back(std::move(subscription));
  }

  // A const member function of a class, on a mutable instance
  {
    TestEventHandler46 handler46;
    auto subscription = event_bus.subscribe<TestEvent46>(&TestEventHandler46::handler_non_mutable, &handler46);
    event_bus.publish(TestEvent46{});
    EXPECT_TRUE(handler46.event_handled);
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent46>(), 2U);
    EXPECT_EQ(TestEventHandler46::const_invocation_count, 3U);
    EXPECT_EQ(TestEventHandler46::non_const_invocation_count, 0U);
    subscribers.push_back(std::move(subscription));
  }

  // A mutable member function of a class, on a mutable instance
  {
    TestEventHandler46 handler46;
    event_bus.add_subscription<TestEvent46>(&TestEventHandler46::handler_mutable, &handler46);
    event_bus.publish(TestEvent46{});
    EXPECT_TRUE(handler46.event_handled);
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent46>(), 3U);
  }

  EXPECT_EQ(TestEventHandler46::const_invocation_count, 5U);
  EXPECT_EQ(TestEventHandler46::non_const_invocation_count, 1U);

  // A static member function
  {
    event_bus.add_subscription<TestEvent47>(TestEventHandler47::handler);
    event_bus.publish(TestEvent47{});
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent47>(), 1U);
  }

  EXPECT_EQ(event_bus.get_total_number_of_subscribers(), 8U);

  {
    // Unsubscribe the mutable handler
    for (const auto& sub : subscribers)
    {
      event_bus.unsubscribe(sub);
    }
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent46>(), 1U);
    event_bus.publish(TestEvent46{});
    EXPECT_EQ(TestEventHandler46::const_invocation_count, 5U);
    EXPECT_EQ(TestEventHandler46::non_const_invocation_count, 2U);

    event_bus.unsubscribe<TestEvent46>();
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent46>(), 0U);
    event_bus.publish(TestEvent46{});
    EXPECT_EQ(TestEventHandler46::const_invocation_count, 5U);
    EXPECT_EQ(TestEventHandler46::non_const_invocation_count, 2U);

    event_bus.unsubscribe<TestEvent42>();
    event_bus.unsubscribe<TestEvent43>();
    event_bus.unsubscribe<TestEvent44>();
    event_bus.unsubscribe<TestEvent45>();
    event_bus.unsubscribe<TestEvent47>();
  }

  EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent42>(), 0U);
  EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent43>(), 0U);
  EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent44>(), 0U);
  EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent45>(), 0U);
  EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent46>(), 0U);
  EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent47>(), 0U);
  EXPECT_EQ(event_bus.get_total_number_of_subscribers(), 0U);
}

TEST(EventBusTest, subscription)
{
  EventBus event_bus;

  {
    // Default construct a subscription and check that it is not valid.
    EventBus::Subscription subscription;
    EXPECT_FALSE(subscription);
    subscription.unsubscribe();
    subscription.release();
  }

  {
    // Auto unsubscribe when subscription goes out of scope.
    std::uint32_t call_count = 0U;
    {
      auto subscription = event_bus.subscribe<TestEvent42>(
          [&call_count](const TestEvent42& event)
          {
            EXPECT_EQ(event.name, "TestEvent42");
            EXPECT_EQ(event.value, 42);
            ++call_count;
          });

      EXPECT_TRUE(subscription);
      event_bus.publish(TestEvent42{});
      EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent42>(), 1U);
      EXPECT_EQ(call_count, 1U);
    }

    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent42>(), 0U);
    event_bus.publish(TestEvent42{}); // Should not call the handler since it has been unsubscribed.
    EXPECT_EQ(call_count, 1U);
  }

  {
    // Manual unsubscribe using the subscription object.
    std::uint32_t call_count = 0U;
    auto subscription = event_bus.subscribe<TestEvent43>(
        [&call_count](const TestEvent43& event)
        {
          EXPECT_EQ(event.name, "TestEvent43");
          EXPECT_EQ(event.value, 43);
          ++call_count;
        });

    EXPECT_TRUE(subscription);
    event_bus.publish(TestEvent43{});
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent43>(), 1U);
    EXPECT_EQ(call_count, 1U);

    subscription.unsubscribe();
    subscription.unsubscribe(); // Double unsubscribe should be safe and have no effect.

    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent43>(), 0U);
    event_bus.publish(TestEvent43{}); // Should not call the handler since it has been unsubscribed.
    EXPECT_EQ(call_count, 1U);
  }

  {
    // Release the subscription object and check that it unsubscribes.
    std::uint32_t call_count = 0U;
    {
      auto subscription = event_bus.subscribe<TestEvent42>(
          [&call_count](const TestEvent42& event)
          {
            EXPECT_EQ(event.name, "TestEvent42");
            EXPECT_EQ(event.value, 42);
            ++call_count;
          });

      subscription.release(); // Should release the subscription without unsubscribing.
      EXPECT_FALSE(subscription);
      EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent42>(), 1U);
      EXPECT_EQ(call_count, 0U);
    }

    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent42>(), 1U);
    event_bus.publish(TestEvent42{});
    EXPECT_EQ(call_count, 1U);
  }

  {
    // Move construct a subscription and check that it moves correctly.
    std::uint32_t call_count = 0U;
    auto subscription1 = event_bus.subscribe<TestEvent43>(
        [&call_count](const TestEvent43& event)
        {
          EXPECT_EQ(event.name, "TestEvent43");
          EXPECT_EQ(event.value, 43);
          ++call_count;
        });

    auto subscription2 = std::move(subscription1);

    EXPECT_FALSE(subscription1);
    EXPECT_TRUE(subscription2);
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent43>(), 1U);
    event_bus.publish(TestEvent43{});
    EXPECT_EQ(call_count, 1U);
  }

  {
    // Move assign a subscription and check that it moves correctly.
    std::uint32_t call_count_1 = 0U;
    std::uint32_t call_count_2 = 0U;

    auto subscription1 = event_bus.subscribe<TestEvent43>(
        [&call_count_1](const TestEvent43& event)
        {
          EXPECT_EQ(event.name, "TestEvent43");
          EXPECT_EQ(event.value, 43);
          ++call_count_1;
        });
    auto subscription2 = event_bus.subscribe<TestEvent43>(
        [&call_count_2](const TestEvent43& event)
        {
          EXPECT_EQ(event.name, "TestEvent43");
          EXPECT_EQ(event.value, 43);
          ++call_count_2;
        });
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent43>(), 2U);

    subscription2 = std::move(subscription1);

    EXPECT_FALSE(subscription1);
    EXPECT_TRUE(subscription2);
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent43>(), 1U);
    event_bus.publish(TestEvent43{});
    EXPECT_EQ(call_count_1, 1U);
    EXPECT_EQ(call_count_2, 0U);
  }
}

TEST(EventBusTest, add_subscription_no_crash_on_destruction)
{
  // This test verifies that using add_subscription (which doesn't return a Subscription)
  // doesn't cause a crash when the EventBus is destroyed without explicit unsubscribe.

  std::uint32_t call_count = 0U;

  {
    EventBus event_bus;

    event_bus.add_subscription<TestEvent42>(
        [&call_count](const TestEvent42& event)
        {
          EXPECT_EQ(event.name, "TestEvent42");
          EXPECT_EQ(event.value, 42);
          ++call_count;
        });

    event_bus.add_subscription<TestEvent43>(
        [&call_count](const TestEvent43& event)
        {
          EXPECT_EQ(event.name, "TestEvent43");
          EXPECT_EQ(event.value, 43);
          ++call_count;
        });

    EXPECT_EQ(event_bus.get_total_number_of_subscribers(), 2U);

    event_bus.publish(TestEvent42{});
    event_bus.publish(TestEvent43{});

    EXPECT_EQ(call_count, 2U);

    // Event bus goes out of scope here WITHOUT calling unsubscribe.
    // This should NOT crash.
  }

  EXPECT_EQ(call_count, 2U);
}

TEST(EventBusTest, subscription_outlives_event_bus)
{
  // This test verifies that a Subscription object can safely outlive the EventBus.
  EventBus::Subscription subscription;
  {
    EventBus event_bus;
    std::uint32_t call_count = 0U;
    subscription = event_bus.subscribe<TestEvent42>(
        [&call_count](const TestEvent42& event)
        {
          EXPECT_EQ(event.name, "TestEvent42");
          EXPECT_EQ(event.value, 42);
          ++call_count;
        });
    EXPECT_TRUE(subscription);
    event_bus.publish(TestEvent42{});
    EXPECT_EQ(call_count, 1U);
    // EventBus goes out of scope here, but subscription survives
  }
  // After EventBus is destroyed:
  // 1. subscription.unsubscribe() should be safe (no-op)
  // 2. subscription destructor should be safe (no-op)

  subscription.unsubscribe(); // Should not crash or cause UB

  // Subscription destructor runs here - should also be safe
}

// --- clear() ---

TEST(EventBusTest, clear)
{
  EventBus event_bus;
  std::uint32_t call_count = 0U;

  event_bus.add_subscription<TestEvent42>([&call_count](const TestEvent42&) { ++call_count; });
  event_bus.add_subscription<TestEvent43>([&call_count](const TestEvent43&) { ++call_count; });
  event_bus.add_subscription<TestEvent44>([&call_count](const TestEvent44&) { ++call_count; });

  EXPECT_EQ(event_bus.get_total_number_of_subscribers(), 3U);
  EXPECT_FALSE(event_bus.empty());

  event_bus.clear();

  EXPECT_EQ(event_bus.get_total_number_of_subscribers(), 0U);
  EXPECT_TRUE(event_bus.empty());

  // Publishing after clear should not invoke any handlers.
  event_bus.publish(TestEvent42{});
  event_bus.publish(TestEvent43{});
  event_bus.publish(TestEvent44{});
  EXPECT_EQ(call_count, 0U);

  // Double clear should be safe.
  event_bus.clear();
  EXPECT_TRUE(event_bus.empty());
}

// --- has_subscribers() ---

TEST(EventBusTest, has_subscribers)
{
  EventBus event_bus;

  EXPECT_FALSE(event_bus.has_subscribers<TestEvent42>());
  EXPECT_FALSE(event_bus.has_subscribers<TestEvent43>());

  event_bus.add_subscription<TestEvent42>([](const TestEvent42&) {});
  EXPECT_TRUE(event_bus.has_subscribers<TestEvent42>());
  EXPECT_FALSE(event_bus.has_subscribers<TestEvent43>());

  event_bus.add_subscription<TestEvent43>([](const TestEvent43&) {});
  EXPECT_TRUE(event_bus.has_subscribers<TestEvent43>());

  event_bus.unsubscribe<TestEvent42>();
  EXPECT_FALSE(event_bus.has_subscribers<TestEvent42>());
  EXPECT_TRUE(event_bus.has_subscribers<TestEvent43>());
}

// --- publish with no subscribers ---

TEST(EventBusTest, publish_no_subscribers)
{
  EventBus event_bus;

  // Publishing to an empty bus should be a silent no-op (no crash, no side effects).
  event_bus.publish(TestEvent42{});
  event_bus.publish(TestEvent43{});

  EXPECT_TRUE(event_bus.empty());
  EXPECT_EQ(event_bus.get_total_number_of_subscribers(), 0U);
}

// --- multiple handlers for the same event type, dispatch order ---

TEST(EventBusTest, multiple_handlers_same_event_dispatch_order)
{
  EventBus event_bus;
  std::vector<int> dispatch_order;

  event_bus.add_subscription<TestEvent42>([&dispatch_order](const TestEvent42&) { dispatch_order.push_back(1); });
  event_bus.add_subscription<TestEvent42>([&dispatch_order](const TestEvent42&) { dispatch_order.push_back(2); });
  event_bus.add_subscription<TestEvent42>([&dispatch_order](const TestEvent42&) { dispatch_order.push_back(3); });

  EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent42>(), 3U);

  event_bus.publish(TestEvent42{});

  // Handlers are dispatched in registration order.
  ASSERT_EQ(dispatch_order.size(), 3U);
  EXPECT_EQ(dispatch_order[0], 1);
  EXPECT_EQ(dispatch_order[1], 2);
  EXPECT_EQ(dispatch_order[2], 3);
}

// --- ThreadSafeEventBus basic functionality ---

TEST(EventBusTest, thread_safe_bus_basic)
{
  ThreadSafeEventBus bus;

  EXPECT_TRUE(bus.empty());
  EXPECT_EQ(bus.get_total_number_of_subscribers(), 0U);
  EXPECT_FALSE(bus.has_subscribers<TestEvent42>());

  std::uint32_t call_count = 0U;
  auto subscription = bus.subscribe<TestEvent42>([&call_count](const TestEvent42&) { ++call_count; });

  EXPECT_TRUE(subscription);
  EXPECT_FALSE(bus.empty());
  EXPECT_EQ(bus.get_number_of_subscribers<TestEvent42>(), 1U);
  EXPECT_TRUE(bus.has_subscribers<TestEvent42>());

  bus.publish(TestEvent42{});
  EXPECT_EQ(call_count, 1U);

  subscription.unsubscribe();
  EXPECT_TRUE(bus.empty());

  bus.publish(TestEvent42{});
  EXPECT_EQ(call_count, 1U); // not called again
}

TEST(EventBusTest, thread_safe_bus_multiple_event_types)
{
  ThreadSafeEventBus bus;

  std::uint32_t count_42 = 0U;
  std::uint32_t count_43 = 0U;

  bus.add_subscription<TestEvent42>([&count_42](const TestEvent42&) { ++count_42; });
  bus.add_subscription<TestEvent43>([&count_43](const TestEvent43&) { ++count_43; });

  EXPECT_EQ(bus.get_total_number_of_subscribers(), 2U);

  bus.publish(TestEvent42{});
  EXPECT_EQ(count_42, 1U);
  EXPECT_EQ(count_43, 0U);

  bus.publish(TestEvent43{});
  EXPECT_EQ(count_42, 1U);
  EXPECT_EQ(count_43, 1U);

  bus.clear();
  EXPECT_TRUE(bus.empty());

  bus.publish(TestEvent42{});
  bus.publish(TestEvent43{});
  EXPECT_EQ(count_42, 1U);
  EXPECT_EQ(count_43, 1U);
}

TEST(EventBusTest, thread_safe_bus_subscription_outlives_bus)
{
  ThreadSafeEventBus::Subscription subscription;
  {
    ThreadSafeEventBus bus;
    subscription = bus.subscribe<TestEvent42>([](const TestEvent42&) {});
    EXPECT_TRUE(subscription);
  }
  // Bus is destroyed; subscription should safely no-op on destruction/unsubscribe.
  subscription.unsubscribe();
}

} // namespace rtw::event_bus
