#include "stl/contracts.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(ContractsQuickEnforceTest, current_semantic)
{
  EXPECT_EQ(rtw::stl::contracts::current_semantic(), rtw::stl::contracts::EvaluationSemantic::QUICK_ENFORCE);
}

TEST(ContractsQuickEnforceTest, simple)
{
  EXPECT_NO_FATAL_FAILURE(RTW_PRE(true, "This precondition should pass"));
  EXPECT_DEATH(RTW_PRE(false), ::testing::SizeIs(0));
  EXPECT_DEATH(RTW_PRE(false, "This precondition should fail"), ::testing::SizeIs(0));

  EXPECT_NO_FATAL_FAILURE(RTW_ASSERT(true, "This precondition should pass"));
  EXPECT_DEATH(RTW_ASSERT(false), ::testing::SizeIs(0));
  EXPECT_DEATH(RTW_ASSERT(false, "This precondition should fail"), ::testing::SizeIs(0));

  EXPECT_NO_FATAL_FAILURE(RTW_POST(true, "This precondition should pass"));
  EXPECT_DEATH(RTW_POST(false), ::testing::SizeIs(0));
  EXPECT_DEATH(RTW_POST(false, "This precondition should fail"), ::testing::SizeIs(0));

  std::int32_t value = 10;
  EXPECT_NO_FATAL_FAILURE(RTW_PRE(value = 20, "This precondition should pass"));
  EXPECT_EQ(value, 20);
  EXPECT_NO_FATAL_FAILURE(RTW_ASSERT(value = 30, "This precondition should fail"));
  EXPECT_EQ(value, 30);
  EXPECT_NO_FATAL_FAILURE(RTW_POST(value = 40, "This postcondition should pass"));
  EXPECT_EQ(value, 40);
}

namespace
{

std::int32_t function_with_contracts(std::int32_t value)
{
  RTW_PRE(value > 0, "Value must be positive");
  value *= 3;
  RTW_ASSERT(value % 2 == 0, "Value must be even");
  RTW_POST(value < 100, "Value must be less than 100");
  return value;
}

std::uint32_t function_throws_exception(std::int32_t value)
{
  value *= 2;
  if (value > 100)
  {
    throw std::runtime_error("Value is too large");
  }
  return value;
}

} // namespace

TEST(ContractsQuickEnforceTest, function_with_contracts)
{
  EXPECT_NO_FATAL_FAILURE(function_with_contracts(10));
  EXPECT_DEATH(function_with_contracts(-10), ::testing::SizeIs(0));
  EXPECT_DEATH(function_with_contracts(11), ::testing::SizeIs(0));
  EXPECT_DEATH(function_with_contracts(50), ::testing::SizeIs(0));
}

TEST(ContractsQuickEnforceTest, function_throws_exception)
{
  EXPECT_NO_FATAL_FAILURE(RTW_PRE(function_throws_exception(10) < 100, "Value must be less than 100"));
  EXPECT_DEATH(RTW_PRE(function_throws_exception(110) < 100, "Value must be less than 100"), ::testing::SizeIs(0));
}
