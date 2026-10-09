/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <array>
#include <cstdio>
#include "testing_tools/auto_fixture.h"
#include "bitserializer/json_archive.h"
#include "json_writer_fixture.h"

using JsonWritersTypes = ::testing::Types<
	BitSerializer::Json::Detail::JsonStringWriter
	, BitSerializer::Json::Detail::JsonStringPrettyWriter
	, BitSerializer::Json::Detail::JsonStreamWriter
	, BitSerializer::Json::Detail::JsonStreamPrettyWriter
>;

// Tests for all implementations of IJsonWriter (without formatting)
TYPED_TEST_SUITE(JsonWriterTest, JsonWritersTypes, );

//------------------------------------------------------------------------------

TYPED_TEST(JsonWriterTest, WriteBoolean)
{
	this->mJsonWriter->WriteValue(false);
	EXPECT_EQ("false", this->TakeResult());

	this->mJsonWriter->WriteValue(true);
	EXPECT_EQ("true", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteNull)
{
	this->mJsonWriter->WriteValue(nullptr);
	EXPECT_EQ("null", this->TakeResult());
}

//-----------------------------------------------------------------------------
// Tests of writing integral values
//-----------------------------------------------------------------------------
TYPED_TEST(JsonWriterTest, WriteUInt)
{
	this->mJsonWriter->WriteValue(std::numeric_limits<uint8_t>::min());
	EXPECT_EQ("0", this->TakeResult());

	this->mJsonWriter->WriteValue(std::numeric_limits<uint8_t>::max());
	EXPECT_EQ("255", this->TakeResult());

	this->mJsonWriter->WriteValue(std::numeric_limits<uint16_t>::max());
	EXPECT_EQ("65535", this->TakeResult());

	this->mJsonWriter->WriteValue(std::numeric_limits<uint32_t>::max());
	EXPECT_EQ("4294967295", this->TakeResult());

	this->mJsonWriter->WriteValue(std::numeric_limits<uint64_t>::max());
	EXPECT_EQ("18446744073709551615", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteInt)
{
	this->mJsonWriter->WriteValue(std::numeric_limits<int8_t>::min());
	EXPECT_EQ("-128", this->TakeResult());

	this->mJsonWriter->WriteValue(std::numeric_limits<int8_t>::max());
	EXPECT_EQ("127", this->TakeResult());

	this->mJsonWriter->WriteValue(std::numeric_limits<int16_t>::min());
	EXPECT_EQ("-32768", this->TakeResult());

	this->mJsonWriter->WriteValue(std::numeric_limits<int32_t>::max());
	EXPECT_EQ("2147483647", this->TakeResult());

	this->mJsonWriter->WriteValue(std::numeric_limits<int64_t>::min());
	EXPECT_EQ("-9223372036854775808", this->TakeResult());
}

//-----------------------------------------------------------------------------
// Tests of writing floating types
//-----------------------------------------------------------------------------
TYPED_TEST(JsonWriterTest, WriteFloat)
{
	this->mJsonWriter->WriteValue(-0.1875f);
	EXPECT_EQ("-0.1875", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteDouble)
{
	this->mJsonWriter->WriteValue(-0.0625);
	EXPECT_EQ("-0.0625", this->TakeResult());
}

//-----------------------------------------------------------------------------
// Tests of writing strings
//-----------------------------------------------------------------------------
TYPED_TEST(JsonWriterTest, WriteEmptyString)
{
	this->mJsonWriter->WriteValue("");
	EXPECT_EQ("\"\"", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteString)
{
	this->mJsonWriter->WriteValue("Hello world!");
	EXPECT_EQ(R"("Hello world!")", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteStringWithEscapingQuatationMarks)
{
	this->mJsonWriter->WriteValue(R"(Test "escaping quotation" marks)");
	EXPECT_EQ("\"Test \\\"escaping quotation\\\" marks\"", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteStringWithEscapingReverseSlashes)
{
	this->mJsonWriter->WriteValue(R"(Test\escaping\reverse\slashes)");
	EXPECT_EQ("\"Test\\\\escaping\\\\reverse\\\\slashes\"", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteStringWithEscapingControlCharacterts)
{
	this->mJsonWriter->WriteValue("Control characters: \n\r\t\b\f");
	EXPECT_EQ("\"Control characters: \\n\\r\\t\\b\\f\"", this->TakeResult());

	this->mJsonWriter->WriteValue("\x05");
	EXPECT_EQ("\"\\u0005\"", this->TakeResult());

	this->mJsonWriter->WriteValue("\x1f");
	EXPECT_EQ("\"\\u001F\"", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteLongStringWithoutEscapes)
{
	const std::string value = this->GenTestString(1000);
	this->mJsonWriter->WriteValue(value);
	EXPECT_EQ("\"" + value + "\"", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteStringWithEscapesAtBlockBoundaries)
{
	// Escapes at SIMD block boundaries (offsets 15, 16, 17).
	for (const size_t offset : { size_t(15), size_t(16), size_t(17) })
	{
		std::string value(offset, 'x');
		value.push_back('"');
		value.append(20, 'y');

		std::string escaped(offset, 'x');
		escaped += "\\\"";
		escaped.append(20, 'y');

		this->mJsonWriter->WriteValue(value);
		EXPECT_EQ("\"" + escaped + "\"", this->TakeResult()) << "offset=" << offset;
	}
}

TYPED_TEST(JsonWriterTest, WriteStringWithAdjacentEscapes)
{
	this->mJsonWriter->WriteValue("\"\\\"\n\r\t\b\f");
	EXPECT_EQ("\"\\\"\\\\\\\"\\n\\r\\t\\b\\f\"", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteStringWithAllControlCharacters)
{
	std::string input;
	for (int i = 0; i < 0x20; ++i) {
		input.push_back(static_cast<char>(i));
	}

	std::string expected = "\"";
	for (int i = 0; i < 0x20; ++i)
	{
		switch (i)
		{
		case '\b': expected += "\\b"; break;
		case '\f': expected += "\\f"; break;
		case '\n': expected += "\\n"; break;
		case '\r': expected += "\\r"; break;
		case '\t': expected += "\\t"; break;
		default:
			{
				char buf[7];
				std::snprintf(buf, sizeof(buf), "\\u%04X", i);
				expected += buf;
			}
			break;
		}
	}
	expected += "\"";

	this->mJsonWriter->WriteValue(input);
	EXPECT_EQ(expected, this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteStringDoesNotEscapeDelAndHighBytes)
{
	// 0x7F (DEL) and bytes >= 0x80 (valid UTF-8) are valid JSON string content and must pass through unchanged.
	std::string value = "abc";
	value.push_back(static_cast<char>(0x7f));            // DEL
	value += "\xC3\xA9";                                 // U+00E9
	value += "\xE4\xB8\x96\xE7\x95\x8C";                 // U+4E16 U+754C
	value.append("def");

	this->mJsonWriter->WriteValue(value);
	EXPECT_EQ("\"" + value + "\"", this->TakeResult());
}

//-----------------------------------------------------------------------------
// Tests of writing arrays
//-----------------------------------------------------------------------------
TYPED_TEST(JsonWriterTest, BeginArray)
{
	this->mJsonWriter->BeginArray();
	EXPECT_EQ("[", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, EndArray)
{
	this->mJsonWriter->BeginArray();
	this->mJsonWriter->EndArray(false);
	// Regular and pretty writers should produce the same result if the array is empty
	EXPECT_EQ("[]", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteArrayElements)
{
	this->mJsonWriter->BeginArray();
	this->mJsonWriter->WriteValue("Hello");
	this->mJsonWriter->WriteValueSeparator();
	this->mJsonWriter->WriteValue(10);
	this->mJsonWriter->WriteValueSeparator();
	this->mJsonWriter->WriteValue(false);
	this->mJsonWriter->EndArray(true);

	constexpr bool isCompact = std::is_same_v<TypeParam, BitSerializer::Json::Detail::JsonStringWriter>
		|| std::is_same_v<TypeParam, BitSerializer::Json::Detail::JsonStreamWriter>;
	if constexpr (isCompact)
	{
		EXPECT_EQ(R"(["Hello",10,false])", this->TakeResult());
	}
	else
	{
		// For pretty writer
		EXPECT_EQ(R"([
	"Hello",
	10,
	false
])", this->TakeResult());
	}
}

//-----------------------------------------------------------------------------
// Tests of writing objects
//-----------------------------------------------------------------------------
TYPED_TEST(JsonWriterTest, BeginObject)
{
	this->mJsonWriter->BeginObject();
	EXPECT_EQ("{", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, EndObject)
{
	this->mJsonWriter->BeginObject();
	this->mJsonWriter->EndObject(false);
	// Regular and pretty writers should produce the same result if the object is empty
	EXPECT_EQ("{}", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteObjectElements)
{
	this->mJsonWriter->BeginObject();
	this->mJsonWriter->WriteKey("Key1");
	this->mJsonWriter->WriteValue("Value1");
	this->mJsonWriter->WriteValueSeparator();

	this->mJsonWriter->WriteKey("Key2");
	this->mJsonWriter->WriteValue(true);
	this->mJsonWriter->EndObject(true);

	constexpr bool isCompact = std::is_same_v<TypeParam, BitSerializer::Json::Detail::JsonStringWriter>
		|| std::is_same_v<TypeParam, BitSerializer::Json::Detail::JsonStreamWriter>;
	if constexpr (isCompact)
	{
		EXPECT_EQ("{\"Key1\":\"Value1\",\"Key2\":true}", this->TakeResult());
	}
	else
	{
		// For pretty writer
		EXPECT_EQ(R"({
	"Key1": "Value1",
	"Key2": true
})", this->TakeResult());
	}
}
