/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/string_like_test_types.h"
#include "csv_archive_fixture.h"

// STD types
#include "bitserializer/types/std/atomic.h"
#include "bitserializer/types/std/chrono.h"
#include "bitserializer/types/std/ctime.h"
#include "bitserializer/types/std/optional.h"
#include "bitserializer/types/std/tuple.h"
#include "bitserializer/types/std/memory.h"
#include "bitserializer/types/std/filesystem.h"
#include "bitserializer/types/std/variant.h"
#include "bitserializer/types/std/map.h"

// STD types (test fixtures)
#include "testing_tools/auto_fixture/std/atomic.h"
#include "testing_tools/auto_fixture/std/chrono.h"
#include "testing_tools/auto_fixture/std/filesystem.h"
#include "testing_tools/auto_fixture/std/memory.h"
#include "testing_tools/auto_fixture/std/optional.h"
#include "testing_tools/auto_fixture/std/variant.h"

using namespace BitSerializer;
using BitSerializer::Csv::CsvArchive;

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeArrayOfClasses)
{
	TestSerializeArray<CsvArchive, TestPointClass>();
	TestSerializeArray<CsvArchive, TestClassWithSubTypes<bool, short, int, size_t, std::string>>();
}

TEST_F(CsvArchiveTests, SerializeArrayWithSpecialNumbersToStream)
{
	if constexpr (std::numeric_limits<long double>::has_infinity)
	{

		TestClassWithSubTypes<float> testArray[3] = { 1.0f, std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() };
		TestSerializeArrayToStream<CsvArchive>(testArray);

		TestClassWithSubTypes<double> testArray2[3] = { 1.0, std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() };
		TestSerializeArrayToStream<CsvArchive>(testArray2);
	}

	if constexpr (std::numeric_limits<double>::has_quiet_NaN)
	{
		TestClassWithSubTypes<float> testArray3[3] = { 1.0f, std::numeric_limits<float>::quiet_NaN(), 2.0f };
		TestSerializeArrayToStream<CsvArchive>(testArray3);

		TestClassWithSubTypes<double> testArray4[3] = { 1.0f, std::numeric_limits<double>::quiet_NaN(), 2.0f };
		TestSerializeArrayToStream<CsvArchive>(testArray4);
	}
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
	ASSERT_TRUE(rootArrayScope.IsOpened());

	for (size_t k = 0; k < 3; k++)
	{
		auto objectScope = rootArrayScope.OpenObjectScope(0);
		ASSERT_TRUE(objectScope.IsOpened());

		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), rootArrayScope.GetPath());
		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), objectScope.GetPath());
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
	ASSERT_TRUE(rootArrayScope.IsOpened());
	for (size_t k = 0; k < 3; k++)
	{
		auto objectScope = rootArrayScope.OpenObjectScope(0);
		ASSERT_TRUE(objectScope.IsOpened());

		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), rootArrayScope.GetPath());
		ASSERT_EQ(CsvArchive::path_separator + Convert::ToString(k), objectScope.GetPath());
	}
}

TEST_F(CsvArchiveTests, ShouldReturnUnopenedObjectScopeWhenNoMoreRows) {
	SerializationOptions options;
	SerializationContext context(options);
	CsvArchive::input_archive_type inputArchive("TestValue\r\n", context);

	auto arrayScope = inputArchive.OpenArrayScope(1);
	ASSERT_TRUE(arrayScope.IsOpened());
	auto objectScope = arrayScope.OpenObjectScope(0);
	EXPECT_FALSE(objectScope.IsOpened());
}

TEST_F(CsvArchiveTests, ShouldVisitKeysInObjectScopeWhenReadValues)
{
	TestVisitKeysInObjectScope<CsvArchive>();
}

TEST_F(CsvArchiveTests, ShouldVisitKeysInObjectScopeWhenSkipValues)
{
	TestVisitKeysInObjectScope<CsvArchive>(true);
}

//-----------------------------------------------------------------------------
// Tests of serialization array of classes
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeClassWithExternalSerializeFunction) {
	TestSerializeArray<CsvArchive, TestClassWithExternalSerialization>();
}

TEST_F(CsvArchiveTests, SerializeArrayOfClassesWithInheritance)
{
	TestSerializeArray<CsvArchive, TestClassWithInheritance<TestPointClass>>();
	TestSerializeArray<CsvArchive, TestClassWithInheritance<TestClassWithExternalSerialization>>();
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
// Native `std::string_view` keys (passed through without conversion to key_type)
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeWithStringViewKey)
{
	[[maybe_unused]] constexpr std::string_view key = "TestValue";
	static_assert(std::is_same_v<decltype(BitSerializer::Detail::ToArchiveKey<CsvArchive>(key)), const std::string_view&>);

	TestSerializeArray<CsvArchive, TestClassWithStringViewKey<int>>();
	TestSerializeArray<CsvArchive, TestClassWithStringViewKey<std::string>>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeArrayOfClassesToStream)
{
	TestClassWithSubTypes<short, int, long, size_t, double, std::string> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<CsvArchive>(testArray);
}

