/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "pch.h"
#include "common_test_entities.h"
#include "bitserializer/bit_serializer.h"
#include "bitserializer/archives/json_restcpp_archive.h"

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
	TestSerializeStlContainer<JsonArchive, std::array<int, 7>>();
}

TEST(STL_Containers, SerializeArrayOfArrays) {
	TestSerializeStlContainer<JsonArchive, std::array<std::array<int, 7>, 3>>();
}

TEST(STL_Containers, SerializeArrayAsClassMember) {
	using test_type = std::array<std::string, 7>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::vector
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeVectorOfInts) {
	TestSerializeStlContainer<JsonArchive, std::vector<int>>();
}

TEST(STL_Containers, SerializeVectorOfVectors) {
	TestSerializeStlContainer<JsonArchive, std::vector<std::vector<int>>>();
}

TEST(STL_Containers, SerializeVectorAsClassMember) {
	using test_type = std::vector<std::string>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubType<test_type>>());
}

TEST(STL_Containers, SerializeVectorOBooleans) {
	TestSerializeStlContainer<JsonArchive, std::vector<bool>>();
}

TEST(STL_Containers, SerializeVectorOBooleansAsClassMember) {
	using test_type = std::vector<bool>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::deque
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeDequeOfFloats) {
	TestSerializeStlContainer<JsonArchive, std::deque<float>>();
}

TEST(STL_Containers, SerializeDequeOfDeques) {
	TestSerializeStlContainer<JsonArchive, std::deque<std::deque<int>>>();
}

TEST(STL_Containers, SerializeDequeAsClassMember) {
	using test_type = std::deque<std::string>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::list
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeListOfInts) {
	TestSerializeStlContainer<JsonArchive, std::list<int>>();
}

TEST(STL_Containers, SerializeListOfLists) {
	TestSerializeStlContainer<JsonArchive, std::list<std::list<int>>>();
}

TEST(STL_Containers, SerializeListAsClassMember) {
	using test_type = std::list<std::string>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::forward_list
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeForwardListOfInts) {
	TestSerializeStlContainer<JsonArchive, std::forward_list<int>>();
}

TEST(STL_Containers, SerializeForwardListOfForwardLists) {
	TestSerializeStlContainer<JsonArchive, std::forward_list<std::forward_list<int>>>();
}

TEST(STL_Containers, SerializeForwardListAsClassMember) {
	using test_type = std::forward_list<std::string>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::set
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeSetOfStrings) {
	TestSerializeStlContainer<JsonArchive, std::set<std::string>>();
}

TEST(STL_Containers, SerializeSetOfSets) {
	TestSerializeStlContainer<JsonArchive, std::set<std::set<int>>>();
}

TEST(STL_Containers, SerializeSetAsClassMember) {
	using test_type = std::set<std::string>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::map
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeMapWithIntAsKey) {
	TestSerializeStlContainer<JsonArchive, std::map<int, int>>();
}

TEST(STL_Containers, SerializeMapWithStringAsKey) {
	TestSerializeStlContainer<JsonArchive, std::map<std::string, int>>();
	TestSerializeStlContainer<JsonArchive, std::map<std::wstring, int>>();
}

TEST(STL_Containers, SerializeMapWithEnumAsKey) {
	TestSerializeStlContainer<JsonArchive, std::map<TestEnum, std::string>>();
}

TEST(STL_Containers, SerializeMapWithClassAsKey) {
	TestSerializeStlContainer<JsonArchive, std::map<TestPointClass, std::string>>();
}

TEST(STL_Containers, SerializeMapWithClassAsKeyAndClassAsValue) {
	TestSerializeStlContainer<JsonArchive, std::map<TestPointClass, TestPointClass>>();
}

TEST(STL_Containers, SerializeMapOfMaps) {
	TestSerializeStlContainer<JsonArchive, std::map<std::string, std::map<int, std::wstring>>>();
}

TEST(STL_Containers, SerializeMapAsClassMember) {
	using test_type = std::map<std::wstring, int>;
	TestSerializeClass<JsonArchive>(BuildFixture<TestClassWithSubType<test_type>>());
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::multimap
//-----------------------------------------------------------------------------
TEST(STL_Containers, SerializeMultimapMapWithIntAsKey) {
	using test_type = std::multimap<int, int>;
	TestSerializeStlContainer<JsonArchive, test_type>(AssertMultimap<test_type>);
}

TEST(STL_Containers, SerializeMultimapMapAsClassMember) {
	using test_type = std::multimap<int, int>;
	auto fixture = TestClassWithSubType<test_type>(AssertMultimap<test_type>);
	BuildFixture(fixture);
	TestSerializeClass<JsonArchive>(fixture);
}
