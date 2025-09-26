/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/common_json_test_methods.h"
#include "bitserializer/rapidjson_archive.h"

// STD types
#include "bitserializer/types/std/atomic.h"
#include "bitserializer/types/std/chrono.h"
#include "bitserializer/types/std/ctime.h"
#include "bitserializer/types/std/optional.h"
#include "bitserializer/types/std/pair.h"
#include "bitserializer/types/std/tuple.h"
#include "bitserializer/types/std/memory.h"
#include "bitserializer/types/std/filesystem.h"

using BitSerializer::Json::RapidJson::JsonArchive;

#pragma warning(push)
#pragma warning(disable: 4566)

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SaveBooleanAsTrueFalse)
{
	bool value = false;
	EXPECT_EQ("false", BitSerializer::SaveObject<JsonArchive>(value));
	value = true;
	EXPECT_EQ("true", BitSerializer::SaveObject<JsonArchive>(value));
}

TEST(RapidJsonArchive, SerializeBoolean)
{
	TestSerializeType<JsonArchive, bool>(false);
	TestSerializeType<JsonArchive, bool>(true);
}

TEST(RapidJsonArchive, SerializeFixedIntegers)
{
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::max());

	TestSerializeType<JsonArchive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<JsonArchive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(RapidJsonArchive, SerializePlatformDependentIntegers)
{
	TestSerializeType<JsonArchive, char>(std::numeric_limits<char>::max());

	TestSerializeType<JsonArchive, short>(std::numeric_limits<short>::min());
	TestSerializeType<JsonArchive, unsigned short>(std::numeric_limits<unsigned short>::max());

	TestSerializeType<JsonArchive, long>(std::numeric_limits<long>::min());
	TestSerializeType<JsonArchive, unsigned long>(std::numeric_limits<unsigned long>::max());

	TestSerializeType<JsonArchive, long long>(std::numeric_limits<long long>::min());
	TestSerializeType<JsonArchive, unsigned long long>(std::numeric_limits<unsigned long long>::max());
}

TEST(RapidJsonArchive, SerializeFloat)
{
	// Min/max floats cannot be tested because of type overflow which happens due lost precision in the RapidJson library
	TestSerializeType<JsonArchive, float>(0.f);
	TestSerializeType<JsonArchive, float>(3.141592654f);
	TestSerializeType<JsonArchive, float>(-3.141592654f);
}

TEST(RapidJsonArchive, SerializeDouble)
{
	TestSerializeType<JsonArchive, double>(std::numeric_limits<double>::min());
	TestSerializeType<JsonArchive, double>(std::numeric_limits<double>::max());
}

TEST(RapidJsonArchive, ShouldAllowToLoadBooleanFromInteger)
{
	bool actual = false;
	BitSerializer::LoadObject<JsonArchive>(actual, "1");
	EXPECT_EQ(true, actual);
}

TEST(RapidJsonArchive, ShouldAllowToLoadFloatFromInteger)
{
	float actual = 0;
	BitSerializer::LoadObject<JsonArchive>(actual, "100");
	EXPECT_EQ(100, actual);
}

TEST(RapidJsonArchive, SerializeNullptr)
{
	TestSerializeType<JsonArchive, std::nullptr_t>(nullptr);
}

//-----------------------------------------------------------------------------
// Tests of serialization any of std::string (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeUtf8Sting)
{
	TestSerializeType<JsonArchive, std::string>("Test ANSI string");
	TestSerializeType<JsonArchive, std::string>(UTF8("Test UTF8 string - Привет мир!"));
}

TEST(RapidJsonArchive, SerializeUnicodeString)
{
	TestSerializeType<JsonArchive, std::wstring>(L"Test wide string - Привет мир!");
	TestSerializeType<JsonArchive, std::u16string>(u"Test UTF-16 string - Привет мир!");
	TestSerializeType<JsonArchive, std::u32string>(U"Test UTF-32 string - Привет мир!");
}

TEST(RapidJsonArchive, SerializeEnum)
{
	TestSerializeType<JsonArchive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeArrayOfBooleans)
{
	TestSerializeArray<JsonArchive, bool>();
}

TEST(RapidJsonArchive, SerializeArrayOfChars)
{
	TestSerializeArray<JsonArchive, char>();
	TestSerializeArray<JsonArchive, unsigned char>();
}

TEST(RapidJsonArchive, SerializeArrayOfIntegers)
{
	TestSerializeArray<JsonArchive, uint16_t>();
	TestSerializeArray<JsonArchive, int64_t>();

	// Test serialize platform dependent types
	TestSerializeArray<JsonArchive, long>();
	TestSerializeArray<JsonArchive, size_t>();
}

