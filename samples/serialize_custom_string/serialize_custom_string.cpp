#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/types/std/map.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

// Some custom string type
class CMyString
{
public:
	CMyString() = default;
	CMyString(const char* str) : mString(str) { }

	bool operator<(const CMyString& rhs) const { return this->mString < rhs.mString; }

	// Required methods for conversion from/to std::string (can be implemented as external functions)
	std::string ToString() const { return mString; }
	void FromString(std::string_view str) { mString = str; }

private:
	std::string mString;
};

// Serializes CMyString with key
template <class TArchive, typename TKey>
bool Serialize(TArchive& archive, TKey&& key, CMyString& value)
{
	constexpr auto hasStringWithKeySupport = can_serialize_value_with_key_v<TArchive, std::string, TKey>;
	static_assert(hasStringWithKeySupport, "BitSerializer. The archive doesn't support serialize string type with key on this level.");

	if constexpr (hasStringWithKeySupport)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string str;
			if (archive.SerializeValue(std::forward<TKey>(key), str))
			{
				value.FromString(str);
				return true;
			}
		}
		else
		{
			std::string str = value.ToString();
			return archive.SerializeValue(std::forward<TKey>(key), str);
		}
	}
	return false;
}

// Serializes CMyString without key
template <class TArchive>
bool Serialize(TArchive& archive, CMyString& value)
{
	constexpr auto hasStringSupport = can_serialize_value_v<TArchive, std::string>;
	static_assert(hasStringSupport, "BitSerializer. The archive doesn't support serialize string type without key on this level.");

	if constexpr (hasStringSupport)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string str;
			if (archive.SerializeValue(str))
			{
				value.FromString(str);
				return true;
			}
		}
		else
		{
			std::string str = value.ToString();
			return archive.SerializeValue(str);
		}
	}
	return false;
}

int main()
{
	// Save list of custom strings to JSON
	std::vector<CMyString> srcStrList = { "Red", "Green", "Blue" };
	std::string jsonResult;
	SerializationOptions serializationOptions;
	serializationOptions.formatOptions.enableFormat = true;
	BitSerializer::SaveObject<JsonArchive>(srcStrList, jsonResult, serializationOptions);
	std::cout << "Saved JSON: " << jsonResult << std::endl;

	// Load JSON-object to std::map based on custom strings
	std::map<CMyString, CMyString> mapResult;
	const std::string srcJson = R"({ "Background": "Blue", "PenColor": "White", "PenSize": "3", "PenOpacity": "50" })";
	BitSerializer::LoadObject<JsonArchive>(mapResult, srcJson);
	std::cout << std::endl << "Loaded map: " << std::endl;
	for (const auto& val : mapResult)
	{
		std::cout << "\t" << val.first.ToString() << ": " << val.second.ToString() << std::endl;
	}

	return 0;
}
