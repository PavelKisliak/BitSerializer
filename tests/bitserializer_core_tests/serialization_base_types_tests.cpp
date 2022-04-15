/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "test_helpers/archive_stub.h"
#include "test_helpers/common_test_methods.h"
#include "test_helpers/common_json_test_methods.h"

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
	TestSerializeType<ArchiveStub, int8_t>(std::numeric_limits<int8_t>::min());
	TestSerializeType<ArchiveStub, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<ArchiveStub, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<ArchiveStub, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(BaseTypes, SerializeFloat) {
	TestSerializeType<ArchiveStub, float>(::BuildFixture<float>());
}

TEST(BaseTypes, SerializeDouble) {
	TestSerializeType<ArchiveStub, double>(::BuildFixture<double>());
}

TEST(BaseTypes, SerializeNullptr) {
	TestSerializeType<ArchiveStub, std::nullptr_t>(nullptr);
}

//-----------------------------------------------------------------------------
// Tests of serialization any of std::string (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(BaseTypes, SerializeUtf8Sting)
{
	TestSerializeType<ArchiveStub, std::string>("Test ANSI string");
	TestSerializeType<ArchiveStub, std::string>(u8"Test UTF8 string - Привет мир!");
}

TEST(BaseTypes, SerializeUnicodeString)
{
	TestSerializeType<ArchiveStub, std::wstring>(L"Test wide string - Привет мир!");
	TestSerializeType<ArchiveStub, std::u16string>(u"Test UTF-16 string - Привет мир!");
	TestSerializeType<ArchiveStub, std::u32string>(U"Test UTF-32 string - Привет мир!");
}

