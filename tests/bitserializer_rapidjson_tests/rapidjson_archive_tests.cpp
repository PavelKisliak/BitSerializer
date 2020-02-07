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

#pragma warning(push)
#pragma warning(disable: 4566)

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeBoolean)
{
	// UTF-8 archive
	TestSerializeType<JsonUtf8Archive, bool>(false);
	TestSerializeType<JsonUtf8Archive, bool>(true);

	// UTF-16 archive
	TestSerializeType<JsonUtf16Archive, bool>(false);
	TestSerializeType<JsonUtf16Archive, bool>(true);
}

TEST(RapidJsonArchive, SerializeInteger)
{
	// UTF-8 archive
	TestSerializeType<JsonUtf8Archive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<JsonUtf8Archive, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<JsonUtf8Archive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<JsonUtf8Archive, uint64_t>(std::numeric_limits<uint64_t>::max());

	// UTF-16 archive
	TestSerializeType<JsonUtf16Archive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<JsonUtf16Archive, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<JsonUtf16Archive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<JsonUtf16Archive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(RapidJsonArchive, SerializeFloat)
{
	// UTF-8 archive
	TestSerializeType<JsonUtf8Archive, float>(::BuildFixture<float>());
	// UTF-16 archive
	TestSerializeType<JsonUtf16Archive, float>(::BuildFixture<float>());
}

TEST(RapidJsonArchive, SerializeDouble)
{
	// UTF-8 archive
	TestSerializeType<JsonUtf8Archive, double>(::BuildFixture<double>());
	// UTF-16 archive
	TestSerializeType<JsonUtf16Archive, double>(::BuildFixture<double>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::string and std::wstring (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeAnsiString)
{
	// UTF-8 archive
	TestSerializeType<JsonUtf8Archive, std::string>("Test ANSI string");
	// UTF-16 archive
	TestSerializeType<JsonUtf16Archive, std::string>("Test ANSI string");
}

TEST(RapidJsonArchive, SerializeUnicodeString)
{
	// UTF-8 archive
	TestSerializeType<JsonUtf8Archive, std::wstring>(L"Test Unicode string - Привет мир!");
	// UTF-16 archive
	TestSerializeType<JsonUtf16Archive, std::wstring>(L"Test Unicode string - Привет мир!");
}

TEST(RapidJsonArchive, SerializeEnum)
{
	// UTF-8 archive
	TestSerializeType<JsonUtf8Archive, TestEnum>(TestEnum::Two);
	// UTF-16 archive
	TestSerializeType<JsonUtf16Archive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeArrayOfBooleans)
{
	// UTF-8 archive
	TestSerializeArray<JsonUtf8Archive, bool>();
	// UTF-16 archive
	TestSerializeArray<JsonUtf16Archive, bool>();
}

TEST(RapidJsonArchive, SerializeArrayOfIntegers)
{
	// UTF-8 archive
	TestSerializeArray<JsonUtf8Archive, int8_t>();
	TestSerializeArray<JsonUtf8Archive, int64_t>();

	// UTF-16 archive
	TestSerializeArray<JsonUtf16Archive, int8_t>();
	TestSerializeArray<JsonUtf16Archive, int64_t>();
}

TEST(RapidJsonArchive, SerializeArrayOfFloats)
{
	// UTF-8 archive
	TestSerializeArray<JsonUtf8Archive, float>();
	TestSerializeArray<JsonUtf8Archive, double>();
	// UTF-16 archive
	TestSerializeArray<JsonUtf16Archive, float>();
	TestSerializeArray<JsonUtf16Archive, double>();
}

TEST(RapidJsonArchive, SerializeArrayOfStrings)
{
	// UTF-8 archive
	TestSerializeArray<JsonUtf8Archive, std::string>();

	// UTF-16 archive
	TestSerializeArray<JsonUtf16Archive, std::string>();
}

TEST(RapidJsonArchive, SerializeArrayOfWStrings)
{
	// UTF-8 archive
	TestSerializeArray<JsonUtf8Archive, std::wstring>();
	// UTF-16 archive
	TestSerializeArray<JsonUtf16Archive, std::wstring>();
}

TEST(RapidJsonArchive, SerializeArrayOfClasses)
{
	// UTF-8 archive
	TestSerializeArray<JsonUtf8Archive, TestPointClass>();
	// UTF-16 archive
	TestSerializeArray<JsonUtf16Archive, TestPointClass>();
}

TEST(RapidJsonArchive, SerializeTwoDimensionalArray)
{
	// UTF-8 archive
	TestSerializeTwoDimensionalArray<JsonUtf8Archive, int32_t>();
	// UTF-16 archive
	TestSerializeTwoDimensionalArray<JsonUtf16Archive, int32_t>();
}

TEST(RapidJsonArchive, ShouldLoadToArrayWithLesserAmountOfElements)
{
	// UTF-8 archive
	TestSerializeArray<JsonUtf8Archive, bool, 7, 5>();
	TestSerializeArray<JsonUtf8Archive, int, 7, 5>();
	TestSerializeArray<JsonUtf8Archive, double, 7, 5>();
	TestSerializeArray<JsonUtf8Archive, std::string, 7, 5>();
	TestSerializeArray<JsonUtf8Archive, TestPointClass, 7, 5>();

	// UTF-16 archive
	TestSerializeArray<JsonUtf16Archive, bool, 7, 5>();
	TestSerializeArray<JsonUtf16Archive, int, 7, 5>();
	TestSerializeArray<JsonUtf16Archive, double, 7, 5>();
	TestSerializeArray<JsonUtf16Archive, std::string, 7, 5>();
	TestSerializeArray<JsonUtf16Archive, TestPointClass, 7, 5>();
}

TEST(RapidJsonArchive, ShouldLoadToArrayWithBiggerAmountOfElements)
{
	// UTF-8 archive
	TestSerializeArray<JsonUtf8Archive, bool, 5, 7>();
	TestSerializeArray<JsonUtf8Archive, int, 5, 7>();
	TestSerializeArray<JsonUtf8Archive, double, 5, 7>();
	TestSerializeArray<JsonUtf8Archive, std::string, 5, 7>();
	TestSerializeArray<JsonUtf8Archive, TestPointClass, 5, 7>();

	// UTF-16 archive
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
	// UTF-8 archive
	TestSerializeClass<JsonUtf8Archive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<JsonUtf8Archive>(TestClassWithSubTypes<bool>(true));

	// UTF-16 archive
	TestSerializeClass<JsonUtf16Archive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<JsonUtf16Archive>(TestClassWithSubTypes<bool>(true));
}

TEST(RapidJsonArchive, SerializeClassWithMemberInteger)
{
	// UTF-8 archive
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
	// UTF-16 archive
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberFloat)
{
	// UTF-8 archive
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTypes<float>>());
	// UTF-16 archive
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTypes<float>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberDouble)
{
	// UTF-8 archive
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTypes<double>>());
	// UTF-16 archive
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTypes<double>>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberString)
{
	// UTF-8 archive
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
	// UTF-16 archive
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
}

TEST(RapidJsonArchive, SerializeClassHierarchy)
{
	// UTF-8 archive
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithInheritance>());
	// UTF-16 archive
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithInheritance>());
}

