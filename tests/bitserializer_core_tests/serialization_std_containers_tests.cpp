/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"

#include "bitserializer/types/std/array.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/types/std/deque.h"
#include "bitserializer/types/std/bitset.h"
#include "bitserializer/types/std/list.h"
#include "bitserializer/types/std/forward_list.h"
#include "bitserializer/types/std/queue.h"
#include "bitserializer/types/std/stack.h"
#include "bitserializer/types/std/set.h"
#include "bitserializer/types/std/unordered_set.h"
#include "bitserializer/types/std/map.h"
#include "bitserializer/types/std/unordered_map.h"
#include "bitserializer/types/std/valarray.h"

//-----------------------------------------------------------------------------
// Tests of serialization for STL containers.
// As containers serialization used methods for serialization of base types,
// there is no need to write special tests for other types of archives.
//-----------------------------------------------------------------------------

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for std::array
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeArrayOfInts) {
	TestSerializeType<ArchiveStub, std::array<int, 7>>();
}

TEST(STD_Containers, SerializeArrayOfArrays) {
	TestSerializeType<ArchiveStub, std::array<std::array<int, 7>, 3>>();
}

TEST(STD_Containers, SerializeArrayAsClassMember) {
	using test_type = std::array<std::string, 7>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::vector
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeVectorOfInts) {
	TestSerializeType<ArchiveStub, std::vector<int>>();
}

TEST(STD_Containers, SerializeVectorWhenTargetContainerIsNotEmpty) {
	TestLoadToNotEmptyContainer<ArchiveStub, std::vector<float>>(1);
	TestLoadToNotEmptyContainer<ArchiveStub, std::vector<float>>(10);
}

TEST(STD_Containers, SerializeVectorOfVectors) {
	TestSerializeType<ArchiveStub, std::vector<std::vector<int>>>();
}

