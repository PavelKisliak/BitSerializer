/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/common_xml_test_methods.h"
#include "bitserializer/pugixml_archive.h"

// STD types
#include "bitserializer/types/std/atomic.h"
#include "bitserializer/types/std/chrono.h"
#include "bitserializer/types/std/ctime.h"
#include "bitserializer/types/std/optional.h"
#include "bitserializer/types/std/pair.h"
#include "bitserializer/types/std/tuple.h"
#include "bitserializer/types/std/memory.h"
#include "bitserializer/types/std/filesystem.h"

using BitSerializer::Xml::PugiXml::XmlArchive;

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SerializeArrayWithKeyOnRootLevel) {
	TestSerializeArray<XmlArchive, int16_t>();
}

TEST(PugiXmlArchive, SerializeArrayOfBooleans) {
	TestSerializeArray<XmlArchive, bool>();
}

TEST(PugiXmlArchive, SerializeArrayOfChars) {
	TestSerializeArray<XmlArchive, char>();
	TestSerializeArray<XmlArchive, unsigned char>();
}

TEST(PugiXmlArchive, SerializeArrayOfIntegers) {
	TestSerializeArray<XmlArchive, uint16_t>();
	TestSerializeArray<XmlArchive, int64_t>();

	// Test serialize platform dependent types
	TestSerializeArray<XmlArchive, long>();
	TestSerializeArray<XmlArchive, size_t>();
}

TEST(PugiXmlArchive, SerializeArrayOfFloats) {
	TestSerializeArray<XmlArchive, float>();
	TestSerializeArray<XmlArchive, double>();
}

TEST(PugiXmlArchive, SerializeArrayWithSpecialNumbersToStream)
{
	if constexpr (std::numeric_limits<double>::has_infinity)
	{
		TestClassWithSubTypes<float> testArray1[3] = { 1.0f, std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() };
		TestSerializeArrayToStream<XmlArchive>(testArray1);

		TestClassWithSubTypes<double> testArray2[3] = { 1.0, std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() };
		TestSerializeArrayToStream<XmlArchive>(testArray2);
	}

	if constexpr (std::numeric_limits<double>::has_quiet_NaN)
	{
		TestClassWithSubTypes<float> testArray1[3] = { 1.0f, std::numeric_limits<float>::quiet_NaN(), 2.0f };
		TestSerializeArrayToStream<XmlArchive>(testArray1);

		TestClassWithSubTypes<double> testArray2[3] = { 1.0, std::numeric_limits<double>::quiet_NaN(), 2.0 };
		TestSerializeArrayToStream<XmlArchive>(testArray2);
	}
}

TEST(PugiXmlArchive, SerializeArrayOfNullptrs) {
	TestSerializeArray<XmlArchive, std::nullptr_t>();
}

TEST(PugiXmlArchive, SerializeArrayOfStrings) {
	TestSerializeArray<XmlArchive, std::string>();
}

TEST(PugiXmlArchive, SerializeArrayOfUnicodeStrings) {
	TestSerializeArray<XmlArchive, std::wstring>();
	TestSerializeArray<XmlArchive, std::u16string>();
	TestSerializeArray<XmlArchive, std::u32string>();
}

TEST(PugiXmlArchive, SerializeArrayOfClasses) {
	TestSerializeArray<XmlArchive, TestPointClass>();
}

TEST(PugiXmlArchive, SerializeTwoDimensionalArray) {
	TestSerializeTwoDimensionalArray<XmlArchive, int32_t>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SerializeClassWithKeyOnRootLevel) {
	TestClassWithSubTypes<int16_t> testValue;
	TestSerializeType<XmlArchive>(BitSerializer::KeyValue("Root", testValue));
}

TEST(PugiXmlArchive, SerializeConstClassWithKeyOnRootLevel) {
	const auto expected = BuildFixture<TestPointClass>();
	std::string outputArchive;
	BitSerializer::SaveObject<XmlArchive>(BitSerializer::KeyValue("Point", expected), outputArchive);
	TestPointClass actual;
	BitSerializer::LoadObject<XmlArchive>(BitSerializer::KeyValue("Point", actual), outputArchive);
	EXPECT_EQ(expected, actual);
}

