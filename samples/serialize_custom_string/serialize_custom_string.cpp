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

	[[nodiscard]] const char* data() const noexcept { return mString.data(); }
	[[nodiscard]] size_t size() const noexcept { return mString.size(); }

	// Required methods for conversion from/to std::string (can be implemented as external functions)
	[[nodiscard]] std::string ToString() const { return mString; }
	void FromString(std::string_view str) { mString = str; }

private:
	std::string mString;
};

// Serializes CMyString with key
template <class TArchive, typename TKey>
bool Serialize(TArchive& archive, TKey&& key, CMyString& value)
{
	if constexpr (TArchive::IsLoading())
	{
		std::string_view stringView;
		if (Detail::SerializeString(archive, std::forward<TKey>(key), stringView))
		{
			value.FromString(stringView);
			return true;
		}
	}
	else
	{
		std::string_view stringView(value.data(), value.size());
		return Detail::SerializeString(archive, std::forward<TKey>(key), stringView);
	}
	return false;
}

// Serializes CMyString without key
template <class TArchive>
bool Serialize(TArchive& archive, CMyString& value)
{
	if constexpr (TArchive::IsLoading())
	{
		std::string_view stringView;
		if (Detail::SerializeString(archive, stringView))
		{
			value.FromString(stringView);
			return true;
		}
		return false;
	}
	else
	{
		std::string_view stringView(value.data(), value.size());
		return Detail::SerializeString(archive, stringView);
	}
}

int main()	// NOLINT(bugprone-exception-escape)
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
