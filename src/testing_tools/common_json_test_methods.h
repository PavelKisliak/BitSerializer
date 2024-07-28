/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
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
	BitSerializer::SerializationOptions options;
	BitSerializer::SerializationContext context(options);
	typename TArchive::input_archive_type inputArchive(static_cast<const OutputFormat&>(outputData), context);
	ASSERT_EQ(inputArchive.GetPath(), "");
	size_t mapSize = 0;
	auto objScope = inputArchive.OpenObjectScope(mapSize);
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), "");

	const auto objectKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestValue");
	const auto expectedObjectPath = TArchive::path_separator + std::string("TestValue");
	auto subScope = objScope->OpenObjectScope(objectKey, 0);
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
	BitSerializer::SerializationOptions options;
	BitSerializer::SerializationContext context(options);
	typename TArchive::output_archive_type outputArchive(outputData, context);

	// Act / Assert
	ASSERT_EQ(outputArchive.GetPath(), "");
	size_t mapSize = 0;
	auto objScope = outputArchive.OpenObjectScope(mapSize);
	ASSERT_TRUE(objScope.has_value());
	ASSERT_EQ(objScope->GetPath(), "");

	const auto objectKey = BitSerializer::Convert::To<typename TArchive::key_type>("TestValue");
	auto subScope = objScope->OpenObjectScope(objectKey, mapSize);
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
	using TestType = TestClassWithSubTwoDimArray<int>;
	TestType testObj;
	::BuildFixture(testObj);

	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	BitSerializer::SaveObject<TArchive>(testObj, outputData);

	// Act / Assert
	BitSerializer::SerializationOptions options;
	BitSerializer::SerializationContext context(options);
	typename TArchive::input_archive_type inputArchive(static_cast<const OutputFormat&>(outputData), context);
	ASSERT_EQ(inputArchive.GetPath(), "");
	size_t mapSize = 0;
	auto objScope = inputArchive.OpenObjectScope(mapSize);
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
			auto expectedPath = expectedObjectPath
				+ TArchive::path_separator + BitSerializer::Convert::ToString(k+1)
				+ TArchive::path_separator + BitSerializer::Convert::ToString(i);
			ASSERT_EQ(subArrayScope->GetPath(), expectedPath);
			ASSERT_TRUE(subArrayScope->SerializeValue(loadValue));
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
	constexpr size_t array1stLevelSize = 3, array2stLevelSize = 5;
	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData;
	BitSerializer::SerializationOptions options;
	BitSerializer::SerializationContext context(options);
	typename TArchive::output_archive_type outputArchive(outputData, context);

	// Act / Assert
	ASSERT_EQ(outputArchive.GetPath(), "");
	auto objScope = outputArchive.OpenObjectScope(1);
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
				+ TArchive::path_separator + BitSerializer::Convert::ToString(k + 1)
				+ TArchive::path_separator + BitSerializer::Convert::ToString(i + 1);
			ASSERT_EQ(subArrayScope->GetPath(), expectedPath);
		}
	}
}

/// <summary>
/// Tests loading JSON from encoded stream with/without writing BOM.
/// </summary>
template <typename TArchive, typename TUtfTraits>
void TestLoadJsonFromEncodedStream(const bool withBom)
{
	// Arrange
	using char_type = typename TUtfTraits::char_type;
	const std::string testAnsiJson = R"({"TestValue":"Hello world!"})";
	std::string sourceStr;
	// Add Bom (if required)
	if (withBom) {
		sourceStr.append(std::cbegin(TUtfTraits::bom), std::cend(TUtfTraits::bom));
	}

	// Simple UTF encoding (just for ANSI range)
	std::basic_string<char_type> temp(BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(begin(testAnsiJson)), BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(cend(testAnsiJson)));
	if constexpr (TUtfTraits::endianness != BitSerializer::Memory::Endian::native) {
		BitSerializer::Memory::Reverse(begin(temp), end(temp));
	}
	sourceStr.append(reinterpret_cast<const char*>(temp.data()), temp.size() * sizeof(char_type));
	std::stringstream inputStream(sourceStr);

	// Act
	TestClassWithSubType<std::string> actual;
	BitSerializer::LoadObject<TArchive>(actual, inputStream);

	// Assert
	EXPECT_EQ("Hello world!", actual.GetValue());
}

/// <summary>
/// Tests saving JSON to encoded stream with/without writing BOM.
/// </summary>
template <typename TArchive, typename TUtfTraits>
void TestSaveJsonToEncodedStream(const bool withBom)
{
	// Arrange
	using char_type = typename TUtfTraits::char_type;
	using string_type = std::basic_string<char_type, std::char_traits<char_type>>;
	static_assert(sizeof(TUtfTraits::bom) % sizeof(char_type) == 0);

	const std::string expectedJsonInAnsi = R"({"TestValue":"Hello world!"})";
	const string_type expectedJson(std::cbegin(expectedJsonInAnsi), std::cend(expectedJsonInAnsi));

	std::stringstream outputStream;
	TestClassWithSubType<std::string> testObj("Hello world!");
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.streamOptions.writeBom = withBom;
	serializationOptions.streamOptions.encoding = TUtfTraits::utfType;

	// Act
	BitSerializer::SaveObject<TArchive>(testObj, outputStream, serializationOptions);

	// Assert
	const auto data = outputStream.str();
	const char* dataIt = data.data();
	auto dataSize = data.size();
	// Test BOM
	if (withBom)
	{
		EXPECT_TRUE(data.size() > sizeof(TUtfTraits::bom));
		std::vector<char> actualBom(dataIt, dataIt + sizeof(TUtfTraits::bom));
		std::vector<char> expectedBom(std::cbegin(TUtfTraits::bom), std::cend(TUtfTraits::bom));
		dataIt += sizeof(TUtfTraits::bom);
		dataSize -= sizeof(TUtfTraits::bom);
		EXPECT_EQ(expectedBom, actualBom);
	}
	// Simple UTF decoding (just for ANSI range)
	string_type actualJson;
	const typename string_type::size_type targetCharCount = dataSize / sizeof(char_type);
	actualJson.append(reinterpret_cast<const char_type*>(dataIt), dataSize / sizeof(char_type));
	if constexpr (TUtfTraits::endianness != BitSerializer::Memory::Endian::native) {
		BitSerializer::Memory::Reverse(actualJson.begin(), actualJson.end());
	}
	// Test JSON
	EXPECT_EQ(expectedJson, actualJson);
}

/// <summary>
/// Tests save JSON with formatting.
/// </summary>
template <typename TArchive>
void TestSaveFormattedJson()
{
	// Arrange
	typename TArchive::preferred_output_format outputStr;
	TestClassWithSubType<std::string> testObj("Hello world!");
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.formatOptions.enableFormat = true;
	serializationOptions.formatOptions.paddingChar = ' ';
	serializationOptions.formatOptions.paddingCharNum = 2;

	// Act
	BitSerializer::SaveObject<TArchive>(testObj, outputStr, serializationOptions);

	// Assert
	const auto expected = BitSerializer::Convert::To<typename TArchive::preferred_output_format>(
		"{\n"
		"  \"TestValue\": \"Hello world!\"\n"
		"}");
	EXPECT_EQ(expected, outputStr);
}
