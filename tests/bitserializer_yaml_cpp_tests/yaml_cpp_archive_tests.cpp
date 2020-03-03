/*******************************************************************************
* Copyright (C) 2020 by Artsiom Marozau                                        *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/

#include "../test_helpers/common_test_methods.h"
#include "../test_helpers/common_json_test_methods.h"
#include "bitserializer_yaml_cpp/yaml_cpp_archive.h"

using BitSerializer::Yaml::YamlCpp::YamlArchive;

//-----------------------------------------------------------------------------
// Tests of serialization for char types (at root scope of archive)
//-----------------------------------------------------------------------------

/// <summary>	Check that value convert to YAML as expected and vice versa. </summary>
/// <param name="value">	Value of char. </param>
/// <param name="yaml"> 	YAML representation of value. </param>

template <typename T>
void TestIOSeparately(T&& value, const std::string& yaml)
{
	{
		const auto& expected = yaml;
		std::string result;
		BitSerializer::SaveObject<YamlArchive>(value, result);
		EXPECT_EQ(result, expected);
	}

	{
		const auto& expected = value;
		std::decay_t<T> result;
		BitSerializer::LoadObject<YamlArchive>(result, yaml);
		EXPECT_EQ(result, expected);
	}
}

TEST(YamlCppArchive, SerializeChar)
{
	auto min = std::numeric_limits<char>::min();
	auto max = std::numeric_limits<char>::max();
	TestIOSeparately(min, std::to_string(min));
	TestIOSeparately(max, std::to_string(max));
}

TEST(YamlCppArchive, SerializeUInt8)
{
	auto min = std::numeric_limits<uint8_t>::min();
	auto max = std::numeric_limits<uint8_t>::max();
	TestIOSeparately(min, std::to_string(min));
	TestIOSeparately(max, std::to_string(max));
}

TEST(YamlCppArchive, SerializeInt8)
{
	auto min = std::numeric_limits<int8_t>::min();
	auto max = std::numeric_limits<int8_t>::max();
	TestIOSeparately(min, std::to_string(min));
	TestIOSeparately(max, std::to_string(max));
}

//-----------------------------------------------------------------------------
// Tests of serialization for fundamental types (at root scope of archive)
//-----------------------------------------------------------------------------

TEST(YamlCppArchive, SerializeBoolean)
{
	TestSerializeType<YamlArchive, bool>(false);
	TestSerializeType<YamlArchive, bool>(true);
}

TEST(YamlCppArchive, SerializeInteger)
{
	TestSerializeType<YamlArchive, uint8_t>(std::numeric_limits<uint8_t>::min());
	TestSerializeType<YamlArchive, uint8_t>(std::numeric_limits<uint8_t>::max());
	TestSerializeType<YamlArchive, int64_t>(std::numeric_limits<int64_t>::min());
	TestSerializeType<YamlArchive, uint64_t>(std::numeric_limits<uint64_t>::max());
}

TEST(YamlCppArchive, SerializeFloat)
{
	TestSerializeType<YamlArchive, float>(::BuildFixture<float>());
}

TEST(YamlCppArchive, SerializeDouble)
{
	TestSerializeType<YamlArchive, double>(::BuildFixture<double>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::string and std::wstring (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(YamlCppArchive, SerializeAnsiString)
{
	TestSerializeType<YamlArchive, std::string>("Test ANSI string");
}

TEST(YamlCppArchive, SerializeUnicodeString)
{
	TestSerializeType<YamlArchive, std::wstring>(L"Test Unicode string - Привет мир!");
}

TEST(YamlCppArchive, SerializeEnum)
{
	TestSerializeType<YamlArchive, TestEnum>(TestEnum::Two);
}

//-----------------------------------------------------------------------------
// Tests of serialization for c-arrays (at root scope of archive)
//-----------------------------------------------------------------------------
TEST(YamlCppArchive, SerializeArrayOfBooleans)
{
	TestSerializeArray<YamlArchive, bool>();
}

TEST(YamlCppArchive, SerializeArrayOfIntegers)
{
	TestSerializeArray<YamlArchive, int8_t>();
	TestSerializeArray<YamlArchive, int64_t>();
}

TEST(YamlCppArchive, SerializeArrayOfFloats)
{
	TestSerializeArray<YamlArchive, float>();
	TestSerializeArray<YamlArchive, double>();
}

TEST(YamlCppArchive, SerializeArrayOfStrings)
{
	TestSerializeArray<YamlArchive, std::string>();
}

TEST(YamlCppArchive, SerializeArrayOfWStrings)
{
	TestSerializeArray<YamlArchive, std::wstring>();
}

TEST(YamlCppArchive, SerializeArrayOfClasses)
{
	TestSerializeArray<YamlArchive, TestPointClass>();
}

TEST(YamlCppArchive, SerializeTwoDimensionalArray)
{
	TestSerializeTwoDimensionalArray<YamlArchive, int32_t>();
}

TEST(YamlCppArchive, ShouldLoadToArrayWithLesserAmountOfElements)
{
	TestSerializeArray<YamlArchive, bool, 7, 5>();
	TestSerializeArray<YamlArchive, int, 7, 5>();
	TestSerializeArray<YamlArchive, double, 7, 5>();
	TestSerializeArray<YamlArchive, std::string, 7, 5>();
	TestSerializeArray<YamlArchive, TestPointClass, 7, 5>();
}

TEST(YamlCppArchive, ShouldLoadToArrayWithBiggerAmountOfElements)
{
	TestSerializeArray<YamlArchive, bool, 5, 7>();
	TestSerializeArray<YamlArchive, int, 5, 7>();
	TestSerializeArray<YamlArchive, double, 5, 7>();
	TestSerializeArray<YamlArchive, std::string, 5, 7>();
	TestSerializeArray<YamlArchive, TestPointClass, 5, 7>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for classes
//-----------------------------------------------------------------------------
TEST(YamlCppArchive, SerializeClassWithMemberBoolean)
{
	TestSerializeClass<YamlArchive>(TestClassWithSubTypes<bool>(false));
	TestSerializeClass<YamlArchive>(TestClassWithSubTypes<bool>(true));
}

TEST(YamlCppArchive, SerializeClassWithMemberInteger)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<int8_t, uint8_t, int64_t, uint64_t>>());
}

TEST(YamlCppArchive, SerializeClassWithMemberFloat)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<float>>());
}

TEST(YamlCppArchive, SerializeClassWithMemberDouble)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<double>>());
}

TEST(YamlCppArchive, SerializeClassWithMemberString)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTypes<std::string, std::wstring>>());
}

TEST(YamlCppArchive, SerializeClassHierarchy)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithInheritance>());
}

TEST(YamlCppArchive, SerializeClassWithMemberClass)
{
	using TestClassType = TestClassWithSubTypes<TestClassWithSubTypes<int64_t>>;
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassType>());
}

TEST(YamlCppArchive, SerializeClassWithSubArray)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubArray<int64_t>>());
}

TEST(YamlCppArchive, SerializeClassWithSubArrayOfClasses)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubArray<TestPointClass>>());
}

TEST(YamlCppArchive, SerializeClassWithSubTwoDimArray)
{
	TestSerializeClass<YamlArchive>(BuildFixture<TestClassWithSubTwoDimArray<int32_t>>());
}

TEST(YamlCppArchive, ShouldIterateKeysInObjectScope)
{
	TestIterateKeysInObjectScope<YamlArchive>();
}

//-----------------------------------------------------------------------------
// Test paths in archive
//-----------------------------------------------------------------------------
TEST(YamlCppArchive, ShouldReturnPathInObjectScopeWhenLoading)
{
	TestGetPathInJsonObjectScopeWhenLoading<YamlArchive>();
}

TEST(YamlCppArchive, ShouldReturnPathInObjectScopeWhenSaving)
{
	TestGetPathInJsonObjectScopeWhenSaving<YamlArchive>();
}

TEST(YamlCppArchive, ShouldReturnPathInArrayScopeWhenLoading)
{
	TestGetPathInJsonArrayScopeWhenLoading<YamlArchive>();
}

TEST(YamlCppArchive, ShouldReturnPathInArrayScopeWhenSaving)
{
	TestGetPathInJsonArrayScopeWhenSaving<YamlArchive>();
}

//-----------------------------------------------------------------------------
// Test the validation for named values (boolean result, which returns by archive's method SerializeValue()).
//-----------------------------------------------------------------------------
TEST(YamlCppArchive, ShouldCollectErrorAboutRequiredNamedValues)
{
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<bool>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<int>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<double>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<std::string>>();
	TestValidationForNamedValues<YamlArchive, TestClassForCheckValidation<TestPointClass>>();
}

//-----------------------------------------------------------------------------
// Tests streams / files
//-----------------------------------------------------------------------------
TEST(YamlCppArchive, SerializeClassToStream)
{
	TestSerializeClassToStream<YamlArchive, char>(BuildFixture<TestPointClass>());
}

TEST(YamlCppArchive, SerializeUnicodeToEncodedStream)
{
	TestClassWithSubType<std::wstring> TestValue(L"Привет мир!");
	TestSerializeClassToStream<YamlArchive, char>(TestValue);
}

TEST(YamlCppArchive, LoadFromUtf8Stream) {
	TestLoadJsonFromEncodedStream<YamlArchive, BitSerializer::Convert::Utf8>(false);
}
TEST(YamlCppArchive, LoadFromUtf8StreamWithBom) {
	TestLoadJsonFromEncodedStream<YamlArchive, BitSerializer::Convert::Utf8>(true);
}

TEST(YamlCppArchive, SaveToUtf8StreamWithBom)
{
	// Arrange
	std::stringstream outputStream;
	TestClassWithSubType<std::string> testObj("Hello world!");

	// Act
	BitSerializer::SaveObject<YamlArchive>(testObj, outputStream);

	// Assert
	const std::string expected = std::string({ char(0xEF), char(0xBB), char(0xBF) }) + "TestValue: Hello world!";
	EXPECT_EQ(expected, outputStream.str());
}

TEST(YamlCppArchive, SaveToUtf8StreamWithoutBom)
{
	// Arrange
	std::stringstream outputStream;
	TestClassWithSubType<std::string> testObj("Hello world!");
	BitSerializer::SerializationOptions serializationOptions;
	serializationOptions.streamOptions.writeBom = false;

	// Act
	BitSerializer::SaveObject<YamlArchive>(testObj, outputStream, serializationOptions);

	// Assert
	EXPECT_EQ("TestValue: Hello world!", outputStream.str());
}

TEST(YamlCppArchive, SerializeClassToFile)
{
	TestSerializeClassToFile<YamlArchive>(BuildFixture<TestPointClass>());
}

//-----------------------------------------------------------------------------
// Tests of errors handling
//-----------------------------------------------------------------------------
TEST(YamlCppArchive, ThrowExceptionWhenBadSyntaxInSource)
{
	int testArr[3];
	EXPECT_THROW(BitSerializer::LoadObject<YamlArchive>(testArr, "[1, 2, 3"), BitSerializer::SerializationException);
}

