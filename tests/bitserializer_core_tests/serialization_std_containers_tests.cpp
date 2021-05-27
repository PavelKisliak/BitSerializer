/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "test_helpers/common_test_methods.h"
#include "test_helpers/archive_stub.h"

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

//-----------------------------------------------------------------------------
// Tests of serialization for STL containers.
// As containers serialization used methods for serialization of base types,
// there is no need to write special tests for other types of archives.
//-----------------------------------------------------------------------------

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for std::array
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeArrayOfInts) {
	TestSerializeStlContainer<ArchiveStub, std::array<int, 7>>();
}

TEST(STL_Containers, SerializeArrayOfArrays) {
	TestSerializeStlContainer<ArchiveStub, std::array<std::array<int, 7>, 3>>();
}

TEST(STL_Containers, SerializeArrayAsClassMember) {
	using test_type = std::array<std::string, 7>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::vector
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeVectorOfInts) {
	TestSerializeStlContainer<ArchiveStub, std::vector<int>>();
}

TEST(STL_Containers, SerializeVectorOfVectors) {
	TestSerializeStlContainer<ArchiveStub, std::vector<std::vector<int>>>();
}

TEST(STL_Containers, SerializeVectorAsClassMember) {
	using test_type = std::vector<std::string>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

TEST(STL_Containers, SerializeVectorOBooleans) {
	TestSerializeStlContainer<ArchiveStub, std::vector<bool>>();
}

TEST(STL_Containers, SerializeVectorOBooleansAsClassMember) {
	using test_type = std::vector<bool>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::deque
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeDequeOfFloats) {
	TestSerializeStlContainer<ArchiveStub, std::deque<float>>();
}

TEST(STL_Containers, SerializeDequeOfDeques) {
	TestSerializeStlContainer<ArchiveStub, std::deque<std::deque<int>>>();
}

TEST(STL_Containers, SerializeDequeAsClassMember) {
	using test_type = std::deque<std::string>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::bitset
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeBitset) {
	TestSerializeStlContainer<ArchiveStub, std::bitset<10>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::list
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeListOfInts) {
	TestSerializeStlContainer<ArchiveStub, std::list<int>>();
}

TEST(STL_Containers, SerializeListOfLists) {
	TestSerializeStlContainer<ArchiveStub, std::list<std::list<int>>>();
}

TEST(STL_Containers, SerializeListAsClassMember) {
	using test_type = std::list<std::string>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::forward_list
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeForwardListOfInts) {
	TestSerializeStlContainer<ArchiveStub, std::forward_list<int>>();
}

TEST(STL_Containers, SerializeForwardListOfForwardLists) {
	TestSerializeStlContainer<ArchiveStub, std::forward_list<std::forward_list<int>>>();
}

TEST(STL_Containers, SerializeForwardListAsClassMember) {
	using test_type = std::forward_list<std::string>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::queue
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeQueueOfFloats) {
	TestSerializeStlContainer<ArchiveStub, std::queue<float>>();
}

TEST(STL_Containers, SerializeQueueAsClassMember) {
	using test_type = std::queue<std::string>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::priority_queue
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializePriorityQueueOfFloats)
{
	TestSerializeStlContainer<ArchiveStub, std::priority_queue<float>>([](
		const std::priority_queue<float>& lhs, const std::priority_queue<float>& rhs)
	{
		EXPECT_EQ(BitSerializer::Detail::GetBaseContainer(lhs), BitSerializer::Detail::GetBaseContainer(rhs));
	});
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::stack
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeStackOfFloats) {
	TestSerializeStlContainer<ArchiveStub, std::stack<float>>();
}

TEST(STL_Containers, SerializeStackAsClassMember) {
	using test_type = std::stack<std::string>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::set
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeSetOfStrings) {
	TestSerializeStlContainer<ArchiveStub, std::set<std::string>>();
}

TEST(STL_Containers, SerializeSetOfSets) {
	TestSerializeStlContainer<ArchiveStub, std::set<std::set<int>>>();
}

TEST(STL_Containers, SerializeSetAsClassMember) {
	using test_type = std::set<std::string>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::unordered_set
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeUnorderedSetOfStrings) {
	TestSerializeStlContainer<ArchiveStub, std::unordered_set<std::string>>();
}

TEST(STL_Containers, SerializeUnorderedSetAsClassMember) {
	using test_type = std::unordered_set<std::string>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::multiset
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeMultiSetOfStrings) {
	TestSerializeStlContainer<ArchiveStub, std::multiset<std::string>>();
}

TEST(STL_Containers, SerializeMultiSetOfMultiSets) {
	TestSerializeStlContainer<ArchiveStub, std::multiset<std::multiset<int>>>();
}

TEST(STL_Containers, SerializeMultiSetAsClassMember) {
	using test_type = std::multiset<std::string>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::map
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeMapWithIntAsKey) {
	TestSerializeStlContainer<ArchiveStub, std::map<int, int>>();
}

TEST(STL_Containers, SerializeMapWithStringAsKey) {
	TestSerializeStlContainer<ArchiveStub, std::map<std::string, int>>();
	TestSerializeStlContainer<ArchiveStub, std::map<std::wstring, int>>();
}

TEST(STL_Containers, SerializeMapWithEnumAsKey) {
	TestSerializeStlContainer<ArchiveStub, std::map<TestEnum, std::string>>();
}

TEST(STL_Containers, SerializeMapWithClassAsKey) {
	TestSerializeStlContainer<ArchiveStub, std::map<TestPointClass, std::string>>();
}

TEST(STL_Containers, SerializeMapWithClassAsKeyAndClassAsValue) {
	TestSerializeStlContainer<ArchiveStub, std::map<TestPointClass, TestPointClass>>();
}

TEST(STL_Containers, SerializeMapOfMaps) {
	TestSerializeStlContainer<ArchiveStub, std::map<std::string, std::map<int, std::wstring>>>();
}

TEST(STL_Containers, SerializeMapAsClassMember) {
	using test_type = std::map<std::wstring, int>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::unordered_map
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeUnorderedMapWithIntAsKey) {
	TestSerializeStlContainer<ArchiveStub, std::unordered_map<int, int>>();
}

TEST(STL_Containers, SerializeUnorderedMapWithStringAsKey) {
	TestSerializeStlContainer<ArchiveStub, std::unordered_map<std::string, int>>();
	TestSerializeStlContainer<ArchiveStub, std::unordered_map<std::wstring, int>>();
}

TEST(STL_Containers, SerializeUnorderedMapWithEnumAsKey) {
	TestSerializeStlContainer<ArchiveStub, std::unordered_map<TestEnum, std::string>>();
}

TEST(STL_Containers, SerializeUnorderedMapWithClassAsKey) {
	TestSerializeStlContainer<ArchiveStub, std::unordered_map<TestPointClass, std::string>>();
}

TEST(STL_Containers, SerializeUnorderedMapWithClassAsKeyAndClassAsValue) {
	TestSerializeStlContainer<ArchiveStub, std::unordered_map<TestPointClass, TestPointClass>>();
}

TEST(STL_Containers, SerializeUnorderedMapOfUnorderedMaps) {
	TestSerializeStlContainer<ArchiveStub, std::unordered_map<std::string, std::unordered_map<int, std::wstring>>>();
}

TEST(STL_Containers, SerializeUnorderedMapAsClassMember) {
	using test_type = std::unordered_map<std::wstring, int>;
	TestSerializeClass<ArchiveStub>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::multimap
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeMultimapMapWithIntAsKey) {
	using test_type = std::multimap<int, int>;
	TestSerializeStlContainer<ArchiveStub, test_type>(AssertMultimap<test_type>);
}

TEST(STL_Containers, SerializeMultimapMapAsClassMember) {
	using test_type = std::multimap<int, int>;
	auto fixture = TestClassWithSubType<test_type>(AssertMultimap<test_type>);
	BuildFixture(fixture);
	TestSerializeClass<ArchiveStub>(fixture);
}
