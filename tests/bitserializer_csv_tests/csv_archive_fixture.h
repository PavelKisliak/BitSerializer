/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "gtest/gtest.h"
#include "bitserializer/csv_archive.h"


class CsvArchiveTests : public ::testing::Test
{
protected:
	/// <summary>
	/// Tests loading CSV from encoded stream with/without writing BOM.
	/// </summary>
	template <typename TUtfTraits>
	void TestLoadCsvFromEncodedStream(const bool withBom)
	{
		// Arrange
		using char_type = typename TUtfTraits::char_type;
		constexpr std::string_view testAnsiCsv = "TestValue\r\nHello world!";
		std::string sourceStr;
		// Add Bom (if required)
		if (withBom) {
			sourceStr.append(std::cbegin(TUtfTraits::bom), std::cend(TUtfTraits::bom));
		}

		// Simple UTF encoding (just for ANSI range)
		std::basic_string<typename TUtfTraits::char_type> temp(BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(begin(testAnsiCsv)), BitSerializer::Memory::MakeIteratorAdapter<BitSerializer::Memory::Endian::native>(cend(testAnsiCsv)));
		if constexpr (TUtfTraits::endianness != BitSerializer::Memory::Endian::native) {
			BitSerializer::Memory::Reverse(begin(temp), end(temp));
		}
		sourceStr.append(reinterpret_cast<const char*>(temp.data()), temp.size() * sizeof(typename TUtfTraits::char_type));
		std::stringstream inputStream(sourceStr);

		// Act
		TestClassWithSubType<std::string> actual[1];
		BitSerializer::LoadObject<BitSerializer::Csv::CsvArchive>(actual, inputStream);

		// Assert
		EXPECT_EQ("Hello world!", actual[0].GetValue());
	}

	/// <summary>
	/// Tests saving CSV to encoded stream with/without writing BOM.
	/// </summary>
	template <typename TUtfTraits>
	void TestSaveCsvToEncodedStream(const bool withBom)
	{
		// Arrange
		using char_type = typename TUtfTraits::char_type;
		using string_type = std::basic_string<char_type, std::char_traits<char_type>>;
		static_assert(sizeof(TUtfTraits::bom) % sizeof(char_type) == 0);

		const std::string expectedCsvInAnsi = "TestValue\r\nHello world!\r\n";
		const string_type expectedCsv(std::cbegin(expectedCsvInAnsi), std::cend(expectedCsvInAnsi));

		std::stringstream outputStream;
		TestClassWithSubType<std::string> testObj[1] = { TestClassWithSubType<std::string>("Hello world!") };
		BitSerializer::SerializationOptions serializationOptions;
		serializationOptions.streamOptions.writeBom = withBom;
		serializationOptions.streamOptions.encoding = TUtfTraits::utfType;

		// Act
		BitSerializer::SaveObject<BitSerializer::Csv::CsvArchive>(testObj, outputStream, serializationOptions);

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
		EXPECT_EQ(expectedCsv, actualJson);
	}
};
