/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "bitserializer/csv_archive.h"
#include "csv_reader_fixture.h"


using testing::Types;
typedef Types<BitSerializer::Csv::Detail::CCsvStringReader, BitSerializer::Csv::Detail::CCsvStreamReader> Implementations;

// Tests for all implementations of ICsvReader
TYPED_TEST_SUITE(CsvReaderTest, Implementations, );

//------------------------------------------------------------------------------


TYPED_TEST(CsvReaderTest, ShouldReturnZeroCurrentIndexAtTheBeginning)
{
	// Arrange
	const std::string csv = "Value1,Value2";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	EXPECT_EQ(0, this->mCsvReader->GetCurrentIndex());
}

TYPED_TEST(CsvReaderTest, ShouldNoParseWhenInputStringIsEmpty)
{
	// Arrange
	const std::string csv;
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	EXPECT_FALSE(this->mCsvReader->ParseNextRow());
}

TYPED_TEST(CsvReaderTest, ShouldReturnHeadersList)
{
	// Arrange
	const std::string csv = R"(Column1,Column2,Column3)";
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	ASSERT_EQ(3, this->mCsvReader->GetHeaders().size());
	EXPECT_EQ("Column1", this->mCsvReader->GetHeaders()[0]);
	EXPECT_EQ("Column2", this->mCsvReader->GetHeaders()[1]);
	EXPECT_EQ("Column3", this->mCsvReader->GetHeaders()[2]);
}

TYPED_TEST(CsvReaderTest, ShouldReturnCurrentIndexWhenUsedHeader)
{
	// Arrange
	const std::string csv = R"(Column1,Column2,Column3
Value1,Value2,Value3
Value1,Value2,Value3
)";
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	EXPECT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_EQ(0, this->mCsvReader->GetCurrentIndex());
	EXPECT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_EQ(1, this->mCsvReader->GetCurrentIndex());
}

TYPED_TEST(CsvReaderTest, ShouldReturnCurrentIndexWhenHeaderisNotUsed)
{
	// Arrange
	const std::string csv = R"(Value1,Value2
Value1,Value2
)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	EXPECT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_EQ(0, this->mCsvReader->GetCurrentIndex());
	EXPECT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_EQ(1, this->mCsvReader->GetCurrentIndex());
}

TYPED_TEST(CsvReaderTest, ShouldReturnFalseWhenNotAllLinesParsed)
{
	// Arrange
	const std::string csv = R"(Column1,Column2,Column3
Value1,Value2,Value3
)";
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	EXPECT_FALSE(this->mCsvReader->IsEnd());
}

TYPED_TEST(CsvReaderTest, ShouldReturnTrueWhenFileIsEmpty)
{
	// Arrange
	this->PrepareCsvReader("", false);

	// Act / Assert
	EXPECT_TRUE(this->mCsvReader->IsEnd());
}

TYPED_TEST(CsvReaderTest, ShouldThrowExceptionWhenExpectedHeaderButFileIsEmpty)
{
	// Arrange / Act / Assert
	EXPECT_THROW(this->PrepareCsvReader("", true), BitSerializer::ParsingException);
}

TYPED_TEST(CsvReaderTest, ShouldReturnTrueWhenEndOfFile)
{
	// Arrange
	const std::string csv = R"(Value1,Value2
)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	EXPECT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_TRUE(this->mCsvReader->IsEnd());
}

TYPED_TEST(CsvReaderTest, ShouldReturnTrueWhenEndOfFileWithoutLastCrLf)
{
	// Arrange
	const std::string csv = "Value1,Value2";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	EXPECT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_TRUE(this->mCsvReader->IsEnd());
}

TYPED_TEST(CsvReaderTest, ShouldReturnTrueWhenSuccessParsedRow)
{
	// Arrange
	const std::string csv = R"(Column1
Value1
)";
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	EXPECT_TRUE(this->mCsvReader->ParseNextRow());
}

TYPED_TEST(CsvReaderTest, ShouldReturnFalseWhenNoMoreRows)
{
	// Arrange
	const std::string csv = R"(Column1
Row1
)";
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	EXPECT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_FALSE(this->mCsvReader->ParseNextRow());
}

TYPED_TEST(CsvReaderTest, ShouldReturnFalseWhenNoAnyRows)
{
	// Arrange
	const std::string csv = R"(Column1)";
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	EXPECT_FALSE(this->mCsvReader->ParseNextRow());
}

TYPED_TEST(CsvReaderTest, ShouldReadValueByHeaderName)
{
	// Arrange
	const std::string csv = R"(Column1,Column2,Column3
Value1,Value2,Value3
)";
	this->PrepareCsvReader(csv, true);

	// Act
	std::string_view actual;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_TRUE(this->mCsvReader->ReadValue("Column2", actual));

	// Assert
	EXPECT_EQ("Value2", actual);
}

