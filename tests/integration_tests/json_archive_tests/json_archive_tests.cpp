/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/common_json_test_methods.h"
#include "bitserializer/json_archive.h"

// STD types
#include "bitserializer/types/std/atomic.h"
#include "bitserializer/types/std/chrono.h"
#include "bitserializer/types/std/ctime.h"
#include "bitserializer/types/std/optional.h"
#include "bitserializer/types/std/pair.h"
#include "bitserializer/types/std/tuple.h"
#include "bitserializer/types/std/memory.h"
#include "bitserializer/types/std/filesystem.h"

using BitSerializer::Json::JsonArchive;

#pragma warning(push)
#pragma warning(disable: 4566)

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(JsonArchive, SaveBooleanAsTrueFalse)
{
	bool value = false;
	EXPECT_EQ("false", BitSerializer::SaveObject<JsonArchive>(value));
	value = true;
	EXPECT_EQ("true", BitSerializer::SaveObject<JsonArchive>(value));
}

TEST(JsonArchive, SerializeBoolean)
{
	TestSerializeType<JsonArchive, bool>(false);
	TestSerializeType<JsonArchive, bool>(true);
}

TEST(JsonArchive, SerializeFixedIntegers)
{
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::max());

	TestSerializeType<JsonArchive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<JsonArchive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(JsonArchive, SerializePlatformDependentIntegers)
{
	TestSerializeType<JsonArchive, char>(std::numeric_limits<char>::max());

	TestSerializeType<JsonArchive, short>(std::numeric_limits<short>::min());
	TestSerializeType<JsonArchive, unsigned short>(std::numeric_limits<unsigned short>::max());

	TestSerializeType<JsonArchive, long>(std::numeric_limits<long>::min());
	TestSerializeType<JsonArchive, unsigned long>(std::numeric_limits<unsigned long>::max());

	TestSerializeType<JsonArchive, long long>(std::numeric_limits<long long>::min());
	TestSerializeType<JsonArchive, unsigned long long>(std::numeric_limits<unsigned long long>::max());
}

TEST(JsonArchive, SerializeFloat)
{
	// Min/max floats cannot be tested because of type overflow which happens due lost precision in the RapidJson library
	TestSerializeType<JsonArchive, float>(0.f);
	TestSerializeType<JsonArchive, float>(3.141592654f);
	TestSerializeType<JsonArchive, float>(-3.141592654f);
}

TEST(JsonArchive, SerializeDouble)
{
	TestSerializeType<JsonArchive, double>(std::numeric_limits<double>::min());
	TestSerializeType<JsonArchive, double>(std::numeric_limits<double>::max());
}

TEST(JsonArchive, ShouldAllowToLoadBooleanFromInteger)
{
	bool actual = false;
	BitSerializer::LoadObject<JsonArchive>(actual, "1");
	EXPECT_EQ(true, actual);
}

TEST(JsonArchive, ShouldAllowToLoadFloatFromInteger)
{
	float actual = 0;
	BitSerializer::LoadObject<JsonArchive>(actual, "100");
	EXPECT_EQ(100, actual);
}

TEST(JsonArchive, SerializeNullptr)
{
	TestSerializeType<JsonArchive, std::nullptr_t>(nullptr);
}

//-----------------------------------------------------------------------------
// Tests of serialization any of std::string (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(JsonArchive, SerializeAnsiSting)
{
	TestSerializeType<JsonArchive, std::string>("Test ANSI string");
}

TEST(JsonArchive, SerializeUtf8Sting)
{
	TestSerializeType<JsonArchive, std::string>(UTF8("Test UTF8 string - Привет мир!"));
}
TEST(JsonArchive, SerializeUnicodeString)
{
	TestSerializeType<JsonArchive, std::wstring>(L"Test wide string - Привет мир!");
	TestSerializeType<JsonArchive, std::u16string>(u"Test UTF-16 string - Привет мир!");
	TestSerializeType<JsonArchive, std::u32string>(U"Test UTF-32 string - Привет мир!");
}

TEST(JsonArchive, SerializeStringWithEscapeSequences)
{
	TestSerializeType<JsonArchive, std::string>("\"\\/\b\f\n\r\t");
}

