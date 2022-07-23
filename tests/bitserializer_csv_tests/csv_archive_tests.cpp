/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "../test_helpers/common_test_methods.h"
#include "csv_archive_fixture.h"

using namespace BitSerializer;
using BitSerializer::Csv::CsvArchive;


//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeArrayOfClasses)
{
	TestSerializeArray<CsvArchive, TestPointClass>();
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ShouldReturnPathInArrayScopeWhenLoading)
{
	// Arrange
	TestPointClass testList[3];
	::BuildFixture(testList);

	std::string outputData;
	BitSerializer::SaveObject<CsvArchive>(testList, outputData);

	// Act / Assert
	CsvArchive::input_archive_type inputArchive(outputData);
	ASSERT_EQ(inputArchive.GetPath(), "");

	auto rootArrayScope = inputArchive.OpenArrayScope(3);
	ASSERT_TRUE(rootArrayScope.has_value());

	for (size_t k = 0; k < 3; k++)
	{
		auto objectScope = rootArrayScope->OpenObjectScope();
		ASSERT_TRUE(objectScope.has_value());

		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), rootArrayScope->GetPath());
		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), objectScope->GetPath());
	}
}

TEST_F(CsvArchiveTests, ShouldReturnPathInArrayScopeWhenSaving)
{
	// Arrange
	TestPointClass testList[3];
	::BuildFixture(testList);

	std::string outputData;
	CsvArchive::output_archive_type outputArchive(outputData);

	// Act / Assert
	auto rootArrayScope = outputArchive.OpenArrayScope(3);
	ASSERT_TRUE(rootArrayScope.has_value());
	for (size_t k = 0; k < 3; k++)
	{
		auto objectScope = rootArrayScope->OpenObjectScope();
		ASSERT_TRUE(objectScope.has_value());

		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), rootArrayScope->GetPath());
		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), objectScope->GetPath());
	}
}

//-----------------------------------------------------------------------------
// Test the validation for named values (boolean result, which returns by archive's method SerializeValue()).
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ShouldCollectErrorsAboutRequiredNamedValues)
{
	TestValidationForNamedValues<CsvArchive, TestClassForCheckValidation<int>>();
}

TEST_F(CsvArchiveTests, ShouldCollectErrorsWhenLoadingFromNotCompatibleTypes)
{
	using SourceStringType = TestClassForCheckCompatibleTypes<std::string>;
	TestValidationForNotCompatibleTypes<CsvArchive, SourceStringType, TestClassForCheckCompatibleTypes<int>>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeArrayToStream)
{
	TestClassWithSubType<std::string> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<CsvArchive, char>(testArray);
}

TEST_F(CsvArchiveTests, SerializeUnicodeToEncodedStream) {
	TestClassWithSubType<std::wstring> TestArray[1] = { TestClassWithSubType<std::wstring>(L"Привет мир!") };
	TestSerializeArrayToStream<CsvArchive, char>(TestArray);
}

TEST_F(CsvArchiveTests, LoadFromUtf8Stream) {
	TestLoadCsvFromEncodedStream<Convert::Utf8>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf8StreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf8>(true);
}
TEST_F(CsvArchiveTests, LoadFromUtf16LeStream) {
	TestLoadCsvFromEncodedStream<Convert::Utf16Le>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf16LeStreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf16Le>(true);
}

TEST_F(CsvArchiveTests, LoadFromUtf16BeStream) {
	TestLoadCsvFromEncodedStream<Convert::Utf16Be>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf16BeStreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf16Be>(true);
}

TEST_F(CsvArchiveTests, LoadFromUtf32LeStream) {
	TestLoadCsvFromEncodedStream<Convert::Utf32Le>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf32LeStreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf32Le>(true);
}

TEST_F(CsvArchiveTests, LoadFromUtf32BeStream) {
	TestLoadCsvFromEncodedStream<Convert::Utf32Be>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf32BeStreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf32Be>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf8Stream) {
	TestSaveCsvToEncodedStream<Convert::Utf8>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf8StreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf8>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf16LeStream) {
	TestSaveCsvToEncodedStream<Convert::Utf16Le>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf16LeStreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf16Le>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf16BeStream) {
	TestSaveCsvToEncodedStream<Convert::Utf16Be>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf16BeStreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf16Be>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf32LeStream) {
	TestSaveCsvToEncodedStream<Convert::Utf32Le>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf32LeStreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf32Le>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf32BeStream) {
	TestSaveCsvToEncodedStream<Convert::Utf32Be>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf32BeStreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf32Be>(true);
}

TEST_F(CsvArchiveTests, SerializeToFile) {
	TestSerializeArrayToFile<CsvArchive>();
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowExceptionWhenBadSyntaxInSource)
{
	TestPointClass testList[1];
	EXPECT_THROW(BitSerializer::LoadObject<CsvArchive>(testList, "x,y\n10"), BitSerializer::SerializationException);
	EXPECT_THROW(BitSerializer::LoadObject<CsvArchive>(testList, "x\n10,20"), BitSerializer::SerializationException);
}
