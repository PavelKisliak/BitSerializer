/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "bitserializer/csv_archive.h"
#include "csv_writer_fixture.h"


using testing::Types;
typedef Types<BitSerializer::Csv::Detail::CCsvStringWriter, BitSerializer::Csv::Detail::CCsvStreamWriter> Implementations;

// Tests for all implementations of ICsvWriter
TYPED_TEST_SUITE(CsvWriterTest, Implementations, );

//------------------------------------------------------------------------------

TYPED_TEST(CsvWriterTest, ShouldWriteHeaderWithValues)
{
	// Arrange
	this->PrepareCsvReader(true);

	// Act
	this->mCsvWriter->WriteValue("Name1", "12");
	this->mCsvWriter->WriteValue("Name2", "512");
	this->mCsvWriter->NextLine();

	// Assert
	const std::string expectedCsv = "Name1,Name2\r\n12,512\r\n";
	EXPECT_EQ(expectedCsv, this->GetResult());
}

TYPED_TEST(CsvWriterTest, ShouldSkipHeaderWhenItDisabled)
{
	// Arrange
	this->PrepareCsvReader(false);

	// Act
	this->mCsvWriter->WriteValue("Name1", "100");
	this->mCsvWriter->WriteValue("Name2", "5");
	this->mCsvWriter->NextLine();

	// Assert
	const std::string expectedCsv = "100,5\r\n";
	EXPECT_EQ(expectedCsv, this->GetResult());
}

TYPED_TEST(CsvWriterTest, ShouldWriteWithCustomSeparator)
{
	// Arrange
	this->PrepareCsvReader(true,';');

	// Act
	this->mCsvWriter->WriteValue("Name1", "Value1");
	this->mCsvWriter->WriteValue("Name2", "Value2");
	this->mCsvWriter->NextLine();

	// Assert
	const std::string expectedCsv = "Name1;Name2\r\nValue1;Value2\r\n";
	EXPECT_EQ(expectedCsv, this->GetResult());
}

TYPED_TEST(CsvWriterTest, ShouldWriteWithSpaceAsCustomSeparator)
{
	// Arrange
	this->PrepareCsvReader(true, ' ');

	// Act
	this->mCsvWriter->WriteValue("Name1", "Value1");
	this->mCsvWriter->WriteValue("Name2", "Value2");
	this->mCsvWriter->NextLine();

	// Assert
	const std::string expectedCsv = "Name1 Name2\r\nValue1 Value2\r\n";
	EXPECT_EQ(expectedCsv, this->GetResult());
}

TYPED_TEST(CsvWriterTest, ShouldWriteWithQuotedSeparator)
{
	// Arrange
	this->PrepareCsvReader(true, ',');

	// Act
	this->mCsvWriter->WriteValue("Name,1", "1,2,3");
	this->mCsvWriter->NextLine();

	// Assert
	const std::string expectedCsv = "\"Name,1\"\r\n\"1,2,3\"\r\n";
	EXPECT_EQ(expectedCsv, this->GetResult());
}

TYPED_TEST(CsvWriterTest, ShouldWriteWithQuotedLineBreak)
{
	// Arrange
	this->PrepareCsvReader(true);

	// Act
	this->mCsvWriter->WriteValue("Column\r\nName", "multi\nline");
	this->mCsvWriter->NextLine();

	// Assert
	const std::string expectedCsv = "\"Column\r\nName\"\r\n\"multi\nline\"\r\n";
	EXPECT_EQ(expectedCsv, this->GetResult());
}

TYPED_TEST(CsvWriterTest, ShouldWriteWithEscapingDoubleQuote)
{
	// Arrange
	this->PrepareCsvReader(true);

	// Act
	this->mCsvWriter->WriteValue("Column\"Name", "1\"2");
	this->mCsvWriter->NextLine();

	// Assert
	const std::string expectedCsv = "\"Column\"\"Name\"\r\n\"1\"\"2\"\r\n";
	EXPECT_EQ(expectedCsv, this->GetResult());
}

TYPED_TEST(CsvWriterTest, ShouldWriteLargeValues)
{
	// Arrange
	constexpr size_t TestValSize = 10000;
	std::string val1(TestValSize, '_'), val2(TestValSize, '_');
	for (size_t i = 0; i < TestValSize; ++i) {
		val1[i] = static_cast<char>('A' + i % 26);
		val2[i] = static_cast<char>('a' + i % 26);
	}
	this->PrepareCsvReader(true);

	// Act
	this->mCsvWriter->WriteValue("Column1", val1);
	this->mCsvWriter->WriteValue("Column2", val2);
	this->mCsvWriter->NextLine();

	// Assert
	const std::string expectedCsv = "Column1,Column2\r\n" + val1 + ',' + val2 + "\r\n";
	EXPECT_EQ(expectedCsv, this->GetResult());
}

TYPED_TEST(CsvWriterTest, ShouldReturnZeroCurrentIndexAtTheBeginning)
{
	// Arrange
	this->PrepareCsvReader(true);

	// Act / Assert
	EXPECT_EQ(0U, this->mCsvWriter->GetCurrentIndex());
}

TYPED_TEST(CsvWriterTest, ShouldReturnCurrentIndexWhenUsedHeader)
{
	// Arrange
	this->PrepareCsvReader(true);

	// Act / Assert
	this->mCsvWriter->WriteValue("Name1", "Value1");
	this->mCsvWriter->NextLine();
	EXPECT_EQ(1U, this->mCsvWriter->GetCurrentIndex());
	this->mCsvWriter->WriteValue("Name1", "Value1");
	this->mCsvWriter->NextLine();
	EXPECT_EQ(2U, this->mCsvWriter->GetCurrentIndex());
}

TYPED_TEST(CsvWriterTest, ShouldReturnCurrentIndexWhenHeaderisNotUsed)
{
	// Arrange
	this->PrepareCsvReader(false);

	// Act / Assert
	this->mCsvWriter->WriteValue("Name1", "Value1");
	this->mCsvWriter->NextLine();
	EXPECT_EQ(1U, this->mCsvWriter->GetCurrentIndex());
	this->mCsvWriter->WriteValue("Name1", "Value1");
	this->mCsvWriter->NextLine();
	EXPECT_EQ(2U, this->mCsvWriter->GetCurrentIndex());
}

TYPED_TEST(CsvWriterTest, ShouldThrowExceptionWhenMismatchNumberOfValuesInRows)
{
	// Arrange
	this->PrepareCsvReader(false);

	// Act / Assert
	this->mCsvWriter->WriteValue("Name1", "1");
	this->mCsvWriter->NextLine();
	this->mCsvWriter->WriteValue("Name1", "10");
	this->mCsvWriter->WriteValue("Name2", "100");
	EXPECT_THROW(this->mCsvWriter->NextLine(), BitSerializer::SerializationException);
}

TYPED_TEST(CsvWriterTest, ShouldWriteBomWhenOutputToStream)
{
	// Arrange
	this->PrepareCsvReader(true, ',', BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip, true);

	// Act / Assert
	this->mCsvWriter->WriteValue("Name1", "Value1");
	this->mCsvWriter->NextLine();

	// Assert
	const std::string expectedCsv = this->IsStreamWriter()
		? "\xEF\xBB\xBFName1\r\nValue1\r\n"
		: "Name1\r\nValue1\r\n";
	EXPECT_EQ(expectedCsv, this->GetResult());
}
