/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <array>
#include "testing_tools/auto_fixture.h"
#include "bitserializer/json_archive.h"
#include "json_writer_fixture.h"

using JsonWritersTypes = ::testing::Types<
	BitSerializer::Json::Detail::CJsonStringWriter
	, BitSerializer::Json::Detail::CJsonStringPrettyWriter
	/*, BitSerializer::Json::Detail::CJsonStreamWriter*/
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
	this->mJsonWriter->WriteValue(3.14f);
	EXPECT_EQ("3.14", this->TakeResult());
}

TYPED_TEST(JsonWriterTest, WriteDouble)
{
	this->mJsonWriter->WriteValue(3.141592654);
	EXPECT_EQ("3.141592654", this->TakeResult());
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

	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStringWriter>)
	{
		EXPECT_EQ(R"(["Hello",10,false])", this->TakeResult());
	}
	else
	{
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

	if constexpr (std::is_same_v<TypeParam, BitSerializer::Json::Detail::CJsonStringWriter>)
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
