/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/auto_fixture.h"
#include "testing_tools/string_utils.h"
#include "bitserializer/json_archive.h"
#include "json_reader_fixture.h"
#include "testing_tools/gtest_asserts.h"

using JsonReadersTypes = ::testing::Types<
	BitSerializer::Json::Detail::CJsonStringReader
	, BitSerializer::Json::Detail::CJsonStreamReader
>;

// Tests for all implementations of IJsonReader
TYPED_TEST_SUITE(JsonReaderTest, JsonReadersTypes, );

#pragma warning(push)
#pragma warning(disable: 4566)

//-----------------------------------------------------------------------------
// Tests of reading `null`
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, ReadNull)
{
	std::nullptr_t value{};
	this->PrepareReader("\n\t null");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
}

TYPED_TEST(JsonReaderTest, ReadNullShouldThrowExceptionWhenEmptyJson)
{
	std::nullptr_t value{};
	this->PrepareReader("");

	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(value);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
}

TYPED_TEST(JsonReaderTest, ReadNullShouldThrowExceptionWhenValueInUpperCase)
{
	for (const auto* testJson : { "NULL", "Null" })
	{
		this->PrepareReader(testJson);
		std::nullptr_t value{};
		BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(1, ex.Line);
		EXPECT_EQ(0, ex.Offset);
		EXPECT_STREQ("Parsing error: Invalid sequence", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadNullShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "true", "123", "[ ]", "{ }" })
	{
		this->PrepareReader(testJson);
		std::nullptr_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadNullShouldSkipValueWhenMismatchedTypes)
{
	std::nullptr_t value{};
	this->PrepareReader("true", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	EXPECT_FALSE(this->mJsonReader->ReadValue(value));
	EXPECT_TRUE(this->mJsonReader->IsEnd()) << "Null should be skipped";
}

//-----------------------------------------------------------------------------
// Tests of reading boolean values
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, ReadBoolean)
{
	bool value = false;

	this->PrepareReader("\n\t true");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(true, value);

	this->PrepareReader("\n\t false");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(false, value);
}

TYPED_TEST(JsonReaderTest, ReadBooleanShouldThrowExceptionWhenEmptyJson)
{
	bool value;
	this->PrepareReader("");

	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(value);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
}

TYPED_TEST(JsonReaderTest, ReadBooleanShouldThrowExceptionWhenValueInUpperCase)
{
	for (const auto *testJson : { "TRUE", "FALSE", "True", "False" })
	{
		this->PrepareReader(testJson);
		bool value{};
		BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(1, ex.Line);
		EXPECT_EQ(0, ex.Offset);
		EXPECT_STREQ("Parsing error: Invalid sequence", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadBooleanShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "\"true\"", "3.14", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		bool value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadBooleanShouldSkipValueWhenMismatchedTypes)
{
	bool value = false;
	this->PrepareReader("[true]", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	EXPECT_FALSE(this->mJsonReader->ReadValue(value));
	EXPECT_FALSE(value);
	EXPECT_TRUE(this->mJsonReader->IsEnd()) << "Value should be skipped";
}

//-----------------------------------------------------------------------------
// Tests of reading integral values
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, ReadInt8)
{
	int8_t value = -1;

	this->PrepareReader("  -128, ");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int8_t>::min(), value);

	this->PrepareReader("\n\r\t127");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int8_t>::max(), value);
}

TYPED_TEST(JsonReaderTest, ReadInt8ShouldThrowExceptionWhenOverflow)
{
	for (const auto *testJson : { "-129", "128" })
	{
		this->PrepareReader(testJson);
		int8_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		EXPECT_STREQ("Overflow: The target field range is insufficient for the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadInt8ShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "3.14", "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		int8_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, ReadUInt8)
{
	uint8_t value = 1;

	this->PrepareReader(" 0, ");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint8_t>::min(), value);

	this->PrepareReader("\n\t 255 ");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint8_t>::max(), value);
}

TYPED_TEST(JsonReaderTest, ReadUInt8ShouldThrowExceptionWhenOverflow)
{
	uint8_t value{};
	this->PrepareReader("256");
	BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
		this->mJsonReader->ReadValue(value);
	});
	EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
	EXPECT_STREQ("Overflow: The target field range is insufficient for the value being loaded", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadUInt8ShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "-1", "3.14", "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		uint8_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, ReadInt16)
{
	int16_t value = -1;

	this->PrepareReader("\n\t -32768");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int16_t>::min(), value);

	this->PrepareReader("\n\t 32767");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int16_t>::max(), value);
}

TYPED_TEST(JsonReaderTest, ReadInt16ShouldThrowExceptionWhenOverflow)
{
	for (const auto *testJson : { "-32769", "32768" })
	{
		this->PrepareReader(testJson);
		int16_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		EXPECT_STREQ("Overflow: The target field range is insufficient for the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadInt16ShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "3.14", "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		int16_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, ReadUInt16)
{
	uint16_t value = 1;
	this->PrepareReader("\n\t 65535");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint16_t>::max(), value);
}

TYPED_TEST(JsonReaderTest, ReadUInt16ShouldThrowExceptionWhenOverflow)
{
	uint16_t value{};
	this->PrepareReader("65536");
	BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
		this->mJsonReader->ReadValue(value);
	});
	EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
	EXPECT_STREQ("Overflow: The target field range is insufficient for the value being loaded", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadUInt16ShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "-1", "3.14", "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		uint16_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, ReadInt32)
{
	int32_t value = -1;

	this->PrepareReader("\n\t -2147483648");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int32_t>::min(), value);

	this->PrepareReader("\n\t 2147483647");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int32_t>::max(), value);
}

