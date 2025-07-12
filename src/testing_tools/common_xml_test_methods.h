/*******************************************************************************
* Copyright (C) 2020-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "gtest/gtest.h"
#include "common_test_entities.h"

/**
 * @brief Tests loading XML from a stream encoded with a specific encoding, with or without a byte order mark (BOM).
 *
 * @tparam TArchive The archive type used for deserialization.
 * @tparam TUtfTraits Traits defining UTF encoding properties such as character type and byte order mark (BOM).
 * @param withBom Indicates whether to prepend the BOM at the start of the stream.
 */
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

	// Simple UTF encoding (just for ANSI range)
	std::basic_string<char_type> temp(
		BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(std::begin(testAnsiXml)),
		BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(std::end(testAnsiXml)));
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

/**
 * @brief Tests saving XML to a stream using a specified encoding, with or without a byte order mark (BOM).
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TUtfTraits Traits defining UTF encoding properties such as character type and byte order mark (BOM).
 * @param withBom Indicates whether to include the BOM at the beginning of the output stream.
 */
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

	BitSerializer::SerializationOptions options;
	options.streamOptions.writeBom = withBom;
	options.streamOptions.encoding = TUtfTraits::utfType;

	// Act
	BitSerializer::SaveObject<TArchive>(testObj, outputStream, options);

	// Assert
	const auto data = outputStream.str();
	const char* dataIt = data.data();
	auto dataSize = data.size();

	// Check BOM presence and correctness
	if (withBom)
	{
		ASSERT_GT(data.size(), sizeof(TUtfTraits::bom)) << "Output size must be greater than BOM length.";

		std::vector<char> actualBom(dataIt, dataIt + sizeof(TUtfTraits::bom));
		std::vector<char> expectedBom(std::cbegin(TUtfTraits::bom), std::cend(TUtfTraits::bom));
		dataIt += sizeof(TUtfTraits::bom);
		dataSize -= sizeof(TUtfTraits::bom);

		EXPECT_EQ(expectedBom, actualBom) << "The written BOM does not match the expected one.";
	}
	// Simple UTF decoding (just for ANSI range)
	string_type actualXml;
	const typename string_type::size_type targetCharCount = dataSize / sizeof(char_type);
	actualXml.append(reinterpret_cast<const char_type*>(dataIt), targetCharCount);
	if constexpr (TUtfTraits::endianness != BitSerializer::Memory::Endian::native) {
		BitSerializer::Memory::Reverse(actualXml.begin(), actualXml.end());
	}
	// Test XML
	EXPECT_TRUE(actualXml.find(expectedXml) == 0) << "Expected XML header was not found at the beginning of the output.";
}

/**
 * @brief Tests XML serialization with formatting enabled.
 *
 * Verifies that the generated XML output includes proper indentation and structure.
 *
 * @tparam TArchive The archive type used for serialization.
 */
template <typename TArchive>
void TestSaveFormattedXml()
{
	// Arrange
	typename TArchive::preferred_output_format outputStr;
	TestClassWithSubType<std::string> testObj("Hello world!");

	BitSerializer::SerializationOptions options;
	options.formatOptions.enableFormat = true;
	options.formatOptions.paddingChar = ' ';
	options.formatOptions.paddingCharNum = 2;

	// Act
	BitSerializer::SaveObject<TArchive>(testObj, outputStr, options);

	// Assert
	const auto expected = BitSerializer::Convert::To<typename TArchive::preferred_output_format>(
		"<?xml version=\"1.0\"?>\n"
		"<root>\n"
		"  <TestValue>Hello world!</TestValue>\n"
		"</root>\n");

	EXPECT_EQ(expected, outputStr) << "Formatted XML output did not match the expected value.";
}