TEST(RapidJsonArchive, SerializeArrayOfFloats)
{
	// Min/max floats cannot be tested because of type overflow which happens due lost precision in the RapidJson library
	TestSerializeType<JsonArchive>(std::vector{ -3.141592654f, 0.0f, -3.141592654f });
	TestSerializeArray<JsonArchive, double>();
}

TEST(RapidJsonArchive, SerializeArrayOfNullptrs)
{
	TestSerializeArray<JsonArchive, std::nullptr_t>();
}

TEST(RapidJsonArchive, SerializeArrayOfStrings)
{
	TestSerializeArray<JsonArchive, std::string>();
}

TEST(RapidJsonArchive, SerializeArrayOfUnicodeStrings)
{
	TestSerializeArray<JsonArchive, std::wstring>();
	TestSerializeArray<JsonArchive, std::u16string>();
	TestSerializeArray<JsonArchive, std::u32string>();
}

TEST(RapidJsonArchive, SerializeArrayOfClasses)
{
	TestSerializeArray<JsonArchive, TestPointClass>();
}

TEST(RapidJsonArchive, SerializeTwoDimensionalArray)
{
	TestSerializeTwoDimensionalArray<JsonArchive, int32_t>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeClassWithMemberBoolean)
{
	TestSerializeType<JsonArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeType<JsonArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(RapidJsonArchive, SerializeClassWithMemberInteger)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t, size_t>>());
	TestSerializeType<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint64_t>::max()));
}

TEST(RapidJsonArchive, SerializeClassWithMemberFloat)
{
	// Min/max floats cannot be tested because of type overflow which happens due lost precision in the RapidJson library
	TestSerializeType<JsonArchive>(TestClassWithSubTypes(-3.141592654f, 0.0f, -3.141592654f));
}

TEST(RapidJsonArchive, SerializeClassWithMemberDouble)
{
	TestSerializeType<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<double>::min(), 0.0, std::numeric_limits<double>::max()));
}

TEST(RapidJsonArchive, SerializeClassWithMemberNullptr)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberString)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring, std::u16string, std::u32string>>());
}

TEST(RapidJsonArchive, SerializeClassWithExternalSerializeFuntion)
{
	TestSerializeType<JsonArchive, TestClassWithExternalSerialization>();
}

TEST(RapidJsonArchive, SerializeClassHierarchy)
{
	TestSerializeType<JsonArchive, TestClassWithInheritance<TestPointClass>>();
	TestSerializeType<JsonArchive, TestClassWithInheritance<TestClassWithExternalSerialization>>();
}

TEST(RapidJsonArchive, SerializeClassWithMemberClass)
{
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeType<JsonArchive>(BuildFixture<TestClassType>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArray)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArrayOfClasses)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubTwoDimArray)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(RapidJsonArchive, ShouldVisitKeysInObjectScopeWhenReadValues)
{
	TestVisitKeysInObjectScope<JsonArchive>();
}

TEST(RapidJsonArchive, ShouldVisitKeysInObjectScopeWhenSkipValues)
{
	TestVisitKeysInObjectScope<JsonArchive>(true);
}

TEST(RapidJsonArchive, SerializeClassInReverseOrder)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, float, std::string>>();
	TestSerializeType<JsonArchive>(fixture);
}

TEST(RapidJsonArchive, SerializeClassInReverseOrderWithSubArray)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, std::array<uint64_t, 5>, std::string>>();
	TestSerializeType<JsonArchive>(fixture);
}

TEST(RapidJsonArchive, SerializeClassInReverseOrderWithSubObject)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, TestPointClass, std::string>>();
	TestSerializeType<JsonArchive>(fixture);
}

TEST(RapidJsonArchive, SerializeClassWithSkippingFields)
{
	TestClassWithVersioning arrayOfObjects[3];
	BuildFixture(arrayOfObjects);
	TestSerializeType<JsonArchive>(arrayOfObjects);
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ShouldReturnPathInObjectScopeWhenLoading)
{
	TestGetPathInJsonObjectScopeWhenLoading<JsonArchive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInObjectScopeWhenSaving)
{
	TestGetPathInJsonObjectScopeWhenSaving<JsonArchive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInArrayScopeWhenLoading)
{
	TestGetPathInJsonArrayScopeWhenLoading<JsonArchive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInArrayScopeWhenSaving)
{
	TestGetPathInJsonArrayScopeWhenSaving<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Tests format output JSON
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SaveWithFormatting)
{
	TestSaveFormattedJson<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeClassToStream) {
	TestSerializeClassToStream<JsonArchive>(BuildFixture<TestPointClass>());
}

TEST(RapidJsonArchive, SerializeArrayOfClassesToStream)
{
	TestClassWithSubTypes<short, int, long, size_t, double, std::string> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<JsonArchive>(testArray);
}

