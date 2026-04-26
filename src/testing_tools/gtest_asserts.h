/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include <memory>
#include <cmath>
#include <gtest/gtest.h>
#include "bitserializer/serialization_detail/generic_container.h"


/**
 * @brief Invokes @p fn with optional @p args and returns the caught exception.
 * @details Supports lambdas, free functions, and member function pointers via @c std::invoke.
 * @tparam TEx Expected exception type (must be copy-constructible).
 * @tparam TFn Callable type.
 * @tparam TArgs Argument types.
 * @param fn Callable to execute.
 * @param args Arguments to forward to @p fn.
 * @return Copy of the caught exception.
 * @note Fails the test if no exception or a wrong type is thrown.
 *
 * @code
 * // 1. Lambda
 * auto ex1 = GTestExpectException<MyErr>([] { do_something(); });
 *
 * // 2. Member function + object + args
 * MyClass obj;
 * auto ex2 = GTestExpectException<MyErr>(&MyClass::process, obj, "data", 42);
 *
 * // 3. Free function + args
 * auto ex3 = GTestExpectException<MyErr>(free_func, "arg");
 * @endcode
 */
template <typename TEx, typename TFn, typename... TArgs>
TEx GTestExpectException(TFn&& fn, TArgs&&... args)
{
	static_assert(std::is_copy_constructible_v<TEx>, "TEx must be copy-constructible.");
	try {
		std::invoke(std::forward<TFn>(fn), std::forward<TArgs>(args)...);
	}
	catch (const TEx& e) {
		return e;
	}
	catch (...) {
		ADD_FAILURE() << "Expected '" << typeid(TEx).name() << "', but got a different exception.";
	}
	ADD_FAILURE() << "Expected '" << typeid(TEx).name() << "', but nothing was thrown.";
	throw std::logic_error("gtest assertion failure");
}

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
	if (std::isnan(expected))
	{
		EXPECT_TRUE(std::isnan(actual));
	}
	else
	{
		EXPECT_FLOAT_EQ(expected, actual);
	}
}

inline void GTestExpectEq(const double& expected, const double& actual)
{
	if (std::isnan(expected))
	{
		EXPECT_TRUE(std::isnan(actual));
	}
	else
	{
		EXPECT_DOUBLE_EQ(expected, actual);
	}
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
		GTestExpectEq(expected.value(), actual.value()); // NOLINT(bugprone-unchecked-optional-access)
	}
}

template<typename TValue, size_t ArraySize>
void GTestExpectEq(const TValue(&expected)[ArraySize], const TValue(&actual)[ArraySize])
{
	for (size_t i = 0; i < ArraySize; ++i)
	{
		GTestExpectEq(expected[i], actual[i]);
	}
}

template<typename TValue>
void GTestExpectEq(const std::valarray<TValue>& expected, const std::valarray<TValue>& actual)
{
	ASSERT_EQ(expected.size(), actual.size());
	for (size_t i = 0; i < actual.size(); ++i)
	{
		GTestExpectEq(expected[i], actual[i]);
	}
}

template<typename TValue>
void GTestExpectEq(const std::priority_queue<TValue>& expected, const std::priority_queue<TValue>& actual)
{
	GTestExpectEq(BitSerializer::Detail::GetBaseContainer(expected), BitSerializer::Detail::GetBaseContainer(actual));
}

template<typename TKey, typename TValue, typename TComparer, typename TAllocator>
void GTestExpectEq(const std::multimap<TKey, TValue, TComparer, TAllocator>& expected, const std::multimap<TKey, TValue, TComparer, TAllocator>& actual)
{
	ASSERT_EQ(expected.size(), actual.size());
	// Order of values can be rearranged after loading
	for (const auto& pair : actual)
	{
		const auto expectedElementsRange = expected.equal_range(pair.first);
		const auto result = std::find(expectedElementsRange.first, expectedElementsRange.second, pair);
		ASSERT_TRUE(result != expectedElementsRange.second);
	}
}
