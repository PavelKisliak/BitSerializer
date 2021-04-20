/*******************************************************************************
* Copyright (C) 2020 by Artsiom Marozau                                        *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "test_helpers/common_test_methods.h"
#include "test_helpers/common_json_test_methods.h"
#include "test_helpers/common_yaml_test_methods.h"
#include "bitserializer/rapidyaml_archive.h"

using YamlArchive = BitSerializer::Yaml::RapidYaml::YamlArchive;

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, SerializeArrayOfBooleans)
{
	TestSerializeArray<YamlArchive, bool>();
}

TEST(RapidYamlArchive, SerializeArrayOfIntegers)
{
	TestSerializeArray<YamlArchive, int8_t>();
	TestSerializeArray<YamlArchive, int64_t>();
}

TEST(RapidYamlArchive, SerializeArrayOfFloats)
{
	TestSerializeArray<YamlArchive, float>();
	TestSerializeArray<YamlArchive, double>();
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

TEST(RapidYamlArchive, ShouldLoadToArrayWithLesserAmountOfElements)
{
	TestSerializeArray<YamlArchive, bool, 7, 5>();
	TestSerializeArray<YamlArchive, int, 7, 5>();
	TestSerializeArray<YamlArchive, double, 7, 5>();
	TestSerializeArray<YamlArchive, std::string, 7, 5>();
	TestSerializeArray<YamlArchive, TestPointClass, 7, 5>();
}

TEST(RapidYamlArchive, ShouldLoadToArrayWithBiggerAmountOfElements)
{
	TestSerializeArray<YamlArchive, bool, 5, 7>();
	TestSerializeArray<YamlArchive, int, 5, 7>();
	TestSerializeArray<YamlArchive, double, 5, 7>();
	TestSerializeArray<YamlArchive, std::string, 5, 7>();
	TestSerializeArray<YamlArchive, TestPointClass, 5, 7>();
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

TEST(RapidYamlArchive, ShouldIterateKeysInObjectScope)
{
	TestIterateKeysInObjectScope<YamlArchive>();
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
// Test the validation for named values (boolean result, which returns by archive's method SerializeValue()).
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ShouldCollectErrorAboutRequiredNamedValues)
{
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<TestPointClass>>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, SerializeClassToStream) {
	TestSerializeClassToStream<YamlArchive, char>(BuildFixture<TestPointClass>());
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

TEST(RapidYamlArchive, SerializeClassToFile) {
	TestSerializeClassToFile<YamlArchive>(BuildFixture<TestPointClass>());
	TestSerializeClassToFile<YamlArchive>(BuildFixture<TestPointClass>());
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ThrowExceptionWhenBadSyntaxInSource)
{
	int testInt[1];
	EXPECT_THROW(BitSerializer::LoadObject<YamlArchive>(testInt, "10 }}"), BitSerializer::SerializationException);
}
