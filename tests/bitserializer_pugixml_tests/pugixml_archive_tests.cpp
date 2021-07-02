/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "bitserializer/pugixml_archive.h"
#include "test_helpers/common_test_methods.h"
#include "test_helpers/common_xml_test_methods.h"

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

TEST(PugiXmlArchive, SerializeArrayOfIntegers) {
	TestSerializeArray<XmlArchive, int8_t>();
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

TEST(PugiXmlArchive, ShouldLoadToArrayWithLesserAmountOfElements) {
	TestSerializeArray<XmlArchive, bool, 7, 5>();
	TestSerializeArray<XmlArchive, int, 7, 5>();
	TestSerializeArray<XmlArchive, double, 7, 5>();
	TestSerializeArray<XmlArchive, std::string, 7, 5>();
	TestSerializeArray<XmlArchive, TestPointClass, 7, 5>();
}

TEST(PugiXmlArchive, ShouldLoadToArrayWithBiggerAmountOfElements) {
	TestSerializeArray<XmlArchive, bool, 5, 7>();
	TestSerializeArray<XmlArchive, int, 5, 7>();
	TestSerializeArray<XmlArchive, double, 5, 7>();
	TestSerializeArray<XmlArchive, std::string, 5, 7>();
	TestSerializeArray<XmlArchive, TestPointClass, 5, 7>();
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

TEST(PugiXmlArchive, ShouldIterateKeysInObjectScope) {
	TestIterateKeysInObjectScope<XmlArchive>();
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
// Test the validation for named values (boolean result, which returns by archive's method SerializeValue()).
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ShouldCollectErrorsAboutRequiredNamedValues) {
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<TestPointClass>>();
}

TEST(PugiXmlArchive, ShouldCollectErrorsWhenLoadingFromNotCompatibleTypes)
{
	using SourceStringType = TestClassForCheckCompatibleTypes<std::string>;
	TestValidationForNotCompatibleTypes<XmlArchive, SourceStringType, TestClassForCheckCompatibleTypes<std::nullptr_t>>();
	TestValidationForNotCompatibleTypes<XmlArchive, SourceStringType, TestClassForCheckCompatibleTypes<bool>>();
	TestValidationForNotCompatibleTypes<XmlArchive, SourceStringType, TestClassForCheckCompatibleTypes<int>>();
	TestValidationForNotCompatibleTypes<XmlArchive, SourceStringType, TestClassForCheckCompatibleTypes<double>>();
	TestValidationForNotCompatibleTypes<XmlArchive, SourceStringType, TestClassForCheckCompatibleTypes<TestPointClass>>();
	TestValidationForNotCompatibleTypes<XmlArchive, SourceStringType, TestClassForCheckCompatibleTypes<int[3]>>();
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

TEST(PugiXmlArchive, SerializeClassToFile) {
	TestSerializeClassToFile<XmlArchive>(BuildFixture<TestPointClass>());
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ThrowExceptionWhenBadSyntaxInSource) {
	auto fixture = BuildFixture<TestClassWithSubTypes<std::string>>();
	EXPECT_THROW(BitSerializer::LoadObject<XmlArchive>(fixture, PUGIXML_TEXT("<root>Hello")), BitSerializer::SerializationException);
}
