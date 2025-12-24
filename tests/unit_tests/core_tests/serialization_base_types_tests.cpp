/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#if defined __has_include && __has_include(<version>)
#include <version>
#endif
#include <gtest/gtest.h>
#include "testing_tools/archive_stub.h"
#include "testing_tools/common_test_methods.h"
#include "testing_tools/common_json_test_methods.h"

//-----------------------------------------------------------------------------
// Tests of serialization for base types.
// For these purposes used archive stub which is simulate JSON structure.
//-----------------------------------------------------------------------------

using namespace BitSerializer;

enum class UnregisteredEnum
{
	One, Two, Three
};

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

	// Test serialize platform dependent types
	TestSerializeType<ArchiveStub, long>(std::numeric_limits<long>::min());
	TestSerializeType<ArchiveStub, size_t>(std::numeric_limits<size_t>::max());
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

TEST(BaseTypes, SerializeStdByte) {
	TestSerializeType<ArchiveStub, std::byte>(std::numeric_limits<std::byte>::min());
	TestSerializeType<ArchiveStub, std::byte>(std::numeric_limits<std::byte>::max());
}

TEST(BaseTypes, SerializeConstValue)
{
	const int expected = BuildFixture<int>();
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);
	int actual;
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_EQ(expected, actual);
}

//-----------------------------------------------------------------------------
// Tests of serialization any of std::string (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(BaseTypes, SerializeUtf8Sting)
{
	TestSerializeType<ArchiveStub, std::string>("Test ANSI string");
	TestSerializeType<ArchiveStub, std::string>(UTF8("Test UTF8 string - Привет мир!"));
#if defined(__cpp_lib_char8_t)
	// ToDo: uncomment when gtest will allow to compare u8strings (https://github.com/google/googletest/issues/4591)
	// TestSerializeType<ArchiveStub, std::u8string>(u8"Test UTF8 string - Привет мир!");
#endif
}

TEST(BaseTypes, SerializeUnicodeString)
{
	TestSerializeType<ArchiveStub, std::wstring>(L"Test wide string - Привет мир!");
	TestSerializeType<ArchiveStub, std::u16string>(u"Test UTF-16 string - Привет мир!");
	TestSerializeType<ArchiveStub, std::u32string>(U"Test UTF-32 string - Привет мир!");
}

//-----------------------------------------------------------------------------
// Tests of serialization for enum
//-----------------------------------------------------------------------------
TEST(BaseTypes, SerializeEnumAsRoot) {
	TestSerializeType<ArchiveStub, TestEnum>(TestEnum::Two);
}

