#include "stl/source_location.h"

#include <gtest/gtest.h>

TEST(SourceLocation, basic)
{
  const auto source_location = rtw::stl::SourceLocation::current();
  EXPECT_EQ(source_location.file_name(), "stl/tests/source_location_test.cpp");
  EXPECT_EQ(source_location.function_name(), "TestBody");
  EXPECT_EQ(source_location.line(), 7);
}

namespace
{

rtw::stl::SourceLocation
current_source_location_forwarder(rtw::stl::SourceLocation source_location = rtw::stl::SourceLocation::current())
{
  return source_location;
}

void test_source_location_with_forwarding()
{
  const auto forwarded_source_location = current_source_location_forwarder();
  EXPECT_EQ(forwarded_source_location.file_name(), "stl/tests/source_location_test.cpp");
  EXPECT_EQ(forwarded_source_location.function_name(), "test_source_location_with_forwarding");
  EXPECT_EQ(forwarded_source_location.line(), 24);
}

} // namespace

TEST(SourceLocation, with_function) { test_source_location_with_forwarding(); }
