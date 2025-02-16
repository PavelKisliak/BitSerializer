/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "csv_archive_fixture.h"

using namespace BitSerializer;
using BitSerializer::Csv::CsvArchive;


//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeArrayOfClasses)
{
	TestSerializeArray<CsvArchive, TestPointClass>();
	TestSerializeArray<CsvArchive, TestClassWithSubTypes<bool, int, std::string>>();
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
	SerializationOptions options;
	SerializationContext context(options);
	CsvArchive::input_archive_type inputArchive(outputData, context);
	ASSERT_EQ(inputArchive.GetPath(), "");

	auto rootArrayScope = inputArchive.OpenArrayScope(3);
	ASSERT_TRUE(rootArrayScope.has_value());

	for (size_t k = 0; k < 3; k++)
	{
		auto objectScope = rootArrayScope->OpenObjectScope(0);  // NOLINT(bugprone-unchecked-optional-access)
		ASSERT_TRUE(objectScope.has_value());

		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), rootArrayScope->GetPath());  // NOLINT(bugprone-unchecked-optional-access)
		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), objectScope->GetPath());  // NOLINT(bugprone-unchecked-optional-access)
	}
}

TEST_F(CsvArchiveTests, ShouldReturnPathInArrayScopeWhenSaving)
{
	// Arrange
	TestPointClass testList[3];
	::BuildFixture(testList);

	std::string outputData;
	SerializationOptions options;
	SerializationContext context(options);
	CsvArchive::output_archive_type outputArchive(outputData, context);

	// Act / Assert
	auto rootArrayScope = outputArchive.OpenArrayScope(3);
	ASSERT_TRUE(rootArrayScope.has_value());
	for (size_t k = 0; k < 3; k++)
	{
		auto objectScope = rootArrayScope->OpenObjectScope(0);  // NOLINT(bugprone-unchecked-optional-access)
		ASSERT_TRUE(objectScope.has_value());

		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), rootArrayScope->GetPath());  // NOLINT(bugprone-unchecked-optional-access)
		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), objectScope->GetPath());  // NOLINT(bugprone-unchecked-optional-access)
	}
}

TEST_F(CsvArchiveTests, ShouldVisitKeysInObjectScopeWhenReadValues)
{
	TestVisitKeysInObjectScope<CsvArchive>();
}

TEST_F(CsvArchiveTests, ShouldVisitKeysInObjectScopeWhenSkipValues)
{
	TestVisitKeysInObjectScope<CsvArchive>(true);
}

TEST_F(CsvArchiveTests, SerializeClassInReverseOrder)
{
	TestSerializeArray<CsvArchive, TestClassWithReverseLoad<bool, int, std::string>>();
}

TEST_F(CsvArchiveTests, SerializeClassWithSkippingFields)
{
	TestClassWithVersioning arrayOfObjects[3];
	BuildFixture(arrayOfObjects);
	TestSerializeType<CsvArchive>(arrayOfObjects);
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeArrayOfClassesToStream)
{
	TestClassWithSubTypes<int, double, std::string> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<CsvArchive>(testArray);
}

TEST_F(CsvArchiveTests, SerializeUnicodeToEncodedStream) {
	TestClassWithSubType<std::wstring> TestArray[1] = { TestClassWithSubType<std::wstring>(L"Привет мир!") };
	TestSerializeArrayToStream<CsvArchive>(TestArray);
}

TEST_F(CsvArchiveTests, LoadFromUtf8Stream) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf8>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf8StreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf8>(true);
}
TEST_F(CsvArchiveTests, LoadFromUtf16LeStream) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf16Le>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf16LeStreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf16Le>(true);
}

TEST_F(CsvArchiveTests, LoadFromUtf16BeStream) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf16Be>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf16BeStreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf16Be>(true);
}

TEST_F(CsvArchiveTests, LoadFromUtf32LeStream) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf32Le>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf32LeStreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf32Le>(true);
}

TEST_F(CsvArchiveTests, LoadFromUtf32BeStream) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf32Be>(false);
}
TEST_F(CsvArchiveTests, LoadFromUtf32BeStreamWithBom) {
	TestLoadCsvFromEncodedStream<Convert::Utf::Utf32Be>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf8Stream) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf8>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf8StreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf8>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf16LeStream) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf16Le>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf16LeStreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf16Le>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf16BeStream) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf16Be>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf16BeStreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf16Be>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf32LeStream) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf32Le>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf32LeStreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf32Le>(true);
}

TEST_F(CsvArchiveTests, SaveToUtf32BeStream) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf32Be>(false);
}
TEST_F(CsvArchiveTests, SaveToUtf32BeStreamWithBom) {
	TestSaveCsvToEncodedStream<Convert::Utf::Utf32Be>(true);
}

