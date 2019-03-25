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
	TestSerializeArray<XmlArchive, int64_t, 7, 5>();
	TestSerializeArray<XmlArchive, double, 7, 5>();
	TestSerializeArray<XmlArchive, std::string, 7, 5>();
	TestSerializeArray<XmlArchive, TestPointClass, 7, 5>();
}

TEST(PugiXmlArchive, ShouldLoadToArrayWithBiggerAmountOfElements) {
	TestSerializeArray<XmlArchive, bool, 5, 7>();
	TestSerializeArray<XmlArchive, int64_t, 5, 7>();
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
// Test the validation for named values (boolean result, which returned from archive methods).
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
TEST(PugiXmlArchive, SerializeClassToWStream) {
	TestSerializeClassToStream<XmlArchive, wchar_t>(BuildFixture<TestPointClass>());
}

TEST(PugiXmlArchive, SerializeClassToFile) {
	TestSerializeClassToFile<XmlArchive, wchar_t>(BuildFixture<TestPointClass>());
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, ThrowExceptionWhenBadSyntaxInSource) {
	auto fixture = BuildFixture<TestClassWithSubTypes<std::string>>();
	EXPECT_THROW(BitSerializer::LoadObject<XmlArchive>(fixture, "<root>Hello"), BitSerializer::SerializationException);
}
