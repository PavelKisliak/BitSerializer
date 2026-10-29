/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "bitserializer/bit_serializer.h"

/**
 * @file benchmark_models.h
 * @brief A set of small, representative models for archive microbenchmarks.
 *
 * The round-trip archives benchmark uses a large model (30 objects) to measure
 * throughput. For an intra-binary A/B a smaller model keeps each iteration short
 * while still exercising the relevant hot path. Different model profiles answer
 * different questions:
 *
 * - `MixedBenchmarkModel`     - a bit of everything (general A/B).
 * - `NumericBenchmarkModel`   - every fundamental number type (number formatting).
 * - `StringBenchmarkModel`    - plain ASCII strings of different lengths (copy path).
 * - `EscapedBenchmarkModel`   - values with quotes/backslashes/control characters.
 * - `Utf8BenchmarkModel`      - 1/2/3/4-byte UTF-8 sequences.
 * - `EscapedKeyBenchmarkModel` - escaped characters in the *keys*.
 *
 * All models are format-agnostic and use an object root. CSV requires an array
 * root, so wrap a model in a container for CSV.
 *
 * UTF-8 test strings are written as byte escapes to keep this source ASCII-only.
 */

namespace BenchModelsDetail
{
	inline std::uint64_t HashCombine(std::uint64_t seed, std::uint64_t value) noexcept
	{
		return seed ^ (value + 0x9E3779B97F4A7C15ULL + (seed << 6) + (seed >> 2));
	}

	/// Cheap content hash (size + first/last byte) - enough to keep the loaded
	/// string observable without hashing the whole payload in the timed loop.
	inline std::uint64_t HashString(const std::string& value) noexcept
	{
		std::uint64_t hash = value.size();
		if (!value.empty())
		{
			hash = HashCombine(hash, static_cast<unsigned char>(value.front()));
			hash = HashCombine(hash, static_cast<unsigned char>(value.back()));
		}
		return hash;
	}

	inline std::uint64_t HashDouble(double value) noexcept
	{
		std::uint64_t bits = 0;
		std::memcpy(&bits, &value, sizeof(bits));
		return bits;
	}

	inline std::uint64_t HashFloat(float value) noexcept
	{
		std::uint32_t bits = 0;
		std::memcpy(&bits, &value, sizeof(bits));
		return bits;
	}
}

struct MixedBenchmarkModel
{
	bool flag = true;
	std::int32_t intValue = 0;
	std::uint64_t uintValue = 0;
	double doubleValue = 0.0;
	std::string shortText;
	std::string longText;
	std::string escapedText;
	std::string utf8Text;

	static constexpr std::size_t FieldCount = 8;

	static MixedBenchmarkModel Make()
	{
		MixedBenchmarkModel model;
		model.flag = true;
		model.intValue = -12345;
		model.uintValue = 1234567890123ULL;
		model.doubleValue = -3.141592654;
		model.shortText = "Short string";
		model.longText = "A string whose purpose is to exercise the long-value path of the serializer";
		model.escapedText = "Test \"escaped\" chars\nsecond line\tand a tab";
		model.utf8Text = "\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82"; // "Privet"
		return model;
	}

	template <typename TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::KeyValue("Flag", flag);
		archive << BitSerializer::KeyValue("IntValue", intValue);
		archive << BitSerializer::KeyValue("UintValue", uintValue);
		archive << BitSerializer::KeyValue("DoubleValue", doubleValue);
		archive << BitSerializer::KeyValue("ShortText", shortText);
		archive << BitSerializer::KeyValue("LongText", longText);
		archive << BitSerializer::KeyValue("EscapedText", escapedText);
		archive << BitSerializer::KeyValue("Utf8Text", utf8Text);
	}

	[[nodiscard]] bool Equals(const MixedBenchmarkModel& rhs) const
	{
		return flag == rhs.flag && intValue == rhs.intValue && uintValue == rhs.uintValue
			&& std::fabs(doubleValue - rhs.doubleValue) <= 1e-9
			&& shortText == rhs.shortText && longText == rhs.longText
			&& escapedText == rhs.escapedText && utf8Text == rhs.utf8Text;
	}

	[[nodiscard]] std::uint64_t Checksum() const
	{
		std::uint64_t hash = BenchModelsDetail::HashCombine(0, static_cast<std::uint64_t>(flag));
		hash = BenchModelsDetail::HashCombine(hash, static_cast<std::uint64_t>(intValue));
		hash = BenchModelsDetail::HashCombine(hash, uintValue);
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashDouble(doubleValue));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(shortText));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(longText));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(escapedText));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(utf8Text));
		return hash;
	}
};

