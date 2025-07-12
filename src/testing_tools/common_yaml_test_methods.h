/*******************************************************************************
* Copyright (C) 2020-2025 by Artsiom Marozau, Pavel Kisliak                    *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "gtest/gtest.h"
#include "common_test_entities.h"

/**
 * @brief Tests loading YAML data from an encoded stream with or without a byte order mark (BOM).
 *
 * This test ensures that the archive can correctly parse YAML input regardless of whether a BOM is present.
 *
 * @tparam TArchive The archive type used for deserialization.
 * @tparam TUtfTraits UTF encoding traits specifying character type, endianness, and BOM.
 * @param withBom True if the stream should include a BOM; false otherwise.
 */
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

	// Simple UTF encoding (just for ANSI range)
	std::basic_string<char_type> temp(
		BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(std::begin(testAnsiYaml)),
		BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(std::end(testAnsiYaml)));
	if constexpr (TUtfTraits::endianness != BitSerializer::Memory::Endian::native) {
		BitSerializer::Memory::Reverse(std::begin(temp), std::end(temp));
	}
	sourceStr.append(reinterpret_cast<const char*>(temp.data()), temp.size() * sizeof(char_type));
	std::stringstream inputStream(sourceStr);

	// Act
	TestClassWithSubType<std::string> actual;
	BitSerializer::LoadObject<TArchive>(actual, inputStream);

	// Assert
	EXPECT_EQ("Hello world!", actual.GetValue()) << "Deserialized value does not match expected string.";
}

/**
 * @brief Tests saving YAML data to a stream with a specific encoding, with or without a byte order mark (BOM).
 *
 * Verifies correct output format and encoding behavior during serialization.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TUtfTraits UTF encoding traits specifying character type, endianness, and BOM.
 * @param withBom True if the stream should include a BOM; false otherwise.
 */
template <typename TArchive, typename TUtfTraits>
void TestSaveYamlToEncodedStream(const bool withBom)
{
	// Arrange
	using char_type = typename TUtfTraits::char_type;
	using string_type = std::basic_string<char_type, std::char_traits<char_type>>;
	static_assert(sizeof(TUtfTraits::bom) % sizeof(char_type) == 0,
		"Size of BOM must be a multiple of the character size.");

	const std::string expectedYamlInAnsi = "TestValue: Hello world!\n";
	const string_type expectedYaml(std::cbegin(expectedYamlInAnsi), std::cend(expectedYamlInAnsi));

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
		EXPECT_EQ(expectedBom, actualBom) << "Written BOM does not match expected BOM.";
	}
	// Simple UTF decoding (just for ANSI range)
	string_type actualYaml;
	const typename string_type::size_type targetCharCount = dataSize / sizeof(char_type);
	actualYaml.append(reinterpret_cast<const char_type*>(dataIt), targetCharCount);
	if constexpr (TUtfTraits::endianness != BitSerializer::Memory::Endian::native) {
		BitSerializer::Memory::Reverse(actualYaml.begin(), actualYaml.end());
	}

	// Ensure the serialized YAML matches the expected content
	EXPECT_EQ(expectedYaml, actualYaml) << "Serialized YAML content does not match expected output.";
}
