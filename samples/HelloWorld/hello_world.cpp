#include <cassert>
#include <iostream>
#include <string>

#include "bitserializer/bit_serializer.h"
#include "bitserializer/archives/json_restcpp_archive.h"

using namespace BitSerializer;

int main()
{
    std::string expected = "Hello world!";
    auto json = SaveObject<JsonArchive>(expected);
    std::string result;
    LoadObject<JsonArchive>(result, json);

    assert(result == expected);
    std::cout << result << std::endl;

    std::cin.get();
    return EXIT_SUCCESS;
}