TEST(JsonArchive, SerializeEnum)
{
	TestSerializeType<JsonArchive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(JsonArchive, SerializeArrayOfBooleans)
{
	TestSerializeArray<JsonArchive, bool>();
}

TEST(JsonArchive, SerializeArrayOfChars)
{
	TestSerializeArray<JsonArchive, char>();
	TestSerializeArray<JsonArchive, unsigned char>();
}

TEST(JsonArchive, SerializeArrayOfIntegers)
{
	TestSerializeArray<JsonArchive, uint16_t>();
	TestSerializeArray<JsonArchive, int64_t>();

	// Test serialize platform dependent types
	TestSerializeArray<JsonArchive, long>();
	TestSerializeArray<JsonArchive, size_t>();
}

TEST(JsonArchive, SerializeArrayOfFloats)
{
	// Min/max floats cannot be tested because of type overflow which happens due lost precision in the RapidJson library
	TestSerializeType<JsonArchive>(std::vector{ -3.141592654f, 0.0f, -3.141592654f });
	TestSerializeArray<JsonArchive, double>();
}

TEST(JsonArchive, SerializeArrayOfNullptrs)
{
	TestSerializeArray<JsonArchive, std::nullptr_t>();
}

TEST(JsonArchive, SerializeArrayOfStrings)
{
	TestSerializeArray<JsonArchive, std::string>();
}

TEST(JsonArchive, SerializeArrayOfUnicodeStrings)
{
	TestSerializeArray<JsonArchive, std::wstring>();
	TestSerializeArray<JsonArchive, std::u16string>();
	TestSerializeArray<JsonArchive, std::u32string>();
}

TEST(JsonArchive, SerializeArrayOfClasses)
{
	TestSerializeArray<JsonArchive, TestPointClass>();
}

TEST(JsonArchive, SerializeTwoDimensionalArray)
{
	TestSerializeTwoDimensionalArray<JsonArchive, int32_t>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(JsonArchive, SerializeClassWithMemberBoolean)
{
	TestSerializeType<JsonArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeType<JsonArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(JsonArchive, SerializeClassWithMemberInteger)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t, size_t>>());
	TestSerializeType<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint64_t>::max()));
}

TEST(JsonArchive, SerializeClassWithMemberFloat)
{
	// Min/max floats cannot be tested because of type overflow which happens due lost precision in the RapidJson library
	TestSerializeType<JsonArchive>(TestClassWithSubTypes(-3.141592654f, 0.0f, -3.141592654f));
}

TEST(JsonArchive, SerializeClassWithMemberDouble)
{
	TestSerializeType<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<double>::min(), 0.0, std::numeric_limits<double>::max()));
}

TEST(JsonArchive, SerializeClassWithMemberNullptr)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(JsonArchive, SerializeClassWithMemberString)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring, std::u16string, std::u32string>>());
}

TEST(JsonArchive, SerializeClassWithExternalSerializeFunction)
{
	TestSerializeType<JsonArchive, TestClassWithExternalSerialization>();
}

TEST(JsonArchive, SerializeClassHierarchy)
{
	TestSerializeType<JsonArchive, TestClassWithInheritance<TestPointClass>>();
	TestSerializeType<JsonArchive, TestClassWithInheritance<TestClassWithExternalSerialization>>();
}

TEST(JsonArchive, SerializeClassWithSubClass)
{
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeType<JsonArchive>(BuildFixture<TestClassType>());
}

TEST(JsonArchive, SerializeClassWithSubArray)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(JsonArchive, SerializeClassWithSubArrayOfClasses)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(JsonArchive, SerializeClassWithSubTwoDimArray)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(JsonArchive, ShouldVisitKeysInObjectScopeWhenReadValues)
{
	TestVisitKeysInObjectScope<JsonArchive>();
}

TEST(JsonArchive, ShouldVisitKeysInObjectScopeWhenSkipValues)
{
	TestVisitKeysInObjectScope<JsonArchive>(true);
}

TEST(JsonArchive, SerializeClassInReverseOrder)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, float, std::string>>();
	TestSerializeType<JsonArchive>(fixture);
}