TEST(PugiXmlArchive, SerializeClassWithMemberBoolean) {
	TestSerializeType<XmlArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeType<XmlArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(PugiXmlArchive, SerializeClassWithMemberInteger) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t, size_t>>());
	TestSerializeType<XmlArchive>(TestClassWithSubTypes(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint64_t>::max()));
}

TEST(PugiXmlArchive, SerializeClassWithMemberFloat) {
	TestSerializeType<XmlArchive>(TestClassWithSubTypes(std::numeric_limits<float>::min(), 0.0f, std::numeric_limits<float>::max()));
}

TEST(PugiXmlArchive, SerializeClassWithMemberDouble) {
	TestSerializeType<XmlArchive>(TestClassWithSubTypes(std::numeric_limits<double>::min(), 0.0, std::numeric_limits<double>::max()));
}

TEST(PugiXmlArchive, SerializeClassWithSpecialNumbers)
{
	if constexpr (std::numeric_limits<double>::has_infinity)
	{
		TestSerializeType<XmlArchive>(TestClassWithSubType(std::numeric_limits<float>::infinity()));
		TestSerializeType<XmlArchive>(TestClassWithSubType(-std::numeric_limits<float>::infinity()));

		TestSerializeType<XmlArchive>(TestClassWithSubType(std::numeric_limits<double>::infinity()));
		TestSerializeType<XmlArchive>(TestClassWithSubType(-std::numeric_limits<double>::infinity()));
	}

	if constexpr (std::numeric_limits<double>::has_quiet_NaN)
	{
		TestSerializeType<XmlArchive>(TestClassWithSubType(-std::numeric_limits<float>::quiet_NaN()));
		TestSerializeType<XmlArchive>(TestClassWithSubType(-std::numeric_limits<double>::quiet_NaN()));
	}
}

TEST(PugiXmlArchive, SerializeClassWithMemberNullptr) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(PugiXmlArchive, SerializeClassWithMemberString) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring, std::u16string, std::u32string>>());
}

TEST(PugiXmlArchive, SerializeClassWithExternalSerializeFunction) {
	TestSerializeType<XmlArchive, TestClassWithExternalSerialization>();
}

TEST(PugiXmlArchive, SerializeClassHierarchy) {
	TestSerializeType<XmlArchive, TestClassWithInheritance<TestPointClass>>();
	TestSerializeType<XmlArchive, TestClassWithInheritance<TestClassWithExternalSerialization>>();
}

TEST(PugiXmlArchive, SerializeClassWithSubClass) {
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeType<XmlArchive>(BuildFixture<TestClassType>());
}

TEST(PugiXmlArchive, SerializeClassWithSubArray) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(PugiXmlArchive, SerializeClassWithSubArrayOfClasses) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(PugiXmlArchive, SerializeClassWithSubTwoDimArray) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(PugiXmlArchive, ShouldAllowToLoadBooleanFromInteger)
{
	TestClassWithSubType<bool> actual(false);
	BitSerializer::LoadObject<XmlArchive>(actual, "<root><TestValue>1</TestValue></root>");
	EXPECT_EQ(true, actual.GetValue());
}

TEST(PugiXmlArchive, ShouldAllowToLoadFloatFromInteger)
{
	TestClassWithSubType<float> actual(0);
	BitSerializer::LoadObject<XmlArchive>(actual, "<root><TestValue>100</TestValue></root>");
	EXPECT_EQ(100, actual.GetValue());
}

TEST(PugiXmlArchive, ShouldVisitKeysInObjectScopeWhenReadValues)
{
	TestVisitKeysInObjectScope<XmlArchive>();
}

TEST(PugiXmlArchive, ShouldVisitKeysInObjectScopeWhenSkipValues)
{
	TestVisitKeysInObjectScope<XmlArchive>(true);
}

TEST(PugiXmlArchive, SerializeClassInReverseOrder)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, float, std::string>>();
	TestSerializeType<XmlArchive>(fixture);
}

TEST(PugiXmlArchive, SerializeClassInReverseOrderWithSubArray)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, std::array<uint64_t, 5>, std::string>>();
	TestSerializeType<XmlArchive>(fixture);
}

TEST(PugiXmlArchive, SerializeClassInReverseOrderWithSubObject)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, TestPointClass, std::string>>();
	TestSerializeType<XmlArchive>(fixture);
}