TYPED_TEST(CsvReaderTest, ShouldParseWithCustomSeparator)
{
	// Arrange
	const std::string csv = R"(Column1;Column2;Column3
Value1;Value2;Value3
)";
	this->PrepareCsvReader(csv, true, ';');

	// Act
	std::string_view actual;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_TRUE(this->mCsvReader->ReadValue("Column2", actual));

	// Assert
	EXPECT_EQ("Value2", actual);
}

TYPED_TEST(CsvReaderTest, ShouldParseWithSpaceAsCustomSeparator)
{
	// Arrange
	const std::string csv = R"(Column1 Column2 Column3
Value1 Value2 Value3
)";
	this->PrepareCsvReader(csv, true, ' ');

	// Act
	std::string_view actual;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_TRUE(this->mCsvReader->ReadValue("Column2", actual));

	// Assert
	EXPECT_EQ("Value2", actual);
}

TYPED_TEST(CsvReaderTest, ShouldReadValuesWithoutHeaders)
{
	// Arrange
	const std::string csv = R"(Value1,Value2)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value1, value2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(value1);
	EXPECT_EQ("Value1", value1);
	this->mCsvReader->ReadValue(value2);
	EXPECT_EQ("Value2", value2);
}

TYPED_TEST(CsvReaderTest, ShouldParseMultipleRowsWithHeader)
{
	// Arrange
	const std::string csv = R"(Column1,Column2
Row1Col1,Row1Col2
Row2Col1,Row2Col2
)";
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	std::string_view row1col1, row2col2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_TRUE(this->mCsvReader->ReadValue("Column1", row1col1));
	EXPECT_EQ("Row1Col1", row1col1);

	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue("Column2", row2col2);
	EXPECT_EQ("Row2Col2", row2col2);
}

TYPED_TEST(CsvReaderTest, ShouldParseMultipleRowsWithoutHeader)
{
	// Arrange
	const std::string csv = R"(Row1Col1,Row1Col2
Row2Col1,Row2Col2
)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view row1col1, row2col1, row2col2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row1col1);
	EXPECT_EQ("Row1Col1", row1col1);
	
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row2col1);
	this->mCsvReader->ReadValue(row2col2);
	EXPECT_EQ("Row2Col1", row2col1);
	EXPECT_EQ("Row2Col2", row2col2);
}

TYPED_TEST(CsvReaderTest, ShouldParseRowsWithEmptyValues)
{
	// Arrange
	const std::string csv = "Row1\n\n\nRow4\n";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view row1, row2, row3, row4;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row1);
	EXPECT_EQ("Row1", row1);

	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row2);
	EXPECT_EQ("", row2);

	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row3);
	EXPECT_EQ("", row3);

	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row4);
	EXPECT_EQ("Row4", row4);

	EXPECT_FALSE(this->mCsvReader->ParseNextRow());
}

TYPED_TEST(CsvReaderTest, ShouldParseLastRowWithEmptyValue)
{
	// Arrange
	const std::string csv = "Row1\n\n";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view row1, row2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row1);
	EXPECT_EQ("Row1", row1);

	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row2);
	EXPECT_EQ("", row2);

	EXPECT_FALSE(this->mCsvReader->ParseNextRow());
}

TYPED_TEST(CsvReaderTest, ShouldParseRowWithoutLastLfCode)
{
	// Arrange
	const std::string csv = "Column1\r\nValue1";
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	std::string_view value1;
	this->mCsvReader->ReadValue("Column1", value1);
	EXPECT_EQ("Value1", value1);
}

TYPED_TEST(CsvReaderTest, ShouldParseRowsWithCrLfCodes)
{
	// Arrange
	const std::string csv = "Row1\r\nRow2\r\n";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view row1, row2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row1);
	EXPECT_EQ("Row1", row1);

	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row2);
	EXPECT_EQ("Row2", row2);
}

TYPED_TEST(CsvReaderTest, ShouldParseRowsWithOnlyLfCode)
{
	// Arrange
	const std::string csv = "Row1\nRow2\n";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view row1, row2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row1);
	EXPECT_EQ("Row1", row1);
	
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(row2);
	EXPECT_EQ("Row2", row2);
}

TYPED_TEST(CsvReaderTest, ShouldReadQuotedValues)
{
	// Arrange
	const std::string csv = R"("Quoted:1,2,3,4,5",Value2)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value1, value2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(value1);
	EXPECT_EQ("Quoted:1,2,3,4,5", value1);
	this->mCsvReader->ReadValue(value2);
	EXPECT_EQ("Value2", value2);
}

TYPED_TEST(CsvReaderTest, ShouldReadEmptyQuotedValues)
{
	// Arrange
	const std::string csv = R"("","")";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value1, value2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(value1);
	EXPECT_EQ("", value1);
	this->mCsvReader->ReadValue(value2);
	EXPECT_EQ("", value2);
}

TYPED_TEST(CsvReaderTest, ShouldReadMultiplyQuotedValues)
{
	// Arrange
	const std::string csv = R"("Quoted value 1","Quoted value 2","Quoted value 3")";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value1, value2, value3;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(value1);
	EXPECT_EQ("Quoted value 1", value1);
	this->mCsvReader->ReadValue(value2);
	EXPECT_EQ("Quoted value 2", value2);
	this->mCsvReader->ReadValue(value3);
	EXPECT_EQ("Quoted value 3", value3);
}

