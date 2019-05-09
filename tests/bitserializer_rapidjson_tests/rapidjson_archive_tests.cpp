/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/

// Define for suppress warning STL4015 : The std::iterator class template (used as a base class to provide typedefs) is deprecated in C++17.
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include "../test_helpers/common_test_methods.h"
#include "../test_helpers/common_json_test_methods.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"

using BitSerializer::Json::RapidJson::JsonUtf8Archive;
using BitSerializer::Json::RapidJson::JsonUtf16Archive;

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeBoolean)
{
	// UTF8
	TestSerializeType<JsonUtf8Archive, bool>(false);
	TestSerializeType<JsonUtf8Archive, bool>(true);

	// UTF16
	TestSerializeType<JsonUtf16Archive, bool>(false);
	TestSerializeType<JsonUtf16Archive, bool>(true);
}

TEST(RapidJsonArchive, SerializeInteger)
{
	// UTF8
	TestSerializeType<JsonUtf8Archive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<JsonUtf8Archive, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<JsonUtf8Archive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<JsonUtf8Archive, uint64_t>(std::numeric_limits<uint64_t>::max());

	// UTF16
	TestSerializeType<JsonUtf16Archive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<JsonUtf16Archive, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<JsonUtf16Archive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<JsonUtf16Archive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(RapidJsonArchive, SerializeFloat)
{
	// UTF8
	TestSerializeType<JsonUtf8Archive, float>(::BuildFixture<float>());
	// UTF16
	TestSerializeType<JsonUtf16Archive, float>(::BuildFixture<float>());
}

TEST(RapidJsonArchive, SerializeDouble)
{
	// UTF8
	TestSerializeType<JsonUtf8Archive, double>(::BuildFixture<double>());
	// UTF16
	TestSerializeType<JsonUtf16Archive, double>(::BuildFixture<double>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::string and std::wstring (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeString)
{
	// UTF8
	TestSerializeType<JsonUtf8Archive, std::string>("Test ANSI string");
	// UTF16
	TestSerializeType<JsonUtf16Archive, std::string>("Test ANSI string");
}

TEST(RapidJsonArchive, SerializeWString)
{
	// UTF8
	TestSerializeType<JsonUtf8Archive, std::wstring>(L"Test wide string");
	// UTF16
	TestSerializeType<JsonUtf16Archive, std::wstring>(L"Test wide string");
}

TEST(RapidJsonArchive, SerializeEnum)
{
	// UTF8
	TestSerializeType<JsonUtf8Archive, TestEnum>(TestEnum::Two);
	// UTF16
	TestSerializeType<JsonUtf16Archive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeArrayOfBooleans)
{
	// UTF8
	TestSerializeArray<JsonUtf8Archive, bool>();
	// UTF16
	TestSerializeArray<JsonUtf16Archive, bool>();
}

TEST(RapidJsonArchive, SerializeArrayOfIntegers)
{
	// UTF8
	TestSerializeArray<JsonUtf8Archive, int8_t>();
	TestSerializeArray<JsonUtf8Archive, int64_t>();

	// UTF16
	TestSerializeArray<JsonUtf16Archive, int8_t>();
	TestSerializeArray<JsonUtf16Archive, int64_t>();
}

TEST(RapidJsonArchive, SerializeArrayOfFloats)
{
	// UTF8
	TestSerializeArray<JsonUtf8Archive, float>();
	TestSerializeArray<JsonUtf8Archive, double>();
	// UTF16
	TestSerializeArray<JsonUtf16Archive, float>();
	TestSerializeArray<JsonUtf16Archive, double>();
}

TEST(RapidJsonArchive, SerializeArrayOfStrings)
{
	// UTF8
	TestSerializeArray<JsonUtf8Archive, std::string>();

	// UTF16
	TestSerializeArray<JsonUtf16Archive, std::string>();
}

TEST(RapidJsonArchive, SerializeArrayOfWStrings)
{
	// UTF8
	TestSerializeArray<JsonUtf8Archive, std::wstring>();
	// UTF16
	TestSerializeArray<JsonUtf16Archive, std::wstring>();
}

TEST(RapidJsonArchive, SerializeArrayOfClasses)
{
	// UTF8
	TestSerializeArray<JsonUtf8Archive, TestPointClass>();
	// UTF16
	TestSerializeArray<JsonUtf16Archive, TestPointClass>();
}

TEST(RapidJsonArchive, SerializeTwoDimensionalArray)
{
	// UTF8
	TestSerializeTwoDimensionalArray<JsonUtf8Archive, int32_t>();
	// UTF16
	TestSerializeTwoDimensionalArray<JsonUtf16Archive, int32_t>();
}

TEST(RapidJsonArchive, ShouldLoadToArrayWithLesserAmountOfElements)
{
	// UTF8
	TestSerializeArray<JsonUtf8Archive, bool, 7, 5>();
	TestSerializeArray<JsonUtf8Archive, int, 7, 5>();
	TestSerializeArray<JsonUtf8Archive, double, 7, 5>();
	TestSerializeArray<JsonUtf8Archive, std::string, 7, 5>();
	TestSerializeArray<JsonUtf8Archive, TestPointClass, 7, 5>();

	// UTF16
	TestSerializeArray<JsonUtf16Archive, bool, 7, 5>();
	TestSerializeArray<JsonUtf16Archive, int, 7, 5>();
	TestSerializeArray<JsonUtf16Archive, double, 7, 5>();
	TestSerializeArray<JsonUtf16Archive, std::string, 7, 5>();
	TestSerializeArray<JsonUtf16Archive, TestPointClass, 7, 5>();
}

TEST(RapidJsonArchive, ShouldLoadToArrayWithBiggerAmountOfElements)
{
	// UTF8
	TestSerializeArray<JsonUtf8Archive, bool, 5, 7>();
	TestSerializeArray<JsonUtf8Archive, int, 5, 7>();
	TestSerializeArray<JsonUtf8Archive, double, 5, 7>();
	TestSerializeArray<JsonUtf8Archive, std::string, 5, 7>();
	TestSerializeArray<JsonUtf8Archive, TestPointClass, 5, 7>();

	// UTF16
	TestSerializeArray<JsonUtf16Archive, bool, 5, 7>();
	TestSerializeArray<JsonUtf16Archive, int, 5, 7>();
	TestSerializeArray<JsonUtf16Archive, double, 5, 7>();
	TestSerializeArray<JsonUtf16Archive, std::string, 5, 7>();
	TestSerializeArray<JsonUtf16Archive, TestPointClass, 5, 7>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeClassWithMemberBoolean)
{
	// UTF8
	TestSerializeClass<JsonUtf8Archive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<JsonUtf8Archive>(TestClassWithSubTypes<bool>(true));

	// UTF16
	TestSerializeClass<JsonUtf16Archive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<JsonUtf16Archive>(TestClassWithSubTypes<bool>(true));
}

TEST(RapidJsonArchive, SerializeClassWithMemberInteger)
{
	// UTF8
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
	// UTF16
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberFloat)
{
	// UTF8
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTypes<float>>());
	// UTF16
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTypes<float>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberDouble)
{
	// UTF8
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTypes<double>>());
	// UTF16
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTypes<double>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberString)
{
	// UTF8
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
	// UTF16
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
}

TEST(RapidJsonArchive, SerializeClassHierarchy)
{
	// UTF8
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithInheritance>());
	// UTF16
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithInheritance>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberClass)
{
	// UTF8
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassType>());

	// UTF16
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassType>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArray)
{
	// UTF8
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubArray<int64_t>>());
	// UTF16
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArrayOfClasses)
{
	// UTF8
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
	// UTF16
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubTwoDimArray)
{
	// UTF8
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
	// UTF16
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(RapidJsonArchive, ShouldIterateKeysInObjectScope)
{
	// UTF8
	TestIterateKeysInObjectScope<JsonUtf8Archive>();
	// UTF16
	TestIterateKeysInObjectScope<JsonUtf16Archive>();
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ShouldReturnPathInObjectScopeWhenLoading)
{
	// UTF8
	TestGetPathInJsonObjectScopeWhenLoading<JsonUtf8Archive>();
	// UTF16
	TestGetPathInJsonObjectScopeWhenLoading<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInObjectScopeWhenSaving)
{
	// UTF8
	TestGetPathInJsonObjectScopeWhenSaving<JsonUtf8Archive>();
	// UTF16
	TestGetPathInJsonObjectScopeWhenSaving<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInArrayScopeWhenLoading)
{
	// UTF8
	TestGetPathInJsonArrayScopeWhenLoading<JsonUtf8Archive>();
	// UTF16
	TestGetPathInJsonArrayScopeWhenLoading<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInArrayScopeWhenSaving)
{
	// UTF8
	TestGetPathInJsonArrayScopeWhenSaving<JsonUtf8Archive>();
	// UTF16
	TestGetPathInJsonArrayScopeWhenSaving<JsonUtf16Archive>();
}

//-----------------------------------------------------------------------------
// Test the validation for named values (boolean result, which returns by archive's method SerializeValue()).
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ShouldCollectErrorAboutRequiredNamedValues)
{
	// UTF8
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<TestPointClass>>();

	// UTF16
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<TestPointClass>>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeClassToWStream)
{
	// UTF8
	TestSerializeClassToStream<JsonUtf8Archive, char>(BuildFixture<TestPointClass>());
	// UTF16
	TestSerializeClassToStream<JsonUtf16Archive, wchar_t>(BuildFixture<TestPointClass>());
}

TEST(RapidJsonArchive, SerializeClassToFile)
{
	// UTF8
	TestSerializeClassToFile<JsonUtf8Archive, char>(BuildFixture<TestPointClass>());
	// UTF16
	TestSerializeClassToFile<JsonUtf16Archive, wchar_t>(BuildFixture<TestPointClass>());
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowExceptionWhenBadSyntaxInSource)
{
	int testInt;
	// UTF8
	EXPECT_THROW(BitSerializer::LoadObject<JsonUtf8Archive>(testInt, "10 }}"), BitSerializer::SerializationException);
	// UTF16
	EXPECT_THROW(BitSerializer::LoadObject<JsonUtf16Archive>(testInt, L"10 }}"), BitSerializer::SerializationException);
}
