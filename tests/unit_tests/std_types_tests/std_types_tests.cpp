/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"

#include "bitserializer/types/std/pair.h"
#include "bitserializer/types/std/tuple.h"
#include "bitserializer/types/std/optional.h"
#include "bitserializer/types/std/memory.h"
#include "bitserializer/types/std/atomic.h"

// STD types (test fixtures)
#include "testing_tools/auto_fixture/std/atomic.h"
#include "testing_tools/auto_fixture/std/memory.h"
#include "testing_tools/auto_fixture/std/optional.h"
#include "testing_tools/auto_fixture/std/pair.h"
#include "testing_tools/auto_fixture/std/tuple.h"

//-----------------------------------------------------------------------------
// Serialization tests for STL types.
// Because for serialization of STL types used base common methods for serialization,
// there is no need to write special tests for other types of archives.
//-----------------------------------------------------------------------------

using namespace BitSerializer;


//-----------------------------------------------------------------------------
TEST(STD_Types, SerializePair) {
	auto pair = BuildFixture<std::pair<std::string, int>>();
	TestSerializeType<ArchiveStub>(pair);
}

TEST(STD_Types, SerializePairAsClassMember) {
	TestClassWithSubType<std::pair<std::string, int>> testEntity;
	TestSerializeType<ArchiveStub>(testEntity);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::tuple
//-----------------------------------------------------------------------------
TEST(STD_Types, SerializeTuple) {
	auto value = BuildFixture<std::tuple<std::string, int, float, bool>>();
	TestSerializeType<ArchiveStub>(value);
}

TEST(STD_Types, SerializeTupleThrowMismatchedTypesExceptionWhenLessSize) {
	TestMismatchedFieldPolicy<ArchiveStub, std::tuple<int, float, bool>, std::tuple<int, float>>(MismatchedTypesPolicy::ThrowError);
}

TEST(STD_Types, SerializeTupleThrowMismatchedTypesExceptionWhenLargerSize) {
	TestMismatchedFieldPolicy<ArchiveStub, std::tuple<int, float>, std::tuple<int, float, bool>>(MismatchedTypesPolicy::ThrowError);
}

TEST(STD_Types, SerializeTupleAsClassMember) {
	TestClassWithSubType<std::tuple<std::string, int, float, bool>> testEntity;
	TestSerializeType<ArchiveStub>(testEntity);
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
	TestSerializeType<ArchiveStub>(TestClassWithSubType<std::optional<float>>());
}

TEST(STD_Types, SerializeOptionalAsClassMemberWithNull) {
	TestSerializeType<ArchiveStub>(TestClassWithSubType<std::optional<float>>(std::nullopt));
}

TEST(STD_Types, LoadOptionalWithValueIntoEmptyOptional) {
	std::optional<std::string> actual;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(std::optional<std::string>("test"), outputArchive);

	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);

	ASSERT_TRUE(actual.has_value());
	EXPECT_EQ("test", actual.value());
}

TEST(STD_Types, LoadOptionalWithValueIntoEmptyOptionalAsClassMember) {
	using TestType = std::optional<std::string>;
	TestClassWithSubType<TestType> actual(TestType{});
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(TestClassWithSubType<TestType>(TestType("test")), outputArchive);

	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);

	ASSERT_TRUE(actual.GetValue().has_value());
	EXPECT_EQ("test", actual.GetValue().value());
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
	TestSerializeType<ArchiveStub>(TestClassWithSubType<std::unique_ptr<std::string>>());
}

TEST(STD_Types, SerializeUniquePtrAsClassMemberWithNull) {
	using TestType = std::unique_ptr<std::string>;
	TestSerializeType<ArchiveStub>(TestClassWithSubType(TestType()));
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
	TestSerializeType<ArchiveStub>(TestClassWithSubType<std::shared_ptr<std::string>>());
}

TEST(STD_Types, SerializeSharedPtrAsClassMemberWithNull) {
	using TestType = std::shared_ptr<std::string>;
	TestSerializeType<ArchiveStub>(TestClassWithSubType(TestType()));
}

TEST(STD_Types, LoadUniquePtrValueIntoEmptyPointer) {
	std::unique_ptr<std::string> actual;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(std::make_unique<std::string>("test"), outputArchive);

	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);

	ASSERT_TRUE(actual);
	EXPECT_EQ("test", *actual);
}

TEST(STD_Types, LoadUniquePtrValueIntoEmptyPointerAsClassMember) {
	using TestType = std::unique_ptr<std::string>;
	TestClassWithSubType<TestType> actual(TestType{});
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(TestClassWithSubType<TestType>(std::make_unique<std::string>("test")), outputArchive);

	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);

	ASSERT_TRUE(actual.GetValue());
	EXPECT_EQ("test", *actual.GetValue());
}

TEST(STD_Types, LoadSharedPtrValueIntoEmptyPointer) {
	std::shared_ptr<std::string> actual;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(std::make_shared<std::string>("test"), outputArchive);

	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);

	ASSERT_TRUE(actual);
	EXPECT_EQ("test", *actual);
}

TEST(STD_Types, LoadSharedPtrValueIntoEmptyPointerAsClassMember) {
	using TestType = std::shared_ptr<std::string>;
	TestClassWithSubType<TestType> actual(TestType{});
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(TestClassWithSubType<TestType>(std::make_shared<std::string>("test")), outputArchive);

	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);

	ASSERT_TRUE(actual.GetValue());
	EXPECT_EQ("test", *actual.GetValue());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::atomic
//-----------------------------------------------------------------------------
TEST(STD_Types, SerializeAtomicAsClassMember)
{
	TestSerializeType<ArchiveStub>(TestClassWithSubTypes<std::atomic_bool, std::atomic_int> ());
}

TEST(STD_Types, SerializeAtomic)
{
	TestSerializeType<ArchiveStub, std::atomic_bool>();
	TestSerializeType<ArchiveStub, std::atomic_int>();
}
