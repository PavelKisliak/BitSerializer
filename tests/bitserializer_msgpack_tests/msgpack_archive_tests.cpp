/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/common_json_test_methods.h"
#include "bitserializer/msgpack_archive.h"
#include "bitserializer/types/std/chrono.h"
#include "bitserializer/types/std/ctime.h"
#include "bitserializer/types/std/map.h"

using namespace BitSerializer;
using BitSerializer::MsgPack::MsgPackArchive;

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeBoolean)
{
	TestSerializeType<MsgPackArchive, bool>(false);
	TestSerializeType<MsgPackArchive, bool>(true);
}

TEST(MsgPackArchive, SerializeUInt8)
{
	TestSerializeType<MsgPackArchive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<MsgPackArchive, uint8_t>(0);
	TestSerializeType<MsgPackArchive, uint8_t>(127);
	TestSerializeType<MsgPackArchive, uint8_t>(std::numeric_limits<uint8_t>::max());
}

TEST(MsgPackArchive, SerializeInt8)
{
	TestSerializeType<MsgPackArchive, int8_t>(std::numeric_limits<int8_t>::min());
	TestSerializeType<MsgPackArchive, int8_t>(-32);
	TestSerializeType<MsgPackArchive, int8_t>(32);
	TestSerializeType<MsgPackArchive, int8_t>(std::numeric_limits<int8_t>::max());
}

TEST(MsgPackArchive, SerializeInt64)
{
	TestSerializeType<MsgPackArchive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<MsgPackArchive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(MsgPackArchive, SerializeFloat)
{
	TestSerializeType<MsgPackArchive, float>(0.f);
	TestSerializeType<MsgPackArchive, float>(3.141592654f);
	TestSerializeType<MsgPackArchive, float>(-3.141592654f);
}

TEST(MsgPackArchive, SerializeDouble)
{
	TestSerializeType<MsgPackArchive, double>(std::numeric_limits<double>::min());
	TestSerializeType<MsgPackArchive, double>(std::numeric_limits<double>::max());
}

TEST(MsgPackArchive, ShouldAllowToLoadBooleanFromInteger)
{
	bool actual = false;
	BitSerializer::LoadObject<MsgPackArchive>(actual, std::string({'\xC3'}));
	EXPECT_EQ(true, actual);
}

TEST(MsgPackArchive, SerializeNullptr)
{
	TestSerializeType<MsgPackArchive, std::nullptr_t>(nullptr);
}

//-----------------------------------------------------------------------------
// Tests of serialization any of std::string (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeUtf8Sting)
{
	TestSerializeType<MsgPackArchive, std::string>("Test ANSI string");
	TestSerializeType<MsgPackArchive, std::string>(UTF8("Test UTF8 string - Привет мир!"));
}

TEST(MsgPackArchive, SerializeUnicodeString)
{
	TestSerializeType<MsgPackArchive, std::wstring>(L"Test wide string - Привет мир!");
	TestSerializeType<MsgPackArchive, std::u16string>(u"Test UTF-16 string - Привет мир!");
	TestSerializeType<MsgPackArchive, std::u32string>(U"Test UTF-32 string - Привет мир!");
}

//-----------------------------------------------------------------------------
// Tests of serialization for enum
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeEnum)
{
	TestSerializeType<MsgPackArchive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for timestamps
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeCTime)
{
	// Arrange
	time_t time = 102030;
	CTimeRef timeRef(time);

	time_t actualTime = 0;
	CTimeRef actualTimeRef(actualTime);

	std::string outputData;

	// Act
	BitSerializer::SaveObject<MsgPackArchive>(timeRef, outputData);
	BitSerializer::LoadObject<MsgPackArchive>(actualTimeRef, outputData);

	// Assert
	EXPECT_EQ(time, actualTime);
}

TEST(MsgPackArchive, SerializeTimestamp32)
{
	auto seconds = std::chrono::duration<uint32_t>::max();
	TestSerializeType<MsgPackArchive>(seconds);
}

TEST(MsgPackArchive, SerializeTimestamp64)
{
	auto seconds = std::chrono::duration<int64_t>::max();
	TestSerializeType<MsgPackArchive>(seconds);
}

TEST(MsgPackArchive, SerializeTimestamp96)
{
	auto ns = std::chrono::nanoseconds::max();
	TestSerializeType<MsgPackArchive>(ns);
}

