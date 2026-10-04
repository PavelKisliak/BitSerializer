/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <filesystem>
#include <fstream>
#include <sstream>
#include "testing_tools/common_test_methods.h"
#include "bitserializer/json_archive.h"
#include "bitserializer/types/std/vector.h"

using BitSerializer::Json::JsonArchive;
using BitSerializer::Json::JsoncArchive;

static_assert(BitSerializer::Json::JsonArchive::archive_type == BitSerializer::ArchiveType::Json);
static_assert(BitSerializer::Json::JsoncArchive::archive_type == BitSerializer::ArchiveType::Jsonc);

#pragma warning(push)
#pragma warning(disable: 4566)

//-----------------------------------------------------------------------------
// Tests of reading JSONC (comments and trailing commas)
//-----------------------------------------------------------------------------
TEST(JsoncArchive, LoadClassWithComments)
{
	const std::string jsonc = R"({
		// A line comment before the first field
		"x": 10, /* an inline block comment */
		"y": 20 // A comment after the last field
	})";

	TestPointClass point;
	BitSerializer::LoadObject<JsoncArchive>(point, jsonc);

	EXPECT_EQ(10, point.x);
	EXPECT_EQ(20, point.y);
}

TEST(JsoncArchive, LoadArrayWithCommentsAndTrailingComma)
{
	const std::string jsonc = R"([
		1, // first
		/* second
		   value */ 2,
		3, // trailing comma below
	])";

	std::vector<int> actual;
	BitSerializer::LoadObject<JsoncArchive>(actual, jsonc);

	EXPECT_EQ((std::vector<int>{ 1, 2, 3 }), actual);
}

TEST(JsoncArchive, LoadNestedObjectWithJsonc)
{
	const std::string jsonc = R"([
		{ "x": 1, "y": 2, }, /* first */
		{ "x": 3, "y": 4 },  // second
	])";

	std::vector<TestPointClass> actual;
	BitSerializer::LoadObject<JsoncArchive>(actual, jsonc);

	ASSERT_EQ(2U, actual.size());
	EXPECT_EQ(1, actual[0].x);
	EXPECT_EQ(2, actual[0].y);
	EXPECT_EQ(3, actual[1].x);
	EXPECT_EQ(4, actual[1].y);
}

TEST(JsoncArchive, LoadFromStream)
{
	const std::string jsonc = R"({
		// Comment inside a stream
		"x": 11,
		"y": 22,
	})";

	std::istringstream inputStream(jsonc);
	TestPointClass point;
	BitSerializer::LoadObject<JsoncArchive>(point, inputStream);

	EXPECT_EQ(11, point.x);
	EXPECT_EQ(22, point.y);
}

TEST(JsoncArchive, LoadFromStreamWithBom)
{
	const std::string jsonc = R"({ "x": 1, "y": 2 })";
	std::string sourceData(
		BitSerializer::Convert::Utf::Utf8::bom,
		sizeof(BitSerializer::Convert::Utf::Utf8::bom));
	sourceData += jsonc;

	std::istringstream inputStream(sourceData);
	TestPointClass point;
	BitSerializer::LoadObject<JsoncArchive>(point, inputStream);

	EXPECT_EQ(1, point.x);
	EXPECT_EQ(2, point.y);
}

TEST(JsoncArchive, LoadFromFile)
{
	const auto testFilePath = std::filesystem::temp_directory_path() / "TestJsonc.jsonc";
	{
		std::ofstream file(testFilePath, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
		file << R"({
			// Comment
			"x": 100,
			"y": 200,
		})";
	}

	TestPointClass point;
	BitSerializer::LoadObjectFromFile<JsoncArchive>(point, testFilePath);
	std::filesystem::remove(testFilePath);

	EXPECT_EQ(100, point.x);
	EXPECT_EQ(200, point.y);
}

//-----------------------------------------------------------------------------
// Tests of writing JSONC (comments are not preserved, output is plain JSON)
//-----------------------------------------------------------------------------
TEST(JsoncArchive, SaveProducesPlainJson)
{
	TestPointClass point(10, 20);
	const auto actual = BitSerializer::SaveObject<JsoncArchive>(point);
	EXPECT_EQ(R"({"x":10,"y":20})", actual);
}

TEST(JsoncArchive, SerializeRoundTrip)
{
	TestPointClass expected(3, 4);
	auto saved = BitSerializer::SaveObject<JsoncArchive>(expected);

	TestPointClass actual;
	BitSerializer::LoadObject<JsoncArchive>(actual, saved);

	EXPECT_EQ(expected, actual);
}

//-----------------------------------------------------------------------------
// Tests that strict JsonArchive still rejects JSONC constructs
//-----------------------------------------------------------------------------
TEST(JsoncArchive, StrictJsonArchiveShouldRejectComments)
{
	const std::string jsonc = R"({ /* comment */ "x": 1, "y": 2 })";
	TestPointClass point;
	EXPECT_THROW(BitSerializer::LoadObject<JsonArchive>(point, jsonc), BitSerializer::ParsingException);
}

#pragma warning(pop)
