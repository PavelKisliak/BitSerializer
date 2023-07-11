#include <cassert>
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/cpprestjson_archive.h"

using JsonArchive = BitSerializer::Json::CppRest::JsonArchive;

int main()
{
	std::string expected = "Hello world!";
	auto json = BitSerializer::SaveObject<JsonArchive>(expected);

	std::string result;
	BitSerializer::LoadObject<JsonArchive>(result, json);

	assert(result == expected);
	std::cout << result << std::endl;

	return EXIT_SUCCESS;
}