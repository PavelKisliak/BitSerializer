/*******************************************************************************
* Copyright (C) 2020-2024 by Artsiom Marozau, Pavel Kisliak                    *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "gtest/gtest.h"
#include "common_test_entities.h"

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

	// Simple UTF encoding (just for ANSI range)
	std::basic_string<char_type> temp(BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(begin(testAnsiYaml)), BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(cend(testAnsiYaml)));
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
	// Simple UTF decoding (just for ANSI range)
	string_type actualYaml;
	const typename string_type::size_type targetCharCount = dataSize / sizeof(char_type);
	actualYaml.append(reinterpret_cast<const char_type*>(dataIt), dataSize / sizeof(char_type));
	if constexpr (TUtfTraits::endianness != BitSerializer::Memory::Endian::native) {
		BitSerializer::Memory::Reverse(actualYaml.begin(), actualYaml.end());
	}
	// Test Yaml
	EXPECT_EQ(expectedYaml, actualYaml);
}
