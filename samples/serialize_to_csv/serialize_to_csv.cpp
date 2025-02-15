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

int main()	// NOLINT(bugprone-exception-escape)
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
