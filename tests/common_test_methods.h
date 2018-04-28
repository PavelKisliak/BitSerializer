/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include "gtest/gtest.h"
#include "auto_fixture.h"

/// <summary>
/// Test template of serialization for fundamental type.
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename T>
void TestSerializeType(T&& value)
{
	typename TArchive::output_format outputArchive;
	::BitSerializer::SaveObject<TArchive>(value, outputArchive);
	ASSERT_FALSE(outputArchive.empty());

	std::decay_t<T> actual;
	::BitSerializer::LoadObject<TArchive>(actual, outputArchive);
	EXPECT_EQ(value, actual);
}

/// <summary>
/// Test template of serialization for c-array.
/// </summary>
/// <param name="value">The value.</param>
template<typename TArchive, typename TValue, size_t ArraySize = 7>
void TestSerializeArray()
{
	TValue testArray[ArraySize];
	BuildFixture(testArray);

	typename TArchive::output_format outputArchive;
	::BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	ASSERT_FALSE(outputArchive.empty());

	TValue actual[ArraySize];
	::BitSerializer::LoadObject<TArchive>(actual, outputArchive);
	for (size_t i = 0; i < ArraySize; i++) {
		ASSERT_EQ(testArray[i], actual[i]);
	}
}

/// <summary>
/// Test template of serialization for two-dimensional c-array.
/// </summary>
/// <param name="value">The value.</param>
template<typename TArchive, typename TValue, size_t ArraySize1 = 3, size_t ArraySize2 = 5>
void TestSerializeTwoDimensionalArray()
{
	TValue testArray[ArraySize1][ArraySize2];
	BuildFixture(testArray);

	typename TArchive::output_format outputArchive;
	::BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	ASSERT_FALSE(outputArchive.empty());

	TValue actual[ArraySize1][ArraySize2];
	::BitSerializer::LoadObject<TArchive>(actual, outputArchive);
	for (size_t i = 0; i < ArraySize1; i++) {
		for (size_t c = 0; c < ArraySize2; c++) {
			ASSERT_EQ(testArray[i][c], actual[i][c]);
		}
	}
}

/// <summary>
/// Test template of serialization for class (must have constant method Assert()).
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename T>
void TestSerializeClass(T&& value)
{
	typename TArchive::output_format outputArchive;
	::BitSerializer::SaveObject<TArchive>(value, outputArchive);
	ASSERT_FALSE(outputArchive.empty());

	std::decay_t<T> actual;
	::BitSerializer::LoadObject<TArchive>(actual, outputArchive);
	value.Assert(actual);
}

/// <summary>
/// Test template of serialization for class with using streams.
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename TStreamElem, typename T>
void TestSerializeClassToStream(T&& value)
{
	// Arrange
	using string_stream_type = std::basic_stringstream<TStreamElem, std::char_traits<TStreamElem>, std::allocator<TStreamElem>>;
	string_stream_type outputStream;

	// Act
	::BitSerializer::SaveObject<TArchive>(value, outputStream);
	outputStream.seekg(0, std::ios::beg);
	std::decay_t<T> actual;
	::BitSerializer::LoadObject<TArchive>(actual, outputStream);

	// Assert
	value.Assert(actual);
}

/// <summary>
/// Test template of serialization for STL containers.
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename TContainer>
void TestSerializeStlContainer(std::optional<std::function<void(const TContainer&, const TContainer&)>> specialAssertFunc = std::nullopt)
{
	// Arrange
	typename TArchive::output_format outputArchive;
	TContainer expected;
	::BuildFixture(expected);

	// Act
	auto jsonResult = SaveObject<JsonArchive>(expected);
	TContainer actual;
	LoadObject<JsonArchive>(actual, jsonResult);

	// Assert
	if (specialAssertFunc.has_value())
		specialAssertFunc.value()(expected, actual);
	else
		EXPECT_EQ(expected, actual);
}

/// <summary>
/// Asserts the multimap container.
/// </summary>
/// <param name="expected">The expected.</param>
/// <param name="actual">The actual.</param>
template <typename TContainer>
void AssertMultimap(const TContainer& expected, const TContainer& actual)
{
	ASSERT_EQ(expected.size(), actual.size());
	// Order of values can be rearranged after loading
	for (auto& elem : actual) {
		auto expectedElementsRange = expected.equal_range(elem.first);
		auto result = std::find(expectedElementsRange.first, expectedElementsRange.second, elem);
		ASSERT_TRUE(result != expectedElementsRange.second);
	}
}