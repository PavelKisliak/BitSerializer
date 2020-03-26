﻿/*******************************************************************************
* Copyright (C) 2020 by Artsiom Marozau                                        *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "../test_helpers/common_test_methods.h"
#include "../test_helpers/common_json_test_methods.h"
#include "../test_helpers/common_yaml_test_methods.h"
#include "bitserializer_rapidyaml/rapidyaml_archive.h"

using BitSerializer::Yaml::RapidYaml::YamlArchive;

//TODO: remove tests for serialize single value
//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, SerializeBoolean)
{
	TestSerializeType<YamlArchive, bool>(false);
	TestSerializeType<YamlArchive, bool>(true);
}

TEST(RapidYamlArchive, SerializeInteger)
{
	TestSerializeType<YamlArchive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<YamlArchive, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<YamlArchive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<YamlArchive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(RapidYamlArchive, SerializeFloat)
{
	TestSerializeType<YamlArchive, float>(::BuildFixture<float>());
}

TEST(RapidYamlArchive, SerializeDouble)
{
	TestSerializeType<YamlArchive, double>(::BuildFixture<double>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::string and std::wstring (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, SerializeAnsiString)
{
	TestSerializeType<YamlArchive, std::string>("Test ANSI string");
}

TEST(RapidYamlArchive, SerializeUnicodeString)
{
	TestSerializeType<YamlArchive, std::wstring>(L"Test Unicode string - Привет мир!");
}

TEST(RapidYamlArchive, SerializeEnum)
{
	TestSerializeType<YamlArchive, TestEnum>(TestEnum::Two);
}

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

//TODO: Fix test below
/*
TEST(RapidYamlArchive, SerializeArrayOfWStrings)
{
	TestSerializeArray<YamlArchive, std::wstring>();
}
*/

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
}

TEST(RapidYamlArchive, SerializeClassWithMemberFloat)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<float>>());
}

TEST(RapidYamlArchive, SerializeClassWithMemberDouble)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<double>>());
}

TEST(RapidYamlArchive, SerializeClassWithMemberString)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
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

/*
//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, ThrowExceptionWhenBadSyntaxInSource)
{
	int testInt;
	EXPECT_THROW(BitSerializer::LoadObject<YamlArchive>(testInt, "10 }}"), BitSerializer::SerializationException);
}
*/

TEST(RapidYamlArchive, Stub) {
	
}