TYPED_TEST(JsonReaderTest, ReadInt32ShouldThrowExceptionWhenOverflow)
{
	for (const auto *testJson : { "-2147483649", "2147483648" })
	{
		this->PrepareReader(testJson);
		int32_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		EXPECT_STREQ("Overflow: The target field range is insufficient for the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadInt32ShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "3.14", "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		int32_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, ReadUInt32)
{
	uint32_t value = 1;

	this->PrepareReader(" \n\t 4294967295");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint32_t>::max(), value);
}

TYPED_TEST(JsonReaderTest, ReadUInt32ShouldThrowExceptionWhenOverflow)
{
	uint32_t value{};
	this->PrepareReader("4294967296");
	BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
		this->mJsonReader->ReadValue(value);
	});
	EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
	EXPECT_STREQ("Overflow: The target field range is insufficient for the value being loaded", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadUInt32ShouldThrowExceptionWhenMismatchedType)
{
	for (const auto *testJson : { "-1", "3.14", "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		uint32_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, ReadInt64)
{
	int64_t value = -1;

	this->PrepareReader("\n\t -9223372036854775808");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int64_t>::min(), value);

	this->PrepareReader("\n\t 9223372036854775807");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<int64_t>::max(), value);
}

TYPED_TEST(JsonReaderTest, ReadInt64ShouldThrowExceptionWhenOverflow)
{
	int64_t value{};
	this->PrepareReader("9223372036854775808");
	BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
		this->mJsonReader->ReadValue(value);
	});
	EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
	EXPECT_STREQ("Overflow: The target field range is insufficient for the value being loaded", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadInt64ShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "3.14", "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		int64_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadInt64ShouldSkipValueWhenMismatchedTypes)
{
	this->PrepareReader("3.14", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	int64_t int64 = 0x1020;
	EXPECT_FALSE(this->mJsonReader->ReadValue(int64));
	EXPECT_EQ(0x1020, int64);
	EXPECT_TRUE(this->mJsonReader->IsEnd()) << "Value should be skipped";
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, ReadUInt64)
{
	uint64_t value = 1;

	this->PrepareReader(" \n\t 18446744073709551615");
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ(std::numeric_limits<uint64_t>::max(), value);
}

TYPED_TEST(JsonReaderTest, ReadUInt64ShouldThrowExceptionWhenOverflow)
{
	uint64_t value{};
	this->PrepareReader("18446744073709551616");
	BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
		this->mJsonReader->ReadValue(value);
	});
	EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
	EXPECT_STREQ("Overflow: The target field range is insufficient for the value being loaded", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadUInt64ShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "-1", "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		uint64_t value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadUInt64ShouldSkipValueWhenMismatchedTypes)
{
	uint64_t uInt64 = 123;
	this->PrepareReader("3.14", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	EXPECT_FALSE(this->mJsonReader->ReadValue(uInt64));
	EXPECT_EQ(123, uInt64);
	EXPECT_TRUE(this->mJsonReader->IsEnd()) << "Value should be skipped";
}

TYPED_TEST(JsonReaderTest, ReadUInt64ShouldThrowExceptionWhenEmptyJson)
{
	uint64_t uInt64;
	this->PrepareReader("");

	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(uInt64);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
}

//-----------------------------------------------------------------------------
// Tests of reading floating types
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, ReadFloat)
{
	this->PrepareReader("\n\t 3.14");
	float value{};

	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_FLOAT_EQ(3.14f, value);
}

TYPED_TEST(JsonReaderTest, ReadFloatShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		float value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadFloatShouldSkipValueWhenMismatchedTypes)
{
	float value;
	this->PrepareReader("true", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	EXPECT_FALSE(this->mJsonReader->ReadValue(value));
	EXPECT_TRUE(this->mJsonReader->IsEnd()) << "Value should be skipped";
}

TYPED_TEST(JsonReaderTest, ReadFloatShouldThrowExceptionWhenEmptyJson)
{
	float value;
	this->PrepareReader("");

	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(value);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, ReadDouble)
{
	this->PrepareReader("\n\t 3.141592654");
	double value{};

	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_DOUBLE_EQ(3.141592654, value);
}

TYPED_TEST(JsonReaderTest, ReadDoubleShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "\"123\"", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		double value{};
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(value);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadDoubleShouldSkipValueWhenMismatchedTypes)
{
	double value;
	this->PrepareReader("true", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	EXPECT_FALSE(this->mJsonReader->ReadValue(value));
	EXPECT_TRUE(this->mJsonReader->IsEnd()) << "Value should be skipped";
}

TYPED_TEST(JsonReaderTest, ReadDoubleShouldThrowExceptionWhenEmptyJson)
{
	double value;
	this->PrepareReader("");

	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(value);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
}

//-----------------------------------------------------------------------------
// Tests of reading strings
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, ReadString)
{
	std::string_view actualStr;

	// Empty string
	this->PrepareReader(R"("")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ("", actualStr);

	// ASCII string with spaces
	this->PrepareReader(R"(	 "  Hello world!  "		)");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ("  Hello world!  ", actualStr);

	// String with digits and symbols
	this->PrepareReader(R"("123!@#$%^&*()_+-=[]{}|;:,.<>?")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ("123!@#$%^&*()_+-=[]{}|;:,.<>?", actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringWithEscapedQuotes)
{
	std::string_view actualStr;

	this->PrepareReader(R"("He said \"Hello\"")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(R"(He said "Hello")", actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringWithEscapeSequences)
{
	std::string_view actualStr;

	// Backslashes
	this->PrepareReader(R"("C:\\Windows\\System32")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(R"(C:\Windows\System32)", actualStr);

	// Common escape sequences
	this->PrepareReader(R"("Line1\nLine2\tTabbed\rEnd")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ("Line1\nLine2\tTabbed\rEnd", actualStr);

	// Forward slashes (optional in JSON)
	this->PrepareReader(R"("https:\/\/example.com")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ("https://example.com", actualStr);

	// Mixed escapes
	this->PrepareReader(R"("\"\\\/\b\f\n\r\t")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ("\"\\/\b\f\n\r\t", actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringWithUnicodeEscape)
{
	std::string_view actualStr;

	// ASCII
	this->PrepareReader(R"("\u0041")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ("A", actualStr);

	// non-ASCII
	this->PrepareReader(R"("\u00E9")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(UTF8("é"), actualStr);

	// Multiple unicode escapes
	this->PrepareReader(R"("\u03B1\u03B2\u03B3")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(UTF8("αβγ"), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenUnexpectedEndInEscapeSequence)
{
	this->PrepareReader(R"("\)");
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(2, ex.Offset);
	EXPECT_STREQ("Parsing error: Unexpected end of input in string escape sequence", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenInvalidEscapeSequence)
{
	this->PrepareReader(R"("\w)");
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(2, ex.Offset);
	EXPECT_STREQ("Parsing error: Invalid escape sequence", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenInvalidUnicodePrefix)
{
	this->PrepareReader(R"("\x00E9")");
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(2, ex.Offset);
	EXPECT_STREQ("Parsing error: Invalid escape sequence", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenIncompleteUnicodeSequence)
{
	this->PrepareReader(R"("\u00E")");
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(6, ex.Offset);
	EXPECT_STREQ("Parsing error: Invalid hex digit in \\u escape", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenUnexpectedEndInUnicodeSequence)
{
	this->PrepareReader(R"("\u00E)");
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(2, ex.Offset);
	EXPECT_STREQ("Parsing error: Unexpected end of input in string escape sequence", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenInvalidInvalidHexDigit)
{
	this->PrepareReader(R"("\u00EG")");
	std::string_view actualStr;
	BitSerializer::ParsingException ex2 = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_EQ(1, ex2.Line);
	EXPECT_EQ(6, ex2.Offset);
	EXPECT_STREQ("Parsing error: Invalid hex digit in \\u escape", ex2.what());
}

TYPED_TEST(JsonReaderTest, ReadStringWithSurrogatePairs)
{
	std::string_view actualStr;
	this->PrepareReader(R"("\uD83D\uDE00test\uD83D\uDE00")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(UTF8("😀test😀"), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringWithMultipleSurrogatePairs)
{
	std::string_view actualStr;
	this->PrepareReader(R"("\uD83D\uDE00\uD83D\uDE01\uD83D\uDE02")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(UTF8("😀😁😂"), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringWithSurrogatePairAndEscapes)
{
	std::string_view actualStr;
	this->PrepareReader(R"("hello\n\uD83D\uDE00\tworld")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(UTF8("hello\n😀\tworld"), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringWithHighBmpUnicodeEscape)
{
	std::string_view actualStr;
	this->PrepareReader(R"("\uFFFD")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(UTF8("\uFFFD"), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringWithBomUnicodeEscape)
{
	std::string_view actualStr;
	this->PrepareReader(R"("\uFEFF")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(UTF8("\uFEFF"), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringWithNullUnicodeEscape)
{
	std::string_view actualStr;
	this->PrepareReader(R"("\u0000")");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(std::string_view("\0", 1), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenLoneLowSurrogate)
{
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->PrepareReader(R"("\uDC00test")");
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_STREQ("Parsing error: Invalid surrogate pair", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenHighSurrogateFollowedByHigh)
{
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->PrepareReader(R"("\uD800\uD800")");
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_STREQ("Parsing error: Invalid low surrogate", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenHighSurrogateNotFollowedByUnicode)
{
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->PrepareReader(R"("\uD800test")");
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_STREQ("Parsing error: Incomplete surrogate pair", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenHighSurrogateAtEndOfString)
{
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->PrepareReader(R"("\uD800")");
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_STREQ("Parsing error: Incomplete surrogate pair", ex.what());
}

//-----------------------------------------------------------------------------
// Tests of reading long strings (larger than the internal read chunk)
//-----------------------------------------------------------------------------
constexpr size_t chunkSize = BitSerializer::Convert::Utf::EncodedStreamReader<char>::DefaultChunkSize;

TYPED_TEST(JsonReaderTest, ReadLongString)
{
	const std::string expectedValue = this->GenTestString(chunkSize * 5);
	std::string_view actualStr;
	this->PrepareReader("\"" + expectedValue + "\"");
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(expectedValue, actualStr);
}

TYPED_TEST(JsonReaderTest, ReadLongStringWithEscapeAtChunkBoundary)
{
	// Place an escape sequence exactly on the boundary of the internal read chunk
	const std::string prefix = this->GenTestString(chunkSize - 4);
	std::string testJson = "\"" + prefix + "\\u0041" + this->GenTestString(chunkSize) + "\"";
	std::string_view actualStr;
	this->PrepareReader(testJson);
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(prefix + "A" + this->GenTestString(chunkSize), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadLongStringWithSurrogatePairAtChunkBoundary)
{
	const std::string prefix = this->GenTestString(chunkSize - 4);
	std::string testJson = "\"" + prefix + "\\uD83D\\uDE00" + this->GenTestString(chunkSize) + "\"";
	std::string_view actualStr;
	this->PrepareReader(testJson);
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(prefix + UTF8("😀") + this->GenTestString(chunkSize), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadLongStringWithEscapedQuoteAtChunkBoundary)
{
	// The escaped quote starts at the very last byte of the internal read chunk
	const std::string prefix = this->GenTestString(chunkSize - 2);
	std::string testJson = "\"" + prefix + "\\\"" + this->GenTestString(chunkSize) + "\"";
	std::string_view actualStr;
	this->PrepareReader(testJson);
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(prefix + "\"" + this->GenTestString(chunkSize), actualStr);
}

TYPED_TEST(JsonReaderTest, ReadLongStringWithMultipleEscapes)
{
	std::string testJson = "\"";
	std::string expected;
	for (int i = 0; i < 10; ++i)
	{
		const std::string part = this->GenTestString(300);
		testJson += part + "\\n\\u0041";
		expected += part + "\n" + "A";
	}
	testJson += "\"";
	std::string_view actualStr;
	this->PrepareReader(testJson);
	EXPECT_TRUE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_EQ(expected, actualStr);
}

TYPED_TEST(JsonReaderTest, ReadLongStringShouldThrowExceptionWhenNoCloseQuotes)
{
	const std::string testJson = "\"" + this->GenTestString(chunkSize * 5);
	this->PrepareReader(testJson);
	std::string_view actualStr;
	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_STREQ("Parsing error: Unterminated string literal", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenEmptyJson)
{
	std::string_view actualStr;
	this->PrepareReader("");

	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>([&] {
		this->mJsonReader->ReadValue(actualStr);
	});
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
	EXPECT_STREQ("Parsing error: No more values to read", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadStringShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "123", "false", "{ }", "[ 123 ]" })
	{
		this->PrepareReader(testJson);
		std::string_view actualStr;
		BitSerializer::SerializationException ex = GTestExpectException<BitSerializer::SerializationException>([&] {
			this->mJsonReader->ReadValue(actualStr);
		});
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, ReadStringShouldSkipValueWhenMismatchedTypes)
{
	this->PrepareReader("true", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	std::string_view actualStr;
	EXPECT_FALSE(this->mJsonReader->ReadValue(actualStr));
	EXPECT_TRUE(this->mJsonReader->IsEnd()) << "Value should be skipped";
}

//-----------------------------------------------------------------------------
// Tests of reading arrays
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, ReadArrayWithEmptySize)
{
	this->PrepareReader("[]");
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	EXPECT_TRUE(this->mJsonReader->IsArrayEnd());
	this->mJsonReader->CloseArray(true);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, ReadArrayWithPrimitives)
{
	std::string testJson = R"([true, false, 123456789, 3.14, "text", null])";
	this->PrepareReader(testJson);
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	ASSERT_FALSE(this->mJsonReader->IsArrayEnd());
	ASSERT_FALSE(this->mJsonReader->IsEnd());

	// Read boolean
	bool boolean = false;
	ASSERT_TRUE(this->mJsonReader->ReadValue(boolean));
	EXPECT_TRUE(boolean);
	this->mJsonReader->ReadValueSeparator();
	ASSERT_TRUE(this->mJsonReader->ReadValue(boolean));
	EXPECT_FALSE(boolean);
	ASSERT_FALSE(this->mJsonReader->IsArrayEnd());
	this->mJsonReader->ReadValueSeparator();

	// Read integer
	int64_t integer = 0;
	ASSERT_TRUE(this->mJsonReader->ReadValue(integer));
	EXPECT_EQ(123456789, integer);
	ASSERT_FALSE(this->mJsonReader->IsArrayEnd());
	this->mJsonReader->ReadValueSeparator();

	// Read floating
	float number = 0.f;
	ASSERT_TRUE(this->mJsonReader->ReadValue(number));
	EXPECT_EQ(3.14f, number);
	ASSERT_FALSE(this->mJsonReader->IsArrayEnd());
	this->mJsonReader->ReadValueSeparator();

	// Read string
	std::string_view str;
	ASSERT_TRUE(this->mJsonReader->ReadValue(str));
	EXPECT_EQ("text", str);
	ASSERT_FALSE(this->mJsonReader->IsArrayEnd());
	this->mJsonReader->ReadValueSeparator();

	// Read null value
	std::nullptr_t nullValue;
	ASSERT_TRUE(this->mJsonReader->ReadValue(nullValue));

	// Check for correct handling of the end of array
	ASSERT_TRUE(this->mJsonReader->IsArrayEnd());
	this->mJsonReader->CloseArray(true);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, ReadArrayOfArrays)
{
	std::string testJson = R"([
	[1, "test"],
	[true]
])";
	this->PrepareReader(testJson);
	ASSERT_TRUE(this->mJsonReader->OpenArray());

	// Read 1st element - child array
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	int64_t integer = 0;
	ASSERT_TRUE(this->mJsonReader->ReadValue(integer));
	this->mJsonReader->ReadValueSeparator();
	EXPECT_EQ(1, integer);

	std::string_view str;
	ASSERT_TRUE(this->mJsonReader->ReadValue(str));
	EXPECT_EQ("test", str);
	this->mJsonReader->CloseArray(true);

	// Read 2nd element - child array
	this->mJsonReader->ReadValueSeparator();
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	bool boolean = false;
	ASSERT_TRUE(this->mJsonReader->ReadValue(boolean));
	EXPECT_TRUE(boolean);
	this->mJsonReader->CloseArray(true);

	// Check for correct handling of the end of array
	this->mJsonReader->CloseArray(true);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
	EXPECT_EQ(4, this->mJsonReader->GetLineNumber());
}

TYPED_TEST(JsonReaderTest, ReadArrayOfObjects)
{
	std::string testJson = R"([
	{},
	{
		"key1": 1,
		"key2": "test"
	}
])";
	this->PrepareReader(testJson);
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	ASSERT_FALSE(this->mJsonReader->IsArrayEnd());

	// Read 1st element - object (empty)
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	ASSERT_TRUE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->CloseObject(true);
	this->mJsonReader->ReadValueSeparator();

	// Read 2nd element - object with two key/value pairs
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	std::string_view key;
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key1", key);
	int integer = 0;
	ASSERT_TRUE(this->mJsonReader->ReadValue(integer));
	EXPECT_EQ(1, integer);

	this->mJsonReader->ReadValueSeparator();
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key2", key);
	std::string_view value;
	ASSERT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_EQ("test", value);
	this->mJsonReader->CloseObject(true);

	// Check for correct handling of the end of array
	ASSERT_TRUE(this->mJsonReader->IsArrayEnd());
	this->mJsonReader->CloseArray(false);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, OpenArrayShouldThrowExceptionWhenEmptyJson)
{
	this->PrepareReader("");
	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::OpenArray, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
	EXPECT_STREQ("Parsing error: No more values to read", ex.what());
}

TYPED_TEST(JsonReaderTest, OpenArrayShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "true", "3.14", "{ }" })
	{
		this->PrepareReader(testJson);
		BitSerializer::SerializationException ex1 = GTestExpectException<BitSerializer::SerializationException>(&TypeParam::OpenArray, this->mJsonReader);
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex1.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex1.what());
	}
}

TYPED_TEST(JsonReaderTest, OpenArrayShouldSkipValueWhenMismatchedTypes)
{
	this->PrepareReader(R"({"key": "value"})", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	EXPECT_FALSE(this->mJsonReader->OpenArray());
	EXPECT_TRUE(this->mJsonReader->IsEnd()) << "Value should be skipped";
}

TYPED_TEST(JsonReaderTest, CloseArrayShouldSkipRemainingElements)
{
	std::string testJson = R"([true, 123456789, 3.14, "text", null])";
	this->PrepareReader(testJson);
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	this->mJsonReader->CloseArray(false);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, CloseArrayShouldThrowExceptionWhenMissingCloseBracket)
{
	this->PrepareReader("[");
	ASSERT_TRUE(this->mJsonReader->OpenArray());

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::CloseArray, this->mJsonReader, false);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(1, ex.Offset);
	EXPECT_STREQ("Parsing error: Missing closing bracket ']' at end of source JSON", ex.what());
}

TYPED_TEST(JsonReaderTest, CloseArrayShouldThrowExceptionWhenRedundantCommaBeforeBracket)
{
	this->PrepareReader("[,]");
	ASSERT_TRUE(this->mJsonReader->OpenArray());

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::CloseArray, this->mJsonReader, false);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(1, ex.Offset);
	EXPECT_STREQ("Parsing error: Unexpected character while skipping value", ex.what());
}

TYPED_TEST(JsonReaderTest, CloseArrayShouldThrowExceptionWhenMissingCommaBetweenElements)
{
	this->PrepareReader(R"([
	true
	false
])");
	ASSERT_TRUE(this->mJsonReader->OpenArray());

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::CloseArray, this->mJsonReader, false);
	EXPECT_EQ(3, ex.Line);
	EXPECT_EQ(9, ex.Offset);
	EXPECT_STREQ("Parsing error: Missing a comma between elements", ex.what());
}

TYPED_TEST(JsonReaderTest, IsArrayEndShouldThrowExceptionWhenMissingCloseBracket)
{
	this->PrepareReader("[");
	ASSERT_TRUE(this->mJsonReader->OpenArray());

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::IsArrayEnd, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(1, ex.Offset);
	EXPECT_STREQ("Parsing error: Missing closing bracket ']' at end of array JSON", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadValueSeparatorShouldThrowExceptionWhenMissingComma)
{
	this->PrepareReader("[true false]");
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	bool value;
	ASSERT_TRUE(this->mJsonReader->ReadValue(value));

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::ReadValueSeparator, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(6, ex.Offset);
	EXPECT_STREQ("Parsing error: Missing a comma between elements", ex.what());
}

//-----------------------------------------------------------------------------
// Tests of reading objects
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, ReadObjectWithEmptySize)
{
	this->PrepareReader("{}");
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	EXPECT_TRUE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->CloseObject(true);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, ReadObjectWithPrimitives)
{
	std::string testJson = R"({
	"key1": true,
	"key2": -123456789,
	"key3": 3.14,
	"key4": "text",
	"key5": null
})";
	this->PrepareReader(testJson);
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());
	ASSERT_FALSE(this->mJsonReader->IsEnd());

	// Read boolean
	std::string_view key;
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key1", key);
	bool boolean = false;
	ASSERT_TRUE(this->mJsonReader->ReadValue(boolean));
	EXPECT_TRUE(boolean);
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->ReadValueSeparator();

	// Read integer
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key2", key);
	int64_t integer;
	ASSERT_TRUE(this->mJsonReader->ReadValue(integer));
	EXPECT_EQ(-123456789, integer);
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->ReadValueSeparator();

	// Read floating
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key3", key);
	float number;
	ASSERT_TRUE(this->mJsonReader->ReadValue(number));
	EXPECT_EQ(3.14f, number);
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->ReadValueSeparator();

	// Read string
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key4", key);
	std::string_view str;
	ASSERT_TRUE(this->mJsonReader->ReadValue(str));
	EXPECT_EQ("text", str);
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->ReadValueSeparator();

	// Read null value
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key5", key);
	std::nullptr_t nullValue;
	ASSERT_TRUE(this->mJsonReader->ReadValue(nullValue));

	// Check for correct handling of the end of object
	ASSERT_TRUE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->CloseObject(true);
	EXPECT_EQ(7, this->mJsonReader->GetLineNumber());
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, ReadObjectWithChildArray)
{
	std::string testJson = R"({
	"key1": true,
	"key2": [1, 2]
})";
	this->PrepareReader(testJson);
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());
	ASSERT_FALSE(this->mJsonReader->IsEnd());

	// Read boolean
	std::string_view key;
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key1", key);
	bool boolean = false;
	ASSERT_TRUE(this->mJsonReader->ReadValue(boolean));
	EXPECT_TRUE(boolean);
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());

	// Read child array
	this->mJsonReader->ReadValueSeparator();
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key2", key);
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	int integer;
	ASSERT_TRUE(this->mJsonReader->ReadValue(integer));
	EXPECT_EQ(1, integer);
	this->mJsonReader->ReadValueSeparator();
	ASSERT_TRUE(this->mJsonReader->ReadValue(integer));
	EXPECT_EQ(2, integer);
	this->mJsonReader->CloseArray(false);

	// Check for correct handling of the end of object
	ASSERT_TRUE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->CloseObject(true);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, ReadObjectWithChildObject)
{
	std::string testJson = R"({
	"key1": {
		"k1": true,
		"k2": -1
	},
	"key2": "test"
})";
	this->PrepareReader(testJson);
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());

	// Read child object
	std::string_view key;
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key1", key);
	ASSERT_TRUE(this->mJsonReader->OpenObject());

	// Read boolean
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("k1", key);
	bool boolean;
	ASSERT_TRUE(this->mJsonReader->ReadValue(boolean));
	EXPECT_TRUE(boolean);
	this->mJsonReader->ReadValueSeparator();

	// Read integer
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("k2", key);
	int integer = 0;
	ASSERT_TRUE(this->mJsonReader->ReadValue(integer));
	EXPECT_EQ(-1, integer);

	// Check for correct handling of the end of child object
	ASSERT_TRUE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->CloseObject(true);

	// Read string from root object
	ASSERT_FALSE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->ReadValueSeparator();
	this->mJsonReader->ReadKey(key);
	EXPECT_EQ("key2", key);
	std::string_view str;
	ASSERT_TRUE(this->mJsonReader->ReadValue(str));
	EXPECT_EQ("test", str);

	// Check for correct handling of the end of object
	ASSERT_TRUE(this->mJsonReader->IsObjectEnd());
	this->mJsonReader->CloseObject(true);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, OpenObjectShouldThrowExceptionEmptyJson)
{
	this->PrepareReader("");

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::OpenObject, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
	EXPECT_STREQ("Parsing error: No more values to read", ex.what());
}

TYPED_TEST(JsonReaderTest, OpenObjectShouldThrowExceptionWhenMismatchedTypes)
{
	for (const auto *testJson : { "true", "3.14", "[ ]" })
	{
		this->PrepareReader(testJson);
		BitSerializer::SerializationException ex1 = GTestExpectException<BitSerializer::SerializationException>(&TypeParam::OpenObject, this->mJsonReader);
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex1.GetErrorCode());
		EXPECT_STREQ("Mismatched types: The type of target field does not match the value being loaded", ex1.what());
	}
}

TYPED_TEST(JsonReaderTest, OpenObjectShouldSkipValueWhenMismatchedTypes)
{
	this->PrepareReader(R"([1,2,3])", BitSerializer::OverflowNumberPolicy::ThrowError, BitSerializer::MismatchedTypesPolicy::Skip);
	EXPECT_FALSE(this->mJsonReader->OpenObject());
	EXPECT_TRUE(this->mJsonReader->IsEnd()) << "Value should be skipped";
}

TYPED_TEST(JsonReaderTest, OpenObjectKeyShouldThrowExceptionWhenInvalidKey)
{
	this->PrepareReader(R"({ key: "value"})");
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	std::string_view key;

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::ReadKey, this->mJsonReader, key);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(2, ex.Offset);
	EXPECT_STREQ("Parsing error: Expected string for JSON object key", ex.what());
}

TYPED_TEST(JsonReaderTest, OpenObjectKeyShouldThrowExceptionWhenMissingColon)
{
	this->PrepareReader(R"({ "key" "value"})");
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	std::string_view key;

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::ReadKey, this->mJsonReader, key);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(8, ex.Offset);
	EXPECT_STREQ("Parsing error: Missing a colon between key and value", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadObjectKeyShouldThrowExceptionWhenIsNotString)
{
	this->PrepareReader(R"({ 1: "value"})");
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	std::string_view key;

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::ReadKey, this->mJsonReader, key);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(2, ex.Offset);
	EXPECT_STREQ("Parsing error: Expected string for JSON object key", ex.what());
}

TYPED_TEST(JsonReaderTest, CloseObjectShouldSkipRemainingElements)
{
	std::string testJson = R"({
	"key1": true,
	"key2": -123456789,
	"key3": 3.14,
	"key4": "text",
	"key5": null
})";
	this->PrepareReader(testJson);
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	this->mJsonReader->CloseObject(false);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, CloseObjectShouldThrowExceptionWhenMissingCloseBracket)
{
	this->PrepareReader("{");
	ASSERT_TRUE(this->mJsonReader->OpenObject());

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::CloseObject, this->mJsonReader, false);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(1, ex.Offset);
	EXPECT_STREQ("Parsing error: Missing closing bracket '}' at end of source JSON", ex.what());
}

TYPED_TEST(JsonReaderTest, CloseObjectShouldThrowExceptionWhenRedundantCommaBeforeBracket)
{
	this->PrepareReader("{,}");
	ASSERT_TRUE(this->mJsonReader->OpenObject());

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::CloseObject, this->mJsonReader, false);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(1, ex.Offset);
	EXPECT_STREQ("Parsing error: Unexpected character while skipping value", ex.what());
}

TYPED_TEST(JsonReaderTest, CloseObjectShouldThrowExceptionWhenMissingCommaBetweenKeyValuePairs)
{
	this->PrepareReader(R"({
	"key1": true
	"key2": -123456789
})");
	ASSERT_TRUE(this->mJsonReader->OpenObject());

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::CloseObject, this->mJsonReader, false);
	EXPECT_EQ(3, ex.Line);
	EXPECT_EQ(17, ex.Offset);
	EXPECT_STREQ("Parsing error: Missing a comma between elements", ex.what());
}

TYPED_TEST(JsonReaderTest, CloseObjectShouldThrowExceptionWhenMissingColonBetweenKeyValue)
{
	this->PrepareReader(R"({
	"key1" true
})");
	ASSERT_TRUE(this->mJsonReader->OpenObject());

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::CloseObject, this->mJsonReader, false);
	EXPECT_EQ(2, ex.Line);
	EXPECT_EQ(10, ex.Offset);
	EXPECT_STREQ("Parsing error: Missing a colon between key and value", ex.what());
}

TYPED_TEST(JsonReaderTest, IsObjectEndShouldThrowExceptionWhenMissingCloseBracket)
{
	this->PrepareReader("{");
	ASSERT_TRUE(this->mJsonReader->OpenObject());

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::IsObjectEnd, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(1, ex.Offset);
	EXPECT_STREQ("Parsing error: Missing closing bracket '}' at end of source JSON", ex.what());
}

//-----------------------------------------------------------------------------
// Tests of get/set positions
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, GetPosition)
{
	std::string testJson = R"([true, 1234])";
	this->PrepareReader(testJson);

	EXPECT_EQ(0U, this->mJsonReader->GetPosition());
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	EXPECT_EQ(1U, this->mJsonReader->GetPosition());

	bool boolean = false;
	EXPECT_TRUE(this->mJsonReader->ReadValue(boolean));
	EXPECT_EQ(5U, this->mJsonReader->GetPosition());

	this->mJsonReader->ReadValueSeparator();
	EXPECT_EQ(6U, this->mJsonReader->GetPosition());
	int integer;
	EXPECT_TRUE(this->mJsonReader->ReadValue(integer));
	EXPECT_EQ(11U, this->mJsonReader->GetPosition());

	this->mJsonReader->CloseArray(true);
	EXPECT_EQ(12U, this->mJsonReader->GetPosition());
}

TYPED_TEST(JsonReaderTest, SetPosition)
{
	std::string testJson = R"({	"key1": true })";
	this->PrepareReader(testJson);

	bool value = false;
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	const size_t startObjectPos = this->mJsonReader->GetPosition();

	std::string_view key;
	this->mJsonReader->ReadKey(key);
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));

	// Rewind to start of the object and read key/value again
	this->mJsonReader->SetPosition(startObjectPos);
	this->mJsonReader->ReadKey(key);
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
}

TYPED_TEST(JsonReaderTest, SetPositionShouldThrowExceptionWhenInvalidPos)
{
	const std::string testJson = "true";
	this->PrepareReader(testJson);

	const std::invalid_argument ex = GTestExpectException<std::invalid_argument>(&TypeParam::SetPosition, this->mJsonReader, testJson.size() + 1);
	EXPECT_STREQ("Internal error: position is out of range of input data", ex.what());
}

TYPED_TEST(JsonReaderTest, IsEnd)
{
	this->PrepareReader("[true,false]");
	EXPECT_FALSE(this->mJsonReader->IsEnd());

	ASSERT_TRUE(this->mJsonReader->OpenArray());
	EXPECT_FALSE(this->mJsonReader->IsEnd());

	bool value = false;
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_FALSE(this->mJsonReader->IsEnd());

	this->mJsonReader->ReadValueSeparator();
	EXPECT_TRUE(this->mJsonReader->ReadValue(value));
	EXPECT_FALSE(this->mJsonReader->IsEnd());

	this->mJsonReader->CloseArray(true);
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

//-----------------------------------------------------------------------------
// Tests for reading value types
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, ReadTypeOfNull)
{
	this->PrepareReader("null");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::Null, this->mJsonReader->ReadValueType());
}

TYPED_TEST(JsonReaderTest, ReadTypeOfBoolean)
{
	constexpr auto expectedType = BitSerializer::Json::Detail::ValueType::Boolean;
	for (const auto *testJson : { "true", "false" })
	{
		this->PrepareReader({ testJson });
		EXPECT_EQ(expectedType, this->mJsonReader->ReadValueType());
	}
}

TYPED_TEST(JsonReaderTest, ReadTypeOfSignedInt)
{
	this->PrepareReader("-1");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::SignedInteger, this->mJsonReader->ReadValueType());

	this->PrepareReader("-12345");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::SignedInteger, this->mJsonReader->ReadValueType());
}

