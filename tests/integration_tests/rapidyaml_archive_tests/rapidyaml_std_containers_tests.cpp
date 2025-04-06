/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "bitserializer/rapidyaml_archive.h"

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

using BitSerializer::Yaml::RapidYaml::YamlArchive;

//-----------------------------------------------------------------------------
// Smoke tests of STD container serialization (more detailed tests in "unit_tests/std_types_tests")
//-----------------------------------------------------------------------------
TEST(RapidYamlArchive, SerializeStdContainers)
{
	TestSerializeType<YamlArchive, std::array<int, 7>>();
	TestSerializeType<YamlArchive, std::vector<int>>();
	TestSerializeType<YamlArchive, std::deque<int>>();
	TestSerializeType<YamlArchive, std::bitset<10>>();
	TestSerializeType<YamlArchive, std::forward_list<int>>();
	TestSerializeType<YamlArchive, std::queue<float>>();
	TestSerializeType<YamlArchive, std::priority_queue<float>>();
	TestSerializeType<YamlArchive, std::stack<float>>();
	TestSerializeType<YamlArchive, std::set<std::string>>();
	TestSerializeType<YamlArchive, std::unordered_set<std::string>>();
	TestSerializeType<YamlArchive, std::unordered_multiset<std::string>>();
	TestSerializeType<YamlArchive, std::multiset<std::string>>();
	TestSerializeType<YamlArchive, std::map<int, int>>();
	TestSerializeType<YamlArchive, std::multimap<int, int>>();
	TestSerializeType<YamlArchive, std::unordered_map<int, int>>();
	TestSerializeType<YamlArchive, std::unordered_multimap<int, int>>();
	TestSerializeType<YamlArchive, std::valarray<int>>();
}
