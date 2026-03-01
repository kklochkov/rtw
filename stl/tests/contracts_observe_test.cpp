#include "stl/contracts.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sstream>

namespace rtw::stl::contracts
{

class StdErrCapture
{
public:
  StdErrCapture() { old_stderr_ = std::cerr.rdbuf(captured_stderr_.rdbuf()); }
  StdErrCapture(const StdErrCapture&) = delete;
  StdErrCapture& operator=(const StdErrCapture&) = delete;
  StdErrCapture(StdErrCapture&&) = delete;
  StdErrCapture& operator=(StdErrCapture&&) = delete;
  ~StdErrCapture() { std::cerr.rdbuf(old_stderr_); }

  std::string captured() const { return captured_stderr_.str(); }

private:
  std::streambuf* old_stderr_;
  std::ostringstream captured_stderr_;
};

} // namespace rtw::stl::contracts

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-avoid-do-while)
#define RTW_EXPECT_THAT_STDERR(expr, matcher)                                                                          \
  do                                                                                                                   \
  {                                                                                                                    \
    rtw::stl::contracts::StdErrCapture stderr_capture;                                                                 \
    expr;                                                                                                              \
    EXPECT_THAT(stderr_capture.captured(), matcher);                                                                   \
  }                                                                                                                    \
  while (false)
// NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-avoid-do-while)

TEST(ContractsObserveTest, current_semantic)
{
  EXPECT_EQ(rtw::stl::contracts::current_semantic(), rtw::stl::contracts::EvaluationSemantic::OBSERVE);
}

TEST(ContractsObserveTest, simple)
{
  EXPECT_NO_FATAL_FAILURE(RTW_PRE(true, "This precondition should pass"));
  RTW_EXPECT_THAT_STDERR(RTW_PRE(false), ::testing::StartsWith("Contract violation detected:\n"));

  EXPECT_NO_FATAL_FAILURE(RTW_ASSERT(true, "This precondition should pass"));
  RTW_EXPECT_THAT_STDERR(RTW_ASSERT(false), ::testing::StartsWith("Contract violation detected:\n"));
  RTW_EXPECT_THAT_STDERR(RTW_ASSERT(false, "This precondition should fail"),
                         ::testing::StartsWith("Contract violation detected:\n"));

  EXPECT_NO_FATAL_FAILURE(RTW_POST(true, "This precondition should pass"));
  RTW_EXPECT_THAT_STDERR(RTW_POST(false), ::testing::StartsWith("Contract violation detected:\n"));
  RTW_EXPECT_THAT_STDERR(RTW_POST(false, "This precondition should fail"),
                         ::testing::StartsWith("Contract violation detected:\n"));

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

TEST(ContractsObserveTest, function_with_contracts)
{
  EXPECT_NO_FATAL_FAILURE(function_with_contracts(10));
  RTW_EXPECT_THAT_STDERR(function_with_contracts(-10), ::testing::StartsWith("Contract violation detected:\n"));
  RTW_EXPECT_THAT_STDERR(function_with_contracts(11), ::testing::StartsWith("Contract violation detected:\n"));
  RTW_EXPECT_THAT_STDERR(function_with_contracts(50), ::testing::StartsWith("Contract violation detected:\n"));
}

TEST(ContractsObserveTest, function_throws_exception)
{
  EXPECT_NO_FATAL_FAILURE(RTW_PRE(function_throws_exception(10) < 100, "Value must be less than 100"));
  RTW_EXPECT_THAT_STDERR(RTW_PRE(function_throws_exception(110) < 100, "Value must be less than 100"),
                         ::testing::StartsWith("Contract violation detected:\n"));
}