TEST(RapidJsonArchive, SerializeUnicodeToEncodedStream) {
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
	TestSerializeClassToStream<JsonArchive>(TestValue);
}

TEST(RapidJsonArchive, LoadFromUtf8Stream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(false);
}
TEST(RapidJsonArchive, LoadFromUtf8StreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(true);
}

TEST(RapidJsonArchive, LoadFromUtf16LeStream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Le>(false);
}
TEST(RapidJsonArchive, LoadFromUtf16LeStreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Le>(true);
}

TEST(RapidJsonArchive, LoadFromUtf16BeStream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Be>(false);
}
TEST(RapidJsonArchive, LoadFromUtf16BeStreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Be>(true);
}

TEST(RapidJsonArchive, LoadFromUtf32LeStream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Le>(false);
}
TEST(RapidJsonArchive, LoadFromUtf32LeStreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Le>(true);
}

TEST(RapidJsonArchive, LoadFromUtf32BeStream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Be>(false);
}
TEST(RapidJsonArchive, LoadFromUtf32BeStreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Be>(true);
}

TEST(RapidJsonArchive, SaveToUtf8Stream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(false);
}
TEST(RapidJsonArchive, SaveToUtf8StreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(true);
}

TEST(RapidJsonArchive, SaveToUtf16LeStream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Le>(false);
}
TEST(RapidJsonArchive, SaveToUtf16LeStreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Le>(true);
}

TEST(RapidJsonArchive, SaveToUtf16BeStream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Be>(false);
}
TEST(RapidJsonArchive, SaveToUtf16BeStreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Be>(true);
}

TEST(RapidJsonArchive, SaveToUtf32LeStream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Le>(false);
}
TEST(RapidJsonArchive, SaveToUtf32LeStreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Le>(true);
}

TEST(RapidJsonArchive, SaveToUtf32BeStream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Be>(false);
}
TEST(RapidJsonArchive, SaveToUtf32BeStreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Be>(true);
}

TEST(RapidJsonArchive, ThrowExceptionWhenUnsupportedStreamEncoding)
{
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.streamOptions.encoding = static_cast<BitSerializer::Convert::Utf::UtfType>(-1);  // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
	std::stringstream outputStream;
	auto testObj = BuildFixture<TestClassWithSubTypes<std::string>>();
	EXPECT_THROW(BitSerializer::SaveObject<JsonArchive>(testObj, outputStream, serializationOptions), BitSerializer::SerializationException);
}

TEST(RapidJsonArchive, SerializeToFile) {
	TestSerializeArrayToFile<JsonArchive>();
	TestSerializeArrayToFile<JsonArchive>(true);
}

TEST(RapidJsonArchive, SerializeToFileThrowExceptionWhenAlreadyExists) {
	TestThrowExceptionWhenFileAlreadyExists<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowExceptionWhenBadSyntaxInSource)
{
	int testInt = 0;
	EXPECT_THROW(BitSerializer::LoadObject<JsonArchive>(testInt, "10 }}"), BitSerializer::SerializationException);
}

TEST(RapidJsonArchive, ThrowParsingExceptionWithCorrectPosition)
{
	const char* testJson = R"([
	{ "x": 10, "y": 20},
	{ "x": 11, y: 21}
])";
	TestPointClass testList[2];
	try
	{
		BitSerializer::LoadObject<JsonArchive>(testList, testJson);
		EXPECT_FALSE(true);
	}
	catch (const BitSerializer::ParsingException& ex)
	{
		EXPECT_TRUE(ex.Offset > 24 && ex.Offset < std::strlen(testJson));
	}
	catch (const std::exception&)
	{
		EXPECT_FALSE(true);
	}
}

