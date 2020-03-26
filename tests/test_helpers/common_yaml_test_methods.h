/*******************************************************************************
* Copyright (C) 2020 by Artsiom Marozau                                        *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/

#pragma once
#include "gtest/gtest.h"
#include "common_test_entities.h"

/// <summary>
/// Test template of serialization for fundamental type.
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename T>
void TestSerializeSingleValueArray(T&& value)
{
	// Arrange
	T testArray[] {value};
	typename TArchive::preferred_output_format outputArchive;
	T actual[1];

	// Act
	BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	if constexpr (std::is_same_v<T, float>) {
		EXPECT_FLOAT_EQ(testArray[0], actual[0]);
	}
	else if constexpr (std::is_same_v<T, double>) {
		EXPECT_DOUBLE_EQ(testArray[0], actual[0]);
	}
	else {
		EXPECT_EQ(testArray[0], actual[0]);
	}
}

/// <summary>
/// Tests loading YAML from encoded stream with/without writing BOM.
/// </summary>
template <typename TArchive, typename TUtfTraits>
void TestLoadYamlFromEncodedStream(const bool withBom)
{
	// Arrange
	using char_type = typename TUtfTraits::char_type;
	const std::string testAnsiYaml = "TestValue: Hello world!\n";
	std::string sourceStr;
	// Add Bom (if required)
	if (withBom) {
		sourceStr.append(std::cbegin(TUtfTraits::bom), std::cend(TUtfTraits::bom));
	}
	// UTF encoding but just for ANSI range
	for (const char ch : testAnsiYaml)
	{
		for (size_t c = 0; c < sizeof(char_type); c++)
		{
			if constexpr (TUtfTraits::lowEndian) {
				sourceStr.push_back(c ? 0 : ch);
			}
			else {
				sourceStr.push_back(c == (sizeof(char_type) - 1) ? ch : 0);
			}
		}
	}
	std::stringstream inputStream(sourceStr);

	// Act
	TestClassWithSubType<std::string> actual;
	BitSerializer::LoadObject<TArchive>(actual, inputStream);

	// Assert
	EXPECT_EQ("Hello world!", actual.GetValue());
}

/// <summary>
/// Tests saving Yaml to encoded stream with/without writing BOM.
/// </summary>
template <typename TArchive, typename TUtfTraits>
void TestSaveYamlToEncodedStream(const bool withBom)
{
	// Arrange
	using char_type = typename TUtfTraits::char_type;
	using string_type = std::basic_string<char_type, std::char_traits<char_type>>;
	static_assert(sizeof(TUtfTraits::bom) % sizeof(char_type) == 0);

	const std::string expectedYamlInAnsi = "TestValue: Hello world!\n";
	const string_type expectedYaml(std::cbegin(expectedYamlInAnsi), std::cend(expectedYamlInAnsi));

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
	// UTF decoding but just for ANSI range
	string_type actualYaml;
	const typename string_type::size_type targetCharCount = dataSize / sizeof(char_type);
	if constexpr (TUtfTraits::lowEndian) {
		actualYaml.append(reinterpret_cast<const char_type*>(dataIt), dataSize / sizeof(char_type));
	}
	else
	{
		for (size_t i = 0; i < targetCharCount; i++)
		{
			char_type ch = 0;
			for (size_t c = 0; c < sizeof(char_type); c++) {
				ch = (ch << 8) | dataIt[i * sizeof(char_type) + c];
			}
			actualYaml.push_back(ch);
		}
	}
	// Test Yaml
	EXPECT_EQ(expectedYaml, actualYaml);
}