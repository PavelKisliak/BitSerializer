/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/archive_stub.h"

// PMR containers on Apple is supported since C++ 20
#if defined(__cpp_lib_memory_resource) && (!defined __apple_build_version__ || ((defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || __cplusplus >= 202002L))

#include "bitserializer/types/std/vector.h"
#include "bitserializer/types/std/deque.h"
#include "bitserializer/types/std/list.h"
#include "bitserializer/types/std/forward_list.h"
#include "bitserializer/types/std/set.h"
#include "bitserializer/types/std/unordered_set.h"
#include "bitserializer/types/std/map.h"
#include "bitserializer/types/std/unordered_map.h"

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr containers.
//-----------------------------------------------------------------------------

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::vector
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeVectorOfInts) {
	TestSerializePmrType<ArchiveStub, std::pmr::vector<int>>();
}

TEST(STD_PMR_Containers, SerializeVectorOfStrings) {
	TestSerializePmrType<ArchiveStub, std::pmr::vector<std::pmr::string>>();
}

TEST(STD_PMR_Containers, SerializeVectorOfVectors) {
	TestSerializePmrType<ArchiveStub, std::pmr::vector<std::pmr::vector<int>>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::deque
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeDequeOfFloats) {
	TestSerializePmrType<ArchiveStub, std::pmr::deque<int>>();
}

TEST(STD_PMR_Containers, SerializeDequeOfStrings) {
	TestSerializePmrType<ArchiveStub, std::pmr::deque<std::pmr::string>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::list
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeListOfInts) {
	TestSerializePmrType<ArchiveStub, std::pmr::list<int>>();
}

TEST(STD_PMR_Containers, SerializeListOfStrings) {
	TestSerializePmrType<ArchiveStub, std::pmr::list<std::pmr::string>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::forward_list
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeForwardListOfInts) {
	TestSerializePmrType<ArchiveStub, std::pmr::forward_list<int>>();
}

TEST(STD_PMR_Containers, SerializeForwardListOfStrings) {
	TestSerializePmrType<ArchiveStub, std::pmr::forward_list<std::pmr::string>>();
}

TEST(STD_PMR_Containers, SerializeForwardListOfForwardLists) {
	TestSerializePmrType<ArchiveStub, std::pmr::forward_list<std::pmr::forward_list<int>>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::set
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeSetOfInts) {
	TestSerializePmrType<ArchiveStub, std::pmr::set<int>>();
}

TEST(STD_PMR_Containers, SerializeSetOfStrings) {
	TestSerializePmrType<ArchiveStub, std::pmr::set<std::pmr::string>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::unordered_set
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeUnorderedSetOfInts) {
	TestSerializePmrType<ArchiveStub, std::pmr::unordered_set<int>>();
}

TEST(STD_PMR_Containers, SerializeUnorderedSetOfStrings) {
	TestSerializePmrType<ArchiveStub, std::pmr::unordered_set<std::pmr::string>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::unordered_multiset
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeUnorderedMultisetOfInts) {
	TestSerializePmrType<ArchiveStub, std::pmr::unordered_multiset<int>>();
}

TEST(STD_PMR_Containers, SerializeUnorderedMultisetOfStrings) {
	TestSerializePmrType<ArchiveStub, std::pmr::unordered_multiset<std::pmr::string>>();
}
//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::multiset
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeMultiSetOfInts) {
	TestSerializePmrType<ArchiveStub, std::pmr::multiset<int>>();
}

TEST(STD_PMR_Containers, SerializeMultiSetOfStrings) {
	TestSerializePmrType<ArchiveStub, std::pmr::multiset<std::pmr::string>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::map
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeMapWithIntAsKey) {
	TestSerializePmrType<ArchiveStub, std::pmr::map<int, int>>();
}

TEST(STD_PMR_Containers, SerializeMapWithStringAsKey) {
	TestSerializePmrType<ArchiveStub, std::pmr::map<std::pmr::string, std::pmr::string>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::multimap
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeMultimapMapWithIntAsKey) {
	TestSerializePmrType<ArchiveStub, std::pmr::multimap<int, int>>();
}

TEST(STD_PMR_Containers, SerializeMultimapMapWithStringAsKey) {
	TestSerializePmrType<ArchiveStub, std::pmr::multimap<std::pmr::string, std::pmr::string>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::pmr::unordered_map
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeUnorderedMapWithIntAsKey) {
	TestSerializePmrType<ArchiveStub, std::pmr::unordered_map<int, int>>();
}

TEST(STD_PMR_Containers, SerializeUnorderedMapWithStringAsKey) {
	TestSerializePmrType<ArchiveStub, std::pmr::unordered_map<std::pmr::string, std::pmr::string>>();
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::unordered_multimap
//-----------------------------------------------------------------------------
TEST(STD_PMR_Containers, SerializeUnorderedMultimapWithIntAsKey) {
	TestSerializePmrType<ArchiveStub, std::pmr::unordered_multimap<int, int>>();
}

TEST(STD_PMR_Containers, SerializeUnorderedMultimapWithStringAsKey) {
	TestSerializePmrType<ArchiveStub, std::pmr::unordered_multimap<std::pmr::string, std::pmr::string>>();
}
#endif
