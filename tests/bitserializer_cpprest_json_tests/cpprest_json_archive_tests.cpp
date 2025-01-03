/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/common_json_test_methods.h"
#include "bitserializer/cpprestjson_archive.h"

using BitSerializer::Json::CppRest::JsonArchive;

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, SaveBooleanAsTrueFalse)
{
	bool value = false;
	EXPECT_EQ("false", BitSerializer::SaveObject<JsonArchive>(value));
	value = true;
	EXPECT_EQ("true", BitSerializer::SaveObject<JsonArchive>(value));
}

TEST(JsonRestCpp, SerializeBoolean) {
	TestSerializeType<JsonArchive, bool>(false);
	TestSerializeType<JsonArchive, bool>(true);
}

TEST(JsonRestCpp, SerializeInteger) {
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<JsonArchive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<JsonArchive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(JsonRestCpp, SerializeFloat) {
	TestSerializeType<JsonArchive, float>(std::numeric_limits<float>::min());
	TestSerializeType<JsonArchive, float>(std::numeric_limits<float>::max());
}

TEST(JsonRestCpp, SerializeDouble) {
	TestSerializeType<JsonArchive, double>(std::numeric_limits<double>::min());
	TestSerializeType<JsonArchive, double>(std::numeric_limits<double>::max());
}

TEST(JsonRestCpp, ShouldAllowToLoadBooleanFromInteger)
{
	bool actual = false;
	BitSerializer::LoadObject<JsonArchive>(actual, "1");
	EXPECT_EQ(true, actual);
}

TEST(JsonRestCpp, ShouldAllowToLoadFloatFromInteger)
{
	float actual = 0;
	BitSerializer::LoadObject<JsonArchive>(actual, "100");
	EXPECT_EQ(100, actual);
}

TEST(JsonRestCpp, SerializeNullptr)
{
	TestSerializeType<JsonArchive, std::nullptr_t>(nullptr);
}

//-----------------------------------------------------------------------------
// Tests of serialization any of std::string (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, SerializeUtf8Sting) {
	TestSerializeType<JsonArchive, std::string>("Test ANSI string");
	TestSerializeType<JsonArchive, std::string>(UTF8("Test UTF8 string - Привет мир!"));
}

TEST(JsonRestCpp, SerializeUnicodeString) {
	TestSerializeType<JsonArchive, std::wstring>(L"Test wide string - Привет мир!");
	TestSerializeType<JsonArchive, std::u16string>(u"Test UTF-16 string - Привет мир!");
	TestSerializeType<JsonArchive, std::u32string>(U"Test UTF-32 string - Привет мир!");
}

TEST(JsonRestCpp, SerializeEnum) {
	TestSerializeType<JsonArchive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, SerializeArrayOfBooleans) {
	TestSerializeArray<JsonArchive, bool>();
}

TEST(JsonRestCpp, SerializeArrayOfChars)
{
	TestSerializeArray<JsonArchive, char>();
	TestSerializeArray<JsonArchive, unsigned char>();
}

TEST(JsonRestCpp, SerializeArrayOfIntegers)
{
	TestSerializeArray<JsonArchive, uint16_t>();
	TestSerializeArray<JsonArchive, int64_t>();
}

TEST(JsonRestCpp, SerializeArrayOfFloats) {
	TestSerializeArray<JsonArchive, float>();
	TestSerializeArray<JsonArchive, double>();
}

TEST(JsonRestCpp, SerializeArrayOfNullptrs)
{
	TestSerializeArray<JsonArchive, std::nullptr_t>();
}

TEST(JsonRestCpp, SerializeArrayOfStrings) {
	TestSerializeArray<JsonArchive, std::string>();
}

TEST(JsonRestCpp, SerializeArrayOfUnicodeStrings) {
	TestSerializeArray<JsonArchive, std::wstring>();
	TestSerializeArray<JsonArchive, std::u16string>();
	TestSerializeArray<JsonArchive, std::u32string>();
}

