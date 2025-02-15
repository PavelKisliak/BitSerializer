#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidyaml_archive.h"
#include "bitserializer/types/std/map.h"

using namespace BitSerializer;
using YamlArchive = BitSerializer::Yaml::RapidYaml::YamlArchive;

class CDevice
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("IP", mIp);
		archive << KeyValue("Owner", mOwner);
	}

	std::string mIp;
	std::string mOwner;
};

int main()	// NOLINT(bugprone-exception-escape)
{
	const char* srcStr = R"(
Tablet:
   IP: 192.168.0.1
   Owner: Artsiom
Desktop:
   IP: 192.168.0.2
   Owner: Pavel
Laptop:
   IP: 192.168.0.3
   Owner: Alex
)";

	// Loading from YAML
	std::map<std::string, CDevice> devices;
	BitSerializer::LoadObject<YamlArchive>(devices, srcStr);

	std::cout << "Loaded devices list from YAML: " << std::endl;
	for (const auto& device : devices)
	{
		std::cout << "Type: " << device.first
			<< "\tIP: " << device.second.mIp
			<< "\tOwner: " << device.second.mOwner
			<< std::endl;
	}

	// Save back to YAML
	std::string outputStr;
	BitSerializer::SaveObject<YamlArchive>(devices, outputStr);
	std::cout << std::endl << "Saved result in YAML:" << std::endl << outputStr << std::endl;

	return 0;
}