TEST_F(CsvArchiveTests, SaveObjectWithExplicitOutputStreamType)
{
	TestSaveObjectWithExplicitOutputStream<CsvArchive>(std::vector<TestPointClass>{ BuildFixture<TestPointClass>() });
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
		EXPECT_EQ(3U, ex.line);
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

TEST_F(CsvArchiveTests, ThrowExceptionWhenUnexpectedNumberOfValues)
{
	TestPointClass testList[1];
	EXPECT_THROW(BitSerializer::LoadObject<CsvArchive>(testList, "x,y\n10,20,30"), BitSerializer::ParsingException);
}

//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenMissedRequiredValue) {
	TestValidationForNamedValues<CsvArchive, TestClassForCheckValidation<int>>();
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowMismatchedTypesExceptionWhenLoadIntToBoolean) {
	TestMismatchedFieldPolicy<CsvArchive, int, bool>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedFieldPolicy<CsvArchive, uint32_t, bool>(MismatchedTypesPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowMismatchedTypesExceptionWhenLoadStringToBoolean) {
	TestMismatchedFieldPolicy<CsvArchive, std::string, bool>(MismatchedTypesPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedFieldPolicy<CsvArchive, std::string, int32_t>(MismatchedTypesPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedFieldPolicy<CsvArchive, std::string, float>(MismatchedTypesPolicy::ThrowError);
}
TEST_F(CsvArchiveTests, ThrowMismatchedTypesExceptionWhenLoadSignedToUnsigned) {
	TestMismatchedFieldPolicy<CsvArchive, int32_t, bool>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedFieldPolicy<CsvArchive, int32_t, uint32_t>(MismatchedTypesPolicy::ThrowError);
}

TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenLoadFloatToInteger) {
	TestMismatchedFieldPolicy<CsvArchive, float, uint32_t>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedFieldPolicy<CsvArchive, double, uint32_t>(MismatchedTypesPolicy::ThrowError);
}

TEST_F(CsvArchiveTests, ThrowMismatchedTypesExceptionWhenLoadValueToNull) {
	TestNullTargetPolicy<CsvArchive>(MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::Skip
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadIntToBoolean) {
	TestMismatchedFieldPolicy<CsvArchive, int, bool>(MismatchedTypesPolicy::Skip);
	TestMismatchedFieldPolicy<CsvArchive, uint32_t, bool>(MismatchedTypesPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedFieldPolicy<CsvArchive, std::string, bool>(MismatchedTypesPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedFieldPolicy<CsvArchive, std::string, int32_t>(MismatchedTypesPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedFieldPolicy<CsvArchive, std::string, float>(MismatchedTypesPolicy::Skip);
	TestMismatchedFieldPolicy<CsvArchive, std::string, double>(MismatchedTypesPolicy::Skip);
}

TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedFieldPolicy<CsvArchive, float, uint32_t>(MismatchedTypesPolicy::Skip);
	TestMismatchedFieldPolicy<CsvArchive, double, uint32_t>(MismatchedTypesPolicy::Skip);
}
TEST_F(CsvArchiveTests, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedFieldPolicy<CsvArchive, std::nullptr_t, bool>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedFieldPolicy<CsvArchive, std::nullptr_t, uint32_t>(MismatchedTypesPolicy::Skip);
	TestMismatchedFieldPolicy<CsvArchive, std::nullptr_t, double>(MismatchedTypesPolicy::ThrowError);
}

TEST_F(CsvArchiveTests, SkipValueWhenLoadValueToNull) {
	TestNullTargetPolicy<CsvArchive>(MismatchedTypesPolicy::Skip);
}

TEST_F(CsvArchiveTests, IgnoreMissingKeyWhenLoadNull) {
	TestMissingNullTargetKey<CsvArchive>();
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::ThrowError
//-----------------------------------------------------------------------------
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

TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenInvalidUtfInValueOfStream) {
	TestClassWithSubType<std::string> source[1] = { TestClassWithSubType<std::string>("\xFE\xFF") };
	SerializationOptions options;
	options.utfEncodingErrorPolicy = Convert::Utf::UtfEncodingErrorPolicy::ThrowError;
	options.streamOptions.encoding = Convert::Utf::UtfType::Utf16le;
	std::ostringstream output;

	const SerializationException ex = GTestExpectException<SerializationException>([&] {
		SaveObject<CsvArchive>(source, output, options);
	});
	EXPECT_EQ(SerializationErrorCode::UtfEncodingError, ex.GetErrorCode());
}

TEST_F(CsvArchiveTests, ThrowSerializationExceptionWhenInvalidUtfInKeyOfStream) {
	TestClassWithDynamicKey<int> source[1];
	source[0].Key = "\xFE\xFF";
	source[0].Value = 1;
	SerializationOptions options;
	options.utfEncodingErrorPolicy = Convert::Utf::UtfEncodingErrorPolicy::ThrowError;
	options.streamOptions.encoding = Convert::Utf::UtfType::Utf16le;
	std::ostringstream output;

	const SerializationException ex = GTestExpectException<SerializationException>([&] {
		SaveObject<CsvArchive>(source, output, options);
	});
	EXPECT_EQ(SerializationErrorCode::UtfEncodingError, ex.GetErrorCode());
}

//-----------------------------------------------------------------------------
// Tests of `std::optional` (additional coverage of MismatchedTypesPolicy handling)
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeStdOptionalAsObjectMember)
{
	// Simple types as members of object
	TestSerializeType<CsvArchive, TestClassWithSubType<std::optional<bool>>[1]>();
	TestSerializeType<CsvArchive, TestClassWithSubType<std::optional<bool>>[1]>({ TestClassWithSubType(std::optional<bool>(std::nullopt)) });

	TestSerializeType<CsvArchive, TestClassWithSubType<std::optional<int>>[1]>();
	TestSerializeType<CsvArchive, TestClassWithSubType<std::optional<int>>[1]>({ TestClassWithSubType(std::optional<int>(std::nullopt)) });

	TestSerializeType<CsvArchive, TestClassWithSubType<std::optional<float>>[1]>();
	TestSerializeType<CsvArchive, TestClassWithSubType<std::optional<float>>[1]>({ TestClassWithSubType(std::optional<float>(std::nullopt)) });

	// String types cannot be represented as Null in CSV
	TestSerializeType<CsvArchive, TestClassWithSubType<std::optional<std::string>>[1]>();
}

//-----------------------------------------------------------------------------
// Smoke tests of STD types serialization (more detailed tests in "unit_tests/std_types_tests")
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeStdTypes)
{
	TestSerializeArray<CsvArchive, TestClassWithSubType<std::atomic_int>>();

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Discriminated;
	TestSerializeArray<CsvArchive, std::variant<TestPointClass>>(options);

	// `std::monostate` is represented by an empty cell in the indexed representation.
	TestSerializeArray<CsvArchive, std::variant<std::monostate, int>>();

	TestSerializeArray<CsvArchive, TestClassWithSubType<std::unique_ptr<std::string>>>();
	TestSerializeArray<CsvArchive, TestClassWithSubType<std::shared_ptr<std::string>>>();

	TestSerializeArray<CsvArchive, TestClassWithSubType<std::filesystem::path>>();

	TestSerializeArray<CsvArchive, TestClassWithSubType<std::chrono::system_clock::time_point>>();
	TestSerializeArray<CsvArchive, TestClassWithSubType<std::chrono::seconds>>();
}

//-----------------------------------------------------------------------------
// Smoke tests of serialization for custom string-like types
// (declared via BITSERIALIZER_DECLARE_STRING_TYPE)
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeStringLikeTypes)
{
	TestSerializeArray<CsvArchive, TestClassWithSubType<TestStringLikeType>>();
	TestSerializeArray<CsvArchive, TestClassWithSubType<TestStdStringLikeType>>();
	TestSerializeArray<CsvArchive, TestClassWithSubType<TestWStringLikeType>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for maps in the flat (pair) representation (MapAsPairs)
//-----------------------------------------------------------------------------
TEST_F(CsvArchiveTests, SerializeMultimapAsPairs)
{
	const std::multimap<std::string, int> srcMap{ {"a", 1}, {"a", 2}, {"b", 3} };
	std::string outputStr;
	BitSerializer::SaveObject<CsvArchive>(MapAsPairs(srcMap), outputStr);

	std::multimap<std::string, int> actualMap;
	BitSerializer::LoadObject<CsvArchive>(MapAsPairs(actualMap), outputStr);
	EXPECT_EQ(srcMap, actualMap);
}

TEST_F(CsvArchiveTests, SerializeMapAsPairsWithCustomFieldNames)
{
	const std::map<std::string, int> srcMap{ {"a", 1}, {"b", 2} };
	std::string outputStr;
	BitSerializer::SaveObject<CsvArchive>(MapAsPairs(srcMap, std::nullopt, "name", "count"), outputStr);
	EXPECT_NE(std::string::npos, outputStr.find("name"));
	EXPECT_NE(std::string::npos, outputStr.find("count"));

	std::map<std::string, int> actualMap;
	BitSerializer::LoadObject<CsvArchive>(MapAsPairs(actualMap, std::nullopt, "name", "count"), outputStr);
	EXPECT_EQ(srcMap, actualMap);
}