TYPED_TEST(JsonReaderTest, ReadTypeOfUnsignedInt)
{
	this->PrepareReader("1");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::UnsignedInteger, this->mJsonReader->ReadValueType());

	this->PrepareReader("12345");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::UnsignedInteger, this->mJsonReader->ReadValueType());
}

TYPED_TEST(JsonReaderTest, ReadTypeOfFloat)
{
	this->PrepareReader("1.0");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::Float, this->mJsonReader->ReadValueType());

	this->PrepareReader("-3.141592654");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::Float, this->mJsonReader->ReadValueType());
}

TYPED_TEST(JsonReaderTest, ReadTypeOfString)
{
	this->PrepareReader(R"("string")");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::String, this->mJsonReader->ReadValueType());
}

TYPED_TEST(JsonReaderTest, ReadTypeOfObject)
{
	this->PrepareReader("{}");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::Object, this->mJsonReader->ReadValueType());
}

TYPED_TEST(JsonReaderTest, ReadTypeOfArray)
{
	this->PrepareReader("[]");
	EXPECT_EQ(BitSerializer::Json::Detail::ValueType::Array, this->mJsonReader->ReadValueType());
}

TYPED_TEST(JsonReaderTest, ReadTypeShouldThrowExceptionWhenInvalidSequence)
{
	this->PrepareReader("//");

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::ReadValueType, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
	EXPECT_STREQ("Parsing error: Invalid sequence", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadTypeShouldThrowExceptionWhenEmptyJson)
{
	this->PrepareReader("");

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::ReadValueType, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
	EXPECT_STREQ("Parsing error: No more values to read", ex.what());
}

TYPED_TEST(JsonReaderTest, ReadTypeShouldThrowExceptionWhenUnexpectedEnd)
{
	this->PrepareReader("-");	// Minus without specifying the actual number

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::ReadValueType, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
	EXPECT_STREQ("Parsing error: Unexpected end of input archive", ex.what());
}

//-----------------------------------------------------------------------------
// Tests for skip values
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, SkipRootPrimitives)
{
	for (std::string testJson : {"true", "false", "123456789", "3.14", "\"text\"", "null"})
	{
		this->PrepareReader(testJson);
		this->mJsonReader->SkipValue();
		EXPECT_TRUE(this->mJsonReader->IsEnd());
	}
}

