/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "bitserializer/pugixml_archive.h"

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

using BitSerializer::Xml::PugiXml::XmlArchive;

//-----------------------------------------------------------------------------
// Smoke tests of STD container serialization (more detailed tests in "unit_tests/std_types_tests")
//-----------------------------------------------------------------------------
TEST(PugiXmlArchive, SerializeStdContainers)
{
	TestSerializeType<XmlArchive, std::array<int, 7>>();
	TestSerializeType<XmlArchive, std::vector<int>>();
	TestSerializeType<XmlArchive, std::deque<int>>();
	TestSerializeType<XmlArchive, std::bitset<10>>();
	TestSerializeType<XmlArchive, std::forward_list<int>>();
	TestSerializeType<XmlArchive, std::queue<float>>();
	TestSerializeType<XmlArchive, std::priority_queue<float>>();
	TestSerializeType<XmlArchive, std::stack<float>>();
	TestSerializeType<XmlArchive, std::set<std::string>>();
	TestSerializeType<XmlArchive, std::unordered_set<std::string>>();
	TestSerializeType<XmlArchive, std::unordered_multiset<std::string>>();
	TestSerializeType<XmlArchive, std::multiset<std::string>>();
	// PugiXml does not support node names in UTF-8
	TestSerializeType<XmlArchive, std::map<std::string, std::string>>(std::map<std::string, std::string>{
		{ "node_1", UTF8("значение_1") }, { "node_2", UTF8("значение_2") }
	});
	TestSerializeType<XmlArchive, std::multimap<std::string, std::string>>(std::multimap<std::string, std::string>{
		{"node", UTF8("value")}, { "node", UTF8("значение") }, { "node", UTF8("value") }
	});
	TestSerializeType<XmlArchive, std::unordered_map<std::string, std::string>>(std::unordered_map<std::string, std::string>{
		{"node_1", UTF8("value_1")}, { "node_2", UTF8("value_2") }, { "node_3", UTF8("value_3") }
	});
	TestSerializeType<XmlArchive, std::unordered_multimap<std::string, std::string>>(std::unordered_multimap<std::string, std::string>{
		{"node", UTF8("value")}, { "node", UTF8("значение") }, { "node", UTF8("value") }
	});
	TestSerializeType<XmlArchive, std::valarray<int>>();
}