TEST(JsonArchive, SerializeClassInReverseOrderWithSubArray)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, std::array<uint64_t, 5>, std::string>>();
	TestSerializeType<JsonArchive>(fixture);
}

TEST(JsonArchive, SerializeClassInReverseOrderWithSubObject)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, TestPointClass, std::string>>();
	TestSerializeType<JsonArchive>(fixture);
}

TEST(JsonArchive, SerializeClassShouldNoChangeMissingFields)
{
	TestSkippingObjectValueWhenMismatchKey<JsonArchive>();
}

TEST(JsonArchive, SerializeClassWithSkippingFields)
{
	TestClassWithVersioning arrayOfObjects[3];
	BuildFixture(arrayOfObjects);
	TestSerializeType<JsonArchive>(arrayOfObjects);
}

//-----------------------------------------------------------------------------
// Tests of serialization for raw JSON
//-----------------------------------------------------------------------------
TEST(JsonArchive, SerializeRawJson)
{
	// Arrange
	std::string testJson = R"({"payload":[1,2,3,4,5,6,7,8,9,10]})";
	JsonArchive::raw_type raw;

	// Act
	BitSerializer::LoadObject<JsonArchive>(raw, testJson);
	// Change the source JSON to ensure that the deserialized raw JSON does not reference it
	testJson.assign(testJson.size(), '-');
	std::string actual;
	BitSerializer::SaveObject<JsonArchive>(raw, actual);

	// Assert
	EXPECT_EQ(R"({"payload":[1,2,3,4,5,6,7,8,9,10]})", actual);
}

TEST(JsonArchive, SerializeRawJsonAsObjectMember)
{
	// Arrange
	std::string testJson = R"({"TestValue": [1,2,3,4,5] })";
	TestClassWithSubType testObj(JsonArchive::raw_type(""));

	// Act
	BitSerializer::LoadObject<JsonArchive>(testObj, testJson);
	// Change the source JSON to ensure that the deserialized raw JSON does not reference it
	testJson.assign(testJson.size(), '-');
	std::string actual;
	BitSerializer::SaveObject<JsonArchive>(testObj.GetValue(), actual);

	// Assert
	EXPECT_EQ(R"([1,2,3,4,5])", actual);
}

TEST(JsonArchive, SerializeRawJsonAsArrayElement)
{
	// Arrange
	std::string testJson = R"([ "Text", 3.14 ])";
	JsonArchive::raw_type rawJson[2];

	// Act
	BitSerializer::LoadObject<JsonArchive>(rawJson, testJson);
	// Change the source JSON to ensure that the deserialized raw JSON does not reference it
	testJson.assign(testJson.size(), '-');
	std::string actual1, actual2;
	BitSerializer::SaveObject<JsonArchive>(rawJson[0], actual1);
	BitSerializer::SaveObject<JsonArchive>(rawJson[1], actual2);

	// Assert
	EXPECT_EQ(R"("Text")", actual1);
	EXPECT_EQ("3.14", actual2);
}


//TEST(JsonArchive, SerializeRawJsonToStream)
//{
//	// Arrange
//	std::istringstream ss(R"({"payload":[1,2,3,4,5,6,7,8,9,10]})");
//	JsonArchive::raw_type raw;
//	BitSerializer::SerializationOptions options;
//	options.streamOptions.writeBom = false;
//
//	// Act
//	BitSerializer::LoadObject<JsonArchive>(raw, ss);
//	std::ostringstream actual;
//	BitSerializer::SaveObject<JsonArchive>(raw, actual, options);
//
//	// Assert
//	EXPECT_EQ(R"({"payload":[1,2,3,4,5,6,7,8,9,10]})", actual.str());
//}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(JsonArchive, ShouldReturnPathInObjectScopeWhenLoading)
{
	TestGetPathInJsonObjectScopeWhenLoading<JsonArchive>();
}

