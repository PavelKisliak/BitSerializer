/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "gtest/gtest.h"
#include "common_test_entities.h"

/// <summary>
/// Tests archive method which should return current path in object scope (when loading).
/// </summary>
template <typename TArchive>
void TestGetPathInJsonObjectScopeWhenLoading()
{
	// Arrange
	TestClassWithSubType<TestPointClass> testObj;
	::BuildFixture(testObj);
	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	BitSerializer::SaveObject<TArchive>(testObj, outputData);

	// Act / Assert
	typename TArchive::input_archive_type inputArchive(static_cast<const OutputFormat&>(outputData));
	ASSERT_EQ(inputArchive.GetPath(), "");
	auto objScope = inputArchive.OpenObjectScope();
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), "");

	const auto objectKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestValue");
	const auto expectedObjectPath = TArchive::path_separator + std::string("TestValue");
	auto subScope = objScope->OpenObjectScope(objectKey);
	ASSERT_TRUE(subScope.has_value());
	ASSERT_EQ(subScope->GetPath(), expectedObjectPath);
}

/// <summary>
/// Tests archive method which should return current path in object scope (when saving).
/// </summary>
template <typename TArchive>
void TestGetPathInJsonObjectScopeWhenSaving()
{
	// Arrange
	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	typename TArchive::output_archive_type outputArchive(outputData);

	// Act / Assert
	ASSERT_EQ(outputArchive.GetPath(), "");
	auto objScope = outputArchive.OpenObjectScope();
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), "");

	const auto objectKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestValue");
	auto subScope = objScope->OpenObjectScope(objectKey);
	ASSERT_TRUE(subScope.has_value());
	ASSERT_EQ(subScope->GetPath(), TArchive::path_separator + BitSerializer::Convert::ToString(objectKey));
}

/// <summary>
/// Tests archive method which should return current path in array scope (when loading).
/// </summary>
template <typename TArchive>
void TestGetPathInJsonArrayScopeWhenLoading()
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
	ASSERT_EQ(inputArchive.GetPath(), "");
	auto objScope = inputArchive.OpenObjectScope();
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), "");

	const auto arrayKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestTwoDimArray");
	const auto expectedObjectPath = TArchive::path_separator + std::string("TestTwoDimArray");
	auto arrayScope = objScope->OpenArrayScope(arrayKey, TestType::Array1stLevelSize);
	ASSERT_TRUE(arrayScope.has_value());
	ASSERT_EQ(arrayScope->GetPath(), expectedObjectPath + TArchive::path_separator + "0");

	int loadValue;
	for (size_t k = 0; k < TestType::Array1stLevelSize; k++)
	{
		auto subArrayScope = arrayScope->OpenArrayScope(TestType::Array2stLevelSize);
		ASSERT_TRUE(subArrayScope.has_value());

		for (size_t i = 0; i < TestType::Array2stLevelSize; i++)
		{
			subArrayScope->SerializeValue(loadValue);
			auto expectedPath = expectedObjectPath
				+ TArchive::path_separator + BitSerializer::Convert::ToString(k)
				+ TArchive::path_separator + BitSerializer::Convert::ToString(i);
			ASSERT_EQ(subArrayScope->GetPath(), expectedPath);
		}
	}
}

/// <summary>
/// Tests archive method which should return current path in array scope (when saving).
/// </summary>
template <typename TArchive>
void TestGetPathInJsonArrayScopeWhenSaving()
{
	// Arrange
	size_t array1stLevelSize = 3, array2stLevelSize = 5;
	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	typename TArchive::output_archive_type outputArchive(outputData);

	// Act / Assert
	ASSERT_EQ(outputArchive.GetPath(), "");
	auto objScope = outputArchive.OpenObjectScope();
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), "");

	const auto arrayKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestTwoDimArray");
	const auto expectedObjectPath = TArchive::path_separator + std::string("TestTwoDimArray");
	auto arrayScope = objScope->OpenArrayScope(arrayKey, array1stLevelSize);
	ASSERT_TRUE(arrayScope.has_value());
	ASSERT_EQ(arrayScope->GetPath(), expectedObjectPath + TArchive::path_separator + "0");

	int saveValue = 0x10203040;
	for (size_t k = 0; k < array1stLevelSize; k++)
	{
		auto subArrayScope = arrayScope->OpenArrayScope(array2stLevelSize);
		ASSERT_TRUE(subArrayScope.has_value());

		for (size_t i = 0; i < array2stLevelSize; i++)
		{
			subArrayScope->SerializeValue(saveValue);
			auto expectedPath = expectedObjectPath
				+ TArchive::path_separator + BitSerializer::Convert::ToString(k)
				+ TArchive::path_separator + BitSerializer::Convert::ToString(i);
			ASSERT_EQ(subArrayScope->GetPath(), expectedPath);
		}
	}
}

/// <summary>
/// Tests loading from UTF-8 stream with BOM.
/// </summary>
template <typename TArchive>
void TestLoadJsonFromUtf8StreamWithBom()
{
	// Arrange
	std::stringstream inputStream(std::string({ char(0xEF), char(0xBB), char(0xBF) }) + "{\"TestValue\":\"Hello world!\"}");

	// Act
	TestClassWithSubType<std::string> actual;
	BitSerializer::LoadObject<TArchive>(actual, inputStream);

	// Assert
	EXPECT_EQ("Hello world!", actual.GetValue());
}

/// <summary>
/// Tests loading from UTF-8 stream with BOM.
/// </summary>
template <typename TArchive>
void TestLoadJsonFromUtf8StreamWithoutBom()
{
	// Arrange
	std::stringstream inputStream(std::string("{\"TestValue\":\"Hello world!\"}"));

	// Act
	TestClassWithSubType<std::string> actual;
	BitSerializer::LoadObject<TArchive>(actual, inputStream);

	// Assert
	EXPECT_EQ("Hello world!", actual.GetValue());
}

/// <summary>
/// Tests saving to UTF-8 stream with writing BOM.
/// </summary>
template <typename TArchive>
void TestSaveJsonToUtf8StreamWithBom()
{
	// Arrange
	std::stringstream outputStream;
	TestClassWithSubType<std::string> testObj("Hello world!");

	// Act
	BitSerializer::SaveObject<TArchive>(testObj, outputStream);

	// Assert
	const std::string expected = std::string({ char(0xEF), char(0xBB), char(0xBF) }) + "{\"TestValue\":\"Hello world!\"}";
	EXPECT_EQ(expected, outputStream.str());
}

/// <summary>
/// Tests saving to UTF-8 stream with disabled writing BOM.
/// </summary>
template <typename TArchive>
void TestSaveJsonToUtf8StreamWithoutBom()
{
	// Arrange
	std::stringstream outputStream;
	TestClassWithSubType<std::string> testObj("Hello world!");
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.streamOptions.writeBom = false;

	// Act
	BitSerializer::SaveObject<TArchive>(testObj, outputStream, serializationOptions);

	// Assert
	EXPECT_EQ("{\"TestValue\":\"Hello world!\"}", outputStream.str());
}