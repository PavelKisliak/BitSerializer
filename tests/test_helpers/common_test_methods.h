/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <filesystem>
#include <optional>
#include "common_test_entities.h"
#include "bitserializer/types/std/vector.h"


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
	GTestExpectEq(value, actual);
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
	for (size_t i = 0; i < std::min(SourceArraySize, TargetArraySize); i++)
	{
		GTestExpectEq(testArray[i], actual[i]);
	}
}

/// <summary>
/// Test template of serialization for vector.
/// </summary>
template<typename TArchive, typename TValue>
void TestSerializeVector(std::initializer_list<TValue> testValues)
{
	// Arrange
	std::vector<TValue> testArray = { std::move(testValues) };
	BuildFixture(testArray);
	typename TArchive::preferred_output_format outputArchive;
	std::vector<TValue> actual;

	// Act
	BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	ASSERT_EQ(testArray.size(), actual.size());
	for (size_t i = 0; i < testArray.size(); i++)
	{
		GTestExpectEq(testArray[i], actual[i]);
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
		GTestExpectEq(testArray[i], actual[i]);
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
			GTestExpectEq(testArray[i][c], actual[i][c]);
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
/// Test template of serialization for array with using streams.
/// </summary>
template <typename TArchive, typename TStreamElem, typename T, size_t ArraySize = 3>
void TestSerializeArrayToStream(T(&testArray)[ArraySize])
{
	// Arrange
	using string_stream_type = std::basic_stringstream<TStreamElem, std::char_traits<TStreamElem>, std::allocator<TStreamElem>>;
	string_stream_type outputStream;
	T actual[ArraySize];

	// Act
	BitSerializer::SaveObject<TArchive>(testArray, outputStream);
	std::string str = outputStream.str();
	BitSerializer::LoadObject<TArchive>(actual, outputStream);

	// Assert
	for (size_t i = 0; i < ArraySize; i++)
	{
		testArray[i].Assert(actual[i]);
	}
}

/// <summary>
/// Test template of serialization to file.
/// </summary>
template <typename TArchive, size_t ArraySize = 3>
void TestSerializeArrayToFile()
{
	// Arrange
	auto path = std::filesystem::temp_directory_path() / "TestArchive.data";
	TestPointClass testArray[ArraySize], actual[ArraySize];
	BuildFixture(testArray);

	// Act
	BitSerializer::SaveObjectToFile<TArchive>(testArray, path);
	BitSerializer::LoadObjectFromFile<TArchive>(actual, path);

	// Assert
	for (size_t i = 0; i < ArraySize; i++)
	{
		testArray[i].Assert(actual[i]);
	}
}

/// <summary>
/// Test template of serialization for STL containers.
/// </summary>
template <typename TArchive, typename TContainer>
void TestSerializeStlContainer()
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
	EXPECT_EQ(expected, actual);
}

/// <summary>
/// Template for test loading to not empty container.
/// </summary>
template <typename TArchive, typename TContainer>
void TestLoadToNotEmptyContainer(size_t targetContainerSize)
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive;
	TContainer expected{};
	::BuildFixture(expected);
	TContainer actual(targetContainerSize);

	// Act
	auto jsonResult = BitSerializer::SaveObject<TArchive>(expected);
	BitSerializer::LoadObject<TArchive>(actual, jsonResult);

	// Assert
	EXPECT_EQ(expected, actual);
}

/// <summary>
/// Test template of serialization for STL containers with custom assert function.
/// </summary>
/// <param name="assertFunc">The assertion function.</param>
template <typename TArchive, typename TContainer>
void TestSerializeStlContainer(std::function<void(const TContainer&, const TContainer&)> assertFunc)
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
	assertFunc(expected, actual);
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
	T testObj[1];
	BuildFixture(testObj);
	typename TArchive::preferred_output_format outputArchive;

	// Act
	bool result = false;
	BitSerializer::SaveObject<TArchive>(testObj, outputArchive);
	try
	{
		BitSerializer::LoadObject<TArchive>(testObj, outputArchive);
		result = true;
	}
	catch (BitSerializer::ValidationException& ex)
	{
		// Assert
		EXPECT_EQ(BitSerializer::SerializationErrorCode::FailedValidation, ex.GetErrorCode());
		EXPECT_EQ(1, ex.GetValidationErrors().size());
	}

	EXPECT_FALSE(result);
}

/// <summary>
/// Test template of validation for loading not compatible types (e.g. number from string).
/// </summary>
///	ToDo: Remove after add testing MismatchedTypes policy to other archives
template <typename TArchive, class TSourceType, class TTargetType>
void TestValidationForNotCompatibleTypes()
{
	// Arrange
	TSourceType sourceObj[1];
	BuildFixture(sourceObj);
	typename TArchive::preferred_output_format outputArchive;

	// Act
	bool result = false;
	BitSerializer::SaveObject<TArchive>(sourceObj, outputArchive);
	try
	{
		TTargetType targetObj[1];
		BitSerializer::LoadObject<TArchive>(targetObj, outputArchive);
		result = true;
	}
	catch (BitSerializer::ValidationException& ex)
	{
		// Assert
		EXPECT_EQ(BitSerializer::SerializationErrorCode::FailedValidation, ex.GetErrorCode());
		EXPECT_EQ(1, ex.GetValidationErrors().size());
	}

	EXPECT_FALSE(result);
}

