/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "../test_helpers/common_test_methods.h"
#include "../test_helpers/common_json_test_methods.h"
#include "bitserializer/rapidjson_archive.h"

using BitSerializer::Json::RapidJson::JsonArchive;

#pragma warning(push)
#pragma warning(disable: 4566)

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeBoolean)
{
	TestSerializeType<JsonArchive, bool>(false);
	TestSerializeType<JsonArchive, bool>(true);
}

TEST(RapidJsonArchive, SerializeInteger)
{
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<JsonArchive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<JsonArchive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(RapidJsonArchive, SerializeFloat)
{
	TestSerializeType<JsonArchive, float>(std::numeric_limits<float>::min());
	TestSerializeType<JsonArchive, float>(std::numeric_limits<float>::max());
}

TEST(RapidJsonArchive, SerializeDouble)
{
	TestSerializeType<JsonArchive, double>(std::numeric_limits<double>::min());
	TestSerializeType<JsonArchive, double>(std::numeric_limits<double>::max());
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
	TestSerializeType<JsonArchive, std::string>(u8"Test UTF8 string - Привет мир!");
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

TEST(RapidJsonArchive, SerializeArrayOfIntegers)
{
	TestSerializeArray<JsonArchive, int8_t>();
	TestSerializeArray<JsonArchive, int64_t>();
}

TEST(RapidJsonArchive, SerializeArrayOfFloats)
{
	TestSerializeArray<JsonArchive, float>();
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

TEST(RapidJsonArchive, ShouldLoadToArrayWithLesserAmountOfElements)
{
	TestSerializeArray<JsonArchive, bool, 7, 5>();
	TestSerializeArray<JsonArchive, int, 7, 5>();
	TestSerializeArray<JsonArchive, double, 7, 5>();
	TestSerializeArray<JsonArchive, std::string, 7, 5>();
	TestSerializeArray<JsonArchive, TestPointClass, 7, 5>();
}

TEST(RapidJsonArchive, ShouldLoadToArrayWithBiggerAmountOfElements)
{
	TestSerializeArray<JsonArchive, bool, 5, 7>();
	TestSerializeArray<JsonArchive, int, 5, 7>();
	TestSerializeArray<JsonArchive, double, 5, 7>();
	TestSerializeArray<JsonArchive, std::string, 5, 7>();
	TestSerializeArray<JsonArchive, TestPointClass, 5, 7>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeClassWithMemberBoolean)
{
	TestSerializeClass<JsonArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<JsonArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(RapidJsonArchive, SerializeClassWithMemberInteger)
{
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
	TestSerializeClass<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint64_t>::max()));
}

TEST(RapidJsonArchive, SerializeClassWithMemberFloat)
{
	TestSerializeClass<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<float>::min(), 0.0f, std::numeric_limits<float>::max()));
}

TEST(RapidJsonArchive, SerializeClassWithMemberDouble)
{
	TestSerializeClass<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<double>::min(), 0.0, std::numeric_limits<double>::max()));
}

TEST(RapidJsonArchive, SerializeClassWithMemberNullptr)
{
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberString)
{
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring, std::u16string, std::u32string>>());
}

TEST(RapidJsonArchive, SerializeClassHierarchy)
{
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithInheritance>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberClass)
{
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassType>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArray)
{
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArrayOfClasses)
{
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubTwoDimArray)
{
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(RapidJsonArchive, ShouldIterateKeysInObjectScope)
{
	TestIterateKeysInObjectScope<JsonArchive>();
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
// Test the validation for named values (boolean result, which returns by archive's method SerializeValue()).
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ShouldCollectErrorAboutRequiredNamedValues)
{
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<TestPointClass>>();
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
	TestSerializeClassToStream<JsonArchive, char>(BuildFixture<TestPointClass>());
}

TEST(RapidJsonArchive, SerializeUnicodeToEncodedStream) {
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
	TestSerializeClassToStream<JsonArchive, char>(TestValue);
}

TEST(RapidJsonArchive, LoadFromUtf8Stream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf8>(false);
}
TEST(RapidJsonArchive, LoadFromUtf8StreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf8>(true);
}

TEST(RapidJsonArchive, LoadFromUtf16LeStream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf16Le>(false);
}
TEST(RapidJsonArchive, LoadFromUtf16LeStreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf16Le>(true);
}

TEST(RapidJsonArchive, LoadFromUtf16BeStream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf16Be>(false);
}
TEST(RapidJsonArchive, LoadFromUtf16BeStreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf16Be>(true);
}

TEST(RapidJsonArchive, LoadFromUtf32LeStream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf32Le>(false);
}
TEST(RapidJsonArchive, LoadFromUtf32LeStreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf32Le>(true);
}

TEST(RapidJsonArchive, LoadFromUtf32BeStream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf32Be>(false);
}
TEST(RapidJsonArchive, LoadFromUtf32BeStreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf32Be>(true);
}

TEST(RapidJsonArchive, SaveToUtf8Stream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf8>(false);
}
TEST(RapidJsonArchive, SaveToUtf8StreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf8>(true);
}

TEST(RapidJsonArchive, SaveToUtf16LeStream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf16Le>(false);
}
TEST(RapidJsonArchive, SaveToUtf16LeStreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf16Le>(true);
}

TEST(RapidJsonArchive, SaveToUtf16BeStream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf16Be>(false);
}
TEST(RapidJsonArchive, SaveToUtf16BeStreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf16Be>(true);
}

TEST(RapidJsonArchive, SaveToUtf32LeStream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf32Le>(false);
}
TEST(RapidJsonArchive, SaveToUtf32LeStreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf32Le>(true);
}

TEST(RapidJsonArchive, SaveToUtf32BeStream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf32Be>(false);
}
TEST(RapidJsonArchive, SaveToUtf32BeStreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf32Be>(true);
}

TEST(RapidJsonArchive, ThrowExceptionWhenUnsupportedStreamEncoding)
{
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.streamOptions.encoding = static_cast<BitSerializer::Convert::UtfType>(-1);
	std::stringstream outputStream;
	auto testObj = BuildFixture<TestClassWithSubTypes<std::string>>();
	EXPECT_THROW(BitSerializer::SaveObject<JsonArchive>(testObj, outputStream, serializationOptions), BitSerializer::SerializationException);
}

TEST(RapidJsonArchive, SerializeClassToFile) {
	TestSerializeClassToFile<JsonArchive>(BuildFixture<TestPointClass>());
	TestSerializeClassToFile<JsonArchive>(BuildFixture<TestPointClass>());
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowExceptionWhenBadSyntaxInSource)
{
	int testInt = 0;
	EXPECT_THROW(BitSerializer::LoadObject<JsonArchive>(testInt, "10 }}"), BitSerializer::SerializationException);
}

#pragma warning(pop)
