#pragma once
#include <optional>
#include <memory>
#include <gtest/gtest.h>

template <typename T>
void GTestExpectEq(const T& expected, const T& actual)
{
	EXPECT_EQ(expected, actual);
}

inline void GTestExpectEq(const float& expected, const float& actual)
{
	EXPECT_FLOAT_EQ(expected, actual);
}

inline void GTestExpectEq(const double& expected, const double& actual)
{
	EXPECT_DOUBLE_EQ(expected, actual);
}

inline void GTestExpectEq(const std::nullptr_t&, const std::nullptr_t&)
{
	// Ignore comparing nullptr types (gtest fails such comparison)
}

template <typename TValue>
void GTestExpectEq(const std::unique_ptr<TValue>& expected, const std::unique_ptr<TValue>& actual)
{
	ASSERT_EQ(!expected, !actual);
	if (expected)
	{
		GTestExpectEq(*expected, *actual);
	}
}

template <typename TValue>
void GTestExpectEq(const std::optional<TValue>& expected, const std::optional<TValue>& actual)
{
	ASSERT_EQ(expected.has_value(), actual.has_value());
	if (expected.has_value())
	{
		GTestExpectEq(expected.value(), actual.value());
	}
}