TYPED_TEST(JsonReaderTest, SkipValueShouldThrowExceptionWhenEmptyJson)
{
	this->PrepareReader("");

	BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::ReadValueType, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(0, ex.Offset);
	EXPECT_STREQ("Parsing error: No more values to read", ex.what());
}

TYPED_TEST(JsonReaderTest, SkipValueShouldThrowExceptionWhenInvalidSequence)
{
	for (const auto *testJson : { "//", "()", "#", "True", "Null" })
	{
		this->PrepareReader(testJson);
		BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::ReadValueType, this->mJsonReader);
		EXPECT_EQ(1, ex.Line);
		EXPECT_EQ(0, ex.Offset);
		EXPECT_STREQ("Parsing error: Invalid sequence", ex.what());
	}
}

TYPED_TEST(JsonReaderTest, SkipStringWithEscapedCharacters)
{
	this->PrepareReader(R"("\"\\\/\b\f\n\r\t")");
	this->mJsonReader->SkipValue();
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipStringWithUnicodeCharacters)
{
	this->PrepareReader(R"("\u03B1\u03B2\u03B3")");
	this->mJsonReader->SkipValue();
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipStringShouldThrowExceptionWhenNoCloseQuotes)
{
	this->PrepareReader(R"("text)");

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::SkipValue, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(5, ex.Offset);
	EXPECT_STREQ("Parsing error: Unterminated string literal", ex.what());
}