TEST(BaseTypes, SerializeEnumAsRootThrowMismatchedTypesExceptionWhenLoadInvalid)
{
	// Save as string
	std::string invalidEnum = "InvalidEnum";
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidEnum, outputArchive);

	try
	{
		// Load as enum
		TestEnum targetEnum;
		BitSerializer::LoadObject<ArchiveStub>(targetEnum, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(BaseTypes, SerializeUnregisteredEnumAsRootShouldThrowException)
{
	try
	{
		UnregisteredEnum testValue = UnregisteredEnum::One;
		ArchiveStub::preferred_output_format outputArchive;
		BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::UnregisteredEnum, ex.GetErrorCode());
		EXPECT_STREQ("Unregistered enum", ex.what());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(BaseTypes, SerializeUnknownEnumAsRootShouldThrowException)
{
	try
	{
		auto testValue = static_cast<TestEnum>(std::numeric_limits<std::underlying_type_t<TestEnum>>::max());  // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
		ArchiveStub::preferred_output_format outputArchive;
		BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::UnregisteredEnum, ex.GetErrorCode());
		const std::string errStr = "Unregistered enum: Enum value (" + std::to_string(std::numeric_limits<int>::max()) + ") is invalid or not registered";
		EXPECT_EQ(errStr, ex.what());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(BaseTypes, SerializeEnumAsClassMember) {
	TestClassWithSubType testEntity(TestEnum::Three);
	TestSerializeType<ArchiveStub>(testEntity);
}

TEST(BaseTypes, SerializeEnumAsClassMemberThrowMismatchedTypesExceptionWhenLoadInvalid)
{
	// Save as string
	TestClassWithSubType<std::string> invalidEnum("InvalidEnum");
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(invalidEnum, outputArchive);

	try
	{
		// Load as enum
		TestClassWithSubType<TestEnum> targetObj;
		BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(BaseTypes, SerializeUnregisteredEnumAsClassMemberShouldThrowException)
{
	try
	{
		TestClassWithSubType objWithInvalidEnum(UnregisteredEnum::One);
		ArchiveStub::preferred_output_format outputArchive;
		BitSerializer::SaveObject<ArchiveStub>(objWithInvalidEnum, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::UnregisteredEnum, ex.GetErrorCode());
		EXPECT_STREQ("Unregistered enum", ex.what());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(BaseTypes, SerializeUnknownEnumAsClassMemberShouldThrowException)
{
	try
	{
		TestClassWithSubType objWithInvalidEnum(static_cast<TestEnum>(std::numeric_limits<std::underlying_type_t<TestEnum>>::max()));  // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
		ArchiveStub::preferred_output_format outputArchive;
		BitSerializer::SaveObject<ArchiveStub>(objWithInvalidEnum, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::UnregisteredEnum, ex.GetErrorCode());
		const std::string errStr = "Unregistered enum: Enum value (" + std::to_string(std::numeric_limits<int>::max()) + ") is invalid or not registered";
		EXPECT_EQ(errStr, ex.what());
		return;
	}
	EXPECT_FALSE(true);
}

//-----------------------------------------------------------------------------
// Tests of serialization for enum as integers
//-----------------------------------------------------------------------------
struct TestEnumAsBin
{
	TestEnumAsBin() = default;
	TestEnumAsBin(TestEnum testEnumValue) : TestEnumValue(testEnumValue) { }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("TestEnum", EnumAsBin(TestEnumValue));
	}

	TestEnum TestEnumValue = TestEnum::One;
};

TEST(BaseTypes, SerializeEnumBinAsRoot)
{
	// Arrange
	TestEnum expected = TestEnum::Three, actual = TestEnum::One;
	ArchiveStub::preferred_output_format testArchive;

	// Act
	BitSerializer::SaveObject<ArchiveStub>(EnumAsBin(expected), testArchive);
	BitSerializer::LoadObject<ArchiveStub>(EnumAsBin(actual), testArchive);

	// Assert
	EXPECT_EQ(expected, actual);
}

TEST(BaseTypes, SerializeEnumTypeAsClassMember)
{
	// Arrange
	TestEnumAsBin expected(TestEnum::Four), actual;
	ArchiveStub::preferred_output_format testArchive;

	// Act
	BitSerializer::SaveObject<ArchiveStub>(expected, testArchive);
	BitSerializer::LoadObject<ArchiveStub>(actual, testArchive);

	// Assert
	EXPECT_EQ(expected.TestEnumValue, actual.TestEnumValue);
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

	// Test serialize platform dependent types
	TestSerializeArray<ArchiveStub, long>();
	TestSerializeArray<ArchiveStub, size_t>();
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

TEST(BaseTypes, ShouldTrimStringFieldsInArray)
{
	// Arrange
	SerializationOptions options;
	options.trimStringFields = true;
	ArchiveStub::preferred_output_format outputArchive{};
	std::vector<std::string> actual = { " value1 ", " value2\n", "\t value3 \t" };

	// Act
	BitSerializer::SaveObject<ArchiveStub>(actual, outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);

	// Assert
	GTestExpectEq({ "value1", "value2", "value3" }, actual);
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
	TestSerializeType<ArchiveStub>(TestClassWithSubType<bool>(false));
	TestSerializeType<ArchiveStub>(TestClassWithSubType<bool>(true));
}

TEST(BaseTypes, SerializeUnion) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestUnion>());
}

TEST(BaseTypes, SerializeClassWithMemberInteger) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
}

TEST(BaseTypes, SerializeClassWithMemberFloat) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubTypes<float>>());
}

TEST(BaseTypes, SerializeClassWithMemberDouble) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubTypes<double>>());
}

TEST(BaseTypes, SerializeClassWithMemberNullptr) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(BaseTypes, SerializeClassWithMemberStdByte) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubTypes<std::byte>>());
}

TEST(BaseTypes, SerializeClassWithMemberString) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
}

TEST(BaseTypes, ShouldTrimStringFieldsInClassMembers)
{
	// Arrange
	SerializationOptions options;
	options.trimStringFields = true;
	ArchiveStub::preferred_output_format outputArchive{};
	TestClassWithSubType actual(std::string(" value "), Refine::TrimWhitespace());

	// Act
	BitSerializer::SaveObject<ArchiveStub>(actual, outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);

	// Assert
	EXPECT_EQ("value", actual.GetValue());
}

TEST(BaseTypes, SerializeClassWithExternalSerializeFunction) {
	TestSerializeType<ArchiveStub, TestClassWithExternalSerialization>();
}

TEST(BaseTypes, SerializeClassWithSubClass) {
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassType>());
}

TEST(BaseTypes, SerializeClassWithSubArray) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(BaseTypes, SerializeClassWithSubArrayOfClasses) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(BaseTypes, SerializeClassWithSubTwoDimArray) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(BaseTypes, ShouldVisitKeysInObjectScope) {
	TestVisitKeysInObjectScope<ArchiveStub>();
}

//-----------------------------------------------------------------------------
// Tests of serialization classes with inheritance
//-----------------------------------------------------------------------------
TEST(BaseTypes, SerializeClassHierarchyWithInternalSerialization) {
	TestSerializeType<ArchiveStub, TestClassWithInheritance<TestPointClass>>();
}

