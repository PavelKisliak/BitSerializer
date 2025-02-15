#include <iostream>
#include <iomanip>

#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/msgpack_archive.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/types/std/map.h"
#include "bitserializer/types/std/chrono.h"

using namespace std::chrono;
using namespace BitSerializer;
using JsonArchive = Json::RapidJson::JsonArchive;
using MsgPackArchive = MsgPack::MsgPackArchive;

// Columns width
constexpr int ArchiveColumnWidth = 8;
constexpr int DataColumnWidth = 90;
constexpr int SizeColumnWidth = 5;

struct TestObject
{
	std::string string;
	int integer;
	bool boolean;
	double floating;
	int array[5];
};

template <class TArchive>
void SerializeObject(TArchive& archive, TestObject& obj)
{
	archive << KeyValue("string", obj.string);
	archive << KeyValue("integer", obj.integer);
	archive << KeyValue("boolean", obj.boolean);
	archive << KeyValue("floating", obj.floating);
	archive << KeyValue("array", obj.array);
}

std::string PrintAsHexString(const std::string& data)
{
	constexpr char hexChars[] = "0123456789ABCDEF";
	std::string result;
	for (const char ch : data)
	{
		if (!result.empty()) result.push_back(' ');
		result.append({ hexChars[(ch & 0xF0) >> 4], hexChars[(ch & 0x0F) >> 0]});
	}
	return result;
}

template <typename TArchive, typename TTestValue>
void TestSaveAs(std::string_view archiveName, TTestValue& testValue)
{
	auto result = BitSerializer::SaveObject<TArchive>(testValue);
	const auto resultSize = std::to_string(result.size());
	if constexpr (TArchive::is_binary) result = PrintAsHexString(result);

	const size_t lines = (result.size() / DataColumnWidth) + (result.size() % DataColumnWidth ? 1 : 0);
	for (size_t i = 0; i < lines; ++i)
	{
		std::cout << std::setw(ArchiveColumnWidth) << std::right;
		std::cout << (i == 0 ? archiveName : "");
		std::cout << " | " << std::setw(DataColumnWidth) << std::left << result.substr(i * DataColumnWidth, DataColumnWidth);
		std::cout << " | " << (i == 0 ? resultSize : "") << std::endl;
	}
}

int main()	// NOLINT(bugprone-exception-escape)
{
	constexpr size_t tableWidth = ArchiveColumnWidth + DataColumnWidth + SizeColumnWidth + 5;
	std::string splitLine(tableWidth, '-');
	std::cout << splitLine << std::endl;
	std::cout << std::setw(ArchiveColumnWidth) << std::right << "Archive" << " | ";
	std::cout << std::setw(DataColumnWidth) << std::left << "Serialized data" << " | Size" << std::endl;
	std::cout << splitLine << std::endl;

	// Case 1: Binary array
	std::vector<char> binArray = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };
	TestSaveAs<JsonArchive>("JSON", binArray);
	TestSaveAs<MsgPackArchive>("MsgPack", binArray);
	std::cout << splitLine << std::endl;

	// Case 2: Map with timepoint as key
	using TimePointSec = time_point<system_clock, seconds>;
	constexpr TimePointSec tp2044_01_01(seconds(2335219200));
	std::map<TimePointSec, std::string> eventsMap{
		{ tp2044_01_01 + minutes(30), "Event1"},
		{ tp2044_01_01 + minutes(65), "Event2"}
	};
	TestSaveAs<JsonArchive>("JSON", eventsMap);
	TestSaveAs<MsgPackArchive>("MsgPack", eventsMap);
	std::cout << splitLine << std::endl;

	// Case 3: Test object with mixed data
	TestObject testObject = { "Hello world!", 1925, true, 3.141592654, { 100,200,300,400,500 } };
	TestSaveAs<JsonArchive>("JSON", testObject);
	TestSaveAs<MsgPackArchive>("MsgPack", testObject);
	std::cout << splitLine << std::endl;

	return 0;
}
