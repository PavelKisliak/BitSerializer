/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "bench_harness.h"
#include "perf_utils.h"
#include "bitserializer/convert.h"
#include "json/writers/json_writer_common.h"

namespace
{
	// Volatile sink so the compiler cannot remove the measured work.
	volatile std::size_t gSink = 0;

	using BitSerializer::Json::Detail::FindFirstEscape;
	using BitSerializer::Json::Detail::FormatNumber;
	using BitSerializer::Json::Detail::GetMaxNumberLength;

	/// Alternative A: a plain scalar escape scan instead of the SIMD/SWAR one.
	std::size_t FindFirstEscapeScalar(std::string_view source, std::size_t pos) noexcept
	{
		for (std::size_t i = pos; i < source.size(); ++i)
		{
			const auto c = static_cast<unsigned char>(source[i]);
			if (c < 0x20 || c == '"' || c == '\\')
			{
				return i;
			}
		}
		return std::string_view::npos;
	}

	/// Alternative B: format through a temporary string instead of the direct buffer.
	template <typename T>
	std::size_t FormatNumberLegacy(T value)
	{
		std::string text;
		BitSerializer::Convert::Detail::To(value, text);
		return text.size();
	}

	template <typename T>
	bool AreNumberFormatsEqual(T value)
	{
		char buffer[GetMaxNumberLength<T>()];
		const std::size_t length = FormatNumber(buffer, value);

		std::string expected;
		BitSerializer::Convert::Detail::To(value, expected);
		return std::string_view(buffer, length) == expected;
	}
}

int main(int argc, char* argv[])	// NOLINT(bugprone-exception-escape)
{
	BenchHarness::Config config;
	if (!BenchHarness::ParseConfig(argc, argv, config))
	{
		return 0;
	}

	BenchHarness::PrepareEnvironment();

	const std::string shortText = "Short string";
	const std::string longText(200, 'x');
	const std::string escapedText = "Test \"escaped\" chars\nsecond line";
	const std::int64_t intValue = 123456789012345;
	const double doubleValue = -3.141592654;

	// All alternatives must produce identical results.
	if (FindFirstEscape(shortText, 0) != FindFirstEscapeScalar(shortText, 0)
		|| FindFirstEscape(longText, 0) != FindFirstEscapeScalar(longText, 0)
		|| FindFirstEscape(escapedText, 0) != FindFirstEscapeScalar(escapedText, 0)
		|| !AreNumberFormatsEqual(intValue)
		|| !AreNumberFormatsEqual(doubleValue))
	{
		return 2;
	}

	BenchHarness::Runner runner(config);
	runner.Add("FindFirstEscape(short): SIMD vs scalar",
		[&] { gSink += FindFirstEscape(shortText, 0); },
		[&] { gSink += FindFirstEscapeScalar(shortText, 0); });
	runner.Add("FindFirstEscape(long): SIMD vs scalar",
		[&] { gSink += FindFirstEscape(longText, 0); },
		[&] { gSink += FindFirstEscapeScalar(longText, 0); });
	runner.Add("FindFirstEscape(escaped): SIMD vs scalar",
		[&] { gSink += FindFirstEscape(escapedText, 0); },
		[&] { gSink += FindFirstEscapeScalar(escapedText, 0); });
	runner.Add("FormatNumber(int64): buffer vs string",
		[&] {
			char buffer[GetMaxNumberLength<std::int64_t>()];
			gSink += FormatNumber(buffer, intValue);
		},
		[&] { gSink += FormatNumberLegacy(intValue); });
	runner.Add("FormatNumber(double): buffer vs string",
		[&] {
			char buffer[GetMaxNumberLength<double>()];
			gSink += FormatNumber(buffer, doubleValue);
		},
		[&] { gSink += FormatNumberLegacy(doubleValue); });

	runner.RunAll();
	return 0;
}
