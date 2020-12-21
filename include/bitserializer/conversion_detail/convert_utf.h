/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <iterator>
#include <algorithm>
#include "convert_enum.h"

namespace BitSerializer::Convert
{
	/// <summary>
	/// UTF encoding type.
	/// </summary>
	enum class UtfType : unsigned char
	{
		Utf8,
		Utf16le,
		Utf16be,
		Utf32le,
		Utf32be
	};

	REGISTER_ENUM_MAP(UtfType)
	{
		{ UtfType::Utf8, "UTF-8" },
		{ UtfType::Utf16le, "UTF-16LE" },
		{ UtfType::Utf16be, "UTF-16BE" },
		{ UtfType::Utf32le, "UTF-32LE" },
		{ UtfType::Utf32be, "UTF-32BE" }
	} END_ENUM_MAP()

	namespace Unicode
	{
		static constexpr uint16_t HighSurrogatesStart = 0xD800;
		static constexpr uint16_t HighSurrogatesEnd = 0xDBFF;
		static constexpr uint16_t LowSurrogatesStart = 0xDC00;
		static constexpr uint16_t LowSurrogatesEnd = 0xDFFF;
	}

	class Utf8
	{
	public:
		using char_type = char;
		static constexpr UtfType utfType = UtfType::Utf8;
		static constexpr char bom[] = { '\xEF', '\xBB', '\xBF' };
		static constexpr bool lowEndian = true;