struct NumericBenchmarkModel
{
	std::int8_t i8 = 0;
	std::int16_t i16 = 0;
	std::int32_t i32 = 0;
	std::int64_t i64 = 0;
	std::uint8_t u8 = 0;
	std::uint16_t u16 = 0;
	std::uint32_t u32 = 0;
	std::uint64_t u64 = 0;
	float f32 = 0.0f;
	double f64 = 0.0;

	static constexpr std::size_t FieldCount = 10;

	static NumericBenchmarkModel Make()
	{
		NumericBenchmarkModel model;
		model.i8 = -100;
		model.i16 = -30000;
		model.i32 = -2000000000;
		model.i64 = -9223372036854775807LL;
		model.u8 = 200;
		model.u16 = 60000;
		model.u32 = 4000000000u;
		model.u64 = 18446744073709551615ULL;
		model.f32 = 3.141592f;
		model.f64 = -3.141592654;
		return model;
	}

	template <typename TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::KeyValue("I8", i8);
		archive << BitSerializer::KeyValue("I16", i16);
		archive << BitSerializer::KeyValue("I32", i32);
		archive << BitSerializer::KeyValue("I64", i64);
		archive << BitSerializer::KeyValue("U8", u8);
		archive << BitSerializer::KeyValue("U16", u16);
		archive << BitSerializer::KeyValue("U32", u32);
		archive << BitSerializer::KeyValue("U64", u64);
		archive << BitSerializer::KeyValue("F32", f32);
		archive << BitSerializer::KeyValue("F64", f64);
	}

	[[nodiscard]] bool Equals(const NumericBenchmarkModel& rhs) const
	{
		return i8 == rhs.i8 && i16 == rhs.i16 && i32 == rhs.i32 && i64 == rhs.i64
			&& u8 == rhs.u8 && u16 == rhs.u16 && u32 == rhs.u32 && u64 == rhs.u64
			&& std::fabs(f32 - rhs.f32) <= 1e-3f && std::fabs(f64 - rhs.f64) <= 1e-9;
	}

	[[nodiscard]] std::uint64_t Checksum() const
	{
		std::uint64_t hash = BenchModelsDetail::HashCombine(0, static_cast<std::uint64_t>(i8));
		hash = BenchModelsDetail::HashCombine(hash, static_cast<std::uint64_t>(i16));
		hash = BenchModelsDetail::HashCombine(hash, static_cast<std::uint64_t>(i32));
		hash = BenchModelsDetail::HashCombine(hash, static_cast<std::uint64_t>(i64));
		hash = BenchModelsDetail::HashCombine(hash, u8);
		hash = BenchModelsDetail::HashCombine(hash, u16);
		hash = BenchModelsDetail::HashCombine(hash, u32);
		hash = BenchModelsDetail::HashCombine(hash, u64);
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashFloat(f32));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashDouble(f64));
		return hash;
	}
};

struct StringBenchmarkModel
{
	std::string emptyText;
	std::string shortText;
	std::string mediumText;
	std::string longText;

	static constexpr std::size_t FieldCount = 4;

	static StringBenchmarkModel Make()
	{
		StringBenchmarkModel model;
		model.emptyText.clear();
		model.shortText = "Short string";
		model.mediumText = "A medium ASCII string used to exercise the copy path of the writer";
		model.longText = std::string(200, 'x');
		return model;
	}

