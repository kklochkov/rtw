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
    event_bus.subscribe<TestEvent42>(
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
    event_bus.subscribe<TestEvent43>(
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
    event_bus.subscribe<TestEvent44>(test_event_handler);
    event_bus.publish(TestEvent44{});
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent44>(), 1U);
  }

  // A functor
  {
    event_bus.subscribe<TestEvent45>(TestEventHandler{});
    event_bus.publish(TestEvent45{});
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent45>(), 1U);
  }

  // A const member function of a class, on a const instance
  {
    const TestEventHandler46 handler46;
    event_bus.subscribe<TestEvent46>(handler46, &TestEventHandler46::handler_non_mutable);
    event_bus.publish(TestEvent46{});
    EXPECT_TRUE(handler46.event_handled);
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent46>(), 1U);
    EXPECT_EQ(TestEventHandler46::const_invocation_count, 1U);
    EXPECT_EQ(TestEventHandler46::non_const_invocation_count, 0U);
  }

  // A const member function of a class, on a mutable instance
  {
    TestEventHandler46 handler46;
    event_bus.subscribe<TestEvent46>(handler46, &TestEventHandler46::handler_non_mutable);
    event_bus.publish(TestEvent46{});
    EXPECT_TRUE(handler46.event_handled);
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent46>(), 2U);
    EXPECT_EQ(TestEventHandler46::const_invocation_count, 3U);
    EXPECT_EQ(TestEventHandler46::non_const_invocation_count, 0U);
  }

  // A mutable member function of a class, on a mutable instance
  {
    TestEventHandler46 handler46;
    event_bus.subscribe<TestEvent46>(handler46, &TestEventHandler46::handler_mutable);
    event_bus.publish(TestEvent46{});
    EXPECT_TRUE(handler46.event_handled);
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent46>(), 3U);
  }

  EXPECT_EQ(TestEventHandler46::const_invocation_count, 5U);
  EXPECT_EQ(TestEventHandler46::non_const_invocation_count, 1U);

  // A static member function
  {
    event_bus.subscribe<TestEvent47>(TestEventHandler47::handler);
    event_bus.publish(TestEvent47{});
    EXPECT_EQ(event_bus.get_number_of_subscribers<TestEvent47>(), 1U);
  }

  EXPECT_EQ(event_bus.get_total_number_of_subscribers(), 8U);
}

} // namespace rtw::event_bus
