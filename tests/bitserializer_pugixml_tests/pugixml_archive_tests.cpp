/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "bitserializer_pugixml/pugixml_archive.h"
#include "test_helpers/common_test_methods.h"

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

TEST(PugiXmlArchive, SerializeArrayOfStrings) {
	TestSerializeArray<XmlArchive, std::string>();
}

TEST(PugiXmlArchive, SerializeArrayOfWStrings) {
	TestSerializeArray<XmlArchive, std::wstring>();
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
}

TEST(PugiXmlArchive, SerializeClassWithMemberFloat) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithSubTypes<float>>());
}

TEST(PugiXmlArchive, SerializeClassWithMemberDouble) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithSubTypes<double>>());
}

TEST(PugiXmlArchive, SerializeClassWithMemberString) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
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

TEST(PugiXmlArchive, SerializeAttributesWithString) {
	TestSerializeClass<XmlArchive>(BuildFixture<TestClassWithAttributes<std::string, std::wstring>>());
}

//-----------------------------------------------------------------------------
// Test the validation for named values (boolean result, which returns by archive's method SerializeValue()).
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ShouldCollectErrorAboutRequiredNamedValues) {
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<XmlArchive, TestClassForCheckValidation<TestPointClass>>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SerializeClassToStream)
{
	TestSerializeClassToStream<XmlArchive, char>(BuildFixture<TestPointClass>());
}

TEST(PugiXmlArchive, SerializeUnicodeToUtf8Stream) {
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
	TestSerializeClassToStream<XmlArchive, char>(TestValue);
}

TEST(PugiXmlArchive, LoadFromUtf8StreamWithBom)
{
	// Arrange
	std::stringstream inputStream(std::string({ char(0xEF), char(0xBB), char(0xBF) }) +
		R"(<?xml version="1.0" encoding="utf-8"?><root><TestValue>Hello world!</TestValue></root>)");

	// Act
	TestClassWithSubType<std::string> actual;
	BitSerializer::LoadObject<XmlArchive>(actual, inputStream);

	// Assert
	EXPECT_EQ("Hello world!", actual.GetValue());
}

TEST(PugiXmlArchive, LoadFromUtf8StreamWithoutBom)
{
	// Arrange
	std::stringstream inputStream(
		std::string(R"(<?xml version="1.0" encoding="utf-8"?><root><TestValue>Hello world!</TestValue></root>)"));

	// Act
	TestClassWithSubType<std::string> actual;
	BitSerializer::LoadObject<XmlArchive>(actual, inputStream);

	// Assert
	EXPECT_EQ("Hello world!", actual.GetValue());
}

TEST(PugiXmlArchive, SaveToUtf8StreamWithBom)
{
	// Arrange
	std::stringstream outputStream;
	TestClassWithSubType<std::string> testObj("Hello world!");

	// Act
	BitSerializer::SaveObject<XmlArchive>(testObj, outputStream);

	// Assert
	const auto result = outputStream.str();
	EXPECT_TRUE(result.find(std::string({ char(0xEF), char(0xBB), char(0xBF) }) + R"(<?xml version)") == 0);
}

TEST(PugiXmlArchive, SaveToUtf8StreamWithoutBom)
{
	// Arrange
	std::stringstream outputStream;
	TestClassWithSubType<std::string> testObj("Hello world!");
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.streamOptions.writeBom = false;

	// Act
	BitSerializer::SaveObject<XmlArchive>(testObj, outputStream, serializationOptions);

	// Assert
	const auto result = outputStream.str();
	EXPECT_TRUE(result.find(R"(<?xml version)") == 0);
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
