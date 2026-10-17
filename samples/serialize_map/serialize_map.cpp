#include <iostream>
#include <map>
#include <string>

#include "bitserializer/bit_serializer.h"
#include "bitserializer/json_archive.h"
#include "bitserializer/rapidyaml_archive.h"
#include "bitserializer/types/std/map.h"

using namespace BitSerializer;

class CDevice
{
public:
	CDevice() = default;
	CDevice(std::string ip, std::string owner)
		: mIp(std::move(ip))
		, mOwner(std::move(owner))
	{ }

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("IP", mIp);
		archive << KeyValue("Owner", mOwner);
	}

	std::string mIp;
	std::string mOwner;
};

// Demonstrates serialization of associative containers (std::map / std::multimap):
// - the default representation (a map is serialized as a plain object);
// - the MapAsDictionary wrapper (a sequence of single-key objects, may keep duplicate keys);
// - the MapAsObject wrapper (the default representation with an explicit load mode).
template <typename TArchive>
void DemonstrateMapSerialization(const char* archiveName)
{
	using TDeviceMap = std::map<std::string, CDevice>;

	const TDeviceMap devices{
		{ "Tablet",  { "192.168.0.1", "Artsiom" } },
		{ "Desktop", { "192.168.0.2", "Pavel" } },
		{ "Laptop",  { "192.168.0.3", "Alex" } },
	};

	std::cout << "============================= " << archiveName << " =============================" << std::endl;

	// 1. The default representation: a map is serialized as a plain object (a set of key-value fields).
	const auto asObject = SaveObject<TArchive>(devices);
	std::cout << "--- map (default, as object) ---" << std::endl << asObject << std::endl << std::endl;

	// 2. MapAsDictionary: a sequence of single-key objects. Unlike a plain object,
	//    it preserves the order of elements and allows duplicate keys.
	const auto asDictionary = SaveObject<TArchive>(MapAsDictionary(devices));
	std::cout << "--- map (MapAsDictionary) ---" << std::endl << asDictionary << std::endl << std::endl;

	// 3. MapAsObject exposes the load mode. With 'UpdateExisting' only the values of
	//    already existing keys are updated, all new keys are ignored.
	TDeviceMap partialDevices{ { "Desktop", {} } };
	LoadObject<TArchive>(MapAsObject(partialDevices, MapLoadMode::UpdateExisting), asObject);
	std::cout << "--- MapAsObject with 'UpdateExisting' mode ---" << std::endl
		<< "Loaded " << partialDevices.size() << " device(s):" << std::endl;
	for (const auto& device : partialDevices) {
		std::cout << device.first << ": " << device.second.mIp << " (" << device.second.mOwner << ")" << std::endl;
	}
	std::cout << std::endl;

	// 4. A multimap serialized as a dictionary keeps duplicate keys.
	const std::multimap<std::string, std::string> tags{
		{ "color", "red" }, { "color", "blue" }, { "size", "L" }
	};
	const auto tagsDictionary = SaveObject<TArchive>(MapAsDictionary(tags));
	std::cout << "--- multimap (MapAsDictionary, keeps duplicate keys) ---" << std::endl
		<< tagsDictionary << std::endl << std::endl;

	std::multimap<std::string, std::string> loadedTags;
	LoadObject<TArchive>(MapAsDictionary(loadedTags), tagsDictionary);
	std::cout << "Loaded " << loadedTags.size() << " tag(s)" << std::endl << std::endl;
}

int main()	// NOLINT(bugprone-exception-escape)
{
	// Pretty-print the output (mainly makes the compact JSON readable).
	DefaultOptions.formatOptions.enableFormat = true;

	DemonstrateMapSerialization<Json::JsonArchive>("JSON");
	DemonstrateMapSerialization<Yaml::RapidYaml::YamlArchive>("YAML");

	return 0;
}
