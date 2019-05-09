/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include <experimental/filesystem>
#include "gtest/gtest.h"
#include "common_test_entities.h"

/// <summary>
/// Test template of serialization for fundamental type.
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename T>
void TestSerializeType(T&& value)
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive;
	std::decay_t<T> actual;

	// Act
	BitSerializer::SaveObject<TArchive>(value, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	EXPECT_EQ(value, actual);
}

/// <summary>
/// Test template of serialization for c-array.
/// </summary>
template<typename TArchive, typename TValue, size_t SourceArraySize = 7, size_t TargetArraySize = 7>
void TestSerializeArray()
{
	// Arrange
	TValue testArray[SourceArraySize];
	BuildFixture(testArray);
	typename TArchive::preferred_output_format outputArchive;
	TValue actual[TargetArraySize];

	// Act
	BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	for (size_t i = 0; i < std::min(SourceArraySize, TargetArraySize); i++) {
		ASSERT_EQ(testArray[i], actual[i]);
	}
}

/// <summary>
/// Test template of serialization for c-array.
/// </summary>
template<typename TArchive, typename TValue, size_t SourceArraySize = 7, size_t TargetArraySize = 7>
void TestSerializeArrayWithKey()
{
	// Arrange
	TValue testArray[SourceArraySize];
	BuildFixture(testArray);
	typename TArchive::preferred_output_format outputArchive;
	TValue actual[TargetArraySize];

	// Act
	BitSerializer::SaveObject<TArchive>(BitSerializer::MakeAutoKeyValue(L"Root", testArray), outputArchive);
	BitSerializer::LoadObject<TArchive>(BitSerializer::MakeAutoKeyValue(L"Root", actual), outputArchive);

	// Assert
	for (size_t i = 0; i < std::min(SourceArraySize, TargetArraySize); i++) {
		ASSERT_EQ(testArray[i], actual[i]);
	}
}

/// <summary>
/// Test template of serialization for two-dimensional c-array.
/// </summary>
template<typename TArchive, typename TValue, size_t ArraySize1 = 3, size_t ArraySize2 = 5>
void TestSerializeTwoDimensionalArray()
{
	// Arrange
	TValue testArray[ArraySize1][ArraySize2];
	BuildFixture(testArray);
	typename TArchive::preferred_output_format outputArchive;
	TValue actual[ArraySize1][ArraySize2];

	// Act
	BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
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
	// Arrange
	typename TArchive::preferred_output_format outputArchive;
	std::decay_t<T> actual;

	// Act
	BitSerializer::SaveObject<TArchive>(value, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	value.Assert(actual);
}

/// <summary>
/// Test template of serialization for class with key (must have constant method Assert()).
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename T>
void TestSerializeClassWithKey(T&& value)
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive;
	std::decay_t<T> actual;

	// Act
	BitSerializer::SaveObject<TArchive>(BitSerializer::MakeAutoKeyValue(L"Root", value), outputArchive);
	BitSerializer::LoadObject<TArchive>(BitSerializer::MakeAutoKeyValue("Root", actual), outputArchive);

	// Assert
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
	std::decay_t<T> actual;

	// Act
	BitSerializer::SaveObject<TArchive>(value, outputStream);
	outputStream.seekg(0, std::ios::beg);
	BitSerializer::LoadObject<TArchive>(actual, outputStream);

	// Assert
	value.Assert(actual);
}

/// <summary>
/// Test template of serialization to file.
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename TStreamElem, typename T>
void TestSerializeClassToFile(T&& value)
{
	// Arrange
	auto path = std::experimental::filesystem::temp_directory_path() / "TestArchive.data";
	std::decay_t<T> actual;

	// Act
	BitSerializer::SaveObjectToFile<TArchive>(value, path);
	BitSerializer::LoadObjectFromFile<TArchive>(actual, path);

	// Assert
	value.Assert(actual);
}

/// <summary>
/// Test template of serialization for STL containers.
/// </summary>
/// <param name="specialAssertFunc">The assertion function.</param>
template <typename TArchive, typename TContainer>
void TestSerializeStlContainer(std::optional<std::function<void(const TContainer&, const TContainer&)>> specialAssertFunc = std::nullopt)
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive;
	TContainer expected{};
	::BuildFixture(expected);
	TContainer actual{};

	// Act
	auto jsonResult = BitSerializer::SaveObject<TArchive>(expected);
	BitSerializer::LoadObject<TArchive>(actual, jsonResult);

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

/// <summary>
/// Test template of validation for named values (boolean result, which returns by archive's method SerializeValue()).
/// </summary>
template <typename TArchive, class T>
void TestValidationForNamedValues()
{
	// Arrange
	T testObj = BuildFixture<T>();
	typename TArchive::preferred_output_format outputArchive;

	// Act
	BitSerializer::SaveObject<TArchive>(testObj, outputArchive);
	const bool saveResult = BitSerializer::Context.IsValid();
	BitSerializer::LoadObject<TArchive>(testObj, outputArchive);
	const bool loadResult = BitSerializer::Context.IsValid();

	// Assert
	ASSERT_TRUE(saveResult);
	ASSERT_FALSE(loadResult);
	testObj.Assert();
}

/// <summary>
/// Template for test iterating keys in the object scope.
/// </summary>
template <typename TArchive>
void TestIterateKeysInObjectScope()
{
	// Arrange
	auto expectedKey1 = BitSerializer::Convert::To<typename TArchive::key_type>("x");
	auto expectedKey2 = BitSerializer::Convert::To<typename TArchive::key_type>("y");
	TestPointClass testObj;
	::BuildFixture(testObj);

	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	BitSerializer::SaveObject<TArchive>(testObj, outputData);
	typename TArchive::input_archive_type inputArchive(static_cast<const OutputFormat&>(outputData));

	// Act / Assert
	auto objScope = inputArchive.OpenObjectScope();
	ASSERT_TRUE(objScope.has_value());

	auto it = objScope->cbegin();
	auto endIt = objScope->cend();
	EXPECT_EQ(expectedKey1, *it);
	EXPECT_TRUE(it != endIt);

	++it;
	EXPECT_EQ(expectedKey2, *it);
	EXPECT_TRUE(it != endIt);

	++it;
	EXPECT_TRUE(it == endIt);
}