TYPED_TEST(JsonReaderTest, SkipLongString)
{
	const std::string testJson = "\"" + this->GenTestString(chunkSize * 5) + "\"";
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipLongStringWithEscapedQuoteAtChunkBoundary)
{
	// The backslash starts at the very last byte of the internal read chunk
	const std::string testJson = "\"" + this->GenTestString(chunkSize - 2) + "\\\"" + this->GenTestString(chunkSize) + "\"";
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipLongStringWithEscapes)
{
	const std::string testJson = "\"" + this->GenTestString(chunkSize * 3) + R"(\"\u0041)" + this->GenTestString(chunkSize * 3) + "\"";
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipLongNumber)
{
	const std::string testJson(chunkSize * 5, '1');
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, SkipArrayWithPrimitives)
{
	std::string testJson = R"([true, false, 123456789, 3.14, "text", null])";
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipArrayWithChildArray)
{
	std::string testJson = R"([
	[ true, false ],
	[ 1, 2, 3],
	[ 3.14 ],
	[ "text" ],
	[ null ]
])";
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_EQ(7, this->mJsonReader->GetLineNumber());
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipArrayWithChildObjects)
{
	std::string testJson = R"([
	{
		"key1": true,
		"key2": false,
		"key3": 1234
	},
	{
		"key4": 3.14,
		"key5": "text",
		"key6": null
	}
])";
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_EQ(12, this->mJsonReader->GetLineNumber());
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipElementOfArray)
{
	std::string testJson = R"([
	{
		"key1": true,
		"key2": 1,
		"key3": "text"
	},
	"test"
])";
	this->PrepareReader(testJson);

	// Skip first element (object)
	ASSERT_TRUE(this->mJsonReader->OpenArray());
	this->mJsonReader->SkipValue();
	this->mJsonReader->ReadValueSeparator();
	EXPECT_EQ(6, this->mJsonReader->GetLineNumber());

	// Read second element
	std::string_view str;
	ASSERT_TRUE(this->mJsonReader->ReadValue(str));
	EXPECT_EQ("test", str);
	EXPECT_EQ(7, this->mJsonReader->GetLineNumber());
}

