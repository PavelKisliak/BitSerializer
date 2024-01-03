/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/common_json_test_methods.h"
#include "msgpack_archive_fixture.h"

using namespace BitSerializer;
using BitSerializer::MsgPack::MsgPackArchive;

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeBoolean)
{
	TestSerializeType<MsgPackArchive, bool>(false);
	TestSerializeType<MsgPackArchive, bool>(true);
}

TEST(MsgPackArchive, SerializeUInt8)
{
	TestSerializeType<MsgPackArchive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<MsgPackArchive, uint8_t>(0);
	TestSerializeType<MsgPackArchive, uint8_t>(127);
	TestSerializeType<MsgPackArchive, uint8_t>(std::numeric_limits<uint8_t>::max());
}

TEST(MsgPackArchive, SerializeInt8)
{
	TestSerializeType<MsgPackArchive, int8_t>(std::numeric_limits<int8_t>::min());
	TestSerializeType<MsgPackArchive, int8_t>(-32);
	TestSerializeType<MsgPackArchive, int8_t>(32);
	TestSerializeType<MsgPackArchive, int8_t>(std::numeric_limits<int8_t>::max());
}

TEST(MsgPackArchive, SerializeInt64)
{
	TestSerializeType<MsgPackArchive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<MsgPackArchive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(MsgPackArchive, SerializeFloat)
{
	TestSerializeType<MsgPackArchive, float>(0.f);
	TestSerializeType<MsgPackArchive, float>(3.141592654f);
	TestSerializeType<MsgPackArchive, float>(-3.141592654f);
}

TEST(MsgPackArchive, SerializeDouble)
{
	TestSerializeType<MsgPackArchive, double>(std::numeric_limits<double>::min());
	TestSerializeType<MsgPackArchive, double>(std::numeric_limits<double>::max());
}

TEST(MsgPackArchive, ShouldAllowToLoadBooleanFromInteger)
{
	bool actual = false;
	BitSerializer::LoadObject<MsgPackArchive>(actual, std::string({'\xC3'}));
	EXPECT_EQ(true, actual);
}

TEST(MsgPackArchive, SerializeNullptr)
{
	TestSerializeType<MsgPackArchive, std::nullptr_t>(nullptr);
}

//-----------------------------------------------------------------------------
// Tests of serialization any of std::string (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeUtf8Sting)
{
	TestSerializeType<MsgPackArchive, std::string>("Test ANSI string");
	TestSerializeType<MsgPackArchive, std::string>(u8"Test UTF8 string - Привет мир!");
}

TEST(MsgPackArchive, SerializeUnicodeString)
{
	TestSerializeType<MsgPackArchive, std::wstring>(L"Test wide string - Привет мир!");
	TestSerializeType<MsgPackArchive, std::u16string>(u"Test UTF-16 string - Привет мир!");
	TestSerializeType<MsgPackArchive, std::u32string>(U"Test UTF-32 string - Привет мир!");
}

TEST(MsgPackArchive, SerializeEnum)
{
	TestSerializeType<MsgPackArchive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeArrayOfBooleans)
{
	TestSerializeArray<MsgPackArchive, bool>();
}

TEST(MsgPackArchive, SerializeArrayOfChars)
{
	TestSerializeArray<MsgPackArchive, char>();
	TestSerializeArray<MsgPackArchive, unsigned char>();
}

TEST(MsgPackArchive, SerializeArrayOfIntegers)
{
	TestSerializeArray<MsgPackArchive, uint16_t>();
	TestSerializeArray<MsgPackArchive, int64_t>();
}

TEST(MsgPackArchive, SerializeArrayOfFloats)
{
	TestSerializeVector<MsgPackArchive, float>({ -3.141592654f, 0.0f, -3.141592654f });
}

TEST(MsgPackArchive, SerializeArrayOfDoubles)
{
	TestSerializeArray<MsgPackArchive, double>();
}

TEST(MsgPackArchive, SerializeArrayOfNullptrs)
{
	TestSerializeArray<MsgPackArchive, std::nullptr_t>();
}

TEST(MsgPackArchive, SerializeArrayOfStrings)
{
	TestSerializeArray<MsgPackArchive, std::string>();
}

TEST(MsgPackArchive, SerializeArrayOfUnicodeStrings)
{
	TestSerializeArray<MsgPackArchive, std::wstring>();
	TestSerializeArray<MsgPackArchive, std::u16string>();
	TestSerializeArray<MsgPackArchive, std::u32string>();
}

TEST(MsgPackArchive, SerializeArrayOfClasses)
{
	TestSerializeArray<MsgPackArchive, TestPointClass>();
}

TEST(MsgPackArchive, SerializeTwoDimensionalArray)
{
	TestSerializeTwoDimensionalArray<MsgPackArchive, int32_t>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeClassWithMemberBoolean)
{
	TestSerializeClass<MsgPackArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<MsgPackArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(MsgPackArchive, SerializeClassWithMemberInteger)
{
	TestSerializeClass<MsgPackArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
	TestSerializeClass<MsgPackArchive>(TestClassWithSubTypes(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint64_t>::max()));
}

TEST(MsgPackArchive, SerializeClassWithMemberFloat)
{
	// Min/max floats cannot be tested because of type overflow which happens due lost precision in the RapidJson library
	TestSerializeClass<MsgPackArchive>(TestClassWithSubTypes(-3.141592654f, 0.0f, -3.141592654f));
}

TEST(MsgPackArchive, SerializeClassWithMemberDouble)
{
	TestSerializeClass<MsgPackArchive>(TestClassWithSubTypes(std::numeric_limits<double>::min(), 0.0, std::numeric_limits<double>::max()));
}

TEST(MsgPackArchive, SerializeClassWithMemberNullptr)
{
	TestSerializeClass<MsgPackArchive>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(MsgPackArchive, SerializeClassWithMemberString)
{
	TestSerializeClass<MsgPackArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring, std::u16string, std::u32string>>());
}

TEST(MsgPackArchive, SerializeClassHierarchy)
{
	TestSerializeClass<MsgPackArchive>(BuildFixture<TestClassWithInheritance>());
}

TEST(MsgPackArchive, SerializeClassWithMemberClass)
{
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<MsgPackArchive>(BuildFixture<TestClassType>());
}

TEST(MsgPackArchive, SerializeClassWithSubArray)
{
	TestSerializeClass<MsgPackArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(MsgPackArchive, SerializeClassWithSubArrayOfClasses)
{
	TestSerializeClass<MsgPackArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(MsgPackArchive, SerializeClassWithSubTwoDimArray)
{
	TestSerializeClass<MsgPackArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(MsgPackArchive, ShouldVisitKeysInObjectScope)
{
	TestVisitKeysInObjectScope<MsgPackArchive>();
}

TEST(MsgPackArchive, SerializeClassInReverseOrder)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, float, std::string>>();
	TestSerializeClass<MsgPackArchive>(fixture);
}

TEST(MsgPackArchive, SerializeClassInReverseOrderWithSubArray)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, std::array<uint64_t, 5>, std::string>>();
	TestSerializeClass<MsgPackArchive>(fixture);
}

TEST(MsgPackArchive, SerializeClassInReverseOrderWithSubObject)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, TestPointClass, std::string>>();
	TestSerializeClass<MsgPackArchive>(fixture);
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ShouldReturnPathInObjectScopeWhenLoading)
{
	TestGetPathInJsonObjectScopeWhenLoading<MsgPackArchive>();
}

//TEST(MsgPackArchive, ShouldReturnPathInObjectScopeWhenSaving)
//{
//	TestGetPathInJsonObjectScopeWhenSaving<MsgPackArchive>();
//}

TEST(MsgPackArchive, ShouldReturnPathInArrayScopeWhenLoading)
{
	TestGetPathInJsonArrayScopeWhenLoading<MsgPackArchive>();
}

//TEST(MsgPackArchive, ShouldReturnPathInArrayScopeWhenSaving)
//{
//	TestGetPathInJsonArrayScopeWhenSaving<MsgPackArchive>();
//}

////-----------------------------------------------------------------------------
//// Tests streams / files
////-----------------------------------------------------------------------------
//TEST(MsgPackArchive, SerializeClassToStream) {
//	TestSerializeClassToStream<MsgPackArchive, char>(BuildFixture<TestPointClass>());
//}
//
//TEST(MsgPackArchive, SerializeUnicodeToEncodedStream) {
//	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
//	TestSerializeClassToStream<MsgPackArchive, char>(TestValue);
//}
//
//TEST(MsgPackArchive, LoadFromUtf8Stream) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf8>(false);
//}
//TEST(MsgPackArchive, LoadFromUtf8StreamWithBom) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf8>(true);
//}
//
//TEST(MsgPackArchive, LoadFromUtf16LeStream) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf16Le>(false);
//}
//TEST(MsgPackArchive, LoadFromUtf16LeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf16Le>(true);
//}
//
//TEST(MsgPackArchive, LoadFromUtf16BeStream) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf16Be>(false);
//}
//TEST(MsgPackArchive, LoadFromUtf16BeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf16Be>(true);
//}
//
//TEST(MsgPackArchive, LoadFromUtf32LeStream) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf32Le>(false);
//}
//TEST(MsgPackArchive, LoadFromUtf32LeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf32Le>(true);
//}
//
//TEST(MsgPackArchive, LoadFromUtf32BeStream) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf32Be>(false);
//}
//TEST(MsgPackArchive, LoadFromUtf32BeStreamWithBom) {
//	TestLoadJsonFromEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf32Be>(true);
//}
//
//TEST(MsgPackArchive, SaveToUtf8Stream) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf8>(false);
//}
//TEST(MsgPackArchive, SaveToUtf8StreamWithBom) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf8>(true);
//}
//
//TEST(MsgPackArchive, SaveToUtf16LeStream) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf16Le>(false);
//}
//TEST(MsgPackArchive, SaveToUtf16LeStreamWithBom) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf16Le>(true);
//}
//
//TEST(MsgPackArchive, SaveToUtf16BeStream) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf16Be>(false);
//}
//TEST(MsgPackArchive, SaveToUtf16BeStreamWithBom) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf16Be>(true);
//}
//
//TEST(MsgPackArchive, SaveToUtf32LeStream) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf32Le>(false);
//}
//TEST(MsgPackArchive, SaveToUtf32LeStreamWithBom) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf32Le>(true);
//}
//
//TEST(MsgPackArchive, SaveToUtf32BeStream) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf32Be>(false);
//}
//TEST(MsgPackArchive, SaveToUtf32BeStreamWithBom) {
//	TestSaveJsonToEncodedStream<MsgPackArchive, BitSerializer::Convert::Utf32Be>(true);
//}
//
//TEST(MsgPackArchive, ThrowExceptionWhenUnsupportedStreamEncoding)
//{
//	BitSerializer::SerializationOptions serializationOptions;
//	serializationOptions.streamOptions.encoding = static_cast<BitSerializer::Convert::UtfType>(-1);
//	std::stringstream outputStream;
//	auto testObj = BuildFixture<TestClassWithSubTypes<std::string>>();
//	EXPECT_THROW(BitSerializer::SaveObject<MsgPackArchive>(testObj, outputStream, serializationOptions), BitSerializer::SerializationException);
//}
//
//TEST(MsgPackArchive, SerializeToFile) {
//	TestSerializeArrayToFile<MsgPackArchive>();
//}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
//TEST(MsgPackArchive, ThrowExceptionWhenBadSyntaxInSource)
//{
//	int testInt = 0;
//	EXPECT_THROW(BitSerializer::LoadObject<MsgPackArchive>(testInt, "10 }}"), BitSerializer::SerializationException);
//}
//
//TEST(MsgPackArchive, ThrowParsingExceptionWithCorrectPosition)
//{
//	const char* testJson = R"([
//	{ "x": 10, "y": 20},
//	{ "x": 11, y: 21}
//])";
//	TestPointClass testList[2];
//	try
//	{
//		BitSerializer::LoadObject<MsgPackArchive>(testList, testJson);
//		EXPECT_FALSE(true);
//	}
//	catch (const BitSerializer::ParsingException& ex)
//	{
//		EXPECT_TRUE(ex.Offset > 24 && ex.Offset < std::strlen(testJson));
//	}
//	catch (const std::exception&)
//	{
//		EXPECT_FALSE(true);
//	}
//}

//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ThrowValidationExceptionWhenMissedRequiredValue) {
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<TestPointClass>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<int[3]>>();
}

//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, bool>(MismatchedTypesPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, int32_t>(MismatchedTypesPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, float>(MismatchedTypesPolicy::ThrowError);
}