TEST(JsonArchive, ShouldReturnPathInArrayScopeWhenLoading)
{
	TestGetPathInJsonArrayScopeWhenLoading<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Tests format output JSON
//-----------------------------------------------------------------------------
TEST(JsonArchive, SaveWithFormatting)
{
	TestSaveFormattedJson<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(JsonArchive, SerializeClassToStream) {
	TestSerializeClassToStream<JsonArchive>(BuildFixture<TestPointClass>());
}

TEST(JsonArchive, SerializeArrayOfClassesToStream)
{
	TestClassWithSubTypes<short, int, long, size_t, double, std::string> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<JsonArchive>(testArray);
}

TEST(JsonArchive, SerializeUnicodeToEncodedStream) {
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
	TestSerializeClassToStream<JsonArchive>(TestValue);
}

TEST(JsonArchive, LoadFromUtf8Stream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(false);
}
TEST(JsonArchive, LoadFromUtf8StreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(true);
}

//TEST(JsonArchive, LoadFromUtf16LeStream) {
//	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Le>(false);
//}
//TEST(JsonArchive, LoadFromUtf16LeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Le>(true);
//}
//
//TEST(JsonArchive, LoadFromUtf16BeStream) {
//	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Be>(false);
//}
//TEST(JsonArchive, LoadFromUtf16BeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Be>(true);
//}
//
//TEST(JsonArchive, LoadFromUtf32LeStream) {
//	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Le>(false);
//}
//TEST(JsonArchive, LoadFromUtf32LeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Le>(true);
//}
//
//TEST(JsonArchive, LoadFromUtf32BeStream) {
//	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Be>(false);
//}
//TEST(JsonArchive, LoadFromUtf32BeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Be>(true);
//}

TEST(JsonArchive, SaveToUtf8Stream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(false);
}
TEST(JsonArchive, SaveToUtf8StreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(true);
}

//TEST(JsonArchive, SaveToUtf16LeStream) {
//	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Le>(false);
//}
//TEST(JsonArchive, SaveToUtf16LeStreamWithBom) {
//	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Le>(true);
//}
//
//TEST(JsonArchive, SaveToUtf16BeStream) {
//	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Be>(false);
//}
//TEST(JsonArchive, SaveToUtf16BeStreamWithBom) {
//	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf16Be>(true);
//}
//
//TEST(JsonArchive, SaveToUtf32LeStream) {
//	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Le>(false);
//}
//TEST(JsonArchive, SaveToUtf32LeStreamWithBom) {
//	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Le>(true);
//}
//
//TEST(JsonArchive, SaveToUtf32BeStream) {
//	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Be>(false);
//}
//TEST(JsonArchive, SaveToUtf32BeStreamWithBom) {
//	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf32Be>(true);
//}

TEST(JsonArchive, ThrowExceptionWhenUnsupportedStreamEncoding)
{
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.streamOptions.encoding = static_cast<BitSerializer::Convert::Utf::UtfType>(-1);  // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
	std::stringstream outputStream;
	auto testObj = BuildFixture<TestClassWithSubTypes<std::string>>();
	EXPECT_THROW(BitSerializer::SaveObject<JsonArchive>(testObj, outputStream, serializationOptions), BitSerializer::SerializationException);
}

TEST(JsonArchive, SerializeToFile) {
	TestSerializeArrayToFile<JsonArchive>();
	TestSerializeArrayToFile<JsonArchive>(true);
}

TEST(JsonArchive, SerializeToFileThrowExceptionWhenAlreadyExists) {
	TestThrowExceptionWhenFileAlreadyExists<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(JsonArchive, ThrowExceptionWhenUnexpectedEnd)
{
	try
	{
		std::string str;
		BitSerializer::LoadObject<JsonArchive>(str, "\"Hello");
		EXPECT_FALSE(true);
	}
	catch (const BitSerializer::ParsingException& ex)
	{
		EXPECT_EQ(6U, ex.Offset);
	}
	catch (const std::exception&)
	{
		EXPECT_FALSE(true);
	}
}

TEST(JsonArchive, ThrowExceptionWhenUnexpectedEndInObject)
{
	try
	{
		TestPointClass point;
		BitSerializer::LoadObject<JsonArchive>(point, R"({ "x":10, "y": })");
		EXPECT_FALSE(true);
	}
	catch (const BitSerializer::ParsingException& ex)
	{
		EXPECT_EQ(15U, ex.Offset);
	}
	catch (const std::exception&)
	{
		EXPECT_FALSE(true);
	}
}

TEST(JsonArchive, ThrowExceptionWhenUnexpectedEndInArray)
{
	try
	{
		bool testArray[3];
		BitSerializer::LoadObject<JsonArchive>(testArray, "[ true, false");
		EXPECT_FALSE(true);
	}
	catch (const BitSerializer::ParsingException& ex)
	{
		EXPECT_EQ(13U, ex.Offset);
	}
	catch (const std::exception&)
	{
		EXPECT_FALSE(true);
	}
}

TEST(JsonArchive, ThrowParsingExceptionWithCorrectPosition)
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
TEST(JsonArchive, ThrowValidationExceptionWhenMissedRequiredValue) {
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
TEST(JsonArchive, ThrowMismatchedTypesExceptionWhenLoadIntToBoolean) {
	TestMismatchedTypesPolicy<JsonArchive, int, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<JsonArchive, uint32_t, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonArchive, ThrowMismatchedTypesExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonArchive, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonArchive, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonArchive, ThrowMismatchedTypesExceptionWhenLoadSignedToUnsigned) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<JsonArchive, int32_t, uint32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(JsonArchive, ThrowMismatchedTypesExceptionWhenLoadNumberToString) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, std::string>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonArchive, ThrowMismatchedTypesExceptionWhenLoadFloatToInt) {
	TestMismatchedTypesPolicy<JsonArchive, float, int>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<JsonArchive, double, int>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(JsonArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::Skip
//-----------------------------------------------------------------------------
TEST(JsonArchive, ThrowValidationExceptionWhenLoadIntToBoolean) {
	TestMismatchedTypesPolicy<JsonArchive, int, bool>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, uint32_t, bool>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonArchive, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonArchive, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonArchive, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, std::string, double>(BitSerializer::MismatchedTypesPolicy::Skip);
}

TEST(JsonArchive, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<JsonArchive, float, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, double, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonArchive, ThrowValidationExceptionWhenLoadFloatToInt) {
	TestMismatchedTypesPolicy<JsonArchive, float, int>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, double, int>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonArchive, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedTypesPolicy<JsonArchive, std::nullptr_t, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<JsonArchive, std::nullptr_t, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, std::nullptr_t, double>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(JsonArchive, ThrowValidationExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonArchive, ThrowValidationExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(JsonArchive, ThrowSerializationExceptionWhenOverflowInt8) {
	TestOverflowNumberPolicy<JsonArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<JsonArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(JsonArchive, ThrowSerializationExceptionWhenOverflowInt16) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<JsonArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(JsonArchive, ThrowSerializationExceptionWhenOverflowInt32) {
	TestOverflowNumberPolicy<JsonArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<JsonArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(JsonArchive, ThrowSerializationExceptionWhenOverflowFloat) {
	TestOverflowNumberPolicy<JsonArchive, double, float>(BitSerializer::OverflowNumberPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::Skip
//-----------------------------------------------------------------------------
TEST(JsonArchive, ThrowValidationExceptionWhenNumberOverflowInt8) {
	TestOverflowNumberPolicy<JsonArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<JsonArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(JsonArchive, ThrowValidationExceptionWhenNumberOverflowInt16) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<JsonArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(JsonArchive, ThrowValidationExceptionWhenNumberOverflowInt32) {
	TestOverflowNumberPolicy<JsonArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<JsonArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(JsonArchive, ThrowValidationExceptionWhenNumberOverflowFloat) {
	TestOverflowNumberPolicy<JsonArchive, double, float>(BitSerializer::OverflowNumberPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test UtfEncodingErrorPolicy
//-----------------------------------------------------------------------------
TEST(JsonArchive, ThrowSerializationExceptionWhenEncodingError) {
	TestEncodingPolicy<JsonArchive>(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::ThrowError);
}

TEST(JsonArchive, ShouldSkipInvalidUtfWhenPolicyIsSkip) {
	TestEncodingPolicy<JsonArchive>(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Tests of `std::optional` (additional coverage of MismatchedTypesPolicy handling)
//-----------------------------------------------------------------------------
TEST(JsonArchive, SerializeStdOptionalAsRootElement)
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

TEST(JsonArchive, SerializeStdOptionalAsObjectMember)
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
TEST(JsonArchive, SerializeStdTypes)
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