TYPED_TEST(JsonReaderTest, SkipArrayShouldThrowExceptionWhenMissingComma)
{
	std::string testJson = "[1 true]";
	this->PrepareReader(testJson);

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::SkipValue, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(3, ex.Offset);
	EXPECT_STREQ("Parsing error: Expected ',' or ']' in array", ex.what());
}

TYPED_TEST(JsonReaderTest, SkipArrayShouldThrowExceptionWhenMissingClosingBracket)
{
	std::string testJson = "[1, 2 ";
	this->PrepareReader(testJson);

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::SkipValue, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(6, ex.Offset);
	EXPECT_STREQ("Parsing error: Expected ',' or ']' in array", ex.what());
}

//-----------------------------------------------------------------------------

TYPED_TEST(JsonReaderTest, SkipObjectWithPrimitives)
{
	std::string testJson = R"({
	"key1": true,
	"key2": false,
	"key3": 1234,
	"key4": 3.14,
	"key5": "text",
	"key6": null
})";
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_EQ(8, this->mJsonReader->GetLineNumber());
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipObjectWithChildObject)
{
	std::string testJson = R"({
	"key1": {
		"key1": true,
		"key2": 1,
		"key3": "text"
	},
	"key2": false
})";
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_EQ(8, this->mJsonReader->GetLineNumber());
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipObjectWithChildArray)
{
	std::string testJson = R"({
	"key1": [true, false, 123456789, 3.14, "text", null],
	"key2": false
})";
	this->PrepareReader(testJson);
	this->mJsonReader->SkipValue();
	EXPECT_EQ(4, this->mJsonReader->GetLineNumber());
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipKeyValueInObject)
{
	std::string testJson = R"({
	"key1": [true, false, 123456789, 3.14, "text", null],
	"key2": "test"
})";
	this->PrepareReader(testJson);

	// Skip first key/value (array)
	ASSERT_TRUE(this->mJsonReader->OpenObject());
	std::string_view key;
	this->mJsonReader->ReadKey(key);
	this->mJsonReader->SkipValue();
	EXPECT_EQ(2, this->mJsonReader->GetLineNumber());
	this->mJsonReader->ReadValueSeparator();

	// Read second key/value
	this->mJsonReader->ReadKey(key);
	std::string_view str;
	ASSERT_TRUE(this->mJsonReader->ReadValue(str));
	EXPECT_EQ("test", str);
	EXPECT_EQ(3, this->mJsonReader->GetLineNumber());
}

