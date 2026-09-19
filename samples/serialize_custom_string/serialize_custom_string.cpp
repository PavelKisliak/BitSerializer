#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/types/std/vector.h"
#include "bitserializer/types/std/map.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

// Some custom string type
class MyString
{
public:
	MyString() = default;
	MyString(const char* str) : mString(str) { }

	bool operator<(const MyString& rhs) const { return this->mString < rhs.mString; }

	[[nodiscard]] const char* Data() const noexcept { return mString.data(); }
	[[nodiscard]] size_t Size() const noexcept { return mString.size(); }

	void FromString(std::string_view str) { mString = str; }

private:
	std::string mString;
};

// Declares MyString as a string-like type: zero-copy view on save, assignment on load.
BITSERIALIZER_DECLARE_STRING_TYPE_EXPLICIT(MyString, &MyString::Data, &MyString::Size, &MyString::FromString)

int main()	// NOLINT(bugprone-exception-escape)
{
	// Save list of custom strings to JSON
	std::vector<MyString> srcStrList = { "Red", "Green", "Blue" };
	std::string jsonResult;
	SerializationOptions serializationOptions;
	serializationOptions.formatOptions.enableFormat = true;
	BitSerializer::SaveObject<JsonArchive>(srcStrList, jsonResult, serializationOptions);
	std::cout << "Saved JSON: " << jsonResult << std::endl;

	// Load JSON-object to std::map based on custom strings
	std::map<MyString, MyString> mapResult;
	const std::string srcJson = R"({ "Background": "Blue", "PenColor": "White", "PenSize": "3", "PenOpacity": "50" })";
	BitSerializer::LoadObject<JsonArchive>(mapResult, srcJson);
	std::cout << std::endl << "Loaded map: " << std::endl;
	for (const auto& val : mapResult)
	{
		std::cout << "\t" << Convert::To<std::string>(val.first) << ": " << Convert::To<std::string>(val.second) << std::endl;
	}

	return 0;
}