TEST_F(CsvArchiveTests, SerializeToFile) {
	TestSerializeArrayToFile<CsvArchive>();
	TestSerializeArrayToFile<CsvArchive>(true);
}

TEST_F(CsvArchiveTests, SerializeToFileThrowExceptionWhenAlreadyExists) {
	TestThrowExceptionWhenFileAlreadyExists<CsvArchive>();
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowParsingExceptionWhenBadSyntaxInSource)
{
	TestPointClass testList[1];
	EXPECT_THROW(BitSerializer::LoadObject<CsvArchive>(testList, "x,y\n10"), BitSerializer::ParsingException);
	EXPECT_THROW(BitSerializer::LoadObject<CsvArchive>(testList, "x\n10,20"), BitSerializer::ParsingException);
}

TEST_F(CsvArchiveTests, ThrowParsingExceptionWithCorrectPosition)
{
	TestPointClass testList[2];
	try
	{
		constexpr char testCsv[] = "x,y\n10,20\n11,\"21\n";
		BitSerializer::LoadObject<CsvArchive>(testList, testCsv);
		EXPECT_FALSE(true);
	}
	catch (const ParsingException& ex)
	{
		EXPECT_EQ(3, ex.Line);
	}
	catch (const std::exception&)
	{
		EXPECT_FALSE(true);
	}
}

TEST_F(CsvArchiveTests, ThrowExceptionWhenUnsupportedSeparator)
{
	SerializationOptions options;
	options.valuesSeparator = '+';
	TestPointClass testList[1];
	EXPECT_THROW(BitSerializer::LoadObject<CsvArchive>(testList, "x+y\n10+20", options), BitSerializer::SerializationException);
}

//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenMissedRequiredValue) {
	TestValidationForNamedValues<CsvArchive, TestClassForCheckValidation<int>>();
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowMismatchedTypesExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<CsvArchive, std::string, bool>(MismatchedTypesPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<CsvArchive, std::string, int32_t>(MismatchedTypesPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<CsvArchive, std::string, float>(MismatchedTypesPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<CsvArchive, float, uint32_t>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<CsvArchive, double, uint32_t>(MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::Skip
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<CsvArchive, std::string, bool>(MismatchedTypesPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<CsvArchive, std::string, int32_t>(MismatchedTypesPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<CsvArchive, std::string, float>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<CsvArchive, std::string, double>(MismatchedTypesPolicy::Skip);
}

TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<CsvArchive, float, uint32_t>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<CsvArchive, double, uint32_t>(MismatchedTypesPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedTypesPolicy<CsvArchive, std::nullptr_t, bool>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<CsvArchive, std::nullptr_t, uint32_t>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<CsvArchive, std::nullptr_t, double>(MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<CsvArchive, int32_t, bool>(OverflowNumberPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenOverflowInt8) {
	TestOverflowNumberPolicy<CsvArchive, int16_t, int8_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<CsvArchive, uint16_t, uint8_t>(OverflowNumberPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenOverflowInt16) {
	TestOverflowNumberPolicy<CsvArchive, int32_t, int16_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<CsvArchive, uint32_t, uint16_t>(OverflowNumberPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenOverflowInt32) {
	TestOverflowNumberPolicy<CsvArchive, int64_t, int32_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<CsvArchive, uint64_t, uint32_t>(OverflowNumberPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenOverflowFloat) {
	TestOverflowNumberPolicy<CsvArchive, double, float>(OverflowNumberPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::Skip
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<CsvArchive, int32_t, bool>(OverflowNumberPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenNumberOverflowInt8) {
	TestOverflowNumberPolicy<CsvArchive, int16_t, int8_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<CsvArchive, uint16_t, uint8_t>(OverflowNumberPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenNumberOverflowInt16) {
	TestOverflowNumberPolicy<CsvArchive, int32_t, int16_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<CsvArchive, uint32_t, uint16_t>(OverflowNumberPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenNumberOverflowInt32) {
	TestOverflowNumberPolicy<CsvArchive, int64_t, int32_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<CsvArchive, uint64_t, uint32_t>(OverflowNumberPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenNumberOverflowFloat) {
	TestOverflowNumberPolicy<CsvArchive, double, float>(OverflowNumberPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test UtfEncodingErrorPolicy
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenEncodingError) {
	TestEncodingPolicy<CsvArchive>(Convert::Utf::UtfEncodingErrorPolicy::ThrowError);
}

TEST_F(CsvArchiveTests, ShouldSkipInvalidUtfWhenPolicyIsSkip) {
	TestEncodingPolicy<CsvArchive>(Convert::Utf::UtfEncodingErrorPolicy::Skip);
}
