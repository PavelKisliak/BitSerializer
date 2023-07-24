/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "tests/test_helpers/common_test_methods.h"
#include "tests/test_helpers/archive_stub.h"

#include "bitserializer/types/std/pair.h"
#include "bitserializer/types/std/tuple.h"
#include "bitserializer/types/std/optional.h"
#include "bitserializer/types/std/memory.h"

//-----------------------------------------------------------------------------
// Serialization tests for STL types.
// Because for serialization of STL types used base common methods for serialization,
// there is no need to write special tests for other types of archives.
//-----------------------------------------------------------------------------

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for std::pair
//-----------------------------------------------------------------------------
TEST(STD_Types, SerializePair) {
	auto pair = BuildFixture<std::pair<std::string, int>>();
	TestSerializeType<ArchiveStub>(pair);
}

TEST(STD_Types, SerializePairAsClassMember) {
	TestClassWithSubType<std::pair<std::string, int>> testEntity;
	TestSerializeClass<ArchiveStub>(testEntity);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::tuple
//-----------------------------------------------------------------------------
TEST(STD_Types, SerializeTuple) {
	auto value = BuildFixture<std::tuple<std::string, int, float, bool>>();
	TestSerializeType<ArchiveStub>(value);
}

TEST(STD_Types, SerializeTupleThrowMismatchedTypesExceptionWhenLessSize) {
	TestMismatchedTypesPolicy<ArchiveStub, std::tuple<int, float, bool>, std::tuple<int, float>>(MismatchedTypesPolicy::ThrowError);
}

TEST(STD_Types, SerializeTupleThrowMismatchedTypesExceptionWhenLargerSize) {
	TestMismatchedTypesPolicy<ArchiveStub, std::tuple<int, float>, std::tuple<int, float, bool>>(MismatchedTypesPolicy::ThrowError);
}

TEST(STD_Types, SerializeTupleAsClassMember) {
	TestClassWithSubType<std::tuple<std::string, int, float, bool>> testEntity;
	TestSerializeClass<ArchiveStub>(testEntity);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::optional
//-----------------------------------------------------------------------------
TEST(STD_Types, SerializeOptional) {
	std::optional<std::string> testValue = "test";
	TestSerializeType<ArchiveStub>(testValue);
}

TEST(STD_Types, SerializeOptionalWithNull) {
	std::optional<int> testValue;
	TestSerializeType<ArchiveStub>(testValue);
}

TEST(STD_Types, SerializeOptionalAsClassMember) {
	TestSerializeClass<ArchiveStub>(TestClassWithSubType<std::optional<float>>());
}

TEST(STD_Types, SerializeOptionalAsClassMemberWithNull) {
	TestSerializeClass<ArchiveStub>(TestClassWithSubType<std::optional<float>>(std::nullopt));
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::unique_ptr
//-----------------------------------------------------------------------------
TEST(STD_Types, SerializeUniquePtr) {
	auto testValue = std::make_unique<std::string>("test");
	TestSerializeType<ArchiveStub>(testValue);
}

TEST(STD_Types, SerializeUniquePtrWithNull) {
	std::unique_ptr<std::string> testValue;
	TestSerializeType<ArchiveStub>(testValue);
}

TEST(STD_Types, SerializeUniquePtrAsClassMember) {
	TestSerializeClass<ArchiveStub>(TestClassWithSubType<std::unique_ptr<std::string>>());
}

TEST(STD_Types, SerializeUniquePtrAsClassMemberWithNull) {
	using TestType = std::unique_ptr<std::string>;
	TestSerializeClass<ArchiveStub>(TestClassWithSubType<std::unique_ptr<std::string>>(TestType()));
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::shared_ptr
//-----------------------------------------------------------------------------
TEST(STD_Types, SerializeSharedPtr) {
	auto testValue = std::make_shared<std::string>("test");
	TestSerializeType<ArchiveStub>(testValue);
}

TEST(STD_Types, SerializeSharedPtrWithNull) {
	std::shared_ptr<std::string> testValue;
	TestSerializeType<ArchiveStub>(testValue);
}

TEST(STD_Types, SerializeSharedPtrAsClassMember) {
	TestSerializeClass<ArchiveStub>(TestClassWithSubType<std::shared_ptr<std::string>>());
}

TEST(STD_Types, SerializeSharedPtrAsClassMemberWithNull) {
	using TestType = std::shared_ptr<std::string>;
	TestSerializeClass<ArchiveStub>(TestClassWithSubType<std::shared_ptr<std::string>>(TestType()));
}
