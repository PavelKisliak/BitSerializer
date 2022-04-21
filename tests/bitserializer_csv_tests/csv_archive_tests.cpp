/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "../test_helpers/common_test_methods.h"
#include "bitserializer/csv_archive.h"

using namespace BitSerializer;
using BitSerializer::Csv::CsvArchive;


//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(CsvArchive, SerializeArrayOfClasses)
{
	TestSerializeArray<CsvArchive, TestPointClass>();
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(CsvArchive, ShouldReturnPathInArrayScopeWhenLoading)
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

TEST(CsvArchive, ShouldReturnPathInArrayScopeWhenSaving)
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
TEST(CsvArchive, ShouldCollectErrorsAboutRequiredNamedValues)
{
	TestValidationForNamedValues<CsvArchive, TestClassForCheckValidation<int>>();
}

TEST(CsvArchive, ShouldCollectErrorsWhenLoadingFromNotCompatibleTypes)
{
	using SourceStringType = TestClassForCheckCompatibleTypes<std::string>;
	TestValidationForNotCompatibleTypes<CsvArchive, SourceStringType, TestClassForCheckCompatibleTypes<int>>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(CsvArchive, SerializeArrayToStream) {
	TestSerializeArrayToStream<CsvArchive, char, TestPointClass>();
}

//TEST(CsvArchive, SerializeUnicodeToEncodedStream) {
//	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
//	TestSerializeClassToStream<CsvArchive, char>(TestValue);
//}
//
//TEST(CsvArchive, LoadFromUtf8Stream) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf8>(false);
//}
//TEST(CsvArchive, LoadFromUtf8StreamWithBom) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf8>(true);
//}
//
//TEST(CsvArchive, LoadFromUtf16LeStream) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf16Le>(false);
//}
//TEST(CsvArchive, LoadFromUtf16LeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf16Le>(true);
//}
//
//TEST(CsvArchive, LoadFromUtf16BeStream) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf16Be>(false);
//}
//TEST(CsvArchive, LoadFromUtf16BeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf16Be>(true);
//}
//
//TEST(CsvArchive, LoadFromUtf32LeStream) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf32Le>(false);
//}
//TEST(CsvArchive, LoadFromUtf32LeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf32Le>(true);
//}
//
//TEST(CsvArchive, LoadFromUtf32BeStream) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf32Be>(false);
//}
//TEST(CsvArchive, LoadFromUtf32BeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<CsvArchive, BitSerializer::Convert::Utf32Be>(true);
//}
//
//TEST(CsvArchive, SaveToUtf8Stream) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf8>(false);
//}
//TEST(CsvArchive, SaveToUtf8StreamWithBom) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf8>(true);
//}
//
//TEST(CsvArchive, SaveToUtf16LeStream) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf16Le>(false);
//}
//TEST(CsvArchive, SaveToUtf16LeStreamWithBom) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf16Le>(true);
//}
//
//TEST(CsvArchive, SaveToUtf16BeStream) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf16Be>(false);
//}
//TEST(CsvArchive, SaveToUtf16BeStreamWithBom) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf16Be>(true);
//}
//
//TEST(CsvArchive, SaveToUtf32LeStream) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf32Le>(false);
//}
//TEST(CsvArchive, SaveToUtf32LeStreamWithBom) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf32Le>(true);
//}
//
//TEST(CsvArchive, SaveToUtf32BeStream) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf32Be>(false);
//}
//TEST(CsvArchive, SaveToUtf32BeStreamWithBom) {
//	TestSaveJsonToEncodedStream<CsvArchive, BitSerializer::Convert::Utf32Be>(true);
//}
//
//TEST(CsvArchive, ThrowExceptionWhenUnsupportedStreamEncoding)
//{
//	BitSerializer::SerializationOptions serializationOptions;
//	serializationOptions.streamOptions.encoding = static_cast<BitSerializer::Convert::UtfType>(-1);
//	std::stringstream outputStream;
//	auto testObj = BuildFixture<TestClassWithSubTypes<std::string>>();
//	EXPECT_THROW(BitSerializer::SaveObject<CsvArchive>(testObj, outputStream, serializationOptions), BitSerializer::SerializationException);
//}
//
//TEST(CsvArchive, SerializeClassToFile) {
//	TestSerializeClassToFile<CsvArchive>(BuildFixture<TestPointClass>());
//	TestSerializeClassToFile<CsvArchive>(BuildFixture<TestPointClass>());
//}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(CsvArchive, ThrowExceptionWhenBadSyntaxInSource)
{
	TestPointClass testList[1];
	EXPECT_THROW(BitSerializer::LoadObject<CsvArchive>(testList, "x,y\n10"), BitSerializer::SerializationException);
	EXPECT_THROW(BitSerializer::LoadObject<CsvArchive>(testList, "x\n10,20"), BitSerializer::SerializationException);
}
