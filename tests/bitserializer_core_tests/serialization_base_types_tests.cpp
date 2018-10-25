/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "pch.h"
#include <filesystem>
#include "../test_helpers/common_test_entities.h"
#include "../test_helpers/archive_stub.h"

//-----------------------------------------------------------------------------
// Tests of serialization for base types.
// For these purposes used archive stub which is simulate JSON structure.
//-----------------------------------------------------------------------------

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(BaseTypes, SerializeBoolean) {
	TestSerializeType<ArchiveStub, bool>(false);
	TestSerializeType<ArchiveStub, bool>(true);
}

TEST(BaseTypes, SerializeInteger) {
	TestSerializeType<ArchiveStub, int32_t>(std::numeric_limits<int8_t>::min());
	TestSerializeType<ArchiveStub, uint32_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<ArchiveStub, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<ArchiveStub, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(BaseTypes, SerializeFloat) {
	TestSerializeType<ArchiveStub, float>(::BuildFixture<float>());
}

TEST(BaseTypes, SerializeDouble) {
	TestSerializeType<ArchiveStub, double>(::BuildFixture<double>());
}

TEST(BaseTypes, SerializeEnum) {
	TestSerializeType<ArchiveStub, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::string and std::wstring (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(BaseTypes, SerializeString) {
	TestSerializeType<ArchiveStub, std::string>("Test ANSI string");
}

TEST(BaseTypes, SerializeWString) {
	TestSerializeType<ArchiveStub, std::wstring>(L"Test wide string");
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(BaseTypes, SerializeArrayOfBooleans) {
	TestSerializeArray<ArchiveStub, bool>();
}

TEST(BaseTypes, SerializeArrayOfIntegers) {
	TestSerializeArray<ArchiveStub, int8_t>();
	TestSerializeArray<ArchiveStub, int64_t>();
}

TEST(BaseTypes, SerializeArrayOfFloats) {
	TestSerializeArray<ArchiveStub, float>();
	TestSerializeArray<ArchiveStub, double>();
}

TEST(BaseTypes, SerializeArrayOfStrings) {
	TestSerializeArray<ArchiveStub, std::string>();
}

TEST(BaseTypes, SerializeArrayOfWStrings) {
	TestSerializeArray<ArchiveStub, std::wstring>();
}

TEST(BaseTypes, SerializeArrayOfClasses) {
	TestSerializeArray<ArchiveStub, TestPointClass>();
}

TEST(BaseTypes, SerializeTwoDimensionalArray) {
	TestSerializeTwoDimensionalArray<ArchiveStub, int32_t>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(BaseTypes, SerializeClassWithMemberBoolean) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubTypes<bool>>());
}

TEST(BaseTypes, SerializeClassWithMemberInteger) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
}

TEST(BaseTypes, SerializeClassWithMemberFloat) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubTypes<float>>());
}

TEST(BaseTypes, SerializeClassWithMemberDouble) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubTypes<double>>());
}

TEST(BaseTypes, SerializeClassWithMemberString) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
}

TEST(BaseTypes, SerializeClassHierarchy) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithInheritance>());
}

TEST(BaseTypes, SerializeClassWithMemberClass) {
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassType>());
}

TEST(BaseTypes, SerializeClassWithSubArray) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(BaseTypes, SerializeClassWithSubArrayOfClasses) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(BaseTypes, SerializeClassWithSubTwoDimArray) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

//-----------------------------------------------------------------------------
// Test the validation for named values (boolean result, which returned from archive methods).
//-----------------------------------------------------------------------------
TEST(BaseTypes, ShouldCollectErrorAboutRequiredNamedValues) {
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<TestPointClass>>();
}