TEST(MsgPackArchive, SerializeTimestamp32AsClassMember)
{
	TestClassWithSubType<std::chrono::duration<uint32_t>> testEntity;
	TestSerializeType<MsgPackArchive>(testEntity);
}

TEST(MsgPackArchive, SerializeTimestamp64AsClassMember)
{
	TestClassWithSubType<std::chrono::duration<int64_t>> testEntity;
	TestSerializeType<MsgPackArchive>(testEntity);
}

TEST(MsgPackArchive, SerializeTimestamp96AsClassMember)
{
	TestClassWithSubType<std::chrono::nanoseconds> testEntity;
	TestSerializeType<MsgPackArchive>(testEntity);
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeArrayOfBooleans)
{
	TestSerializeArray<MsgPackArchive, bool>();
}

TEST(MsgPackArchive, SerializeArrayOfChars)
{
	TestSerializeArray<MsgPackArchive, char>();
	TestSerializeArray<MsgPackArchive, unsigned char>();
}

TEST(MsgPackArchive, SerializeArrayOfIntegers)
{
	TestSerializeArray<MsgPackArchive, uint16_t>();
	TestSerializeArray<MsgPackArchive, int64_t>();
}

TEST(MsgPackArchive, SerializeArrayOfFloats)
{
	TestSerializeArray<MsgPackArchive, float>();
}

TEST(MsgPackArchive, SerializeArrayOfDoubles)
{
	TestSerializeArray<MsgPackArchive, double>();
}

TEST(MsgPackArchive, SerializeArrayOfNullptrs)
{
	TestSerializeArray<MsgPackArchive, std::nullptr_t>();
}

TEST(MsgPackArchive, SerializeArrayOfStrings)
{
	TestSerializeArray<MsgPackArchive, std::string>();
}

TEST(MsgPackArchive, SerializeArrayOfUnicodeStrings)
{
	TestSerializeArray<MsgPackArchive, std::wstring>();
	TestSerializeArray<MsgPackArchive, std::u16string>();
	TestSerializeArray<MsgPackArchive, std::u32string>();
}

TEST(MsgPackArchive, SerializeArrayOfClasses)
{
	TestSerializeArray<MsgPackArchive, TestPointClass>();
}

TEST(MsgPackArchive, SerializeTwoDimensionalArray)
{
	TestSerializeTwoDimensionalArray<MsgPackArchive, int32_t>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeClassWithMemberBoolean)
{
	TestSerializeType<MsgPackArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeType<MsgPackArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(MsgPackArchive, SerializeClassWithMemberInteger)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
	TestSerializeType<MsgPackArchive>(TestClassWithSubTypes(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint64_t>::max()));
}

TEST(MsgPackArchive, SerializeClassWithMemberFloat)
{
	TestSerializeType<MsgPackArchive>(TestClassWithSubTypes(-3.141592654f, 0.0f, -3.141592654f));
}

TEST(MsgPackArchive, SerializeClassWithMemberDouble)
{
	TestSerializeType<MsgPackArchive>(TestClassWithSubTypes(std::numeric_limits<double>::min(), 0.0, std::numeric_limits<double>::max()));
}

TEST(MsgPackArchive, SerializeClassWithMemberNullptr)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(MsgPackArchive, SerializeClassWithMemberString)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring, std::u16string, std::u32string>>());
}

TEST(MsgPackArchive, SerializeClassHierarchy)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithInheritance>());
}

TEST(MsgPackArchive, SerializeClassWithMemberClass)
{
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassType>());
}

TEST(MsgPackArchive, SerializeClassWithSubArray)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(MsgPackArchive, SerializeClassWithSubArrayOfClasses)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(MsgPackArchive, SerializeClassWithSubTwoDimArray)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(MsgPackArchive, ShouldVisitKeysInObjectScopeWhenReadValues)
{
	TestVisitKeysInObjectScope<MsgPackArchive>();
}

TEST(MsgPackArchive, ShouldVisitKeysInObjectScopeWhenSkipValues)
{
	TestVisitKeysInObjectScope<MsgPackArchive>(true);
}

TEST(MsgPackArchive, SerializeClassInReverseOrder)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, float, std::string>>();
	TestSerializeType<MsgPackArchive>(fixture);
}

TEST(MsgPackArchive, SerializeClassInReverseOrderWithSubArray)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, std::array<uint64_t, 5>, std::string>>();
	TestSerializeType<MsgPackArchive>(fixture);
}

