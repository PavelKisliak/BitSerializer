/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

#include "bench_harness.h"
#include "perf_utils.h"
#include "json/writers/json_writer_common.h"

namespace
{
	// Volatile sink so the compiler cannot remove the measured work.
	volatile std::size_t gSink = 0;

	using BitSerializer::Json::Detail::FindFirstEscape;
	using BitSerializer::Json::Detail::WriteString;

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

	void WriteStringScalarScan(std::string_view source, std::string& target)
	{
		static constexpr char hexChars[] = "0123456789ABCDEF";
		const std::size_t origSize = target.size();
		const std::size_t firstEscape = FindFirstEscapeScalar(source, 0);
		if (firstEscape == std::string_view::npos)
		{
			target.reserve(origSize + 2 + source.size());
			target.push_back('"');
			target.append(source);
			target.push_back('"');
			return;
		}

		target.reserve(origSize + 2 + source.size() + 5 * (source.size() - firstEscape));
		target.push_back('"');
		std::size_t pos = 0;
		std::size_t escapePos = firstEscape;
		do
		{
			if (escapePos > pos)
			{
				target.append(source.data() + pos, escapePos - pos);
			}
			const auto uc = static_cast<unsigned char>(source[escapePos]);
			switch (uc)
			{
			case '"':  target.append("\\\""); break;
			case '\\': target.append("\\\\"); break;
			case '\b': target.append("\\b"); break;
			case '\f': target.append("\\f"); break;
			case '\n': target.append("\\n"); break;
			case '\r': target.append("\\r"); break;
			case '\t': target.append("\\t"); break;
			default:
				{
					const char unicodeEscape[6] = { '\\', 'u', '0', '0', hexChars[uc >> 4], hexChars[uc & 0x0F] };
					target.append(unicodeEscape, 6);
				}
				break;
			}
			pos = escapePos + 1;
			escapePos = FindFirstEscapeScalar(source, pos);
		} while (escapePos != std::string_view::npos);

		if (pos < source.size())
		{
			target.append(source.data() + pos, source.size() - pos);
		}
		target.push_back('"');
	}

	/// Alternative B: replace `reserve + push_back + append` with `resize + memcpy`.
	/// Note: the result is compiler-dependent — it is faster on MSVC but slower on
	/// Clang for long strings (Clang does not eliminate the resize zero-fill there).
	void WriteStringResize(std::string_view source, std::string& target)
	{
		const std::size_t origSize = target.size();
		if (FindFirstEscape(source, 0) != std::string_view::npos)
		{
			WriteString(source, target);
			return;
		}
		target.resize(origSize + 2 + source.size());
		char* const dst = target.data() + origSize;
		dst[0] = '"';
		if (!source.empty())
		{
			std::memcpy(dst + 1, source.data(), source.size());
		}
		dst[1 + source.size()] = '"';
	}

	std::string Quoted(std::string_view source, void(*writer)(std::string_view, std::string&))
	{
		std::string out;
		writer(source, out);
		return out;
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
	const std::string mediumText = "A string whose purpose is to exercise the long-value path of the serializer";
	const std::string escapedText = "Test \"escaped\" chars\nsecond line";

	// All implementations must produce identical output.
	for (const std::string* text : { &shortText, &mediumText, &longText, &escapedText })
	{
		const std::string reference = Quoted(*text, WriteString);
		if (Quoted(*text, WriteStringScalarScan) != reference || Quoted(*text, WriteStringResize) != reference)
		{
			return 2;
		}
	}

	std::string target;

	BenchHarness::Runner runner(config);
	runner.Add("WriteString(short): SIMD vs scalar scan",
		[&] { target.clear(); WriteString(shortText, target); gSink += target.size(); },
		[&] { target.clear(); WriteStringScalarScan(shortText, target); gSink += target.size(); });
	runner.Add("WriteString(long): SIMD vs scalar scan",
		[&] { target.clear(); WriteString(longText, target); gSink += target.size(); },
		[&] { target.clear(); WriteStringScalarScan(longText, target); gSink += target.size(); });
	runner.Add("WriteString(short): current vs resize+memcpy",
		[&] { target.clear(); WriteString(shortText, target); gSink += target.size(); },
		[&] { target.clear(); WriteStringResize(shortText, target); gSink += target.size(); });
	runner.Add("WriteString(long): current vs resize+memcpy",
		[&] { target.clear(); WriteString(longText, target); gSink += target.size(); },
		[&] { target.clear(); WriteStringResize(longText, target); gSink += target.size(); });

	runner.RunAll();
	return 0;
}