TEST(RapidJsonArchive, SerializeClassWithMemberClass)
{
	// UTF-8 archive
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassType>());

	// UTF-16 archive
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassType>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArray)
{
	// UTF-8 archive
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubArray<int64_t>>());
	// UTF-16 archive
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubArrayOfClasses)
{
	// UTF-8 archive
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
	// UTF-16 archive
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(RapidJsonArchive, SerializeClassWithSubTwoDimArray)
{
	// UTF-8 archive
	TestSerializeClass<JsonUtf8Archive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
	// UTF-16 archive
	TestSerializeClass<JsonUtf16Archive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(RapidJsonArchive, ShouldIterateKeysInObjectScope)
{
	// UTF-8 archive
	TestIterateKeysInObjectScope<JsonUtf8Archive>();
	// UTF-16 archive
	TestIterateKeysInObjectScope<JsonUtf16Archive>();
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ShouldReturnPathInObjectScopeWhenLoading)
{
	// UTF-8 archive
	TestGetPathInJsonObjectScopeWhenLoading<JsonUtf8Archive>();
	// UTF-16 archive
	TestGetPathInJsonObjectScopeWhenLoading<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInObjectScopeWhenSaving)
{
	// UTF-8 archive
	TestGetPathInJsonObjectScopeWhenSaving<JsonUtf8Archive>();
	// UTF-16 archive
	TestGetPathInJsonObjectScopeWhenSaving<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInArrayScopeWhenLoading)
{
	// UTF-8 archive
	TestGetPathInJsonArrayScopeWhenLoading<JsonUtf8Archive>();
	// UTF-16 archive
	TestGetPathInJsonArrayScopeWhenLoading<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, ShouldReturnPathInArrayScopeWhenSaving)
{
	// UTF-8 archive
	TestGetPathInJsonArrayScopeWhenSaving<JsonUtf8Archive>();
	// UTF-16 archive
	TestGetPathInJsonArrayScopeWhenSaving<JsonUtf16Archive>();
}

//-----------------------------------------------------------------------------
// Test the validation for named values (boolean result, which returns by archive's method SerializeValue()).
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ShouldCollectErrorAboutRequiredNamedValues)
{
	// UTF-8 archive
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<JsonUtf8Archive, TestClassForCheckValidation<TestPointClass>>();

	// UTF-16 archive
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<JsonUtf16Archive, TestClassForCheckValidation<TestPointClass>>();
}

//-----------------------------------------------------------------------------
// Tests format output JSON
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SaveWithFormatting)
{
	// UTF-8 archive
	TestSaveFormattedJson<JsonUtf8Archive>();
	// UTF-16 archive
	TestSaveFormattedJson<JsonUtf16Archive>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeClassToStream)
{
	// UTF-8 archive
	TestSerializeClassToStream<JsonUtf8Archive, char>(BuildFixture<TestPointClass>());
	// UTF-16 archive
	TestSerializeClassToStream<JsonUtf16Archive, wchar_t>(BuildFixture<TestPointClass>());
}

