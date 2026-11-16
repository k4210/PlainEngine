#include <expected.h>

#include <gtest/gtest.h>
#include <string>
#include <system_error>

TEST(ExpectedTests, HoldsValuesAndErrors)
{
    const PlainEngine::Core::Expected<std::string, std::error_code> defaultValue;
    EXPECT_TRUE(defaultValue);
    EXPECT_TRUE(defaultValue->empty());

    const PlainEngine::Core::Expected<std::string, std::error_code> value{"data"};
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "data");

    const auto errorCode = std::make_error_code(std::errc::io_error);
    const PlainEngine::Core::Expected<std::string, std::error_code> error{
        PlainEngine::Core::MakeUnexpected(errorCode)};
    EXPECT_FALSE(error);
    EXPECT_EQ(error.error(), errorCode);

    const PlainEngine::Core::Expected<void, std::error_code> success;
    EXPECT_TRUE(success);
    success.value();
}