TEST(JsonRestCpp, SerializeArrayOfClasses) {
	TestSerializeArray<JsonArchive, TestPointClass>();
}

TEST(JsonRestCpp, SerializeTwoDimensionalArray) {
	TestSerializeTwoDimensionalArray<JsonArchive, int32_t>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, SerializeClassWithMemberBoolean) {
	TestSerializeType<JsonArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeType<JsonArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(JsonRestCpp, SerializeClassWithMemberInteger) {
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
	TestSerializeType<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint64_t>::max()));
}

TEST(JsonRestCpp, SerializeClassWithMemberFloat) {
	TestSerializeType<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<float>::min(), 0.0f, std::numeric_limits<float>::max()));
}

TEST(JsonRestCpp, SerializeClassWithMemberDouble) {
	TestSerializeType<JsonArchive>(TestClassWithSubTypes(std::numeric_limits<double>::min(), 0.0, std::numeric_limits<double>::max()));
}

TEST(JsonRestCpp, SerializeClassWithMemberNullptr)
{
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(JsonRestCpp, SerializeClassWithMemberString) {
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring, std::u16string, std::u32string>>());
}

TEST(JsonRestCpp, SerializeClassHierarchy) {
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithInheritance>());
}

TEST(JsonRestCpp, SerializeClassWithMemberClass) {
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeType<JsonArchive>(BuildFixture<TestClassType>());
}

TEST(JsonRestCpp, SerializeClassWithSubArray) {
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(JsonRestCpp, SerializeClassWithSubArrayOfClasses) {
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(JsonRestCpp, SerializeClassWithSubTwoDimArray) {
	TestSerializeType<JsonArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(JsonRestCpp, ShouldVisitKeysInObjectScopeWhenReadValues)
{
	TestVisitKeysInObjectScope<JsonArchive>();
}

TEST(JsonRestCpp, ShouldVisitKeysInObjectScopeWhenSkipValues)
{
	TestVisitKeysInObjectScope<JsonArchive>(true);
}

TEST(JsonRestCpp, SerializeClassInReverseOrder)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, float, std::string>>();
	TestSerializeType<JsonArchive>(fixture);
}

TEST(JsonRestCpp, SerializeClassInReverseOrderWithSubArray)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, std::array<uint64_t, 5>, std::string>>();
	TestSerializeType<JsonArchive>(fixture);
}

TEST(JsonRestCpp, SerializeClassInReverseOrderWithSubObject)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, TestPointClass, std::string>>();
	TestSerializeType<JsonArchive>(fixture);
}

TEST(JsonRestCpp, SerializeClassWithSkippingFields)
{
	TestClassWithVersioning arrayOfObjects[3];
	BuildFixture(arrayOfObjects);
	TestSerializeType<JsonArchive>(arrayOfObjects);
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ShouldReturnPathInObjectScopeWhenLoading) {
	TestGetPathInJsonObjectScopeWhenLoading<JsonArchive>();
}

TEST(JsonRestCpp, ShouldReturnPathInObjectScopeWhenSaving) {
	TestGetPathInJsonObjectScopeWhenSaving<JsonArchive>();
}

TEST(JsonRestCpp, ShouldReturnPathInArrayScopeWhenLoading) {
	TestGetPathInJsonArrayScopeWhenLoading<JsonArchive>();
}

TEST(JsonRestCpp, ShouldReturnPathInArrayScopeWhenSaving) {
	TestGetPathInJsonArrayScopeWhenSaving<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, SerializeClassToStream) {
	TestSerializeClassToStream<JsonArchive>(BuildFixture<TestPointClass>());
}

TEST(JsonRestCpp, SerializeArrayOfClassesToStream)
{
	TestClassWithSubTypes<int, float, std::string, TestPointClass> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<JsonArchive>(testArray);
}

TEST(JsonRestCpp, SerializeUnicodeToEncodedStream) {
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
	TestSerializeClassToStream<JsonArchive>(TestValue);
}

TEST(JsonRestCpp, LoadFromUtf8Stream) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(false);
}