TEST(PugiXmlArchive, SerializeClassWithSkippingFields)
{
	TestClassWithVersioning arrayOfObjects[3];
	BuildFixture(arrayOfObjects);
	TestSerializeType<XmlArchive>(arrayOfObjects);
}

//-----------------------------------------------------------------------------
// Tests of serialization for attributes
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SerializeAttributesWithBoolean) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithAttributes<bool>>());
}

TEST(PugiXmlArchive, SerializeAttributesWithIntegers) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithAttributes<int8_t, uint8_t, int64_t, uint64_t>>());
}

TEST(PugiXmlArchive, SerializeAttributesWithFloats) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithAttributes<float, double>>());
}

TEST(PugiXmlArchive, SerializeAttributesWithNullptr) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithAttributes<std::nullptr_t>>());
}

TEST(PugiXmlArchive, SerializeAttributesWithString) {
	TestSerializeType<XmlArchive>(BuildFixture<TestClassWithAttributes<std::string, std::wstring>>());
}

//-----------------------------------------------------------------------------
// Tests format output XML
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SaveWithFormatting) {
	TestSaveFormattedXml<XmlArchive>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SerializeClassToStream) {
	TestSerializeClassToStream<XmlArchive>(BuildFixture<TestPointClass>());
}

TEST(PugiXmlArchive, SerializeArrayOfClassesToStream)
{
	TestClassWithSubTypes<short, int, long, size_t, double, std::string> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<XmlArchive>(testArray);
}

TEST(PugiXmlArchive, SerializeUnicodeToUtf8Stream) {
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
	TestSerializeClassToStream<XmlArchive>(TestValue);
}

TEST(PugiXmlArchive, LoadFromUtf8Stream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf8>(false);
}
TEST(PugiXmlArchive, LoadFromUtf8StreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf8>(true);
}

TEST(PugiXmlArchive, LoadFromUtf16LeStream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf16Le>(false);
}
TEST(PugiXmlArchive, LoadFromUtf16LeStreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf16Le>(true);
}

TEST(PugiXmlArchive, LoadFromUtf16BeStream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf16Be>(false);
}
TEST(PugiXmlArchive, LoadFromUtf16BeStreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf16Be>(true);
}

TEST(PugiXmlArchive, LoadFromUtf32LeStream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf32Le>(false);
}
TEST(PugiXmlArchive, LoadFromUtf32LeStreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf32Le>(true);
}

TEST(PugiXmlArchive, LoadFromUtf32BeStream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf32Be>(false);
}
TEST(PugiXmlArchive, LoadFromUtf32BeStreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf32Be>(true);
}

TEST(PugiXmlArchive, SaveToUtf8Stream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf8>(false);
}
TEST(PugiXmlArchive, SaveToUtf8StreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf8>(true);
}

TEST(PugiXmlArchive, SaveToUtf16LeStream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf16Le>(false);
}
TEST(PugiXmlArchive, SaveToUtf16LeStreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf16Le>(true);
}

TEST(PugiXmlArchive, SaveToUtf16BeStream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf16Be>(false);
}
TEST(PugiXmlArchive, SaveToUtf16BeStreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf16Be>(true);
}

TEST(PugiXmlArchive, SaveToUtf32LeStream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf32Le>(false);
}
TEST(PugiXmlArchive, SaveToUtf32LeStreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf32Le>(true);
}

TEST(PugiXmlArchive, SaveToUtf32BeStream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf32Be>(false);
}
TEST(PugiXmlArchive, SaveToUtf32BeStreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf::Utf32Be>(true);
}

TEST(PugiXmlArchive, ThrowExceptionWhenUnsupportedStreamEncoding)
{
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.streamOptions.encoding = static_cast<BitSerializer::Convert::Utf::UtfType>(-1);  // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
	std::stringstream outputStream;
	auto testObj = BuildFixture<TestClassWithSubTypes<std::string>>();
	EXPECT_THROW(BitSerializer::SaveObject<XmlArchive>(testObj, outputStream, serializationOptions), BitSerializer::SerializationException);
}

TEST(PugiXmlArchive, SerializeToFile) {
	TestSerializeArrayToFile<XmlArchive>();
	TestSerializeArrayToFile<XmlArchive>(true);
}