TEST(RapidJsonArchive, SerializeUnicodeToEncodedStream)
{
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");

	// UTF-8 archive
	TestSerializeClassToStream<JsonUtf8Archive, char>(TestValue);
	// UTF-16 archive
	TestSerializeClassToStream<JsonUtf16Archive, wchar_t>(TestValue);
}

TEST(RapidJsonArchive, LoadFromUtf8StreamWithBom)
{
	// UTF-8 archive
	TestLoadJsonFromUtf8StreamWithBom<JsonUtf8Archive>();
	// UTF-16 archive
	TestLoadJsonFromUtf8StreamWithBom<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, LoadFromUtf8StreamWithoutBom)
{
	// UTF-8 archive
	TestLoadJsonFromUtf8StreamWithoutBom<JsonUtf8Archive>();
	// UTF-16 archive
	TestLoadJsonFromUtf8StreamWithoutBom<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, SaveToUtf8StreamWithBom)
{
	// UTF-8 archive
	TestSaveJsonToUtf8StreamWithBom<JsonUtf8Archive>();
	// UTF-16 archive
	TestSaveJsonToUtf8StreamWithBom<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, SaveToUtf8StreamWithoutBom)
{
	// UTF-8 archive
	TestSaveJsonToUtf8StreamWithoutBom<JsonUtf8Archive>();
	// UTF-16 archive
	TestSaveJsonToUtf8StreamWithoutBom<JsonUtf16Archive>();
}

TEST(RapidJsonArchive, SerializeClassToFile)
{
	// UTF-8 archive
	TestSerializeClassToFile<JsonUtf8Archive>(BuildFixture<TestPointClass>());
	// UTF-16 archive
	TestSerializeClassToFile<JsonUtf16Archive>(BuildFixture<TestPointClass>());
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, ThrowExceptionWhenBadSyntaxInSource)
{
	int testInt;
	// UTF-8 archive
	EXPECT_THROW(BitSerializer::LoadObject<JsonUtf8Archive>(testInt, "10 }}"), BitSerializer::SerializationException);
	// UTF-16 archive
	EXPECT_THROW(BitSerializer::LoadObject<JsonUtf16Archive>(testInt, L"10 }}"), BitSerializer::SerializationException);
}

#pragma warning(pop)
