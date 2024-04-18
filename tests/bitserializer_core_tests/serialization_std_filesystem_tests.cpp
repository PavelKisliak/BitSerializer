/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"
#include "testing_tools/bin_archive_stub.h"

#include "bitserializer/types/std/filesystem.h"
#include "bitserializer/types/std/array.h"


using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for std::filesystem::path
//-----------------------------------------------------------------------------
TEST(STD_Filesystem, SerializePath) {
	auto path = std::filesystem::temp_directory_path();
	TestSerializeType<ArchiveStub>(path);
}

TEST(STD_Filesystem, SerializePathWithUnicode) {
	auto path = std::filesystem::temp_directory_path() / UTF8("Привет мир!.txt");
	TestSerializeType<ArchiveStub>(path);
}

TEST(STD_Filesystem, SerializeArrayOfPaths) {
	TestSerializeStlContainer<ArchiveStub, std::array<std::filesystem::path, 100>>();
}

TEST(STD_Filesystem, SerializePathAsClassMember)
{
	TestClassWithSubType testEntity(std::filesystem::temp_directory_path());
	TestSerializeType<ArchiveStub>(testEntity);
}

TEST(STD_Filesystem, SerializePathWithUnicodeAsClassMember)
{
	TestClassWithSubType testEntity(std::filesystem::temp_directory_path() / u"Привет мир!.txt");
	TestSerializeType<ArchiveStub>(testEntity);
}
