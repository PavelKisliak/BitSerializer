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
