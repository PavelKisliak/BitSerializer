/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/common_xml_test_methods.h"
#include "bitserializer/pugixml_archive.h"

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

TEST(PugiXmlArchive, SerializeArrayOfChars)
{
	TestSerializeArray<XmlArchive, char>();
	TestSerializeArray<XmlArchive, unsigned char>();
}

TEST(PugiXmlArchive, SerializeArrayOfIntegers)
{
	TestSerializeArray<XmlArchive, uint16_t>();
	TestSerializeArray<XmlArchive, int64_t>();
}

TEST(PugiXmlArchive, SerializeArrayOfFloats) {
	TestSerializeArray<XmlArchive, float>();
	TestSerializeArray<XmlArchive, double>();
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
	TestSerializeClassWithKey<XmlArchive>(BuildFixture<TestClassWithSubTypes<int16_t>>());
}

TEST(PugiXmlArchive, SerializeClassWithMemberBoolean) {
	TestSerializeClass<XmlArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<XmlArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(PugiXmlArchive, SerializeClassWithMemberInteger) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
	TestSerializeClass<XmlArchive>(TestClassWithSubTypes(std::numeric_limits<int64_t>::min(), std::numeric_limits<uint64_t>::max()));
}

TEST(PugiXmlArchive, SerializeClassWithMemberFloat) {
	TestSerializeClass<XmlArchive>(TestClassWithSubTypes(std::numeric_limits<float>::min(), 0.0f, std::numeric_limits<float>::max()));
}

TEST(PugiXmlArchive, SerializeClassWithMemberDouble) {
	TestSerializeClass<XmlArchive>(TestClassWithSubTypes(std::numeric_limits<double>::min(), 0.0, std::numeric_limits<double>::max()));
}

TEST(PugiXmlArchive, SerializeClassWithMemberNullptr) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithSubTypes<std::nullptr_t>>());
}

TEST(PugiXmlArchive, SerializeClassWithMemberString) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring, std::u16string, std::u32string>>());
}

TEST(PugiXmlArchive, SerializeClassHierarchy) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithInheritance>());
}

TEST(PugiXmlArchive, SerializeClassWithMemberClass) {
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassType>());
}

TEST(PugiXmlArchive, SerializeClassWithSubArray) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(PugiXmlArchive, SerializeClassWithSubArrayOfClasses) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(PugiXmlArchive, SerializeClassWithSubTwoDimArray) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
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
	TestSerializeClass<XmlArchive>(fixture);
}

TEST(PugiXmlArchive, SerializeClassInReverseOrderWithSubArray)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, std::array<uint64_t, 5>, std::string>>();
	TestSerializeClass<XmlArchive>(fixture);
}

TEST(PugiXmlArchive, SerializeClassInReverseOrderWithSubObject)
{
	auto fixture = BuildFixture<TestClassWithReverseLoad<int, bool, TestPointClass, std::string>>();
	TestSerializeClass<XmlArchive>(fixture);
}

//-----------------------------------------------------------------------------
// Tests of serialization for attributes
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SerializeAttributesWithBoolean) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithAttributes<bool>>());
}

TEST(PugiXmlArchive, SerializeAttributesWithIntegers) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithAttributes<int8_t, uint8_t, int64_t, uint64_t>>());
}

TEST(PugiXmlArchive, SerializeAttributesWithFloats) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithAttributes<float, double>>());
}

TEST(PugiXmlArchive, SerializeAttributesWithNullptr) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithAttributes<std::nullptr_t>>());
}

TEST(PugiXmlArchive, SerializeAttributesWithString) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithAttributes<std::string, std::wstring>>());
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
	TestSerializeClassToStream<XmlArchive, char>(BuildFixture<TestPointClass>());
}

TEST(PugiXmlArchive, SerializeArrayOfClassesToStream)
{
	TestClassWithSubTypes<int, float, std::string, TestPointClass> testArray[3];
	BuildFixture(testArray);
	TestSerializeArrayToStream<XmlArchive, char>(testArray);
}

TEST(PugiXmlArchive, SerializeUnicodeToUtf8Stream) {
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
	TestSerializeClassToStream<XmlArchive, char>(TestValue);
}

TEST(PugiXmlArchive, LoadFromUtf8Stream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf8>(false);
}
TEST(PugiXmlArchive, LoadFromUtf8StreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf8>(true);
}

TEST(PugiXmlArchive, LoadFromUtf16LeStream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf16Le>(false);
}
TEST(PugiXmlArchive, LoadFromUtf16LeStreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf16Le>(true);
}

TEST(PugiXmlArchive, LoadFromUtf16BeStream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf16Be>(false);
}
TEST(PugiXmlArchive, LoadFromUtf16BeStreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf16Be>(true);
}

TEST(PugiXmlArchive, LoadFromUtf32LeStream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf32Le>(false);
}
TEST(PugiXmlArchive, LoadFromUtf32LeStreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf32Le>(true);
}

TEST(PugiXmlArchive, LoadFromUtf32BeStream) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf32Be>(false);
}
TEST(PugiXmlArchive, LoadFromUtf32BeStreamWithBom) {
	TestLoadXmlFromEncodedStream<XmlArchive, BitSerializer::Convert::Utf32Be>(true);
}

TEST(PugiXmlArchive, SaveToUtf8Stream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf8>(false);
}
TEST(PugiXmlArchive, SaveToUtf8StreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf8>(true);
}

TEST(PugiXmlArchive, SaveToUtf16LeStream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf16Le>(false);
}
TEST(PugiXmlArchive, SaveToUtf16LeStreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf16Le>(true);
}

TEST(PugiXmlArchive, SaveToUtf16BeStream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf16Be>(false);
}
TEST(PugiXmlArchive, SaveToUtf16BeStreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf16Be>(true);
}

TEST(PugiXmlArchive, SaveToUtf32LeStream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf32Le>(false);
}
TEST(PugiXmlArchive, SaveToUtf32LeStreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf32Le>(true);
}

TEST(PugiXmlArchive, SaveToUtf32BeStream) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf32Be>(false);
}
TEST(PugiXmlArchive, SaveToUtf32BeStreamWithBom) {
	TestSaveXmlToEncodedStream<XmlArchive, BitSerializer::Convert::Utf32Be>(true);
}

TEST(PugiXmlArchive, ThrowExceptionWhenUnsupportedStreamEncoding)
{
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.streamOptions.encoding = static_cast<BitSerializer::Convert::UtfType>(-1);
	std::stringstream outputStream;
	auto testObj = BuildFixture<TestClassWithSubTypes<std::string>>();
	EXPECT_THROW(BitSerializer::SaveObject<XmlArchive>(testObj, outputStream, serializationOptions), BitSerializer::SerializationException);
}

TEST(PugiXmlArchive, SerializeToFile) {
	TestSerializeArrayToFile<XmlArchive>();
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