TYPED_TEST(JsonReaderTest, SkipObjectWithEscapeSequences)
{
	std::string testJson = R"({
	"\u03B1\u03B2\u03B3": "\u03B1\u03B2\u03B3",
	"\"\\\/\b\f\n\r\t": "\"\\\/\b\f\n\r\t"
})";
	this->PrepareReader(testJson);

	// Skip whole object
	this->mJsonReader->SkipValue();
	EXPECT_TRUE(this->mJsonReader->IsEnd());
}

TYPED_TEST(JsonReaderTest, SkipObjectShouldThrowExceptionWhenMissingColon)
{
	std::string testJson = R"({	"key1" true })";
	this->PrepareReader(testJson);

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::SkipValue, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(9, ex.Offset);
	EXPECT_STREQ("Parsing error: Expected ':' in object", ex.what());
}

TYPED_TEST(JsonReaderTest, SkipObjectShouldThrowExceptionWhenMissingComma)
{
	std::string testJson = R"({
	"key1": true
	"key2": false
})";
	this->PrepareReader(testJson);

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::SkipValue, this->mJsonReader);
	EXPECT_EQ(3, ex.Line);
	EXPECT_EQ(17, ex.Offset);
	EXPECT_STREQ("Parsing error: Expected ',' or '}' in object", ex.what());
}

