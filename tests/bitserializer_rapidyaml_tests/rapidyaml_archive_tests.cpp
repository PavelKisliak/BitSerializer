/*******************************************************************************
* Copyright (C) 2020-2023 by Artsiom Marozau, Pavel Kisliak                    *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/common_json_test_methods.h"
#include "testing_tools/common_yaml_test_methods.h"
#include "bitserializer/rapidyaml_archive.h"

using YamlArchive = BitSerializer::Yaml::RapidYaml::YamlArchive;

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, SerializeArrayOfBooleans)
{
	TestSerializeArray<YamlArchive, bool>();
}

TEST(RapidYamlArchive, SerializeArrayOfChars)
{
	TestSerializeArray<YamlArchive, char>();
	TestSerializeArray<YamlArchive, unsigned char>();
}

TEST(RapidYamlArchive, SerializeArrayOfIntegers)
{
	TestSerializeArray<YamlArchive, uint16_t>();
	TestSerializeArray<YamlArchive, int64_t>();
}

TEST(RapidYamlArchive, SerializeArrayOfFloats)
{
	TestSerializeArray<YamlArchive, float>();
	TestSerializeArray<YamlArchive, double>();
}

TEST(RapidYamlArchive, SerializeArrayOfNullptrs)
{
	TestSerializeArray<YamlArchive, std::nullptr_t>();
}

TEST(RapidYamlArchive, SerializeArrayOfStrings)
{
	TestSerializeArray<YamlArchive, std::string>();
}

TEST(RapidYamlArchive, SerializeArrayOfUnicodeStrings)
{
	TestSerializeArray<YamlArchive, std::wstring>();
	TestSerializeArray<YamlArchive, std::u16string>();
	TestSerializeArray<YamlArchive, std::u32string>();
}

TEST(RapidYamlArchive, SerializeArrayOfClasses)
{
	TestSerializeArray<YamlArchive, TestPointClass>();
}

TEST(RapidYamlArchive, SerializeTwoDimensionalArray)
{
	TestSerializeTwoDimensionalArray<YamlArchive, int32_t>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, SerializeClassWithMemberBoolean)
{
	TestSerializeClass<YamlArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<YamlArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(RapidYamlArchive, SerializeClassWithMemberInteger)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
	TestSerializeClass<YamlArchive>(TestClassWithSubTypes(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint64_t>::max()));
}

TEST(RapidYamlArchive, SerializeClassWithMemberFloat)
{
	TestSerializeClass<YamlArchive>(TestClassWithSubTypes(std::numeric_limits<float>::min(), 0.0f, std::numeric_limits<float>::max()));
}

TEST(RapidYamlArchive, SerializeClassWithMemberDouble)
{
	TestSerializeClass<YamlArchive>(TestClassWithSubTypes(std::numeric_limits<double>::min(), 0.0, std::numeric_limits<double>::max()));
}

TEST(RapidYamlArchive, SerializeClassWithMemberNullptr)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(RapidYamlArchive, SerializeClassWithMemberString)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring, std::u16string, std::u32string>>());
}

TEST(RapidYamlArchive, SerializeClassHierarchy)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithInheritance>());
}

TEST(RapidYamlArchive, SerializeClassWithMemberClass)
{
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassType>());
}

TEST(RapidYamlArchive, SerializeClassWithSubArray)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(RapidYamlArchive, SerializeClassWithSubArrayOfClasses)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(RapidYamlArchive, SerializeClassWithSubTwoDimArray)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(RapidYamlArchive, ShouldAllowToLoadBooleanFromInteger)
{
	TestClassWithSubType<bool> actual(false);
	BitSerializer::LoadObject<YamlArchive>(actual, "TestValue: 1");
	EXPECT_EQ(true, actual.GetValue());
}

TEST(RapidYamlArchive, ShouldAllowToLoadFloatFromInteger)
{
	TestClassWithSubType<float> actual(0);
	BitSerializer::LoadObject<YamlArchive>(actual, "TestValue: 100");
	EXPECT_EQ(100, actual.GetValue());
}

TEST(RapidYamlArchive, ShouldVisitKeysInObjectScopeWhenReadValues)
{
	TestVisitKeysInObjectScope<YamlArchive>();
}

TEST(RapidYamlArchive, ShouldVisitKeysInObjectScopeWhenSkipValues)
{
	TestVisitKeysInObjectScope<YamlArchive>(true);
}

TEST(RapidYamlArchive, SerializeClassInReverseOrder)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, float, std::string>>();
	TestSerializeClass<YamlArchive>(fixture);
}

TEST(RapidYamlArchive, SerializeClassInReverseOrderWithSubArray)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, std::array<uint64_t, 5>, std::string>>();
	TestSerializeClass<YamlArchive>(fixture);
}

TEST(RapidYamlArchive, SerializeClassInReverseOrderWithSubObject)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, TestPointClass, std::string>>();
	TestSerializeClass<YamlArchive>(fixture);
}

TEST(RapidYamlArchive, SerializeClassWithSkippingFields)
{
	TestClassWithVersioning arrayOfObjects[3];
	BuildFixture(arrayOfObjects);
	TestSerializeType<YamlArchive>(arrayOfObjects);
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ShouldReturnPathInObjectScopeWhenLoading)
{
	TestGetPathInJsonObjectScopeWhenLoading<YamlArchive>();
}

TEST(RapidYamlArchive, ShouldReturnPathInObjectScopeWhenSaving)
{
	TestGetPathInJsonObjectScopeWhenSaving<YamlArchive>();
}

TEST(RapidYamlArchive, ShouldReturnPathInArrayScopeWhenLoading)
{
	TestGetPathInJsonArrayScopeWhenLoading<YamlArchive>();
}

TEST(RapidYamlArchive, ShouldReturnPathInArrayScopeWhenSaving)
{
	TestGetPathInJsonArrayScopeWhenSaving<YamlArchive>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, SerializeClassToStream) {
	TestSerializeClassToStream<YamlArchive, char>(BuildFixture<TestPointClass>());
}

TEST(RapidYamlArchive, SerializeArrayOfClassesToStream)
{
	TestClassWithSubTypes<int, float, std::string, TestPointClass> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<YamlArchive, char>(testArray);
}

TEST(RapidYamlArchive, SerializeUnicodeToEncodedStream) {
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
	TestSerializeClassToStream<YamlArchive, char>(TestValue);
}

TEST(RapidYamlArchive, LoadFromUtf8Stream) {
	TestLoadYamlFromEncodedStream<YamlArchive, BitSerializer::Convert::Utf8>(false);
}
TEST(RapidYamlArchive, LoadFromUtf8StreamWithBom) {
	TestLoadYamlFromEncodedStream<YamlArchive, BitSerializer::Convert::Utf8>(true);
}

TEST(RapidYamlArchive, SaveToUtf8Stream) {
	TestSaveYamlToEncodedStream<YamlArchive, BitSerializer::Convert::Utf8>(false);
}
TEST(RapidYamlArchive, SaveToUtf8StreamWithBom) {
	TestSaveYamlToEncodedStream<YamlArchive, BitSerializer::Convert::Utf8>(true);
}

TEST(RapidYamlArchive, SerializeToFile) {
	TestSerializeArrayToFile<YamlArchive>();
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ThrowExceptionWhenBadSyntaxInSource)
{
	int testInt[2];
	EXPECT_THROW(BitSerializer::LoadObject<YamlArchive>(testInt, "- 10\n20"), BitSerializer::SerializationException);
}

TEST(RapidYamlArchive, ThrowParsingExceptionWithCorrectPosition)
{
	TestPointClass testList[2];
	try
	{
		const auto testYaml = "- 10\n- 20\n30";
		BitSerializer::LoadObject<YamlArchive>(testList, testYaml);
		EXPECT_FALSE(true);
	}
	catch (const BitSerializer::ParsingException& ex)
	{
		EXPECT_EQ(3, ex.Line);
	}
	catch (const std::exception&)
	{
		EXPECT_FALSE(true);
	}
}

//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ThrowValidationExceptionWhenMissedRequiredValue) {
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<TestPointClass>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<int[3]>>();
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ThrowMismatchedTypesExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<YamlArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(RapidYamlArchive, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<YamlArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(RapidYamlArchive, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<YamlArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(RapidYamlArchive, ThrowSerializationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<YamlArchive, float, uint32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<YamlArchive, double, uint32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(RapidYamlArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<YamlArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(RapidYamlArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<YamlArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::Skip
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<YamlArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(RapidYamlArchive, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<YamlArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(RapidYamlArchive, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<YamlArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<YamlArchive, std::string, double>(BitSerializer::MismatchedTypesPolicy::Skip);
}

TEST(RapidYamlArchive, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<YamlArchive, float, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<YamlArchive, double, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(RapidYamlArchive, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedTypesPolicy<YamlArchive, std::nullptr_t, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<YamlArchive, std::nullptr_t, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<YamlArchive, std::nullptr_t, double>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(RapidYamlArchive, ThrowValidationExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<YamlArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(RapidYamlArchive, ThrowValidationExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<YamlArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ThrowSerializationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<YamlArchive, int32_t, bool>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(RapidYamlArchive, ThrowSerializationExceptionWhenOverflowInt8) {
	TestOverflowNumberPolicy<YamlArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<YamlArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(RapidYamlArchive, ThrowSerializationExceptionWhenOverflowInt16) {
	TestOverflowNumberPolicy<YamlArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<YamlArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(RapidYamlArchive, ThrowSerializationExceptionWhenOverflowInt32) {
	TestOverflowNumberPolicy<YamlArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<YamlArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(RapidYamlArchive, ThrowSerializationExceptionWhenOverflowFloat) {
	TestOverflowNumberPolicy<YamlArchive, double, float>(BitSerializer::OverflowNumberPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::Skip
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ThrowValidationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<YamlArchive, int32_t, bool>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(RapidYamlArchive, ThrowValidationExceptionWhenNumberOverflowInt8) {
	TestOverflowNumberPolicy<YamlArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<YamlArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(RapidYamlArchive, ThrowValidationExceptionWhenNumberOverflowInt16) {
	TestOverflowNumberPolicy<YamlArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<YamlArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(RapidYamlArchive, ThrowValidationExceptionWhenNumberOverflowInt32) {
	TestOverflowNumberPolicy<YamlArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<YamlArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(RapidYamlArchive, ThrowValidationExceptionWhenNumberOverflowFloat) {
	TestOverflowNumberPolicy<YamlArchive, double, float>(BitSerializer::OverflowNumberPolicy::Skip);
}