TEST(MsgPackArchive, SerializeClassInReverseOrderWithSubObject)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, TestPointClass, std::string>>();
	TestSerializeType<MsgPackArchive>(fixture);
}

TEST(MsgPackArchive, SerializeClassWithSkippingFields)
{
	TestClassWithVersioning arrayOfObjects[3];
	BuildFixture(arrayOfObjects);
	TestSerializeType<MsgPackArchive>(arrayOfObjects);
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes with non-string keys (MsgPack feature)
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeClassWithIntAsKey)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<int8_t>>());
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<int16_t>>());
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<int32_t>>());
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<int64_t>>());
}

TEST(MsgPackArchive, SerializeClassWithUIntAsKey)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<uint8_t>>());
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<uint16_t>>());
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<uint32_t>>());
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<uint64_t>>());
}

TEST(MsgPackArchive, SerializeClassWithFloatAsKey)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<float>>());
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<double>>());
}

TEST(MsgPackArchive, SerializeClassWithTimestampAsKey)
{
	TestSerializeType<MsgPackArchive>(BuildFixture<TestClassWithCustomKey<Detail::CBinTimestamp>>());
}

//-----------------------------------------------------------------------------
// Test serialization std::map
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeMapWithIntAsKey)
{
	TestSerializeType<MsgPackArchive>(std::map<int8_t, int>{
		{ std::numeric_limits<int8_t>::min(), 1 },
		{ std::numeric_limits<int8_t>::max(), 2 }
	});

	TestSerializeType<MsgPackArchive>(std::map<int64_t, int>{
		{ std::numeric_limits<int64_t>::min(), 1 },
		{ std::numeric_limits<int64_t>::max(), 2 }
	});
}

TEST(MsgPackArchive, SerializeMapWithUnsignedIntAsKey)
{
	TestSerializeType<MsgPackArchive>(std::map<uint8_t, std::string>{
		{ std::numeric_limits<uint8_t>::min(), "1" },
		{ std::numeric_limits<uint8_t>::max(), "2" }
	});

	TestSerializeType<MsgPackArchive>(std::map<uint64_t, std::string>{
		{ std::numeric_limits<uint64_t>::min(), "1" },
		{ std::numeric_limits<uint64_t>::max(), "2" }
	});
}

TEST(MsgPackArchive, SerializeMapWithfloatAsKey) {
	TestSerializeType<MsgPackArchive, std::map<float, int>>();
	TestSerializeType<MsgPackArchive, std::map<double, std::string>>();
}

TEST(MsgPackArchive, SerializeMapWithChronoDurationAsKey) {
	TestSerializeType<MsgPackArchive, std::map<std::chrono::nanoseconds, int>>();
	TestSerializeType<MsgPackArchive, std::map<std::chrono::nanoseconds, std::u16string>>();
}

TEST(MsgPackArchive, SerializeMapWithChronoTimePointAsKey) {
	TestSerializeType<MsgPackArchive, std::map<std::chrono::system_clock::time_point, int>>();
	TestSerializeType<MsgPackArchive, std::map<std::chrono::system_clock::time_point, std::u32string>>();
}

TEST(MsgPackArchive, SerializeMapWithStringAsKey) {
	TestSerializeType<MsgPackArchive, std::map<std::string, int>>();
	TestSerializeType<MsgPackArchive, std::map<std::wstring, std::string>>();
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ShouldReturnPathInObjectScopeWhenLoading)
{
	TestGetPathInJsonObjectScopeWhenLoading<MsgPackArchive>();
}

TEST(MsgPackArchive, ShouldReturnPathInArrayScopeWhenLoading)
{
	TestGetPathInJsonArrayScopeWhenLoading<MsgPackArchive>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeClassToStream) {
	TestSerializeClassToStream<MsgPackArchive, char>(BuildFixture<TestPointClass>());
}

TEST(MsgPackArchive, SerializeArrayOfClassesToStream)
{
	TestClassWithSubTypes<int, float, std::string, TestPointClass> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<MsgPackArchive, char>(testArray);
}

TEST(MsgPackArchive, SerializeToFile) {
	TestSerializeArrayToFile<MsgPackArchive>();
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ThrowExceptionWhenUnexpectedEnd)
{
	try
	{
		int testInt = 0;
		BitSerializer::LoadObject<MsgPackArchive>(testInt, "\xD1\x80");
		EXPECT_FALSE(true);
	}
	catch (const ParsingException& ex)
	{
		EXPECT_EQ(1, ex.Offset);
	}
	catch (const std::exception&)
	{
		EXPECT_FALSE(true);
	}
}

