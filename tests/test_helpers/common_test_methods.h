/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include <filesystem>
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
/// <param name="value">The value.</param>
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

	// Assert
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);
	for (size_t i = 0; i < std::min(SourceArraySize, TargetArraySize); i++) {
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
	// Arrange
	TValue testArray[ArraySize1][ArraySize2];
	BuildFixture(testArray);
	typename TArchive::preferred_output_format outputArchive;
	TValue actual[ArraySize1][ArraySize2];

	// Act
	BitSerializer::SaveObject<TArchive>(testArray, outputArchive);

	// Assert
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);
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

	// Assert
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);
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
	auto path = std::filesystem::temp_directory_path() / "TestArchive.data";
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
/// <param name="value">The value.</param>
template <typename TArchive, typename TContainer>
void TestSerializeStlContainer(std::optional<std::function<void(const TContainer&, const TContainer&)>> specialAssertFunc = std::nullopt)
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive;
	TContainer expected;
	::BuildFixture(expected);
	TContainer actual;

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
/// Test template of validation for named values (boolean result, which returned from archive methods).
/// </summary>
template <typename TArchive, class T>
void TestValidationForNamedValues()
{
	// Arrange
	T testObj;
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
/// Tests archive method which should return key by index.
/// </summary>
template <typename TArchive>
void TestGetKeyByIndex()
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

	auto actualSize = objScope->GetSize();
	ASSERT_EQ(2, actualSize);

	auto actualKey1 = objScope->GetKeyByIndex(0);
	EXPECT_EQ(expectedKey1, actualKey1);

	auto actualKey2 = objScope->GetKeyByIndex(1);
	EXPECT_EQ(expectedKey2, actualKey2);
}

/// <summary>
/// Tests archive method which should return current path in object scope (when loading).
/// </summary>
template <typename TArchive>
void TestGetPathInObjectScopeWhenLoading()
{
	// Arrange
	TestClassWithSubType<TestPointClass> testObj;
	::BuildFixture(testObj);
	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	BitSerializer::SaveObject<TArchive>(testObj, outputData);

	// Act / Assert
	typename TArchive::input_archive_type inputArchive(static_cast<const OutputFormat&>(outputData));
	ASSERT_EQ(inputArchive.GetPath(), L"");
	auto objScope = inputArchive.OpenObjectScope();
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), L"");

	const auto objectKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestSubValue");
	const auto expectedObjectPath = TArchive::path_separator + std::wstring(L"TestSubValue");
	auto subScope = objScope->OpenObjectScope(objectKey);
	ASSERT_TRUE(subScope.has_value());
	ASSERT_EQ(subScope->GetPath(), expectedObjectPath);
}

/// <summary>
/// Tests archive method which should return current path in object scope (when saving).
/// </summary>
template <typename TArchive>
void TestGetPathInObjectScopeWhenSaving()
{
	// Arrange
	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	typename TArchive::output_archive_type outputArchive(outputData);

	// Act / Assert
	ASSERT_EQ(outputArchive.GetPath(), L"");
	auto objScope = outputArchive.OpenObjectScope();
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), L"");

	const auto objectKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestSubValue");
	auto subScope = objScope->OpenObjectScope(objectKey);
	ASSERT_TRUE(subScope.has_value());
	ASSERT_EQ(subScope->GetPath(), TArchive::path_separator + objectKey);
}

/// <summary>
/// Tests archive method which should return current path in array scope (when loading).
/// </summary>
template <typename TArchive>
void TestGetPathInArrayScopeWhenLoading()
{
	// Arrange
	using TestType = TestClassWithSubTwoDimArray<TestClassWithSubType<int>>;
	TestType testObj;
	::BuildFixture(testObj);

	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	BitSerializer::SaveObject<TArchive>(testObj, outputData);

	// Act / Assert
	typename TArchive::input_archive_type inputArchive(static_cast<const OutputFormat&>(outputData));
	ASSERT_EQ(inputArchive.GetPath(), L"");
	auto objScope = inputArchive.OpenObjectScope();
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), L"");

	const auto arrayKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestTwoDimArray");
	const auto expectedObjectPath = TArchive::path_separator + std::wstring(L"TestTwoDimArray");
	auto arrayScope = objScope->OpenArrayScope(arrayKey, TestType::Array1stLevelSize);
	ASSERT_TRUE(arrayScope.has_value());
	ASSERT_EQ(arrayScope->GetPath(), expectedObjectPath + TArchive::path_separator + L"0");

	int loadValue;
	for (size_t k = 0; k < TestType::Array1stLevelSize; k++)
	{
		auto subArrayScope = arrayScope->OpenArrayScope(TestType::Array2stLevelSize);
		ASSERT_TRUE(subArrayScope.has_value());

		for (size_t i = 0; i < TestType::Array2stLevelSize; i++)
		{
			subArrayScope->SerializeValue(loadValue);
			auto expectedPath = expectedObjectPath
				+ TArchive::path_separator + BitSerializer::Convert::ToWString(k)
				+ TArchive::path_separator + BitSerializer::Convert::ToWString(i);
			ASSERT_EQ(subArrayScope->GetPath(), expectedPath);
		}
	}
}

/// <summary>
/// Tests archive method which should return current path in array scope (when saving).
/// </summary>
template <typename TArchive>
void TestGetPathInArrayScopeWhenSaving()
{
	// Arrange
	size_t array1stLevelSize = 3, array2stLevelSize = 5;
	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	typename TArchive::output_archive_type outputArchive(outputData);

	// Act / Assert
	ASSERT_EQ(outputArchive.GetPath(), L"");
	auto objScope = outputArchive.OpenObjectScope();
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), L"");

	const auto arrayKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestTwoDimArray");
	const auto expectedObjectPath = TArchive::path_separator + std::wstring(L"TestTwoDimArray");
	auto arrayScope = objScope->OpenArrayScope(arrayKey, array1stLevelSize);
	ASSERT_TRUE(arrayScope.has_value());
	ASSERT_EQ(arrayScope->GetPath(), expectedObjectPath + TArchive::path_separator + L"0");

	int saveValue = 0x10203040;
	for (size_t k = 0; k < array1stLevelSize; k++)
	{
		auto subArrayScope = arrayScope->OpenArrayScope(array2stLevelSize);
		ASSERT_TRUE(subArrayScope.has_value());

		for (size_t i = 0; i < array2stLevelSize; i++)
		{
			subArrayScope->SerializeValue(saveValue);
			auto expectedPath = expectedObjectPath
				+ TArchive::path_separator + BitSerializer::Convert::ToWString(k)
				+ TArchive::path_separator + BitSerializer::Convert::ToWString(i);
			ASSERT_EQ(subArrayScope->GetPath(), expectedPath);
		}
	}
}
