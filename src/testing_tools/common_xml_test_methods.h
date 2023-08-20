/*******************************************************************************
* Copyright (C) 2020-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "gtest/gtest.h"
#include "common_test_entities.h"


/// <summary>
/// Tests loading XML from encoded stream with/without writing BOM.
/// </summary>
template <typename TArchive, typename TUtfTraits>
void TestLoadXmlFromEncodedStream(const bool withBom)
{
	// Arrange
	using char_type = typename TUtfTraits::char_type;
	const std::string testAnsiXml = R"(<?xml version="1.0"?><root><TestValue>Hello world!</TestValue></root>)";
	std::string sourceStr;
	// Add Bom (if required)
	if (withBom) {
		sourceStr.append(std::cbegin(TUtfTraits::bom), std::cend(TUtfTraits::bom));
	}
	// UTF encoding but just for ANSI range
	for (const char ch : testAnsiXml)
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
/// Tests saving XML to encoded stream with/without writing BOM.
/// </summary>
template <typename TArchive, typename TUtfTraits>
void TestSaveXmlToEncodedStream(const bool withBom)
{
	// Arrange
	using char_type = typename TUtfTraits::char_type;
	using string_type = std::basic_string<char_type, std::char_traits<char_type>>;
	static_assert(sizeof(TUtfTraits::bom) % sizeof(char_type) == 0);

	const std::string expectedXmlInAnsi = R"(<?xml version="1.0")";
	const string_type expectedXml(std::cbegin(expectedXmlInAnsi), std::cend(expectedXmlInAnsi));

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
	string_type actualXml;
	const typename string_type::size_type targetCharCount = dataSize / sizeof(char_type);
	if constexpr (TUtfTraits::lowEndian) {
		actualXml.append(reinterpret_cast<const char_type*>(dataIt), dataSize / sizeof(char_type));
	}
	else
	{
		for (size_t i = 0; i < targetCharCount; i++)
		{
			char_type ch = 0;
			for (size_t c = 0; c < sizeof(char_type); c++) {
				ch = (ch << 8) | dataIt[i * sizeof(char_type) + c];
			}
			actualXml.push_back(ch);
		}
	}
	// Test XML
	EXPECT_TRUE(actualXml.find(expectedXml) == 0);
}

/// <summary>
/// Tests save XML with formatting.
/// </summary>
template <typename TArchive>
void TestSaveFormattedXml()
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
		"<?xml version=\"1.0\"?>\n"
		"<root>\n"
		"  <TestValue>Hello world!</TestValue>\n"
		"</root>\n");
	EXPECT_EQ(expected, outputStr);
}