TYPED_TEST(JsonReaderTest, SkipObjectShouldThrowExceptionWhenMissingClosingBracket)
{
	std::string testJson = R"({	"key1": true )";
	this->PrepareReader(testJson);

	const BitSerializer::ParsingException ex = GTestExpectException<BitSerializer::ParsingException>(&TypeParam::SkipValue, this->mJsonReader);
	EXPECT_EQ(1, ex.Line);
	EXPECT_EQ(15, ex.Offset);
	EXPECT_STREQ("Parsing error: Expected ',' or '}' in object", ex.what());
}

//-----------------------------------------------------------------------------
// Tests of reading from UTF encoded streams (stream reader only)
//-----------------------------------------------------------------------------
TYPED_TEST(JsonReaderTest, ReadFromUtf8StreamWithBom)
{
	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStreamReader>)
	{
		std::string sourceStr = std::string(
			BitSerializer::Convert::Utf::Utf8::bom, sizeof(BitSerializer::Convert::Utf::Utf8::bom)) + R"({"key":"value"})";
		this->PrepareStreamReader(std::move(sourceStr));
		this->AssertReadKeyValue("key", "value");
	}
}

TYPED_TEST(JsonReaderTest, ReadFromUtf16LeStream)
{
	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStreamReader>)
	{
		auto json = NativeStringToLittleEndian(u"{\"key\":\"value\"}");
		this->PrepareStreamReader(std::string(reinterpret_cast<const char*>(json.data()), json.size() * sizeof(char16_t)));
		this->AssertReadKeyValue("key", "value");
	}
}

TYPED_TEST(JsonReaderTest, ReadFromUtf16LeStreamWithBom)
{
	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStreamReader>)
	{
		std::string sourceStr = std::string(BitSerializer::Convert::Utf::Utf16Le::bom, sizeof(BitSerializer::Convert::Utf::Utf16Le::bom));
		auto json = NativeStringToLittleEndian(u"{\"key\":\"value\"}");
		sourceStr.append(reinterpret_cast<const char*>(json.data()), json.size() * sizeof(char16_t));
		this->PrepareStreamReader(std::move(sourceStr));
		this->AssertReadKeyValue("key", "value");
	}
}

TYPED_TEST(JsonReaderTest, ReadFromUtf16BeStream)
{
	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStreamReader>)
	{
		auto json = NativeStringToBigEndian(u"{\"key\":\"value\"}");
		this->PrepareStreamReader(std::string(reinterpret_cast<const char*>(json.data()), json.size() * sizeof(char16_t)));
		this->AssertReadKeyValue("key", "value");
	}
}

TYPED_TEST(JsonReaderTest, ReadFromUtf32LeStream)
{
	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStreamReader>)
	{
		auto json = NativeStringToLittleEndian(U"{\"key\":\"value\"}");
		this->PrepareStreamReader(std::string(reinterpret_cast<const char*>(json.data()), json.size() * sizeof(char32_t)));
		this->AssertReadKeyValue("key", "value");
	}
}

TYPED_TEST(JsonReaderTest, ReadFromUtf32BeStream)
{
	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStreamReader>)
	{
		auto json = NativeStringToBigEndian(U"{\"key\":\"value\"}");
		this->PrepareStreamReader(std::string(reinterpret_cast<const char*>(json.data()), json.size() * sizeof(char32_t)));
		this->AssertReadKeyValue("key", "value");
	}
}

TYPED_TEST(JsonReaderTest, ReadFromUtf16LeStreamWithSurrogatePair)
{
	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStreamReader>)
	{
		// JSON: {"msg":"Hi😀"} where 😀 = U+1F600 = surrogate pair 0xD83D, 0xDE00
		auto json = NativeStringToLittleEndian(u"{\"msg\":\"Hi😀\"}");
		std::string sourceStr(reinterpret_cast<const char*>(json.data()), json.size() * sizeof(char16_t));
		this->PrepareStreamReader(std::move(sourceStr));
		this->AssertReadKeyValue("msg", std::string("Hi\xF0\x9F\x98\x80", 6));
	}
}

TYPED_TEST(JsonReaderTest, ReadFromUtf16BeStreamWithSurrogatePair)
{
	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStreamReader>)
	{
		// JSON: {"msg":"Hi😀"} as UTF-16BE
		auto json = NativeStringToBigEndian(u"{\"msg\":\"Hi😀\"}");
		std::string sourceStr(reinterpret_cast<const char*>(json.data()), json.size() * sizeof(char16_t));
		this->PrepareStreamReader(std::move(sourceStr));
		this->AssertReadKeyValue("msg", std::string("Hi\xF0\x9F\x98\x80", 6));
	}
}

#pragma warning(pop)