//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowValidationExceptionWhenMissedRequiredValue) {
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<TestPointClass>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<int[3]>>();
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowMismatchedTypesExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(RapidJsonArchive, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(RapidJsonArchive, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(RapidJsonArchive, ThrowMismatchedTypesExceptionWhenLoadNumberToString) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, std::string>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(RapidJsonArchive, ThrowMismatchedTypesExceptionWhenLoadFloatToInt) {
	TestMismatchedTypesPolicy<JsonArchive, float, int>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<JsonArchive, double, int>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(RapidJsonArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(RapidJsonArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::Skip
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(RapidJsonArchive, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(RapidJsonArchive, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, std::string, double>(BitSerializer::MismatchedTypesPolicy::Skip);
}

TEST(RapidJsonArchive, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<JsonArchive, float, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, double, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(RapidJsonArchive, ThrowValidationExceptionWhenLoadFloatToInt) {
	TestMismatchedTypesPolicy<JsonArchive, float, int>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, double, int>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(RapidJsonArchive, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedTypesPolicy<JsonArchive, std::nullptr_t, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<JsonArchive, std::nullptr_t, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, std::nullptr_t, double>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(RapidJsonArchive, ThrowValidationExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(RapidJsonArchive, ThrowValidationExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowSerializationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, bool>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(RapidJsonArchive, ThrowSerializationExceptionWhenOverflowInt8) {
	TestOverflowNumberPolicy<JsonArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<JsonArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(RapidJsonArchive, ThrowSerializationExceptionWhenOverflowInt16) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<JsonArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(RapidJsonArchive, ThrowSerializationExceptionWhenOverflowInt32) {
	TestOverflowNumberPolicy<JsonArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<JsonArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(RapidJsonArchive, ThrowSerializationExceptionWhenOverflowFloat) {
	TestOverflowNumberPolicy<JsonArchive, double, float>(BitSerializer::OverflowNumberPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::Skip
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowValidationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, bool>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(RapidJsonArchive, ThrowValidationExceptionWhenNumberOverflowInt8) {
	TestOverflowNumberPolicy<JsonArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<JsonArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(RapidJsonArchive, ThrowValidationExceptionWhenNumberOverflowInt16) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<JsonArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(RapidJsonArchive, ThrowValidationExceptionWhenNumberOverflowInt32) {
	TestOverflowNumberPolicy<JsonArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<JsonArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(RapidJsonArchive, ThrowValidationExceptionWhenNumberOverflowFloat) {
	TestOverflowNumberPolicy<JsonArchive, double, float>(BitSerializer::OverflowNumberPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test UtfEncodingErrorPolicy
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowSerializationExceptionWhenEncodingError) {
	TestEncodingPolicy<JsonArchive>(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::ThrowError);
}

TEST(RapidJsonArchive, ShouldSkipInvalidUtfWhenPolicyIsSkip) {
	TestEncodingPolicy<JsonArchive>(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Tests of `std::optional` (additional coverage of MismatchedTypesPolicy handling)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeStdOptionalAsRootElement)
{
	// Simple types as root element
	TestSerializeType<JsonArchive>(std::optional<bool>());
	TestSerializeType<JsonArchive>(std::optional<bool>(std::nullopt));

	TestSerializeType<JsonArchive>(std::optional<int>());
	TestSerializeType<JsonArchive>(std::optional<int>(std::nullopt));

	TestSerializeType<JsonArchive>(std::optional<float>());
	TestSerializeType<JsonArchive>(std::optional<float>(std::nullopt));

	TestSerializeType<JsonArchive>(std::optional<std::string>());
	TestSerializeType<JsonArchive>(std::optional<std::string>(std::nullopt));

	// Object as root element
	TestSerializeType<JsonArchive>(std::optional<TestPointClass>());
	TestSerializeType<JsonArchive>(std::optional<TestPointClass>(std::nullopt));

	// Array as root element
	TestSerializeType<JsonArchive>(std::optional<std::vector<int>>());
	TestSerializeType<JsonArchive>(std::optional<std::vector<int>>(std::nullopt));
}

TEST(RapidJsonArchive, SerializeStdOptionalAsObjectMember)
{
	// Simple types as members of object
	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<bool>>>();
	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<bool>>>(TestClassWithSubType(std::optional<bool>(std::nullopt)));

	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<int>>>();
	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<int>>>(TestClassWithSubType(std::optional<int>(std::nullopt)));

	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<float>>>();
	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<float>>>(TestClassWithSubType(std::optional<float>(std::nullopt)));

	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<std::string>>>();
	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<std::string>>>(TestClassWithSubType(std::optional<std::string>(std::nullopt)));

	// Object as member of object
	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<TestPointClass>>>();
	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<TestPointClass>>>(TestClassWithSubType(std::optional<TestPointClass>(std::nullopt)));

	// Array as member of object
	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<std::vector<int>>>>();
	TestSerializeType<JsonArchive, TestClassWithSubType<std::optional<std::vector<int>>>>(TestClassWithSubType(std::optional<std::vector<int>>(std::nullopt)));
}

//-----------------------------------------------------------------------------
// Smoke tests of STD types serialization (more detailed tests in "unit_tests/std_types_tests")
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeStdTypes)
{
	TestSerializeType<JsonArchive, std::atomic_int>();
	TestSerializeType<JsonArchive, std::pair<std::string, int>>();
	TestSerializeType<JsonArchive, std::tuple<std::string, int, float, bool>>();

	TestSerializeType<JsonArchive>(std::make_unique<std::string>("test"));
	TestSerializeType<JsonArchive>(std::make_shared<std::string>("test"));

	TestSerializeType<JsonArchive>(std::filesystem::temp_directory_path());

	TestSerializeType<JsonArchive, std::chrono::system_clock::time_point>();
	TestSerializeType<JsonArchive, std::chrono::seconds>();
}

#pragma warning(pop)
