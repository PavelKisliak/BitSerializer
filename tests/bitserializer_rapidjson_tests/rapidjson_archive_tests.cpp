/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/

// Define for suppress warning STL4015 : The std::iterator class template (used as a base class to provide typedefs) is deprecated in C++17.
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include "../test_helpers/common_test_methods.h"
#include "../test_helpers/common_json_test_methods.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"

using BitSerializer::Json::RapidJson::JsonArchive;

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeBoolean) {
	TestSerializeType<JsonArchive, bool>(false);
	TestSerializeType<JsonArchive, bool>(true);
}

TEST(RapidJsonArchive, SerializeInteger) {
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<JsonArchive, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<JsonArchive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<JsonArchive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(RapidJsonArchive, SerializeFloat) {
	TestSerializeType<JsonArchive, float>(::BuildFixture<float>());
}

TEST(RapidJsonArchive, SerializeDouble) {
	TestSerializeType<JsonArchive, double>(::BuildFixture<double>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::string and std::wstring (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeString) {
	TestSerializeType<JsonArchive, std::string>("Test ANSI string");
}

TEST(RapidJsonArchive, SerializeWString) {
	TestSerializeType<JsonArchive, std::wstring>(L"Test wide string");
}

TEST(RapidJsonArchive, SerializeEnum) {
	TestSerializeType<JsonArchive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeArrayOfBooleans) {
	TestSerializeArray<JsonArchive, bool>();
}

TEST(RapidJsonArchive, SerializeArrayOfIntegers) {
	TestSerializeArray<JsonArchive, int8_t>();
	TestSerializeArray<JsonArchive, int64_t>();
}

TEST(RapidJsonArchive, SerializeArrayOfFloats) {
	TestSerializeArray<JsonArchive, float>();
	TestSerializeArray<JsonArchive, double>();
}

TEST(RapidJsonArchive, SerializeArrayOfStrings) {
	TestSerializeArray<JsonArchive, std::string>();
}

TEST(RapidJsonArchive, SerializeArrayOfWStrings) {
	TestSerializeArray<JsonArchive, std::wstring>();
}

TEST(RapidJsonArchive, SerializeArrayOfClasses) {
	TestSerializeArray<JsonArchive, TestPointClass>();
}

TEST(RapidJsonArchive, SerializeTwoDimensionalArray) {
	TestSerializeTwoDimensionalArray<JsonArchive, int32_t>();
}

TEST(RapidJsonArchive, ShouldLoadToArrayWithLesserAmountOfElements) {
	TestSerializeArray<JsonArchive, bool, 7, 5>();
	TestSerializeArray<JsonArchive, int, 7, 5>();
	TestSerializeArray<JsonArchive, double, 7, 5>();
	TestSerializeArray<JsonArchive, std::string, 7, 5>();
	TestSerializeArray<JsonArchive, TestPointClass, 7, 5>();
}

TEST(RapidJsonArchive, ShouldLoadToArrayWithBiggerAmountOfElements) {
	TestSerializeArray<JsonArchive, bool, 5, 7>();
	TestSerializeArray<JsonArchive, int, 5, 7>();
	TestSerializeArray<JsonArchive, double, 5, 7>();
	TestSerializeArray<JsonArchive, std::string, 5, 7>();
	TestSerializeArray<JsonArchive, TestPointClass, 5, 7>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeClassWithMemberBoolean) {
	TestSerializeClass<JsonArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<JsonArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(RapidJsonArchive, SerializeClassWithMemberInteger) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberFloat) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<float>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberDouble) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<double>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberString) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
}

TEST(RapidJsonArchive, SerializeClassHierarchy) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithInheritance>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberClass) {
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassType>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArray) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArrayOfClasses) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubTwoDimArray) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(RapidJsonArchive, ShouldIterateKeysInObjectScope) {
	TestIterateKeysInObjectScope<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ShouldReturnPathInObjectScopeWhenLoading) {
	TestGetPathInJsonObjectScopeWhenLoading<JsonArchive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInObjectScopeWhenSaving) {
	TestGetPathInJsonObjectScopeWhenSaving<JsonArchive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInArrayScopeWhenLoading) {
	TestGetPathInJsonArrayScopeWhenLoading<JsonArchive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInArrayScopeWhenSaving) {
	TestGetPathInJsonArrayScopeWhenSaving<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Test the validation for named values (boolean result, which returned from archive methods).
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ShouldCollectErrorAboutRequiredNamedValues) {
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<TestPointClass>>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeClassToWStream) {
	TestSerializeClassToStream<JsonArchive, wchar_t>(BuildFixture<TestPointClass>());
}

TEST(RapidJsonArchive, SerializeClassToFile) {
	TestSerializeClassToFile<JsonArchive, wchar_t>(BuildFixture<TestPointClass>());
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowExceptionWhenBadSyntaxInSource) {
	int testInt;
	EXPECT_THROW(BitSerializer::LoadObject<JsonArchive>(testInt, L"10 }}"), BitSerializer::SerializationException);
}
