/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <filesystem>
#include "../test_helpers/common_test_methods.h"
#include "bitserializer_json_restcpp/json_restcpp_archive.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
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
	TestSerializeType<JsonArchive, float>(::BuildFixture<float>());
}

TEST(JsonRestCpp, SerializeDouble) {
	TestSerializeType<JsonArchive, double>(::BuildFixture<double>());
}

TEST(JsonRestCpp, SerializeEnum) {
	TestSerializeType<JsonArchive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::string and std::wstring (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, SerializeString) {
	TestSerializeType<JsonArchive, std::string>("Test ANSI string");
}

TEST(JsonRestCpp, SerializeWString) {
	TestSerializeType<JsonArchive, std::wstring>(L"Test wide string");
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, SerializeArrayOfBooleans) {
	TestSerializeArray<JsonArchive, bool>();
}

TEST(JsonRestCpp, SerializeArrayOfIntegers) {
	TestSerializeArray<JsonArchive, int8_t>();
	TestSerializeArray<JsonArchive, int64_t>();
}

TEST(JsonRestCpp, SerializeArrayOfFloats) {
	TestSerializeArray<JsonArchive, float>();
	TestSerializeArray<JsonArchive, double>();
}

TEST(JsonRestCpp, SerializeArrayOfStrings) {
	TestSerializeArray<JsonArchive, std::string>();
}

TEST(JsonRestCpp, SerializeArrayOfWStrings) {
	TestSerializeArray<JsonArchive, std::wstring>();
}

TEST(JsonRestCpp, SerializeArrayOfClasses) {
	TestSerializeArray<JsonArchive, TestPointClass>();
}

TEST(JsonRestCpp, SerializeTwoDimensionalArray) {
	TestSerializeTwoDimensionalArray<JsonArchive, int32_t>();
}

TEST(JsonRestCpp, ShouldLoadToArrayWithLesserAmountOfElements) {
	TestSerializeArray<JsonArchive, bool, 7, 5>();
	TestSerializeArray<JsonArchive, int64_t, 7, 5>();
	TestSerializeArray<JsonArchive, double, 7, 5>();
	TestSerializeArray<JsonArchive, std::string, 7, 5>();
	TestSerializeArray<JsonArchive, TestPointClass, 7, 5>();
}

TEST(JsonRestCpp, ShouldLoadToArrayWithBiggerAmountOfElements) {
	TestSerializeArray<JsonArchive, bool, 5, 7>();
	TestSerializeArray<JsonArchive, int64_t, 5, 7>();
	TestSerializeArray<JsonArchive, double, 5, 7>();
	TestSerializeArray<JsonArchive, std::string, 5, 7>();
	TestSerializeArray<JsonArchive, TestPointClass, 5, 7>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, SerializeClassWithMemberBoolean) {
	TestSerializeClass<JsonArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<JsonArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(JsonRestCpp, SerializeClassWithMemberInteger) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
}

TEST(JsonRestCpp, SerializeClassWithMemberFloat) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<float>>());
}

TEST(JsonRestCpp, SerializeClassWithMemberDouble) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<double>>());
}

TEST(JsonRestCpp, SerializeClassWithMemberString) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
}

TEST(JsonRestCpp, SerializeClassHierarchy) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithInheritance>());
}

TEST(JsonRestCpp, SerializeClassWithMemberClass) {
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassType>());
}

TEST(JsonRestCpp, SerializeClassWithSubArray) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(JsonRestCpp, SerializeClassWithSubArrayOfClasses) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(JsonRestCpp, SerializeClassWithSubTwoDimArray) {
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(JsonRestCpp, ShouldGetKeyByIndexInObjectScope) {
	TestGetKeyByIndex<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ShouldReturnPathInObjectScopeWhenLoading) {
	TestGetPathInObjectScopeWhenLoading<JsonArchive>();
}

TEST(JsonRestCpp, ShouldReturnPathInObjectScopeWhenSaving) {
	TestGetPathInObjectScopeWhenSaving<JsonArchive>();
}

TEST(JsonRestCpp, ShouldReturnPathInArrayScopeWhenLoading) {
	TestGetPathInArrayScopeWhenLoading<JsonArchive>();
}

TEST(JsonRestCpp, ShouldReturnPathInArrayScopeWhenSaving) {
	TestGetPathInArrayScopeWhenLoading<JsonArchive>();
}

//-----------------------------------------------------------------------------
// Test the validation for named values (boolean result, which returned from archive methods).
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ShouldCollectErrorAboutRequiredNamedValues) {
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<JsonArchive, TestClassForCheckValidation<TestPointClass>>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, SerializeClassToStream) {
	TestSerializeClassToStream<JsonArchive, utility::char_t>(BuildFixture<TestPointClass>());
}

TEST(JsonRestCpp, SerializeClassToFile) {
	TestSerializeClassToFile<JsonArchive, utility::char_t>(BuildFixture<TestPointClass>());
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(JsonRestCpp, ThrowExceptionWhenBadSyntaxInSource) {
	int testInt;
	EXPECT_THROW(LoadObject<JsonArchive>(testInt, _XPLATSTR("10 }}")), SerializationException);
}