TYPED_TEST(CsvReaderTest, ShouldReadQuotedLineBreaksInValue)
{
	// Arrange
	const std::string csv = "Value1,\"Multi\r\nline\nvalue2\"";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value1, value2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(value1);
	EXPECT_EQ("Value1", value1);
	this->mCsvReader->ReadValue(value2);
	EXPECT_EQ("Multi\r\nline\nvalue2", value2);
}

TYPED_TEST(CsvReaderTest, ShouldReadEscapedQuotesInValue)
{
	// Arrange
	const std::string csv = R"("Quoted:""1,2,3,4,5""",Value2)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value1, value2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(value1);
	EXPECT_EQ(R"(Quoted:"1,2,3,4,5")", value1);
	this->mCsvReader->ReadValue(value2);
	EXPECT_EQ("Value2", value2);
}

TYPED_TEST(CsvReaderTest, ShouldReadLargeValues)
{
	// Arrange
	constexpr size_t TestValSize = 10000;
	std::string expectedVal1(TestValSize, '_'), expectedVal2(TestValSize, '_');
	for (size_t i = 0; i < TestValSize; ++i) {
		expectedVal1[i] = static_cast<char>('A' + i % 26);
		expectedVal2[i] = static_cast<char>('a' + i % 26);
	}
	const std::string csv = "Column1,Column2\r\n" + expectedVal1 + "," + expectedVal2;
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	std::string_view value1, value2;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue("Column1", value1);
	EXPECT_EQ(expectedVal1, value1);
	this->mCsvReader->ReadValue("Column2", value2);
	EXPECT_EQ(expectedVal2, value2);
}

TYPED_TEST(CsvReaderTest, ShouldThrowExceptionWhenReadMoreValuesThanExistsInRow)
{
	// Arrange
	const std::string csv = "Value1,Value2";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	this->mCsvReader->ReadValue(value);
	this->mCsvReader->ReadValue(value);
	EXPECT_THROW(this->mCsvReader->ReadValue(value), BitSerializer::SerializationException);
}

TYPED_TEST(CsvReaderTest, ShouldThrowExceptionWhenOnlyOneDoubleQuotes)
{
	// Arrange
	const std::string csv = R"(")";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value;
	ASSERT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_THROW(this->mCsvReader->ReadValue(value), BitSerializer::ParsingException);
}

TYPED_TEST(CsvReaderTest, ShouldThrowExceptionWhenMissedStartQuotes)
{
	// Arrange
	const std::string csv = R"(Value1",Value2)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value;
	this->mCsvReader->ParseNextRow();
	EXPECT_THROW(this->mCsvReader->ReadValue(value), BitSerializer::ParsingException);
}

TYPED_TEST(CsvReaderTest, ShouldThrowExceptionWhenDoubleQuotesIsNotRightAfterSeparator)
{
	// Arrange
	const std::string csv = R"(Value1, "Value2")";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value;
	this->mCsvReader->ParseNextRow();
	this->mCsvReader->ReadValue(value);
	EXPECT_THROW(this->mCsvReader->ReadValue(value), BitSerializer::ParsingException);
}

TYPED_TEST(CsvReaderTest, ShouldThrowExceptionWhenNotEscapedDoubleQuotes)
{
	// Arrange
	const std::string csv = R"("Value1,Value"2)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value;
	this->mCsvReader->ParseNextRow();
	EXPECT_THROW(this->mCsvReader->ReadValue(value), BitSerializer::ParsingException);
}

TYPED_TEST(CsvReaderTest, ShouldThrowExceptionWhenMissedEndQuotes)
{
	// Arrange
	const std::string csv = R"("Value1,Value2)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	std::string_view value;
	this->mCsvReader->ParseNextRow();
	EXPECT_THROW(this->mCsvReader->ReadValue(value), BitSerializer::ParsingException);
}

TYPED_TEST(CsvReaderTest, ShouldThrowExceptionWhenMismatchNumberOfHeadersAndValues)
{
	// Arrange
	const std::string csv = R"(Column1,Column2
Value1,Value2,Value3
)";
	this->PrepareCsvReader(csv, true);

	// Act / Assert
	EXPECT_THROW(this->mCsvReader->ParseNextRow(), BitSerializer::ParsingException);
}

TYPED_TEST(CsvReaderTest, ShouldThrowExceptionWhenMismatchNumberOfValuesInRows)
{
	// Arrange
	const std::string csv = R"(Value1,Value2,Value3
Value1,Value2,Value3,Value4
)";
	this->PrepareCsvReader(csv, false);

	// Act / Assert
	EXPECT_TRUE(this->mCsvReader->ParseNextRow());
	EXPECT_THROW(this->mCsvReader->ParseNextRow(), BitSerializer::ParsingException);
}