TEST(BaseTypes, SerializeClassHierarchyWithExternalSerialization) {
	TestSerializeType<ArchiveStub, TestClassWithInheritance<TestClassWithExternalSerialization>>();
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
	archive << KeyValue("Value", fixture.value);
}

TEST(BaseTypes, ShouldSerializeClassViaGlobalSerializeObject) {
	TestSerializeType<ArchiveStub>(BuildFixture<TestGlobalSerializeObjectFixture>());
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubTypes<TestGlobalSerializeObjectFixture>>());
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
	TestSerializeType<ArchiveStub>(BuildFixture<TestGlobalSerializeArrayFixture>());
	TestSerializeType<ArchiveStub>(BuildFixture<TestClassWithSubTypes<TestGlobalSerializeArrayFixture>>());
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
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(BaseTypes, ShouldCollectErrorsAboutRequiredNamedValues) {
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<ArchiveStub, TestClassForCheckValidation<TestPointClass>>();
}

//-----------------------------------------------------------------------------
TEST(BaseTypes, ThrowMismatchedTypesExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<ArchiveStub, std::string, bool>(MismatchedTypesPolicy::ThrowError);
}
TEST(BaseTypes, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<ArchiveStub, std::string, int32_t>(MismatchedTypesPolicy::ThrowError);
}
TEST(BaseTypes, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<ArchiveStub, std::string, float>(MismatchedTypesPolicy::ThrowError);
}

TEST(BaseTypes, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<ArchiveStub, std::string, bool>(MismatchedTypesPolicy::Skip);
}
TEST(BaseTypes, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<ArchiveStub, std::string, int32_t>(MismatchedTypesPolicy::Skip);
}
TEST(BaseTypes, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<ArchiveStub, std::string, float>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<ArchiveStub, std::string, double>(MismatchedTypesPolicy::Skip);
}

TEST(BaseTypes, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<ArchiveStub, float, uint32_t>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<ArchiveStub, double, uint32_t>(MismatchedTypesPolicy::Skip);
}
TEST(BaseTypes, ThrowValidationExceptionWhenLoadFloatToInt) {
	TestMismatchedTypesPolicy<ArchiveStub, float, int>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<ArchiveStub, double, int>(MismatchedTypesPolicy::Skip);
}
TEST(BaseTypes, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedTypesPolicy<ArchiveStub, std::nullptr_t, bool>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<ArchiveStub, std::nullptr_t, uint32_t>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<ArchiveStub, std::nullptr_t, double>(MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------

TEST(BaseTypes, ThrowSerializationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<ArchiveStub, int32_t, bool>(OverflowNumberPolicy::ThrowError);
}
TEST(BaseTypes, ThrowSerializationExceptionWhenOverflowInt8) {
	TestOverflowNumberPolicy<ArchiveStub, int16_t, int8_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<ArchiveStub, uint16_t, uint8_t>(OverflowNumberPolicy::ThrowError);
}
TEST(BaseTypes, ThrowSerializationExceptionWhenOverflowInt16) {
	TestOverflowNumberPolicy<ArchiveStub, int32_t, int16_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<ArchiveStub, uint32_t, uint16_t>(OverflowNumberPolicy::ThrowError);
}
TEST(BaseTypes, ThrowSerializationExceptionWhenOverflowInt32) {
	TestOverflowNumberPolicy<ArchiveStub, int64_t, int32_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<ArchiveStub, uint64_t, uint32_t>(OverflowNumberPolicy::ThrowError);
}
TEST(BaseTypes, ThrowSerializationExceptionWhenOverflowFloat) {
	TestOverflowNumberPolicy<ArchiveStub, double, float>(OverflowNumberPolicy::ThrowError);
}

TEST(BaseTypes, ThrowValidationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<ArchiveStub, int32_t, bool>(OverflowNumberPolicy::Skip);
}
TEST(BaseTypes, ThrowValidationExceptionWhenNumberOverflowInt8) {
	TestOverflowNumberPolicy<ArchiveStub, int16_t, int8_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<ArchiveStub, uint16_t, uint8_t>(OverflowNumberPolicy::Skip);
}
TEST(BaseTypes, ThrowValidationExceptionWhenNumberOverflowInt16) {
	TestOverflowNumberPolicy<ArchiveStub, int32_t, int16_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<ArchiveStub, uint32_t, uint16_t>(OverflowNumberPolicy::Skip);
}
TEST(BaseTypes, ThrowValidationExceptionWhenNumberOverflowInt32) {
	TestOverflowNumberPolicy<ArchiveStub, int64_t, int32_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<ArchiveStub, uint64_t, uint32_t>(OverflowNumberPolicy::Skip);
}
TEST(BaseTypes, ThrowValidationExceptionWhenNumberOverflowFloat) {
	TestOverflowNumberPolicy<ArchiveStub, double, float>(OverflowNumberPolicy::Skip);
}
