/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "test_helpers/common_test_methods.h"
#include "test_helpers/archive_stub.h"

#include "bitserializer/types/std/pair.h"
#include "bitserializer/types/std/optional.h"

//-----------------------------------------------------------------------------
// Serialization tests for STL types.
// Because for serialization of STL types used base common methods for serialization,
// there is no need to write special tests for other types of archives.
//-----------------------------------------------------------------------------

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for std::pair
//-----------------------------------------------------------------------------
TEST(STL_Types, SerializePair) {
	auto pair = BuildFixture<std::pair<std::string, int>>();
	TestSerializeType<ArchiveStub>(pair);
}

TEST(STL_Types, SerializePairAsClassMember) {
	TestClassWithSubType<std::pair<std::string, int>> testEntity;
	BuildFixture(testEntity);
	TestSerializeClass<ArchiveStub>(testEntity);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::optional
//-----------------------------------------------------------------------------
TEST(STL_Types, SerializeOptional) {
	std::optional<std::string> testValue = "test";
	TestSerializeType<ArchiveStub>(testValue);
}

TEST(STL_Types, SerializeOptionalWithNull) {
	std::optional<int> testValue;
	TestSerializeType<ArchiveStub>(testValue);
}

TEST(STL_Types, SerializeOptionalAsClassMember) {
	TestSerializeOptionalAsClassMember<ArchiveStub, float>();
	TestSerializeOptionalAsClassMember<ArchiveStub, TestPointClass>();
}

TEST(STL_Types, SerializeOptionalAsClassMemberWithNull) {
	TestSerializeOptionalAsClassMember<ArchiveStub, int>(std::nullopt);
}