TEST(MsgPackArchive, ThrowExceptionWhenNoMoreValuesToRead)
{
	const char* testMsgPack = "\x92"
		"\x82\xA1x\x05\xA1y\x06"
		"\x82\xA1x\x07\xA1y";
	try
	{
		TestPointClass testList[2];
		BitSerializer::LoadObject<MsgPackArchive>(testList, testMsgPack);
		EXPECT_FALSE(true);
	}
	catch (const ParsingException& ex)
	{
		EXPECT_EQ(ex.Offset, std::strlen(testMsgPack));
	}
	catch (const std::exception&)
	{
		EXPECT_FALSE(true);
	}
}

//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ThrowValidationExceptionWhenMissedRequiredValue) {
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<TestPointClass>>();
	TestValidationForNamedValues<MsgPackArchive, TestClassForCheckValidation<int[3]>>();
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ThrowPolicyForArrayWhenMismatchedLoadStringToBoolean) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, bool>(MismatchedTypesPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, int32_t>(MismatchedTypesPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, float>(MismatchedTypesPolicy::ThrowError);
}

TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadNumberToString) {
	TestMismatchedTypesPolicy<MsgPackArchive, int32_t, std::string>(MismatchedTypesPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<MsgPackArchive, float, int>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<MsgPackArchive, double, int>(MismatchedTypesPolicy::ThrowError);
}

TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<MsgPackArchive, int32_t, int32_t[3]>(MismatchedTypesPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToBinArray) {
	TestMismatchedTypesPolicy<MsgPackArchive, int32_t, char[3]>(MismatchedTypesPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<MsgPackArchive, int32_t, TestPointClass>(MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::Skip
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, bool>(MismatchedTypesPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, int32_t>(MismatchedTypesPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<MsgPackArchive, std::string, float>(MismatchedTypesPolicy::Skip);
}

TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<MsgPackArchive, float, int>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<MsgPackArchive, double, int>(MismatchedTypesPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedTypesPolicy<MsgPackArchive, std::nullptr_t, bool>(MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<MsgPackArchive, std::nullptr_t, uint32_t>(MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<MsgPackArchive, std::nullptr_t, double>(MismatchedTypesPolicy::ThrowError);
}

TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<MsgPackArchive, int32_t, int32_t[3]>(MismatchedTypesPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadIntegerToBinArray) {
	TestMismatchedTypesPolicy<MsgPackArchive, int32_t, char[3]>(MismatchedTypesPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<MsgPackArchive, int32_t, TestPointClass>(MismatchedTypesPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<MsgPackArchive, int32_t, bool>(OverflowNumberPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowInt8) {
	TestOverflowNumberPolicy<MsgPackArchive, int16_t, int8_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<MsgPackArchive, uint16_t, uint8_t>(OverflowNumberPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowInt16) {
	TestOverflowNumberPolicy<MsgPackArchive, int32_t, int16_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<MsgPackArchive, uint32_t, uint16_t>(OverflowNumberPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowInt32) {
	TestOverflowNumberPolicy<MsgPackArchive, int64_t, int32_t>(OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<MsgPackArchive, uint64_t, uint32_t>(OverflowNumberPolicy::ThrowError);
}
TEST(MsgPackArchive, ThrowSerializationExceptionWhenOverflowFloat) {
	TestOverflowNumberPolicy<MsgPackArchive, double, float>(OverflowNumberPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::Skip
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, ThrowValidationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<MsgPackArchive, int32_t, bool>(OverflowNumberPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenNumberOverflowInt8) {
	TestOverflowNumberPolicy<MsgPackArchive, int16_t, int8_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<MsgPackArchive, uint16_t, uint8_t>(OverflowNumberPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenNumberOverflowInt16) {
	TestOverflowNumberPolicy<MsgPackArchive, int32_t, int16_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<MsgPackArchive, uint32_t, uint16_t>(OverflowNumberPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenNumberOverflowInt32) {
	TestOverflowNumberPolicy<MsgPackArchive, int64_t, int32_t>(OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<MsgPackArchive, uint64_t, uint32_t>(OverflowNumberPolicy::Skip);
}
TEST(MsgPackArchive, ThrowValidationExceptionWhenNumberOverflowFloat) {
	TestOverflowNumberPolicy<MsgPackArchive, double, float>(OverflowNumberPolicy::Skip);
}
