### [BitSerializer](../README.md) / CSV

Supported load/save **CSV** from:

- std::string: UTF-8
- std::stream: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE (auto-detection encoding with/without BOM)

### How to install
The CSV archive does not require any third party dependencies, but since this part is not "header only", it needs to be built. The recommended way is to use one of supported package managers, but you can do it manually just via CMake commands. For avoid binary incompatibility issues, please build with the same compiler options that are used in your project (C++ standard, optimizations flags, runtime type, etc).

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

#### Conan 2
Add the BitSerializer recipe to `conanfile.txt` in your project and enable `with_csv` option:
```
[requires]
bitserializer/0.75

[options]
bitserializer/*:with_csv=True
```
Usage the library will be related to selected Conan generator, if your choice is `CMakeDeps`, than linking will be classic:
```cmake
find_package(bitserializer CONFIG REQUIRED)
target_link_libraries(main PRIVATE BitSerializer::csv-archive)
```

#### CMake install to Unix system
```sh
$ git clone https://github.com/PavelKisliak/BitSerializer.git
$ cmake bitserializer -B bitserializer/build -DBUILD_CSV_ARCHIVE=ON
$ sudo cmake --build bitserializer/build --config Debug --target install
$ sudo cmake --build bitserializer/build --config Release --target install
```
By default, will be built a static library, add the CMake parameter `-DBUILD_SHARED_LIBS=ON` to build shared.

#### CMake install to your project directory
You can install BitSerializer to your "ThirdParty" directory in your project.
Set correct path in `%TargetInstallDir%` (for example 'D:/MyProject/libs/bitserializer') before run.
```shell
> git clone https://Pavel_Kisliak@bitbucket.org/Pavel_Kisliak/bitserializer.git
> cmake bitserializer -B bitserializer/build -DCMAKE_INSTALL_PREFIX:PATH=%TargetInstallDir% -DBUILD_CSV_ARCHIVE=ON
> sudo cmake --build bitserializer/build --config Debug --target install
> sudo cmake --build bitserializer/build --config Release --target install
```
By default, will be built a static library, add the CMake parameter `-DBUILD_SHARED_LIBS=ON` to build shared.

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
#include "bitserializer/types/std/chrono.h"

using namespace std::chrono;
using namespace BitSerializer;
using CsvArchive = BitSerializer::Csv::CsvArchive;

class CUserScore
{
public:
    template <class TArchive>
    void Serialize(TArchive& archive)
    {
        archive << KeyValue("Player", Player);
        archive << KeyValue("Score", Score);
        archive << KeyValue("Datetime", Datetime);
        archive << KeyValue("Duration", Duration);
    }

    std::string Player;
    uint64_t Score = 0;
    time_point<system_clock, seconds> Datetime;
    seconds Duration{};
};

int main()
{
    constexpr auto tp2023_01_01 = time_point<system_clock, seconds>(seconds(1672531200));
    std::vector<CUserScore> highScores = {
        {"Ivan", 99565, tp2023_01_01 + 15h + 3min, 2min + 10s},
        {"Carl", 90580, tp2023_01_01 - 10h + 2min, 2min + 56s},
        {"Kate", 75005, tp2023_01_01 - 1325h + 7min, 1min + 43s},
        {"Alex", 67950, tp2023_01_01 - 3467h + 50min + 12s, 1min + 30s},
        {"Luke", 54060, tp2023_01_01 - 4600h + 8min + 4s, 1min + 25s},
    };

    // Save to CSV
    std::string outputStr;
    BitSerializer::SaveObject<CsvArchive>(highScores, outputStr);
    std::cout << "Saved result in CSV:" << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << outputStr << std::endl;

    // Load from CSV
    std::vector<CUserScore> parsedHighScores;
    BitSerializer::LoadObject<CsvArchive>(parsedHighScores, outputStr);

    std::cout << std::endl << "Parsed CSV:" << std::endl << std::endl;
    std::cout << "Player  | Score         | Datetime             | Duration" << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;
    for (const auto& parsedHighScore : parsedHighScores)
    {
        std::cout
            << parsedHighScore.Player << " \t| "
            << parsedHighScore.Score << " \t| "
            << Convert::ToString(parsedHighScore.Datetime) << " | "
            << Convert::ToString(parsedHighScore.Duration)
            << std::endl;
    }

    return 0;
}
```
[See full sample](../samples/serialize_to_csv/serialize_to_csv.cpp)
