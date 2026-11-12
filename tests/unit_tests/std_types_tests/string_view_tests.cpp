/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include "testing_tools/archive_stub.h"
#include "bitserializer/bit_serializer.h"

using namespace BitSerializer;

namespace
{
	struct StringViewEntity
	{
		std::string_view value;

		template <class TArchive>
		void Serialize(TArchive& archive)
		{
			archive << KeyValue("value", value);
		}
	};

	struct StringEntity
	{
		std::string value;

		template <class TArchive>
		void Serialize(TArchive& archive)
		{
			archive << KeyValue("value", value);
		}
	};
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::basic_string_view (save-only)
//-----------------------------------------------------------------------------
TEST(STD_StringView, SerializeAnsiString)
{
	const std::string_view expected = "Test string";
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	std::string actual;
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_EQ(std::string(expected), actual);
}

TEST(STD_StringView, SerializeEmptyString)
{
	const std::string_view expected;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	std::string actual = "not empty";
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_TRUE(actual.empty());
}

TEST(STD_StringView, SerializeWideString)
{
	const std::wstring_view expected = L"Test wide string";
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	std::wstring actual;
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_EQ(std::wstring(expected), actual);
}

TEST(STD_StringView, SerializeUtf16String)
{
	const std::u16string_view expected = u"Test UTF-16 string";
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	std::u16string actual;
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_EQ(std::u16string(expected), actual);
}

TEST(STD_StringView, SerializeUtf32String)
{
	const std::u32string_view expected = U"Test UTF-32 string";
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	std::u32string actual;
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_EQ(std::u32string(expected), actual);
}

#if defined(__cpp_lib_char8_t)
TEST(STD_StringView, SerializeUtf8String)
{
	const std::u8string_view expected = u8"Test UTF-8 string";
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	std::u8string actual;
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_TRUE(actual == std::u8string(expected));
}
#endif

TEST(STD_StringView, SerializeAsField)
{
	StringViewEntity expected{ "Field value" };
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(expected, outputArchive);

	StringEntity actual;
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	EXPECT_EQ(std::string(expected.value), actual.value);
}

TEST(STD_StringView, SerializeToBinaryArchive)
{
	const std::string_view expected = "Binary archive string";
	BinArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<BinArchiveStub>(expected, outputArchive);

	std::string actual;
	BitSerializer::LoadObject<BinArchiveStub>(actual, outputArchive);
	EXPECT_EQ(std::string(expected), actual);
}
