/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "bitserializer/msgpack_archive.h"

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

using namespace BitSerializer;
using BitSerializer::MsgPack::MsgPackArchive;

//-----------------------------------------------------------------------------
// Test serialization std::map
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeMapWithIntAsKey)
{
	TestSerializeType<MsgPackArchive>(std::map<int8_t, int>{
		{ std::numeric_limits<int8_t>::min(), 1 },
		{ std::numeric_limits<int8_t>::max(), 2 }
	});

	TestSerializeType<MsgPackArchive>(std::map<int64_t, int>{
		{ std::numeric_limits<int64_t>::min(), 1 },
		{ std::numeric_limits<int64_t>::max(), 2 }
	});
}

TEST(MsgPackArchive, SerializeMapWithUnsignedIntAsKey)
{
	TestSerializeType<MsgPackArchive>(std::map<uint8_t, std::string>{
		{ std::numeric_limits<uint8_t>::min(), "1" },
		{ std::numeric_limits<uint8_t>::max(), "2" }
	});

	TestSerializeType<MsgPackArchive>(std::map<uint64_t, std::string>{
		{ std::numeric_limits<uint64_t>::min(), "1" },
		{ std::numeric_limits<uint64_t>::max(), "2" }
	});
}

TEST(MsgPackArchive, SerializeMapWithfloatAsKey) {
	TestSerializeType<MsgPackArchive, std::map<float, int>>();
	TestSerializeType<MsgPackArchive, std::map<double, std::string>>();
}

TEST(MsgPackArchive, SerializeMapWithChronoDurationAsKey) {
	TestSerializeType<MsgPackArchive, std::map<std::chrono::nanoseconds, int>>();
	TestSerializeType<MsgPackArchive, std::map<std::chrono::nanoseconds, std::u16string>>();
}

TEST(MsgPackArchive, SerializeMapWithChronoTimePointAsKey) {
	TestSerializeType<MsgPackArchive, std::map<std::chrono::system_clock::time_point, int>>();
	TestSerializeType<MsgPackArchive, std::map<std::chrono::system_clock::time_point, std::u32string>>();
}

TEST(MsgPackArchive, SerializeMapWithStringAsKey) {
	TestSerializeType<MsgPackArchive, std::map<std::string, int>>();
	TestSerializeType<MsgPackArchive, std::map<std::wstring, std::string>>();
}

//-----------------------------------------------------------------------------
// Smoke tests of STD container serialization (more detailed tests in "unit_tests/std_types_tests")
//-----------------------------------------------------------------------------
TEST(MsgPackArchive, SerializeStdContainers)
{
	TestSerializeType<MsgPackArchive, std::array<int, 7>>();
	TestSerializeType<MsgPackArchive, std::vector<int>>();
	TestSerializeType<MsgPackArchive, std::deque<int>>();
	TestSerializeType<MsgPackArchive, std::bitset<10>>();
	TestSerializeType<MsgPackArchive, std::forward_list<int>>();
	TestSerializeType<MsgPackArchive, std::queue<float>>();
	TestSerializeType<MsgPackArchive, std::priority_queue<float>>();
	TestSerializeType<MsgPackArchive, std::stack<float>>();
	TestSerializeType<MsgPackArchive, std::set<std::string>>();
	TestSerializeType<MsgPackArchive, std::unordered_set<std::string>>();
	TestSerializeType<MsgPackArchive, std::unordered_multiset<std::string>>();
	TestSerializeType<MsgPackArchive, std::multiset<std::string>>();
	TestSerializeType<MsgPackArchive, std::map<int, int>>();

	TestSerializeType<MsgPackArchive, std::map<std::string, int>>(std::map<std::string, int>{
		{UTF8("нода_0"), 0}, { "node_1", 1 }, { "node_2", 2 }, { "node_3", 3 }
	});

	TestSerializeType<MsgPackArchive, std::multimap<int, int>>();
	TestSerializeType<MsgPackArchive, std::unordered_map<int, int>>();
	TestSerializeType<MsgPackArchive, std::unordered_multimap<int, int>>();
	TestSerializeType<MsgPackArchive, std::valarray<int>>();
}
