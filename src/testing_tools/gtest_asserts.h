/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include <memory>
#include <gtest/gtest.h>

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void GTestExpectEq(T expected, T actual)
{
	EXPECT_EQ(expected, actual);
}

template <typename T, std::enable_if_t<std::is_class_v<T> || std::is_union_v<T>, int> = 0>
void GTestExpectEq(const T& expected, const T& actual)
{
	if constexpr (has_assert_method_v<std::decay_t<T>>)
	{
		expected.Assert(actual);
	}
	else
	{
		EXPECT_EQ(expected, actual);
	}
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
void GTestExpectEq(T expected, T actual)
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
void GTestExpectEq(const std::shared_ptr<TValue>& expected, const std::shared_ptr<TValue>& actual)
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

template<typename TValue, size_t ArraySize>
void GTestExpectEq(const TValue(&expected)[ArraySize], const TValue(&actual)[ArraySize])
{
	for (int i = 0; i < ArraySize; ++i)
	{
		GTestExpectEq(expected[i], actual[i]);
	}
}

template<typename TValue>
void GTestExpectEq(const std::valarray<TValue>& expected, const std::valarray<TValue>& actual)
{
	ASSERT_EQ(expected.size(), actual.size());
	for (int i = 0; i < actual.size(); ++i)
	{
		GTestExpectEq(expected[i], actual[i]);
	}
}