	template <typename TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::KeyValue("EmptyText", emptyText);
		archive << BitSerializer::KeyValue("ShortText", shortText);
		archive << BitSerializer::KeyValue("MediumText", mediumText);
		archive << BitSerializer::KeyValue("LongText", longText);
	}

	[[nodiscard]] bool Equals(const StringBenchmarkModel& rhs) const
	{
		return emptyText == rhs.emptyText && shortText == rhs.shortText
			&& mediumText == rhs.mediumText && longText == rhs.longText;
	}

	[[nodiscard]] std::uint64_t Checksum() const
	{
		std::uint64_t hash = BenchModelsDetail::HashString(emptyText);
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(shortText));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(mediumText));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(longText));
		return hash;
	}
};

struct EscapedBenchmarkModel
{
	std::string quotes;
	std::string backslashes;
	std::string controlChars;
	std::string mixed;

	static constexpr std::size_t FieldCount = 4;

	static EscapedBenchmarkModel Make()
	{
		EscapedBenchmarkModel model;
		model.quotes = R"(He said "hello" and "goodbye")";
		model.backslashes = R"(C:\path\to\some\file.txt)";
		model.controlChars = "\x01\x02\x1F";
		model.mixed = "line1\nline2\t\"quoted\"\r\b\f";
		return model;
	}

	template <typename TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::KeyValue("Quotes", quotes);
		archive << BitSerializer::KeyValue("Backslashes", backslashes);
		archive << BitSerializer::KeyValue("ControlChars", controlChars);
		archive << BitSerializer::KeyValue("Mixed", mixed);
	}

	[[nodiscard]] bool Equals(const EscapedBenchmarkModel& rhs) const
	{
		return quotes == rhs.quotes && backslashes == rhs.backslashes
			&& controlChars == rhs.controlChars && mixed == rhs.mixed;
	}

	[[nodiscard]] std::uint64_t Checksum() const
	{
		std::uint64_t hash = BenchModelsDetail::HashString(quotes);
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(backslashes));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(controlChars));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(mixed));
		return hash;
	}
};

struct Utf8BenchmarkModel
{
	std::string ascii;
	std::string twoByte;	///< Cyrillic ("Privet").
	std::string threeByte;	///< CJK ("Nihongo").
	std::string fourByte;	///< Emoji (U+1F600).

	static constexpr std::size_t FieldCount = 4;

	static Utf8BenchmarkModel Make()
	{
		Utf8BenchmarkModel model;
		model.ascii = "Plain ASCII string";
		model.twoByte = "\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82";
		model.threeByte = "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E";
		model.fourByte = "\xF0\x9F\x98\x80\xF0\x9F\x98\x81\xF0\x9F\x98\x82";
		return model;
	}

	template <typename TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::KeyValue("Ascii", ascii);
		archive << BitSerializer::KeyValue("TwoByte", twoByte);
		archive << BitSerializer::KeyValue("ThreeByte", threeByte);
		archive << BitSerializer::KeyValue("FourByte", fourByte);
	}

	[[nodiscard]] bool Equals(const Utf8BenchmarkModel& rhs) const
	{
		return ascii == rhs.ascii && twoByte == rhs.twoByte
			&& threeByte == rhs.threeByte && fourByte == rhs.fourByte;
	}

	[[nodiscard]] std::uint64_t Checksum() const
	{
		std::uint64_t hash = BenchModelsDetail::HashString(ascii);
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(twoByte));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(threeByte));
		hash = BenchModelsDetail::HashCombine(hash, BenchModelsDetail::HashString(fourByte));
		return hash;
	}
};

struct EscapedKeyBenchmarkModel
{
	std::string value;

	static constexpr std::size_t FieldCount = 5;

	static EscapedKeyBenchmarkModel Make()
	{
		EscapedKeyBenchmarkModel model;
		model.value = "value";
		return model;
	}

	template <typename TArchive>
	void Serialize(TArchive& archive)
	{
		archive << BitSerializer::KeyValue("Plain", value);
		archive << BitSerializer::KeyValue("With\"Quote", value);
		archive << BitSerializer::KeyValue("With\\Backslash", value);
		archive << BitSerializer::KeyValue("With\nNewline", value);
		archive << BitSerializer::KeyValue("With\tTab", value);
	}

	[[nodiscard]] bool Equals(const EscapedKeyBenchmarkModel& rhs) const
	{
		return value == rhs.value;
	}

	[[nodiscard]] std::uint64_t Checksum() const
	{
		return BenchModelsDetail::HashString(value);
	}
};