TEST(PugiXmlArchive, SerializeToFileThrowExceptionWhenAlreadyExists) {
	TestThrowExceptionWhenFileAlreadyExists<XmlArchive>();
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ThrowExceptionWhenBadSyntaxInSource) {
	auto fixture = BuildFixture<TestClassWithSubTypes<std::string>>();
	EXPECT_THROW(BitSerializer::LoadObject<XmlArchive>(fixture, PUGIXML_TEXT("<root>Hello")), BitSerializer::ParsingException);
}

TEST(PugiXmlArchive, ThrowParsingExceptionWithCorrectPosition)
{
	const std::string testJson = R"(<root>
	<object>
		<x>10</x>
		<y>20</y>
	<object>
		<x>10</x>
		y>20</y>
	<object>
</root>)";
	TestPointClass testList[2];
	try
	{
		BitSerializer::LoadObject<XmlArchive>(testList, testJson);
		EXPECT_FALSE(true);
	}
	catch (const BitSerializer::ParsingException& ex)
	{
		EXPECT_TRUE(ex.Offset > 63 && ex.Offset < testJson.size());
	}
	catch (const std::exception&)
	{
		EXPECT_FALSE(true);
	}
}

//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ThrowValidationExceptionWhenMissedRequiredValue) {
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<TestPointClass>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<int[3]>>();
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ThrowMismatchedTypesExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<XmlArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(PugiXmlArchive, ThrowMismatchedTypesExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<XmlArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(PugiXmlArchive, ThrowMismatchedTypesExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<XmlArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(PugiXmlArchive, ThrowMismatchedTypesExceptionWhenLoadSignedToUnsigned) {
	TestMismatchedTypesPolicy<XmlArchive, int32_t, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<XmlArchive, int32_t, uint32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(PugiXmlArchive, ThrowSerializationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<XmlArchive, float, uint32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<XmlArchive, double, uint32_t>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(PugiXmlArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<XmlArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}
TEST(PugiXmlArchive, ThrowMismatchedTypesExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<XmlArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test MismatchedTypesPolicy::Skip
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ThrowValidationExceptionWhenLoadStringToBoolean) {
	TestMismatchedTypesPolicy<XmlArchive, std::string, bool>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(PugiXmlArchive, ThrowValidationExceptionWhenLoadStringToInteger) {
	TestMismatchedTypesPolicy<XmlArchive, std::string, int32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(PugiXmlArchive, ThrowValidationExceptionWhenLoadStringToFloat) {
	TestMismatchedTypesPolicy<XmlArchive, std::string, float>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<XmlArchive, std::string, double>(BitSerializer::MismatchedTypesPolicy::Skip);
}

TEST(PugiXmlArchive, ThrowValidationExceptionWhenLoadFloatToInteger) {
	TestMismatchedTypesPolicy<XmlArchive, float, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<XmlArchive, double, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(PugiXmlArchive, ThrowValidationExceptionWhenLoadNullToAnyType) {
	// It doesn't matter what kind of MismatchedTypesPolicy is used, should throw only validation exception
	TestMismatchedTypesPolicy<XmlArchive, std::nullptr_t, bool>(BitSerializer::MismatchedTypesPolicy::ThrowError);
	TestMismatchedTypesPolicy<XmlArchive, std::nullptr_t, uint32_t>(BitSerializer::MismatchedTypesPolicy::Skip);
	TestMismatchedTypesPolicy<XmlArchive, std::nullptr_t, double>(BitSerializer::MismatchedTypesPolicy::ThrowError);
}

TEST(PugiXmlArchive, ThrowValidationExceptionWhenLoadIntegerToArray) {
	TestMismatchedTypesPolicy<XmlArchive, int32_t, int32_t[3]>(BitSerializer::MismatchedTypesPolicy::Skip);
}
TEST(PugiXmlArchive, ThrowValidationExceptionWhenLoadIntegerToObject) {
	TestMismatchedTypesPolicy<XmlArchive, int32_t, TestPointClass>(BitSerializer::MismatchedTypesPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::ThrowError
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ThrowSerializationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<XmlArchive, int32_t, bool>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(PugiXmlArchive, ThrowSerializationExceptionWhenOverflowInt8) {
	TestOverflowNumberPolicy<XmlArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<XmlArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(PugiXmlArchive, ThrowSerializationExceptionWhenOverflowInt16) {
	TestOverflowNumberPolicy<XmlArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<XmlArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(PugiXmlArchive, ThrowSerializationExceptionWhenOverflowInt32) {
	TestOverflowNumberPolicy<XmlArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
	TestOverflowNumberPolicy<XmlArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::ThrowError);
}
TEST(PugiXmlArchive, ThrowSerializationExceptionWhenOverflowFloat) {
	TestOverflowNumberPolicy<XmlArchive, double, float>(BitSerializer::OverflowNumberPolicy::ThrowError);
}

//-----------------------------------------------------------------------------
// Test OverflowNumberPolicy::Skip
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ThrowValidationExceptionWhenOverflowBool) {
	TestOverflowNumberPolicy<XmlArchive, int32_t, bool>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(PugiXmlArchive, ThrowValidationExceptionWhenNumberOverflowInt8) {
	TestOverflowNumberPolicy<XmlArchive, int16_t, int8_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<XmlArchive, uint16_t, uint8_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(PugiXmlArchive, ThrowValidationExceptionWhenNumberOverflowInt16) {
	TestOverflowNumberPolicy<XmlArchive, int32_t, int16_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<XmlArchive, uint32_t, uint16_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(PugiXmlArchive, ThrowValidationExceptionWhenNumberOverflowInt32) {
	TestOverflowNumberPolicy<XmlArchive, int64_t, int32_t>(BitSerializer::OverflowNumberPolicy::Skip);
	TestOverflowNumberPolicy<XmlArchive, uint64_t, uint32_t>(BitSerializer::OverflowNumberPolicy::Skip);
}
TEST(PugiXmlArchive, ThrowValidationExceptionWhenNumberOverflowFloat) {
	TestOverflowNumberPolicy<XmlArchive, double, float>(BitSerializer::OverflowNumberPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Test UtfEncodingErrorPolicy
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ThrowSerializationExceptionWhenEncodingError) {
	TestEncodingPolicy<XmlArchive>(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::ThrowError);
}

TEST(PugiXmlArchive, ShouldSkipInvalidUtfWhenPolicyIsSkip) {
	TestEncodingPolicy<XmlArchive>(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip);
}

//-----------------------------------------------------------------------------
// Tests of `std::optional` (additional coverage of MismatchedTypesPolicy handling)
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SerializeStdOptionalAsObjectMember)
{
	// Simple types as members of object
	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<bool>>>();
	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<bool>>>(TestClassWithSubType(std::optional<bool>(std::nullopt)));

	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<int>>>();
	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<int>>>(TestClassWithSubType(std::optional<int>(std::nullopt)));

	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<float>>>();
	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<float>>>(TestClassWithSubType(std::optional<float>(std::nullopt)));

	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<std::string>>>();
	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<std::string>>>(TestClassWithSubType(std::optional<std::string>(std::nullopt)));

	// Object as member of object
	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<TestPointClass>>>();
	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<TestPointClass>>>(TestClassWithSubType(std::optional<TestPointClass>(std::nullopt)));

	// Array as member of object
	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<std::vector<int>>>>();
	TestSerializeType<XmlArchive, TestClassWithSubType<std::optional<std::vector<int>>>>(TestClassWithSubType(std::optional<std::vector<int>>(std::nullopt)));
}

//-----------------------------------------------------------------------------
// Smoke tests of STD types serialization (more detailed tests in "unit_tests/std_types_tests")
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SerializeStdTypes)
{
	TestSerializeArray<XmlArchive, std::atomic_int>();
	TestSerializeType<XmlArchive, std::pair<std::string, int>>();
	TestSerializeType<XmlArchive, std::tuple<std::string, int, float, bool>>();

	TestSerializeType<XmlArchive, TestClassWithSubType<std::unique_ptr<std::string>>>(TestClassWithSubType(std::make_unique<std::string>("test")));
	TestSerializeType<XmlArchive, TestClassWithSubType<std::shared_ptr<std::string>>>(TestClassWithSubType(std::make_shared<std::string>("test")));

	TestSerializeType<XmlArchive, TestClassWithSubType<std::filesystem::path>>(TestClassWithSubType(std::filesystem::temp_directory_path()));

	TestSerializeType<XmlArchive, TestClassWithSubType<std::chrono::system_clock::time_point>>();
	TestSerializeType<XmlArchive, TestClassWithSubType<std::chrono::seconds>>();
}