		template<typename TInIt, typename TOutChar, typename TAllocator>
		static void Decode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr, const TOutChar errSym = '?')
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "Input stream should represents sequence of 8-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have at least 16-bit characters");

			int tails;  // NOLINT(cppcoreguidelines-init-variables)
			while (in != end)
			{
				uint32_t sym = *in;  // NOLINT(bugprone-signed-char-misuse)
				if ((sym & 0b10000000) == 0) { tails = 1; }
				else if ((sym & 0b11100000) == 0b11000000) { tails = 2; sym &= 0b00011111; }
				else if ((sym & 0b11110000) == 0b11100000) { tails = 3; sym &= 0b00001111; }
				else if ((sym & 0b11111000) == 0b11110000) { tails = 4; sym &= 0b00000111; }
				// Overlong sequence (was prohibited in the RFC 3629 since November 2003)
				else if ((sym & 0b11111100) == 0b11111000)
				{
					std::advance(in, std::min<size_t>(5, std::distance(in, end)));
					outStr.push_back(errSym);
					continue;
				}
				else if ((sym & 0b11111110) == 0b11111100)
				{
					std::advance(in, std::min<size_t>(6, std::distance(in, end)));
					outStr.push_back(errSym);
					continue;
				}
				else
				{
					// Invalid start code
					outStr.push_back(errSym);
					++in;
					continue;
				}

				// Decode following tails
				++in;
				for (auto i = 1; i < tails; ++i)
				{
					if (in == end)
					{
						sym = errSym;
						break;
					}

					const auto nextTail = static_cast<uint8_t>(*in);
					if ((nextTail & 0b11000000) == 0b10000000)
					{
						sym <<= 6;
						sym |= nextTail & 0b00111111;
					}
					else
					{
						// Interrupt decoding the sequence if tail has bad signature
						std::advance(in, std::min<size_t>(static_cast<size_t>(tails) - i, std::distance(in, end)));
						sym = errSym;
						break;
					}
					++in;
				}

				// Surrogates pairs are prohibited in the UTF8
				if (sym >= Unicode::HighSurrogatesStart && sym <= Unicode::LowSurrogatesEnd)
				{
					sym = errSym;
				}
				// Decode as surrogate pair when character exceeds UTF16 range
				else if (sizeof(TOutChar) == 2 && sym > 0xFFFF)
				{
					sym -= 0x10000;
					outStr.append({
						static_cast<TOutChar>(Unicode::HighSurrogatesStart | ((sym >> 10) & 0x3FF)),
						static_cast<TOutChar>(Unicode::LowSurrogatesStart | (sym & 0x3FF))
						});
					continue;
				}

				outStr.push_back(static_cast<TOutChar>(sym));
			}
		}

		template<typename TInIt, typename TOutChar, typename TAllocator>
		static void Encode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr, const char errSym = '?')
		{
			static_assert(sizeof(TOutChar) == sizeof(char), "Output string must be 8-bit characters (e.g. std::string)");

			using InCharType = decltype(*in);
			while (in != end)
			{
				uint32_t sym = *in;
				++in;
				if (sym < 0x80)
				{
					outStr.push_back(static_cast<char>(sym));
					continue;
				}

				// Handle surrogates for UTF-16 (decode before encoding to UTF8)
				if constexpr (sizeof(InCharType) == sizeof(char16_t))
				{
					if (sym >= Unicode::HighSurrogatesStart && sym <= Unicode::LowSurrogatesEnd)
					{
						// Low surrogate character cannot be first and should present second part
						if (sym >= Unicode::LowSurrogatesStart || in == end)
						{
							outStr.push_back(errSym);
							continue;
						}
						// Surrogate characters are always written as pairs (low follows after high)
						const char16_t low = *in;
						if (low >= Unicode::LowSurrogatesStart && sym <= Unicode::LowSurrogatesEnd)
						{
							sym = 0x10000 + ((sym & 0x3FF) << 10 | (low & 0x3FF));
							++in;
						}
						else
						{
							outStr.push_back(errSym);
							continue;
						}
					}
				}

				if (sym < 0x800)
				{
					outStr.append({
						static_cast<char>(0b11000000 | (sym >> 6)),
						static_cast<char>(0b10000000 | (sym & 0b00111111))
						});
				}
				else if (sym < 0x10000)
				{
					outStr.append({
						static_cast<char>(0b11100000 | (sym >> 12)),
						static_cast<char>(0b10000000 | ((sym >> 6) & 0b00111111)),
						static_cast<char>(0b10000000 | ((sym & 0b00111111)))
						});
				}
				else
				{
					outStr.append({
						static_cast<char>(0b11110000 | (sym >> 18)),
						static_cast<char>(0b10000000 | ((sym >> 12) & 0b00111111)),
						static_cast<char>(0b10000000 | ((sym >> 6) & 0b00111111)),
						static_cast<char>(0b10000000 | ((sym & 0b00111111)))
						});
				}
			}
		}
	};

	
	class Utf16Le
	{
	public:
		using char_type = char16_t;
		static constexpr UtfType utfType = UtfType::Utf16le;
		static constexpr char bom[] = { '\xFF', '\xFE' };
		static constexpr bool lowEndian = true;

		template<typename TInIt, typename TOutChar, typename TAllocator>
		static void Decode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr, const TOutChar errSym = '?')
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "Input stream should represents sequence of 16-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have at least 16-bit characters");

			while (in != end)
			{
				TOutChar sym = *in;
				++in;

				// Handle surrogates when decode to UTF32
				if constexpr (sizeof(TOutChar) > sizeof(char16_t))
				{
					if (sym >= Unicode::HighSurrogatesStart && sym <= Unicode::LowSurrogatesEnd)
					{
						// Low surrogate character cannot be first and should present second part
						if (sym >= Unicode::LowSurrogatesStart || in == end)
						{
							outStr.push_back(errSym);
							continue;
						}

						// Surrogate characters are always written as pairs (low follows after high)
						const char16_t low = *in;
						if (low >= Unicode::LowSurrogatesStart && sym <= Unicode::LowSurrogatesEnd)
						{
							sym = 0x10000 + ((sym & 0x3FF) << 10 | (low & 0x3FF));
							++in;
						}
						else
						{
							outStr.push_back(errSym);
							continue;
						}
					}
				}
	
				outStr.push_back(sym);
			}
		}

		template<typename TInIt, typename TOutChar, typename TAllocator>
		static void Encode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr, const char errSym = '?')
		{
			static_assert(sizeof(TOutChar) == sizeof(char16_t), "Output string must be 16-bit characters (e.g. std::wstring, std::u16string)");

			while (in != end)
			{
				uint32_t sym = *in;
				++in;
				if (sym < 0x10000)
				{
					outStr.push_back(static_cast<TOutChar>(sym));
				}
				else
				{
					// Encode as surrogate pair
					sym -= 0x10000;
					outStr.push_back(static_cast<TOutChar>(Unicode::HighSurrogatesStart | (sym >> 10)));
					outStr.push_back(static_cast<TOutChar>(Unicode::LowSurrogatesStart | (sym & 0x3FF)));
				}
			}
		}
	};


	class Utf16Be
	{
	public:
		using char_type = char16_t;
		static constexpr UtfType utfType = UtfType::Utf16be;
		static constexpr char bom[] = { '\xFE', '\xFF' };
		static constexpr bool lowEndian = false;

		template<typename TInIt, typename TOutChar, typename TAllocator>
		static void Decode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr, const TOutChar errSym = '?')
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "Input stream should represents sequence of 16-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have at least 16-bit characters");

			while (in != end)
			{
				TOutChar sym = static_cast<char16_t>((*in >> 8) | (*in << 8));
				++in;

				// Handle surrogates when decode to UTF32
				if constexpr (sizeof(TOutChar) >= sizeof(char32_t))
				{
					if (sym >= Unicode::HighSurrogatesStart && sym <= Unicode::LowSurrogatesEnd)
					{
						// Low surrogate character cannot be first and should present second part
						if (sym >= Unicode::LowSurrogatesStart || in == end)
						{
							outStr.push_back(errSym);
							continue;
						}

						// Surrogate characters are always written as pairs (low follows after high)
						const char16_t low = ((*in >> 8) | (*in << 8));
						if (low >= Unicode::LowSurrogatesStart && sym <= Unicode::LowSurrogatesEnd)
						{
							sym = 0x10000 + ((sym & 0x3FF) << 10 | (low & 0x3FF));
							++in;
						}
						else
						{
							outStr.push_back(errSym);
							continue;
						}
					}
				}

				outStr.push_back(static_cast<TOutChar>(sym));
			}
		}

		template<typename TInIt, typename TOutChar, typename TAllocator>
		static void Encode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr, const char errSym = '?')
		{
			static_assert(sizeof(TOutChar) == sizeof(char16_t), "Output string must be 16-bit characters (e.g. std::wstring, std::u16string)");

			auto swapByteOrder = [](TOutChar sym) { return (sym >> 8) | (sym << 8); };
			while (in != end)
			{
				uint32_t sym = *in;
				++in;
				if (sym < 0x10000)
				{
					outStr.push_back(swapByteOrder(static_cast<TOutChar>(sym)));
				}
				else
				{
					// Encode as surrogate pair
					sym -= 0x10000;
					outStr.push_back(swapByteOrder(static_cast<TOutChar>(Unicode::HighSurrogatesStart | (sym >> 10))));
					outStr.push_back(swapByteOrder(static_cast<TOutChar>(Unicode::LowSurrogatesStart | (sym & 0x3FF))));
				}
			}
		}
	};

	class Utf32Le
	{
	public:
		using char_type = char32_t;
		static constexpr UtfType utfType = UtfType::Utf32le;
		static constexpr char bom[] = { '\xFF', '\xFE', '\x00', '\x00' };
		static constexpr bool lowEndian = true;

		// to be implemented
	};


	class Utf32Be
	{
	public:
		using char_type = char32_t;
		static constexpr UtfType utfType = UtfType::Utf32be;
		static constexpr char bom[] = { '\x00', '\x00', '\xFE', '\xFF' };
		static constexpr bool lowEndian = false;

		// to be implemented
	};

	//------------------------------------------------------------------------------
	
	/// <summary>
	/// Checks that passed string type starts with BOM which is specified in TUtfTraits.
	/// </summary>
	template<class TUtfTraits, class T>
	static bool StartsWithBom(T&& inputString)
	{
		auto it = std::cbegin(inputString);
		const auto endIt = std::cend(inputString);
		for (const char ch : TUtfTraits::bom)
		{
			if (it == endIt || *it != ch) return false;
			++it;
		}
		return true;
	}

	/// <summary>
	/// Detects an encoding of stream by checking BOM.
	/// </summary>
	static UtfType DetectEncoding(std::istream& inputStream, bool skipBomWhenFound = true)
	{
		static constexpr size_t readChunkSize = 4;

		// Read first bytes for check BOM
		std::string buffer(readChunkSize, 0);
		const auto origPos = inputStream.tellg();
		inputStream.read(buffer.data(), readChunkSize);

		// Return UTF-8 when BOM does not exist
		UtfType detectedUtf = UtfType::Utf8;
		size_t detectedBomSize = 0;
		if (StartsWithBom<Utf8>(buffer))
		{
			detectedUtf = UtfType::Utf8;
			detectedBomSize = sizeof Utf8::bom;
		}
		else if (StartsWithBom<Utf16Le>(buffer))
		{
			detectedUtf = UtfType::Utf16le;
			detectedBomSize = sizeof Utf16Le::bom;
		}
		else if (StartsWithBom<Utf16Be>(buffer))
		{
			detectedUtf = UtfType::Utf16be;
			detectedBomSize = sizeof Utf16Be::bom;
		}
		else if (StartsWithBom<Utf32Le>(buffer))
		{
			detectedUtf = UtfType::Utf32le;
			detectedBomSize = sizeof Utf32Le::bom;
		}
		else if (StartsWithBom<Utf32Be>(buffer))
		{
			detectedUtf = UtfType::Utf32be;
			detectedBomSize = sizeof Utf32Be::bom;
		}

		// Get back to stream position
		if (skipBomWhenFound)
		{
			if (readChunkSize != detectedBomSize) {
				inputStream.seekg(origPos + std::streamoff(detectedBomSize));
			}
		}
		else {
			inputStream.seekg(origPos);
		}
		return detectedUtf;
	}
}