/// <summary>
/// Template for test overflow target value when deserialization.
/// </summary>
template <typename TArchive, class TSourceType, class TTargetType>
void TestOverflowNumberPolicy(BitSerializer::OverflowNumberPolicy overflowNumberPolicy)
{
	// Arrange
	static_assert(std::is_arithmetic_v<TSourceType> && std::is_arithmetic_v<TTargetType>);
	static_assert(sizeof (TSourceType) >= sizeof (TTargetType));

	TSourceType testValue;
	if constexpr (std::is_floating_point_v<TTargetType>)
	{
		if constexpr (std::is_floating_point_v<TTargetType>) {
			testValue = TSourceType(std::numeric_limits<TTargetType>::max()) * 1.00001;
		}
		else {
			testValue = std::numeric_limits<TSourceType>::min();
		}
	}
	else
	{
		if constexpr (std::is_floating_point_v<TSourceType>)
		{
			// Test cast from floating point number to integer
			testValue = TSourceType(3.141592654f);
		}
		else
		{
			if constexpr (std::is_signed_v<TTargetType>) {
				testValue = TSourceType(std::numeric_limits<TTargetType>::min()) - 1;
			}
			else {
				testValue = TSourceType(std::numeric_limits<TTargetType>::max()) + 1;
			}
		}
	}
	TestClassWithSubType<TSourceType> sourceObj[1] = { TestClassWithSubType<TSourceType>(testValue) };
	TestClassWithSubType<TTargetType, true> targetObj[1];

	BitSerializer::SerializationOptions options;
	options.overflowNumberPolicy = overflowNumberPolicy;
	typename TArchive::preferred_output_format outputArchive;
	BitSerializer::SaveObject<TArchive>(sourceObj, outputArchive);

	// Act / Assert
	switch (overflowNumberPolicy)
	{
	case BitSerializer::OverflowNumberPolicy::ThrowError:
		try
		{
			BitSerializer::LoadObject<TArchive>(targetObj, outputArchive, options);
			EXPECT_TRUE(false);
		}
		catch (const BitSerializer::SerializationException& ex)
		{
			EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		}
		break;

	case BitSerializer::OverflowNumberPolicy::Skip:
		try
		{
			BitSerializer::LoadObject<TArchive>(targetObj, outputArchive, options);
			EXPECT_TRUE(false);
		}
		catch (const BitSerializer::ValidationException& ex)
		{
			EXPECT_EQ(BitSerializer::SerializationErrorCode::FailedValidation, ex.GetErrorCode());
			EXPECT_EQ(1, ex.GetValidationErrors().size());
		}
		break;
	}
}

/// <summary>
/// Template for test loading mismatched types (e.g. number from string).
/// </summary>
template <typename TArchive, class TSourceType, class TTargetType>
void TestMismatchedTypesPolicy(BitSerializer::MismatchedTypesPolicy mismatchedTypesPolicy)
{
	// Arrange
	static_assert(!std::is_same_v<TSourceType, TTargetType>);

	TestClassWithSubType<TSourceType> sourceObj[1];
	TestClassWithSubType<TTargetType, true> targetObj[1];

	BitSerializer::SerializationOptions options;
	options.mismatchedTypesPolicy = mismatchedTypesPolicy;
	typename TArchive::preferred_output_format outputArchive;
	BitSerializer::SaveObject<TArchive>(sourceObj, outputArchive);

	// Act / Assert
	switch (mismatchedTypesPolicy)
	{
	case BitSerializer::MismatchedTypesPolicy::ThrowError:
		try
		{
			BitSerializer::LoadObject<TArchive>(targetObj, outputArchive, options);
			EXPECT_TRUE(false);
		}
		catch (const BitSerializer::SerializationException& ex)
		{
			EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		}
		break;

	case BitSerializer::MismatchedTypesPolicy::Skip:
		try
		{
			BitSerializer::LoadObject<TArchive>(targetObj, outputArchive, options);
			EXPECT_TRUE(false);
		}
		catch (const BitSerializer::ValidationException& ex)
		{
			EXPECT_EQ(BitSerializer::SerializationErrorCode::FailedValidation, ex.GetErrorCode());
			EXPECT_EQ(1, ex.GetValidationErrors().size());
		}
		break;
	}
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
	const BitSerializer::SerializationOptions options;
	BitSerializer::SerializationContext context(options);
	typename TArchive::input_archive_type inputArchive(static_cast<const OutputFormat&>(outputData), context);

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
