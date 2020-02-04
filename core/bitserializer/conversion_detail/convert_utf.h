/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <iterator>
#include <algorithm>

namespace BitSerializer::Convert
{
	static constexpr uint16_t Utf16HighSurrogatesStart = 0xD800;
	static constexpr uint16_t Utf16HighSurrogatesEnd = 0xDBFF;
	static constexpr uint16_t Utf16LowSurrogatesStart = 0xDC00;
	static constexpr uint16_t Utf16LowSurrogatesEnd = 0xDFFF;

	class Utf8
	{
	public:
		static constexpr char Bom[] = { char(0xEF), char(0xBB), char(0xBF) };

		template<class T>
		static bool StartsWithBom(T&& inputString) 
		{
			auto it = std::cbegin(inputString);
			const auto endIt = std::cend(inputString);
			for (const char ch : Bom)
			{
				if (it == endIt || *it != ch) return false;
				++it;
			}
			return true;
		}

		template<class TInIt, typename TOutChar, typename TAllocator>
		static void Decode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr, const TOutChar errSym = '?')
		{
			int tails;
			while (in != end)
			{
				uint32_t sym = *in;
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
						std::advance(in, std::min<size_t>(tails - i, std::distance(in, end)));
						sym = errSym;
						break;
					}
					++in;
				}

				// Surrogates pairs are prohibited in the UTF8
				if (sym >= Utf16HighSurrogatesStart && sym <= Utf16LowSurrogatesEnd)
				{
					sym = errSym;
				}
				// Decode as surrogate pair when character exceeds UTF16 range
				else if (sizeof(TOutChar) == 2 && sym > 0xFFFF)
				{
					sym -= 0x10000;
					outStr.append({
						static_cast<TOutChar>(Utf16HighSurrogatesStart | ((sym >> 10) & 0x3FF)),
						static_cast<TOutChar>(Utf16LowSurrogatesStart | (sym & 0x3FF))
						});
					continue;
				}

				outStr.push_back(static_cast<TOutChar>(sym));
			}
		}

		template<class TInIt, typename TAllocator>
		static void Encode(TInIt in, const TInIt end, std::basic_string<char, std::char_traits<char>, TAllocator>& outStr, const char errSym = '?')
		{
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
				if constexpr (sizeof(InCharType) == 2)
				{
					if (sym >= Utf16HighSurrogatesStart && sym <= Utf16LowSurrogatesEnd)
					{
						// Low surrogate character cannot be first
						if (sym >= Utf16LowSurrogatesStart)
						{
							outStr.push_back(errSym);
							continue;
						}

						if (in == end)
						{
							outStr.push_back(errSym);
							break;
						}

						// Surrogate characters are always written as pairs (high followed by low)
						const uint32_t low = *in;
						if (low >= Utf16LowSurrogatesStart && sym <= Utf16LowSurrogatesEnd)
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
}