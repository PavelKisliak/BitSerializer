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
		archive << MakeKeyValue("Player", Player);
		archive << MakeKeyValue("Score", Score);
		archive << MakeKeyValue("Datetime", Datetime);
		archive << MakeKeyValue("IsPremium", IsPremium);
	}

	std::string Player;
	uint64_t Score = 0;
	std::chrono::system_clock::time_point Datetime;
	bool IsPremium = false;
};

int main()
{
	constexpr auto tp2023_01_01 = system_clock::time_point(seconds(1672531200));
	std::vector<CUserScore> highScores = {
		{"Ivan", 99565, tp2023_01_01 + 15h + 3min, false},
		{"Carl", 90580, tp2023_01_01 - 10h + 2min, true},
		{"Kate", 75005, tp2023_01_01 - 1325h + 7min, false},
		{"Alex", 67950, tp2023_01_01 - 3467h + 50min + 12s, true},
		{"Luke", 54060, tp2023_01_01 - 4600h + 8min + 4s, false},
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
	std::cout << "Player  | Score         | Datetime             | IsPremium" << std::endl;
	std::cout << "----------------------------------------------------------" << std::endl;
	for (const auto& parsedHighScore : parsedHighScores)
	{
		std::cout
			<< parsedHighScore.Player << " \t| "
			<< parsedHighScore.Score << " \t| "
			<< Convert::ToString(parsedHighScore.Datetime) << " | "
			<< Convert::ToString(parsedHighScore.IsPremium)
			<< std::endl;
	}

	return 0;
}
