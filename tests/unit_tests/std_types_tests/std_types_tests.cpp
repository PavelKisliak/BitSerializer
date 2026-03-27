/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"

#include "bitserializer/types/std/pair.h"
#include "bitserializer/types/std/tuple.h"
#include "bitserializer/types/std/optional.h"
#include "bitserializer/types/std/variant.h"
#include "bitserializer/types/std/memory.h"
#include "bitserializer/types/std/atomic.h"

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
	TestMismatchedTypesPolicy<ArchiveStub, std::tuple<int, float, bool>, std::tuple<int, float>>(MismatchedTypesPolicy::ThrowError);
}

TEST(STD_Types, SerializeTupleThrowMismatchedTypesExceptionWhenLargerSize) {
	TestMismatchedTypesPolicy<ArchiveStub, std::tuple<int, float>, std::tuple<int, float, bool>>(MismatchedTypesPolicy::ThrowError);
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

//-----------------------------------------------------------------------------
// Tests of serialization for std::variant
//-----------------------------------------------------------------------------
TEST(STD_Types, SerializeVariantWithPrimitiveAlternative) {
	TestSerializeType<ArchiveStub>(std::variant<int, std::string, float>(123));
}

TEST(STD_Types, SerializeVariantWithStringAlternative) {
	TestSerializeType<ArchiveStub>(std::variant<int, std::string, float>(std::string("test")));
}

TEST(STD_Types, SerializeVariantWithObjectAlternative) {
	TestSerializeType<ArchiveStub>(std::variant<int, TestPointClass, std::vector<int>>(TestPointClass(10, 20)));
}

TEST(STD_Types, SerializeVariantWithArrayAlternative) {
	TestSerializeType<ArchiveStub>(std::variant<int, TestPointClass, std::vector<int>>(std::vector<int>{ 1, 2, 3, 4 }));
}

TEST(STD_Types, SerializeVariantAsClassMember) {
	using VariantType = std::variant<int, std::string, TestPointClass, std::vector<int>>;
	TestSerializeType<ArchiveStub>(TestClassWithSubType(VariantType(TestPointClass(7, 11))));
	TestSerializeType<ArchiveStub>(TestClassWithSubType(VariantType(std::vector<int>{ 3, 1, 4 })));
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
