### [BitSerializer](../README.md) / YAML (based on RapidYAML)

Supported load/save **CSV** from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (auto-detection encoding with/without BOM)

### How to install
The CSV archive does not require any third party dependencies, but since this part is not "header only", it needs to be built. Currently library supports only static linkage. The recommended way is to use one of supported package managers, but you can do it manually just via CMake commands.
For avoid binary incompatibility issues, please build with the same compiler options that are used in your project (C++ standard, optimizations flags, runtime type, etc).
#### VCPKG
Add BitSerializer to manifest file (`vcpkg.json`) with `csv-archive` feature:
```json
{
    "dependencies": [
        {
            "name": "bitserializer",
            "features": [ "csv-archive" ]
        }
    ]
}
```
If your project is based on VS solution you can just include next header files for start use:
```cpp
#include "bitserializer/bit_serializer.h"
#include "bitserializer/csv_archive.h"
```
If you are using CMake, you need to link the library:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::csv-archive)
```
#### Conan (publish v0.50 is in progress)
Add the BitSerializer recipe to `conanfile.txt` in your project and enable `with_csv` option:
```
[requires]
bitserializer/0.50

[options]
bitserializer:with_csv=True
```
Usage the library will be related to selected Conan generator, if your choice is `cmake_find_package_multi`, than linking will be classic:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::csv-archive)
```
#### CMake install to Unix system
```
git clone https://Pavel_Kisliak@bitbucket.org/Pavel_Kisliak/bitserializer.git
cmake bitserializer -B bitserializer/build -DBUILD_CSV_ARCHIVE=ON
sudo cmake --build bitserializer/build --config Debug --target install
sudo cmake --build bitserializer/build --config Release --target install
```
#### CMake install to your project directory
You can install BitSerializer to your "ThirdParty" directory in your project.
Set correct path in `%TargetInstallDir%` (for example 'D:/MyProject/libs/bitserializer') before run.
```
git clone https://Pavel_Kisliak@bitbucket.org/Pavel_Kisliak/bitserializer.git
cmake bitserializer -B bitserializer/build -DCMAKE_INSTALL_PREFIX:PATH=%TargetInstallDir% -DBUILD_CSV_ARCHIVE=ON
sudo cmake --build bitserializer/build --config Debug --target install
sudo cmake --build bitserializer/build --config Release --target install
```
You will need to explicitly specify the path where to find the library:
```cmake
find_package(bitserializer CONFIG REQUIRED
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/libs/bitserializer
    NO_DEFAULT_PATH)
target_link_libraries(main PRIVATE BitSerializer::csv-archive)
```

### Configure values separator
The value separator can be configured via `SerializationOptions`, list of allowed characters: ',', ';', '\t', ' ', '|'.
```cpp
	SerializationOptions options;
	options.valuesSeparator = '\t';
	BitSerializer::LoadObject<CsvArchive>(targetList, sourceCsv, options);
```

### Example
Below example shows how to save and load list of entities from **CSV**.
```cpp
#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/csv_archive.h"
#include "bitserializer/types/std/vector.h"

using namespace BitSerializer;
using CsvArchive = BitSerializer::Csv::CsvArchive;

class CUserScore
{
public:
	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		archive << MakeAutoKeyValue("Player", Player);
		archive << MakeAutoKeyValue("Score", Score);
		archive << MakeAutoKeyValue("IsPremium", IsPremium);
	}

	std::string Player;
	uint64_t Score = 0;
	bool IsPremium = false;
};

int main()
{
	std::vector<CUserScore> highScores = {
		{"Ivan", 100565, false},
		{"Carl", 90580, true},
		{"Kate", 75005, false},
		{"Alex", 67950, true},
		{"Luke", 54060, false},
	};

	// Save to CSV
	std::string outputStr;
	BitSerializer::SaveObject<CsvArchive>(highScores, outputStr);
	std::cout << std::endl << "Saved result in CSV:" << std::endl << outputStr << std::endl;

	// Load from CSV
	std::vector<CUserScore> parsedHighScores;
	BitSerializer::LoadObject<CsvArchive>(parsedHighScores, outputStr);

	std::cout << std::endl << "Parsed CSV:" << std::endl << std::endl;
	std::cout << "Player \t| Score \t| IsPremium" << std::endl;
	std::cout << "------------------------------------" << std::endl;
	for (const auto& parsedHighScore : parsedHighScores)
	{
		std::cout
			<< parsedHighScore.Player << " \t| "
			<< parsedHighScore.Score << " \t| "
			<< std::boolalpha << parsedHighScore.IsPremium
			<< std::endl;
	}

	return 0;
}
```
[See full sample](../samples/serialize_to_csv/serialize_to_csv.cpp)