TEST(STD_Containers, SerializeVectorAsClassMember) {
	using test_type = std::vector<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

TEST(STD_Containers, SerializeVectorOfBooleans) {
	TestSerializeType<ArchiveStub, std::vector<bool>>();
}

TEST(STD_Containers, SerializeVectorOfBooleansWhenTargetContainerIsNotEmpty) {
	TestLoadToNotEmptyContainer<ArchiveStub, std::vector<bool>>(1);
	TestLoadToNotEmptyContainer<ArchiveStub, std::vector<bool>>(10);
}

TEST(STD_Containers, SerializeVectorOfBooleansAsClassMember) {
	using test_type = std::vector<bool>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::deque
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeDequeOfFloats) {
	TestSerializeType<ArchiveStub, std::deque<int>>();
}

TEST(STD_Containers, SerializeDequeWhenTargetContainerIsNotEmpty) {
	TestLoadToNotEmptyContainer<ArchiveStub, std::deque<float>>(1);
	TestLoadToNotEmptyContainer<ArchiveStub, std::deque<float>>(10);
}

TEST(STD_Containers, SerializeDequeOfDeques) {
	TestSerializeType<ArchiveStub, std::deque<std::deque<int>>>();
}

TEST(STD_Containers, SerializeDequeAsClassMember) {
	using test_type = std::deque<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::bitset
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeBitset) {
	TestSerializeType<ArchiveStub, std::bitset<10>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::list
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeListOfInts) {
	TestSerializeType<ArchiveStub, std::list<int>>();
}

TEST(STD_Containers, SerializeListWhenTargetContainerIsNotEmpty) {
	TestLoadToNotEmptyContainer<ArchiveStub, std::list<float>>(1);
	TestLoadToNotEmptyContainer<ArchiveStub, std::list<float>>(10);
}

TEST(STD_Containers, SerializeListOfLists) {
	TestSerializeType<ArchiveStub, std::list<std::list<int>>>();
}

TEST(STD_Containers, SerializeListAsClassMember) {
	using test_type = std::list<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::forward_list
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeForwardListOfInts) {
	TestSerializeType<ArchiveStub, std::forward_list<int>>();
}

TEST(STD_Containers, SerializeForwardListWhenTargetContainerIsNotEmpty) {
	TestLoadToNotEmptyContainer<ArchiveStub, std::forward_list<float>>(1);
	TestLoadToNotEmptyContainer<ArchiveStub, std::forward_list<float>>(10);
}

TEST(STD_Containers, SerializeForwardListOfForwardLists) {
	TestSerializeType<ArchiveStub, std::forward_list<std::forward_list<int>>>();
}

TEST(STD_Containers, SerializeForwardListAsClassMember) {
	using test_type = std::forward_list<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::queue
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeQueueOfFloats) {
	TestSerializeType<ArchiveStub, std::queue<float>>();
}

TEST(STD_Containers, SerializeQueueAsClassMember) {
	using test_type = std::queue<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::priority_queue
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializePriorityQueueOfFloats) {
	using test_type = std::priority_queue<float>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::stack
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeStackOfFloats) {
	TestSerializeType<ArchiveStub, std::stack<float>>();
}

TEST(STD_Containers, SerializeStackAsClassMember) {
	using test_type = std::stack<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::set
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeSetOfStrings) {
	TestSerializeType<ArchiveStub, std::set<std::string>>();
}

TEST(STD_Containers, SerializeSetOfSets) {
	TestSerializeType<ArchiveStub, std::set<std::set<int>>>();
}

TEST(STD_Containers, SerializeSetAsClassMember) {
	using test_type = std::set<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::unordered_set
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeUnorderedSetOfStrings) {
	TestSerializeType<ArchiveStub, std::unordered_set<std::string>>();
}

TEST(STD_Containers, SerializeUnorderedSetAsClassMember) {
	using test_type = std::unordered_set<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::unordered_multiset
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeUnorderedMultisetOfStrings) {
	TestSerializeType<ArchiveStub, std::unordered_multiset<std::string>>();
}

TEST(STD_Containers, SerializeUnorderedMultisetAsClassMember) {
	using test_type = std::unordered_multiset<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::multiset
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeMultiSetOfStrings) {
	TestSerializeType<ArchiveStub, std::multiset<std::string>>();
}

TEST(STD_Containers, SerializeMultiSetOfMultiSets) {
	TestSerializeType<ArchiveStub, std::multiset<std::multiset<int>>>();
}

TEST(STD_Containers, SerializeMultiSetAsClassMember) {
	using test_type = std::multiset<std::string>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::map
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeMapWithIntAsKey) {
	TestSerializeType<ArchiveStub, std::map<int, int>>();
}

TEST(STD_Containers, SerializeMapWithChronoDurationAsKey) {
	TestSerializeType<ArchiveStub, std::map<std::chrono::seconds, int>>();
}

TEST(STD_Containers, SerializeMapWithStringAsKey) {
	TestSerializeType<ArchiveStub, std::map<std::string, int>>();
	TestSerializeType<ArchiveStub, std::map<std::wstring, int>>();
}

TEST(STD_Containers, SerializeMapWithEnumAsKey) {
	TestSerializeType<ArchiveStub, std::map<TestEnum, std::string>>();
}

TEST(STD_Containers, SerializeMapWithClassAsKey) {
	TestSerializeType<ArchiveStub, std::map<TestPointClass, std::string>>();
}

TEST(STD_Containers, SerializeMapWithClassAsKeyAndClassAsValue) {
	TestSerializeType<ArchiveStub, std::map<TestPointClass, TestPointClass>>();
}

TEST(STD_Containers, SerializeMapOfMaps) {
	TestSerializeType<ArchiveStub, std::map<std::string, std::map<int, std::wstring>>>();
}

TEST(STD_Containers, SerializeMapAsClassMember) {
	using test_type = std::map<std::wstring, int>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

TEST(STD_Containers, SerializeMapThrowMismatchedTypesExceptionWhenLoadInvalidValue)
{
	// Save with negative number as map key
	TestClassWithSubType sourceObj(
		std::map<int32_t, int32_t>{{-23613, 4543534}}
	);
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(sourceObj, outputArchive);

	try
	{
		// Load to map with unsigned int as key type
		TestClassWithSubType<std::map<uint32_t, int32_t>> targetObj;
		BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

TEST(STD_Containers, SerializeMapThrowOverflowTypeExceptionWhenLoadTooBigKey)
{
	// Save with big number as map key
	TestClassWithSubType sourceObj(
		 std::map<int32_t, int32_t>{{10324678, 4543534}}
	);
	ArchiveStub::preferred_output_format outputArchive;
	BitSerializer::SaveObject<ArchiveStub>(sourceObj, outputArchive);

	try
	{
		// Load to map with small int as key type
		TestClassWithSubType<std::map<int8_t, int32_t>> targetObj;
		BitSerializer::LoadObject<ArchiveStub>(targetObj, outputArchive);
	}
	catch (const SerializationException& ex)
	{
		EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		return;
	}
	EXPECT_FALSE(true);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::unordered_map
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeUnorderedMapWithIntAsKey) {
	TestSerializeType<ArchiveStub, std::unordered_map<int, int>>();
}

TEST(STD_Containers, SerializeUnorderedMapWithStringAsKey) {
	TestSerializeType<ArchiveStub, std::unordered_map<std::string, int>>();
	TestSerializeType<ArchiveStub, std::unordered_map<std::wstring, int>>();
}

TEST(STD_Containers, SerializeUnorderedMapWithEnumAsKey) {
	TestSerializeType<ArchiveStub, std::unordered_map<TestEnum, std::string>>();
}

TEST(STD_Containers, SerializeUnorderedMapWithClassAsKey) {
	TestSerializeType<ArchiveStub, std::unordered_map<TestPointClass, std::string>>();
}

TEST(STD_Containers, SerializeUnorderedMapWithClassAsKeyAndClassAsValue) {
	TestSerializeType<ArchiveStub, std::unordered_map<TestPointClass, TestPointClass>>();
}

TEST(STD_Containers, SerializeUnorderedMapOfUnorderedMaps) {
	TestSerializeType<ArchiveStub, std::unordered_map<std::string, std::unordered_map<int, std::wstring>>>();
}

TEST(STD_Containers, SerializeUnorderedMapAsClassMember) {
	using test_type = std::unordered_map<std::wstring, int>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::unordered_multimap
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeUnorderedMultimapWithIntAsKey) {
	TestSerializeType<ArchiveStub, std::unordered_multimap<int, int>>();
}

TEST(STD_Containers, SerializeUnorderedMultimapWithStringAsKey) {
	TestSerializeType<ArchiveStub, std::unordered_multimap<std::string, int>>();
	TestSerializeType<ArchiveStub, std::unordered_multimap<std::wstring, int>>();
}

TEST(STD_Containers, SerializeUnorderedMultimapWithEnumAsKey) {
	TestSerializeType<ArchiveStub, std::unordered_multimap<TestEnum, std::string>>();
}

TEST(STD_Containers, SerializeUnorderedMultimapWithClassAsKey) {
	TestSerializeType<ArchiveStub, std::unordered_multimap<TestPointClass, std::string>>();
}

TEST(STD_Containers, SerializeUnorderedMultimapWithClassAsKeyAndClassAsValue) {
	TestSerializeType<ArchiveStub, std::unordered_multimap<TestPointClass, TestPointClass>>();
}

TEST(STD_Containers, SerializeUnorderedMultimapOfUnorderedMaps) {
	TestSerializeType<ArchiveStub, std::unordered_multimap<std::string, std::unordered_multimap<int, std::wstring>>>();
}

TEST(STD_Containers, SerializeUnorderedMultimapAsClassMember) {
	using test_type = std::unordered_multimap<std::wstring, int>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::multimap
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeMultimapMapWithIntAsKey) {
	using test_type = std::map<int, int>;
	TestSerializeType<ArchiveStub, test_type>();
}

TEST(STD_Containers, SerializeMultimapMapAsClassMember) {
	using test_type = std::multimap<int, int>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::valarray
//-----------------------------------------------------------------------------
TEST(STD_Containers, SerializeValarrayWithIntAsKey) {
	using test_type = std::valarray<int>;
	TestSerializeType<ArchiveStub, test_type>();
}

TEST(STD_Containers, SerializeValarrayAsClassMember) {
	using test_type = std::valarray<float>;
	TestSerializeType<ArchiveStub, TestClassWithSubType<test_type>>();
}