TEST(JsonRestCpp, LoadFromUtf8StreamWithBom) {
	TestLoadJsonFromEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(true);
}

TEST(JsonRestCpp, SaveToUtf8Stream) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(false);
}

TEST(JsonRestCpp, SaveToUtf8StreamWithBom) {
	TestSaveJsonToEncodedStream<JsonArchive, BitSerializer::Convert::Utf::Utf8>(true);
}

TEST(JsonRestCpp, SerializeToFile) {
	TestSerializeArrayToFile<JsonArchive>();
	TestSerializeArrayToFile<JsonArchive>(true);
}

TEST(JsonRestCpp, SerializeToFileThrowExceptionWhenAlreadyExists) {
	TestThrowExceptionWhenFileAlreadyExists<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ThrowExceptionWhenBadSyntaxInSource) {
	int testInt = 0;
	EXPECT_THROW(BitSerializer::LoadObject<JsonArchive>(testInt, "10 }}"), BitSerializer::ParsingException);
}

//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ThrowValidationExceptionWhenMissedRequiredValue) {
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
TEST(JsonRestCpp, ThrowMismatchedTypesExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonRestCpp, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonRestCpp, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(JsonRestCpp, ThrowMismatchedTypesExceptionWhenLoadNumberToString) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, std::string>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonRestCpp, ThrowMismatchedTypesExceptionWhenLoadFloatToInt) {
	TestMismatchedTypesPolicy<JsonArchive, float, int>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<JsonArchive, double, int>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(JsonRestCpp, ThrowMismatchedTypesExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(JsonRestCpp, ThrowMismatchedTypesExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::Skip
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonRestCpp, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonRestCpp, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<JsonArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, std::string, double>(BitSerializer::MismatchedTypesPolicy::Skip);
}

TEST(JsonRestCpp, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<JsonArchive, float, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, double, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonRestCpp, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedTypesPolicy<JsonArchive, std::nullptr_t, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<JsonArchive, std::nullptr_t, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<JsonArchive, std::nullptr_t, double>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(JsonRestCpp, ThrowValidationExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(JsonRestCpp, ThrowValidationExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<JsonArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ThrowSerializationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, bool>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(JsonRestCpp, ThrowSerializationExceptionWhenOverflowInt8) {
	TestOverflowNumberPolicy<JsonArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<JsonArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(JsonRestCpp, ThrowSerializationExceptionWhenOverflowInt16) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<JsonArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(JsonRestCpp, ThrowSerializationExceptionWhenOverflowInt32) {
	TestOverflowNumberPolicy<JsonArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<JsonArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(JsonRestCpp, ThrowSerializationExceptionWhenOverflowFloat) {
	TestOverflowNumberPolicy<JsonArchive, double, float>(BitSerializer::OverflowNumberPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::Skip
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ThrowValidationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, bool>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(JsonRestCpp, ThrowValidationExceptionWhenNumberOverflowInt8) {
	TestOverflowNumberPolicy<JsonArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<JsonArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(JsonRestCpp, ThrowValidationExceptionWhenNumberOverflowInt16) {
	TestOverflowNumberPolicy<JsonArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<JsonArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(JsonRestCpp, ThrowValidationExceptionWhenNumberOverflowInt32) {
	TestOverflowNumberPolicy<JsonArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<JsonArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(JsonRestCpp, ThrowValidationExceptionWhenNumberOverflowFloat) {
	TestOverflowNumberPolicy<JsonArchive, double, float>(BitSerializer::OverflowNumberPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test UtfEncodingErrorPolicy
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ThrowSerializationExceptionWhenEncodingError) {
	TestEncodingPolicy<JsonArchive>(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::ThrowError);
}

// ToDo: This test is disabled due to difficulties with handling encoding error in the CppRestJson library (policy `UtfEncodingErrorPolicy::Skip` does not work)
//TEST(JsonRestCpp, ShouldSkipInvalidUtfWhenPolicyIsSkip) {
//	TestEncodingPolicy<JsonArchive>(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip);
//}
