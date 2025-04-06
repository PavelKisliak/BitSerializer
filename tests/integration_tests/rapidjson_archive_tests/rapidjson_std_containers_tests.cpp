/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "bitserializer/rapidjson_archive.h"

// STD containers
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

using BitSerializer::Json::RapidJson::JsonArchive;

//-----------------------------------------------------------------------------
// Smoke tests of STD container serialization (more detailed tests in "unit_tests/std_types_tests")
//-----------------------------------------------------------------------------
TEST(RapidJsonArchive, SerializeStdContainers)
{
	TestSerializeType<JsonArchive, std::array<int, 7>>();
	TestSerializeType<JsonArchive, std::vector<int>>();
	TestSerializeType<JsonArchive, std::deque<int>>();
	TestSerializeType<JsonArchive, std::bitset<10>>();
	TestSerializeType<JsonArchive, std::forward_list<int>>();
	TestSerializeType<JsonArchive, std::queue<float>>();
	TestSerializeType<JsonArchive, std::priority_queue<float>>();
	TestSerializeType<JsonArchive, std::stack<float>>();
	TestSerializeType<JsonArchive, std::set<std::string>>();
	TestSerializeType<JsonArchive, std::unordered_set<std::string>>();
	TestSerializeType<JsonArchive, std::unordered_multiset<std::string>>();
	TestSerializeType<JsonArchive, std::multiset<std::string>>();
	TestSerializeType<JsonArchive, std::map<int, int>>();
	TestSerializeType<JsonArchive, std::multimap<int, int>>();
	TestSerializeType<JsonArchive, std::unordered_map<int, int>>();
	TestSerializeType<JsonArchive, std::unordered_multimap<int, int>>();
	TestSerializeType<JsonArchive, std::valarray<int>>();
}