TEST(BaseTypes, SerializeEnum) {
	TestSerializeType<ArchiveStub, TestEnum>(TestEnum::Two);
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

TEST(BaseTypes, SerializeArrayOfNullptrs) {
	TestSerializeArray<ArchiveStub, std::nullptr_t>();
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

TEST(BaseTypes, ShouldThrowExceptionWhenLoadToArrayWithLesserAmountOfElements) {
	EXPECT_THROW((TestSerializeArray<ArchiveStub, bool, 7, 5>()), BitSerializer::SerializationException);
	EXPECT_THROW((TestSerializeArray<ArchiveStub, int64_t, 7, 5>()), BitSerializer::SerializationException);
	EXPECT_THROW((TestSerializeArray<ArchiveStub, double, 7, 5>()), BitSerializer::SerializationException);
	EXPECT_THROW((TestSerializeArray<ArchiveStub, std::string, 7, 5>()), BitSerializer::SerializationException);
	EXPECT_THROW((TestSerializeArray<ArchiveStub, TestPointClass, 7, 5>()), BitSerializer::SerializationException);
}

TEST(BaseTypes, ShouldThrowExceptionWhenLoadToArrayWithBiggerAmountOfElements) {
	EXPECT_THROW((TestSerializeArray<ArchiveStub, bool, 5, 7>()), BitSerializer::SerializationException);
	EXPECT_THROW((TestSerializeArray<ArchiveStub, int64_t, 5, 7>()), BitSerializer::SerializationException);
	EXPECT_THROW((TestSerializeArray<ArchiveStub, double, 5, 7>()), BitSerializer::SerializationException);
	EXPECT_THROW((TestSerializeArray<ArchiveStub, std::string, 5, 7>()), BitSerializer::SerializationException);
	EXPECT_THROW((TestSerializeArray<ArchiveStub, TestPointClass, 5, 7>()), BitSerializer::SerializationException);
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes and unions
//-----------------------------------------------------------------------------
TEST(BaseTypes, SerializeClassWithMemberBoolean) {
	TestSerializeClass<ArchiveStub>(TestClassWithSubType<bool>(false));
	TestSerializeClass<ArchiveStub>(TestClassWithSubType<bool>(true));
}

TEST(BaseTypes, SerializeUnion) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestUnion>());
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

TEST(BaseTypes, SerializeClassWithMemberNullptr) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
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

TEST(BaseTypes, ShouldIterateKeysInObjectScope) {
	TestIterateKeysInObjectScope<ArchiveStub>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes with globally defined SerializeObject() function
//-----------------------------------------------------------------------------
class TestGlobalSerializeObjectFixture
{
public:
	static void BuildFixture(TestGlobalSerializeObjectFixture& fixture) {
		::BuildFixture(fixture.value);
	}

	void Assert(const TestGlobalSerializeObjectFixture& rhs) const {
		EXPECT_EQ(value, rhs.value);
	}

	bool operator==(const TestGlobalSerializeObjectFixture& rhs) const noexcept {
		return value == rhs.value;
	}

	int value = 0;
};

template<typename TArchive>
void SerializeObject(TArchive& archive, TestGlobalSerializeObjectFixture& fixture)
{
	archive << MakeAutoKeyValue("Value", fixture.value);
}

TEST(BaseTypes, ShouldSerializeClassViaGlobalSerializeObject) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestGlobalSerializeObjectFixture>());
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubTypes<TestGlobalSerializeObjectFixture>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes which represents list of objects with globally defined SerializeArray() function
//-----------------------------------------------------------------------------
class TestGlobalSerializeArrayFixture
{
public:
	static void BuildFixture(TestGlobalSerializeArrayFixture& fixture) {
		::BuildFixture(fixture.values);
	}

	void Assert(const TestGlobalSerializeArrayFixture& rhs) const {
		for (size_t c = 0; c < sizeof values / sizeof(int); c++) {
			EXPECT_EQ(values[c], rhs.values[c]);
		}
	}

	bool operator==(const TestGlobalSerializeArrayFixture& rhs) const noexcept {
		for (size_t c = 0; c < sizeof values / sizeof(int); c++) {
			if (values[c] != rhs.values[c]) {
				return false;
			}
		}
		return true;
	}

	int values[3];
};

template<typename TArchive>
void SerializeArray(TArchive& archive, TestGlobalSerializeArrayFixture& fixture)
{
	for (int& value : fixture.values) {
		archive << value;
	}
}

TEST(BaseTypes, ShouldSerializeArrayViaGlobalSerializeArray) {
	TestSerializeClass<ArchiveStub>(BuildFixture<TestGlobalSerializeArrayFixture>());
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubTypes<TestGlobalSerializeArrayFixture>>());
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(BaseTypes, ShouldReturnPathInObjectScopeWhenLoading) {
	TestGetPathInJsonObjectScopeWhenLoading<ArchiveStub>();
}

TEST(BaseTypes, ShouldReturnPathInObjectScopeWhenSaving) {
	TestGetPathInJsonObjectScopeWhenSaving<ArchiveStub>();
}

TEST(BaseTypes, ShouldReturnPathInArrayScopeWhenLoading) {
	TestGetPathInJsonArrayScopeWhenLoading<ArchiveStub>();
}

TEST(BaseTypes, ShouldReturnPathInArrayScopeWhenSaving) {
	TestGetPathInJsonArrayScopeWhenSaving<ArchiveStub>();
}

//-----------------------------------------------------------------------------
// Test the validation for named values (boolean result, which returned from archive methods).
//-----------------------------------------------------------------------------
TEST(BaseTypes, ShouldCollectErrorsAboutRequiredNamedValues) {
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<TestPointClass>>();
}

TEST(BaseTypes, ShouldCollectErrorsWhenLoadingFromNotCompatibleTypes)
{
	using SourceStringType = TestClassForCheckCompatibleTypes<std::string>;
	TestValidationForNotCompatibleTypes<ArchiveStub, SourceStringType, TestClassForCheckCompatibleTypes<std::nullptr_t>>();
	TestValidationForNotCompatibleTypes<ArchiveStub, SourceStringType, TestClassForCheckCompatibleTypes<bool>>();
	TestValidationForNotCompatibleTypes<ArchiveStub, SourceStringType, TestClassForCheckCompatibleTypes<int>>();
	TestValidationForNotCompatibleTypes<ArchiveStub, SourceStringType, TestClassForCheckCompatibleTypes<double>>();
	TestValidationForNotCompatibleTypes<ArchiveStub, SourceStringType, TestClassForCheckCompatibleTypes<TestPointClass>>();
	TestValidationForNotCompatibleTypes<ArchiveStub, SourceStringType, TestClassForCheckCompatibleTypes<int[3]>>();
}