TEST(MsgPackArchive, ThrowSerializationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<MsgPackArchive, float, uint32_t>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<MsgPackArchive, double, uint32_t>(MismatchedTypesPolicy::ThrowError);
}

TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadNumberToString) {
	TestMismatchedTypesPolicy<MsgPackArchive, int32_t, std::string>(MismatchedTypesPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadFloatToInt) {
	TestMismatchedTypesPolicy<MsgPackArchive, float, int>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<MsgPackArchive, double, int>(MismatchedTypesPolicy::ThrowError);
}

TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, bool>(MismatchedTypesPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, int32_t>(MismatchedTypesPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, float>(MismatchedTypesPolicy::Skip);
}

TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<MsgPackArchive, float, uint32_t>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<MsgPackArchive, double, uint32_t>(MismatchedTypesPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadFloatToInt) {
	TestMismatchedTypesPolicy<MsgPackArchive, float, int>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<MsgPackArchive, double, int>(MismatchedTypesPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedTypesPolicy<MsgPackArchive, std::nullptr_t, bool>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<MsgPackArchive, std::nullptr_t, uint32_t>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<MsgPackArchive, std::nullptr_t, double>(MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------

TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<MsgPackArchive, int32_t, bool>(OverflowNumberPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowInt8) {
	TestOverflowNumberPolicy<MsgPackArchive, int16_t, int8_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<MsgPackArchive, uint16_t, uint8_t>(OverflowNumberPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowInt16) {
	TestOverflowNumberPolicy<MsgPackArchive, int32_t, int16_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<MsgPackArchive, uint32_t, uint16_t>(OverflowNumberPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowInt32) {
	TestOverflowNumberPolicy<MsgPackArchive, int64_t, int32_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<MsgPackArchive, uint64_t, uint32_t>(OverflowNumberPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowFloat) {
	TestOverflowNumberPolicy<MsgPackArchive, double, float>(OverflowNumberPolicy::ThrowError);
}

TEST(MsgPackArchive, ThrowValidationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<MsgPackArchive, int32_t, bool>(OverflowNumberPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenNumberOverflowInt8) {
	TestOverflowNumberPolicy<MsgPackArchive, int16_t, int8_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<MsgPackArchive, uint16_t, uint8_t>(OverflowNumberPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenNumberOverflowInt16) {
	TestOverflowNumberPolicy<MsgPackArchive, int32_t, int16_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<MsgPackArchive, uint32_t, uint16_t>(OverflowNumberPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenNumberOverflowInt32) {
	TestOverflowNumberPolicy<MsgPackArchive, int64_t, int32_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<MsgPackArchive, uint64_t, uint32_t>(OverflowNumberPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenNumberOverflowFloat) {
	TestOverflowNumberPolicy<MsgPackArchive, double, float>(OverflowNumberPolicy::Skip);
}
