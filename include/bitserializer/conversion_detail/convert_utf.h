/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include <cassert>
#include <istream>
#include <iterator>
#include <string>
#include <cstring>
#include "convert_enum.h"

namespace BitSerializer::Convert
{
	/// <summary>
	/// UTF encoding type.
	/// </summary>
	enum class UtfType
	{
		Utf8,
		Utf16le,
		Utf16be,
		Utf32le,
		Utf32be
	};

	REGISTER_ENUM(UtfType, {
		{ UtfType::Utf8, "UTF-8" },
		{ UtfType::Utf16le, "UTF-16LE" },
		{ UtfType::Utf16be, "UTF-16BE" },
		{ UtfType::Utf32le, "UTF-32LE" },
		{ UtfType::Utf32be, "UTF-32BE" }
	})
	DECLARE_ENUM_STREAM_OPS(BitSerializer::Convert::UtfType)

	namespace Unicode
	{
		static constexpr uint16_t HighSurrogatesStart = 0xD800;
		static constexpr uint16_t HighSurrogatesEnd = 0xDBFF;
		static constexpr uint16_t LowSurrogatesStart = 0xDC00;
		static constexpr uint16_t LowSurrogatesEnd = 0xDFFF;

		inline bool IsInSurrogatesRange(const char32_t sym) noexcept
		{
			return sym >= HighSurrogatesStart && sym <= LowSurrogatesEnd;
		}
	}

	/// <summary>
	/// Encode error policy.
	/// </summary>
	enum class EncodeErrorPolicy
	{
		WriteErrorMark,
		ThrowException,
		Skip
	};

	namespace Detail
	{
		template<typename TChar, typename TAllocator>
		void HandleEncodingError(std::basic_string<TChar, std::char_traits<TChar>, TAllocator>& outStr, EncodeErrorPolicy encodePolicy, const TChar* errorMark)
		{
			switch (encodePolicy)
			{
			case EncodeErrorPolicy::WriteErrorMark:
				outStr.append(errorMark);
				break;
			case EncodeErrorPolicy::ThrowException:
				throw std::runtime_error("Unable to parse wrong UTF sequence");
			case EncodeErrorPolicy::Skip:
				break;
			}
		}

		template <typename TChar>
		constexpr const TChar* GetDefaultErrorMark() noexcept
		{
			if constexpr (sizeof(TChar) == sizeof(char)) {
				return reinterpret_cast<const TChar*>(u8"☐");
			}
			if constexpr (sizeof(TChar) == sizeof(char16_t)) {
				return reinterpret_cast<const TChar*>(u"☐");
			}
			else if constexpr (sizeof(TChar) == sizeof(char32_t))
			{
				return reinterpret_cast<const TChar*>(U"☐");
			}
			return nullptr;
		}
	}

	class Utf8
	{
	public:
		using char_type = char;
		static constexpr UtfType utfType = UtfType::Utf8;
		static constexpr char bom[] = { '\xEF', '\xBB', '\xBF' };
		static constexpr bool lowEndian = true;

		/// <summary>
		/// Decodes UTF-8 to UTF-16 or UTF-32.
		///	Returns iterator to the last not decoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Decode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::WriteErrorMark, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "Input stream should represents sequence of 8-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have at least 16-bit characters");

			int tails;
			TInIt startTailPos = in;
			while (in != end)
			{
				uint32_t sym = static_cast<unsigned char>(*in);
				if ((sym & 0b10000000) == 0) { tails = 1; }
				else if ((sym & 0b11100000) == 0b11000000) { tails = 2; sym &= 0b00011111; }
				else if ((sym & 0b11110000) == 0b11100000) { tails = 3; sym &= 0b00001111; }
				else if ((sym & 0b11111000) == 0b11110000) { tails = 4; sym &= 0b00000111; }
				// Overlong sequence (was prohibited in the RFC 3629 since November 2003)
				else if ((sym & 0b11111100) == 0b11111000)
				{
					std::advance(in, std::min<size_t>(5, std::distance(in, end)));
					Detail::HandleEncodingError(outStr, encodePolicy, errorMark);
					startTailPos = in;
					continue;
				}
				else if ((sym & 0b11111110) == 0b11111100)
				{
					std::advance(in, std::min<size_t>(6, std::distance(in, end)));
					Detail::HandleEncodingError(outStr, encodePolicy, errorMark);
					startTailPos = in;
					continue;
				}
				else
				{
					// Invalid start code
					Detail::HandleEncodingError(outStr, encodePolicy, errorMark);
					++in;
					startTailPos = in;
					continue;
				}

				// Decode following tails
				++in;
				bool isDecodedSym = true;
				for (auto i = 1; i < tails; ++i)
				{
					if (in == end)
					{
						isDecodedSym = false;
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
						std::advance(in, (std::min<size_t>)(static_cast<size_t>(tails) - i, std::distance(in, end)));
						Detail::HandleEncodingError(outStr, encodePolicy, errorMark);
						isDecodedSym = false;
						break;
					}
					++in;
				}

				if (isDecodedSym)
				{
					// Surrogates pairs are prohibited in the UTF-8
					if (Unicode::IsInSurrogatesRange(sym))
					{
						Detail::HandleEncodingError(outStr, encodePolicy, errorMark);
						continue;
					}
					// Decode as surrogate pair when character exceeds UTF-16 range
					if (sizeof(TOutChar) == 2 && sym > 0xFFFF)
					{
						sym -= 0x10000;
						outStr.append({
							static_cast<TOutChar>(Unicode::HighSurrogatesStart | ((sym >> 10) & 0x3FF)),
							static_cast<TOutChar>(Unicode::LowSurrogatesStart | (sym & 0x3FF))
							});
						continue;
					}

					outStr.push_back(static_cast<TOutChar>(sym));
					startTailPos = in;
				}
			}
			return startTailPos;
		}

		/// <summary>
		/// Encodes to UTF-8 from UTF-16 or UTF-32.
		///	Returns iterator to the last not encoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Encode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::WriteErrorMark, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(TOutChar) == sizeof(char), "Output string must be 8-bit characters (e.g. std::string)");

			using InCharType = decltype(*in);
			TInIt startTailPos = in;
			while (in != end)
			{
				uint32_t sym = *in;
				++in;
				if (sym < 0x80)
				{
					outStr.push_back(static_cast<char>(sym));
					startTailPos = in;
					continue;
				}

				// Handle surrogates for UTF-16 (decode before encoding to UTF-8)
				if constexpr (sizeof(InCharType) == sizeof(char16_t))
				{
					if (Unicode::IsInSurrogatesRange(sym))
					{
						// Low surrogate character cannot be first and should present second part
						if (sym >= Unicode::LowSurrogatesStart)
						{
							Detail::HandleEncodingError(outStr, encodePolicy, errorMark);
							startTailPos = in;
							continue;
						}
						// Check if it's end of input string
						if (in == end)
						{
							// Should return iterator to first character in surrogate pair
							return startTailPos;
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
							Detail::HandleEncodingError(outStr, encodePolicy, errorMark);
							startTailPos = in;
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
				startTailPos = in;
			}
			return startTailPos;
		}
	};

	
	class Utf16Le
	{
	public:
		using char_type = char16_t;
		static constexpr UtfType utfType = UtfType::Utf16le;
		static constexpr char bom[] = { '\xFF', '\xFE' };
		static constexpr bool lowEndian = true;

		/// <summary>
		/// Decodes UTF-16LE to UTF-32, UTF-16 (copies 'as is') or UTF-8.
		///	Returns iterator to the last not decoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Decode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::WriteErrorMark, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "Input stream should represents sequence of 16-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char) || sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have 8, 16 or 32-bit characters");

			if constexpr (sizeof(TOutChar) == sizeof(char))
			{
				return Utf8::Encode(in, end, outStr, encodePolicy, errorMark);
			}
			else
			{
				TInIt startTailPos = in;
				while (in != end)
				{
					TOutChar sym = *in;
					++in;

					if constexpr (sizeof(TOutChar) == sizeof(char16_t))
					{
						// Do not copy only first part of surrogate pair
						if (in == end && (sym >= Unicode::HighSurrogatesStart && sym < Unicode::HighSurrogatesEnd)) {
							break;
						}
					}
					else if constexpr (sizeof(TOutChar) == sizeof(char32_t))
					{
						// Handle surrogates
						if (Unicode::IsInSurrogatesRange(sym))
						{
							// Low surrogate character cannot be first and should present second part
							if (sym >= Unicode::LowSurrogatesStart)
							{
								Detail::HandleEncodingError(outStr, encodePolicy, errorMark);
								continue;
							}
							// Check if it's end of input string
							if (in == end)
							{
								// Should return iterator to first character in surrogate pair
								return startTailPos;
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
								Detail::HandleEncodingError(outStr, encodePolicy, errorMark);
								continue;
							}
						}
					}

					outStr.push_back(sym);
					startTailPos = in;
				}
				return startTailPos;
			}
		}

		/// <summary>
		/// Encodes to UTF-16LE from UTF-32, UTF-16 (copies 'as is') or UTF-8.
		///	Returns iterator to the last not encoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Encode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::WriteErrorMark, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(TOutChar) == sizeof(char16_t), "Output string must be 16-bit characters (e.g. std::u16string)");

			using TInCharType = decltype(*in);
			if constexpr (sizeof(TInCharType) == sizeof(char))
			{
				return Utf8::Decode(in, end, outStr, encodePolicy, errorMark);
			}
			else if constexpr (sizeof(TInCharType) == sizeof(char16_t))
			{
				TInIt startTailPos = in;
				for (; in != end; ++in)
				{
					TOutChar sym = *in;
					// Do not copy only first part of surrogate pair
					if (in == end && (sym >= Unicode::HighSurrogatesStart && sym < Unicode::HighSurrogatesEnd)) {
						break;
					}
					outStr.push_back(sym);
					startTailPos = in;
				}
				return startTailPos;
			}
			else if constexpr (sizeof(TInCharType) == sizeof(char32_t))
			{
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
			return in;
		}
	};


	class Utf16Be
	{
		template <typename TBaseIt>
		class U16BeConstIterator
		{
		public:
			explicit U16BeConstIterator(TBaseIt it)
				: mBaseIt(std::move(it))
			{ }

			char16_t operator*() const noexcept {
				return (*mBaseIt >> 8) | (*mBaseIt << 8);
			}

			bool operator==(const TBaseIt& rhs) const { return mBaseIt == rhs; }
			bool operator!=(const TBaseIt& rhs) const { return mBaseIt != rhs; }
			TBaseIt& operator++() { return ++mBaseIt; }
			operator const TBaseIt& () const { return mBaseIt; }

		private:
			TBaseIt mBaseIt;
		};

	public:
		using char_type = char16_t;
		static constexpr UtfType utfType = UtfType::Utf16be;
		static constexpr char bom[] = { '\xFE', '\xFF' };
		static constexpr bool lowEndian = false;

		/// <summary>
		/// Decodes UTF-16BE to UTF-32, UTF-16 or UTF-8.
		///	Returns iterator to the last not decoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Decode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::WriteErrorMark, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "Input stream should represents sequence of 16-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char) || sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have 8, 16 or 32-bit characters");

			return Utf16Le::Decode(U16BeConstIterator<TInIt>(in), U16BeConstIterator<TInIt>(end), outStr, encodePolicy, errorMark);
		}

		/// <summary>
		/// Encodes UTF-16BE from UTF-32, UTF-16 or UTF-8.
		///	Returns iterator to the last not encoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Encode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::ThrowException, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(TOutChar) == sizeof(char16_t), "Output string must be 16-bit characters (e.g. std::u16string)");

			const size_t startOutPos = outStr.size();
			auto it = Utf16Le::Encode(in, end, outStr, encodePolicy, errorMark);

			std::for_each(outStr.begin() + startOutPos, outStr.end(), [](TOutChar& sym) {
				sym = static_cast<TOutChar>(sym >> 8) | static_cast<TOutChar>(sym << 8);
			});
			return it;
		}
	};

	class Utf32Le
	{
	public:
		using char_type = char32_t;
		static constexpr UtfType utfType = UtfType::Utf32le;
		static constexpr char bom[] = { '\xFF', '\xFE', '\x00', '\x00' };
		static constexpr bool lowEndian = true;

		/// <summary>
		/// Decodes UTF-32LE to UTF-32 (copies 'as is'), UTF-16 or UTF-8.
		///	Returns iterator to the last not decoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Decode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::WriteErrorMark, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "Input stream should represents sequence of 32-bit characters");

			if constexpr (sizeof(TOutChar) == sizeof(char_type))
			{
				for (; in != end; ++in) {
					outStr.push_back(static_cast<TOutChar>(*in));
				}
			}
			else if constexpr (sizeof(TOutChar) == sizeof(Utf16Le::char_type))
			{
				return Utf16Le::Encode(in, end, outStr, encodePolicy, errorMark);
			}
			else if constexpr (sizeof(TOutChar) == sizeof(Utf8::char_type))
			{
				return Utf8::Encode(in, end, outStr, encodePolicy, errorMark);
			}
			return in;
		}

		/// <summary>
		/// Encodes UTF-32LE from UTF-32 (copies 'as is'), UTF-16 or UTF-8.
		///	Returns iterator to the last not encoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Encode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::WriteErrorMark, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(TOutChar) == sizeof(char32_t), "Output string must be 32-bit characters (e.g. std::u32string)");

			using TInCharType = decltype(*in);
			if constexpr (sizeof(TInCharType) == sizeof(char_type))
			{
				for (; in != end; ++in) {
					outStr.push_back(static_cast<TOutChar>(*in));
				}
			}
			else if constexpr (sizeof(TInCharType) == sizeof(Utf16Le::char_type))
			{
				return Utf16Le::Decode(in, end, outStr, encodePolicy, errorMark);
			}
			else if constexpr (sizeof(TInCharType) == sizeof(Utf8::char_type))
			{
				return Utf8::Decode(in, end, outStr, encodePolicy, errorMark);
			}
			return in;
		}
	};


	class Utf32Be
	{
		template <typename TBaseIt>
		class U32BeConstIterator
		{
		public:
			explicit U32BeConstIterator(TBaseIt it)
				: mBaseIt(std::move(it))
			{ }

			auto operator*() const noexcept {
				return (*mBaseIt >> 24) | ((*mBaseIt << 8) & 0x00FF0000) | ((*mBaseIt >> 8) & 0x0000FF00) | (*mBaseIt << 24);
			}

			bool operator==(const TBaseIt& rhs) const { return mBaseIt == rhs; }
			bool operator!=(const TBaseIt& rhs) const { return mBaseIt != rhs; }
			TBaseIt& operator++() noexcept { return ++mBaseIt; }
			operator const TBaseIt&() const { return mBaseIt; }

		private:
			TBaseIt mBaseIt;
		};

	public:
		using char_type = char32_t;
		static constexpr UtfType utfType = UtfType::Utf32be;
		static constexpr char bom[] = { '\x00', '\x00', '\xFE', '\xFF' };
		static constexpr bool lowEndian = false;

		/// <summary>
		/// Decodes UTF-32BE to UTF-32, UTF-16 or UTF-8.
		///	Returns iterator to the last not decoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Decode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::WriteErrorMark, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			static_assert(sizeof(decltype(*in)) == sizeof(char_type), "Input stream should represents sequence of 32-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char) || sizeof(TOutChar) == sizeof(char16_t) || sizeof(TOutChar) == sizeof(char32_t), "Output string should have 8, 16 or 32-bit characters");

			return Utf32Le::Decode(U32BeConstIterator<TInIt>(in), U32BeConstIterator<TInIt>(end), outStr, encodePolicy, errorMark);
		}

		/// <summary>
		/// Encodes UTF-32BE from UTF-32, UTF-16 or UTF-8
		///	Returns iterator to the last not encoded character if the sequence (tails or surrogates) breaks at end of the input string.
		/// </summary>
		template<typename TInIt, typename TOutChar, typename TAllocator>
		static TInIt Encode(TInIt in, const TInIt end, std::basic_string<TOutChar, std::char_traits<TOutChar>, TAllocator>& outStr,
			EncodeErrorPolicy encodePolicy = EncodeErrorPolicy::WriteErrorMark, const TOutChar* errorMark = Detail::GetDefaultErrorMark<TOutChar>())
		{
			using TInCharType = decltype(*in);
			static_assert(sizeof(TInCharType) == sizeof(char) || sizeof(TInCharType) == sizeof(char16_t) || sizeof(TInCharType) == sizeof(char32_t), "Input stream should represents sequence of 8, 16 or 32-bit characters");
			static_assert(sizeof(TOutChar) == sizeof(char_type), "Output string must be 32-bit characters (e.g. std::u32string)");

			const size_t startOutPos = outStr.size();
			auto it = Utf32Le::Encode(in, end, outStr, encodePolicy, errorMark);

			std::for_each(outStr.begin() + startOutPos, outStr.end(), [](TOutChar& sym) {
				sym = (sym >> 24) | ((sym << 8) & 0x00FF0000) | ((sym >> 8) & 0x0000FF00) | (sym << 24);
			});
			return it;
		}
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
	/// Detects encoding of string.
	/// </summary>
	static UtfType DetectEncoding(std::string_view inputString, size_t& out_dataOffset)
	{
		if (inputString.empty())
		{
			out_dataOffset = 0;
			return UtfType::Utf8;
		}

		out_dataOffset = 0;
		UtfType utfType = UtfType::Utf8;
		if (StartsWithBom<Utf8>(inputString))
		{
			out_dataOffset = sizeof Utf8::bom;
			utfType = UtfType::Utf8;
		}
		else if (StartsWithBom<Utf32Le>(inputString))
		{
			out_dataOffset = sizeof Utf32Le::bom;
			utfType = UtfType::Utf32le;
		}
		else if (StartsWithBom<Utf32Be>(inputString))
		{
			out_dataOffset = sizeof Utf32Be::bom;
			utfType = UtfType::Utf32be;
		}
		else if (StartsWithBom<Utf16Le>(inputString))
		{
			out_dataOffset = sizeof Utf16Le::bom;
			utfType = UtfType::Utf16le;
		}
		else if (StartsWithBom<Utf16Be>(inputString))
		{
			out_dataOffset = sizeof Utf16Be::bom;
			utfType = UtfType::Utf16be;
		}
		else
		{
			// Analysis data
			for (size_t i = 0; i < inputString.size(); ++i)
			{
				// Detecting UTF-32 (LE/BE)
				if (i % sizeof(Utf32Le::char_type) == 0 && i + sizeof(Utf32Le::char_type) < inputString.size())
				{
					if (const uint32_t sym = *reinterpret_cast<const uint32_t*>(&inputString[i]); sym != 0)
					{
						if ((sym & 0b11111111111111110000000000000000) == 0)
						{
							utfType = UtfType::Utf32le;
							break;
						}
						else if ((sym & 0b00000000000000001111111111111111) == 0)
						{
							utfType = UtfType::Utf32be;
							break;
						}
					}
				}
				// Detecting UTF-16 (LE/BE)
				if (i % sizeof(Utf16Le::char_type) == 0 && i + sizeof(Utf16Le::char_type) < inputString.size())
				{
					if (const uint16_t sym = *reinterpret_cast<const uint16_t*>(&inputString[i]); sym != 0)
					{
						if ((sym & 0b1111111100000000) == 0)
						{
							utfType = UtfType::Utf16le;
							break;
						}
						else if ((sym & 0b0000000011111111) == 0)
						{
							utfType = UtfType::Utf16be;
							break;
						}
					}
				}
			}
		}
		return utfType;
	}


	/// <summary>
	/// Detects an encoding of stream.
	/// </summary>
	static UtfType DetectEncoding(std::istream& inputStream, bool skipBomWhenFound = true)
	{
		// Read first characters to temporary buffer
		static constexpr size_t tempBufferSize = 128;
		char buffer[tempBufferSize];
		const auto origPos = inputStream.tellg();
		inputStream.read(buffer, tempBufferSize);
		const auto lastReadSize = static_cast<size_t>(inputStream.gcount());

		// Detect encoding
		size_t dataOffset = 0;
		const auto detectedUtf = DetectEncoding(std::string_view(buffer, lastReadSize), dataOffset);

		// Get back to stream position
		if (inputStream.eof())
		{
			// Need reset EOF for able to rewind stream
			inputStream.clear();
		}
		if (skipBomWhenFound)
		{
			if (lastReadSize != dataOffset)
			{
				inputStream.seekg(origPos + static_cast<std::streamoff>(dataOffset));
			}
		}
		else
		{
			inputStream.seekg(origPos);
		}
		return detectedUtf;
	}


	/// <summary>
	/// Writes BOM (Byte order mark) to output stream.
	/// </summary>
	static void WriteBom(std::ostream& outputStream, UtfType encoding)
	{
		switch (encoding)
		{
		case UtfType::Utf8:
			outputStream.write(Utf8::bom, sizeof Utf8::bom);
			break;
		case UtfType::Utf16le:
			outputStream.write(Utf16Le::bom, sizeof Utf16Le::bom);
			break;
		case UtfType::Utf16be:
			outputStream.write(Utf16Be::bom, sizeof Utf16Be::bom);
			break;
		case UtfType::Utf32le:
			outputStream.write(Utf32Le::bom, sizeof Utf32Le::bom);
			break;
		case UtfType::Utf32be:
			outputStream.write(Utf32Be::bom, sizeof Utf32Be::bom);
			break;
		}
	}


	/// <summary>
	/// Allows to read streams in various UTF encodings with automatic detection.
	/// </summary>
	template <typename TTargetUtfType, size_t ChunkSize = 256>
	class CEncodedStreamReader
	{
	public:
		using utf_type = TTargetUtfType;
		using target_char_type = typename TTargetUtfType::char_type;
		static constexpr size_t chunk_size = ChunkSize;

		CEncodedStreamReader(const CEncodedStreamReader&) = delete;
		CEncodedStreamReader(CEncodedStreamReader&& encodedStreamReader) = delete;
		CEncodedStreamReader& operator=(const CEncodedStreamReader&) = delete;
		CEncodedStreamReader& operator=(CEncodedStreamReader&&) = delete;
		~CEncodedStreamReader() = default;

		CEncodedStreamReader(std::istream& inputStream, EncodeErrorPolicy encodeErrorPolicy = EncodeErrorPolicy::WriteErrorMark,
			const target_char_type* errorMark = Detail::GetDefaultErrorMark<target_char_type>())
			: mInputStream(inputStream)
			, mEncodeErrorPolicy(encodeErrorPolicy)
			, mErrorMark(errorMark)
		{
			static_assert((ChunkSize % 4) == 0, "Chunk size size must be a multiple of 4");
			static_assert(std::is_same_v<TTargetUtfType, Utf8> || std::is_same_v<TTargetUtfType, Utf16Le> || std::is_same_v<TTargetUtfType, Utf32Le>, 
				"TTargetUtfType can be only UTF-8, UTF-16Le or UTF-32Le");

			if (ReadNextEncodedChunk())
			{
				size_t bomSize = 0;
				mUtfType = DetectEncoding(std::string_view(mStartDataPtr, mEndDataPtr - mStartDataPtr), bomSize);
				mStartDataPtr += bomSize;
			}
		}

		template<typename TAllocator>
		bool ReadChunk(std::basic_string<target_char_type, std::char_traits<target_char_type>, TAllocator>& outStr)
		{
			if (IsEnd())
			{
				return false;
			}

			if (!ReadNextEncodedChunk() && mStartDataPtr == mEndDataPtr)
			{
				return false;
			}

			const auto prevOutSize = outStr.size();
			switch (mUtfType)
			{
			case UtfType::Utf8:
				if constexpr (std::is_same_v<TTargetUtfType, Utf8>)
				{
					outStr.append(mStartDataPtr, mEndDataPtr);
					mStartDataPtr = mEndDataPtr = mEncodedBuffer;
				}
				else
				{
					mStartDataPtr = TTargetUtfType::Encode(mStartDataPtr, mEndDataPtr, outStr, mEncodeErrorPolicy, mErrorMark);
				}
				break;
			case UtfType::Utf16le:
				mStartDataPtr = reinterpret_cast<char*>(Utf16Le::Decode(
					reinterpret_cast<Utf16Le::char_type*>(mStartDataPtr), GetAlignedEndDataPtr<Utf16Le::char_type>(), outStr, mEncodeErrorPolicy, mErrorMark));
				break;
			case UtfType::Utf16be:
				mStartDataPtr = reinterpret_cast<char*>(Utf16Be::Decode(
					reinterpret_cast<Utf16Be::char_type*>(mStartDataPtr), GetAlignedEndDataPtr<Utf16Be::char_type>(), outStr, mEncodeErrorPolicy, mErrorMark));
				break;
			case UtfType::Utf32le:
				mStartDataPtr = reinterpret_cast<char*>(Utf32Le::Decode(
					reinterpret_cast<Utf32Le::char_type*>(mStartDataPtr), GetAlignedEndDataPtr<Utf32Le::char_type>(), outStr, mEncodeErrorPolicy, mErrorMark));
				break;
			case UtfType::Utf32be:
				mStartDataPtr = reinterpret_cast<char*>(Utf32Be::Decode(
					reinterpret_cast<Utf32Be::char_type*>(mStartDataPtr), GetAlignedEndDataPtr<Utf32Be::char_type>(), outStr, mEncodeErrorPolicy, mErrorMark));
				break;
			default:
				break;
			}
			assert(mStartDataPtr <= mEndDataPtr);

			// Process as error when left few not decoded bytes at the end of stream (not complete UTF sequence)
			if (mInputStream.eof() && mStartDataPtr != mEndDataPtr)
			{
				Detail::HandleEncodingError(outStr, mEncodeErrorPolicy, mErrorMark);
				mStartDataPtr = mEndDataPtr = mEncodedBuffer;
			}

			return prevOutSize != outStr.size();
		}

		[[nodiscard]] bool IsEnd() const {
			return mStartDataPtr == mEndDataPtr && mInputStream.eof();
		}

		[[nodiscard]] UtfType GetSourceUtfType() const noexcept {
			return mUtfType;
		}

	private:
		template <typename T>
		T* GetAlignedEndDataPtr() noexcept {
			return reinterpret_cast<T*>(mEndDataPtr - ((mEndDataPtr - mStartDataPtr) % sizeof(T)));
		}

		bool ReadNextEncodedChunk()
		{
			if (mStartDataPtr == mEndBufferPtr)
			{
				mStartDataPtr = mEndDataPtr = mEncodedBuffer;
			}
			else if (mStartDataPtr != mEncodedBuffer)
			{
				// Squeeze buffer
				std::memcpy(mEncodedBuffer, mStartDataPtr, mEndDataPtr - mStartDataPtr);
				mEndDataPtr -= mStartDataPtr - mEncodedBuffer;
				mStartDataPtr = mEncodedBuffer;
			}

			// Read next chunk
			mInputStream.read(mEndDataPtr, static_cast<std::streamsize>(mEndBufferPtr - mEndDataPtr));
			const auto lastReadSize = mInputStream.gcount();
			mEndDataPtr += lastReadSize;
			assert(mStartDataPtr >= mEncodedBuffer && mStartDataPtr <= mEndDataPtr);
			return lastReadSize != 0;
		}

		UtfType mUtfType;
		std::istream& mInputStream;
		EncodeErrorPolicy mEncodeErrorPolicy;
		const target_char_type* mErrorMark;
		char mEncodedBuffer[ChunkSize];
		char* const mEndBufferPtr = mEncodedBuffer + ChunkSize;
		char* mStartDataPtr = mEncodedBuffer;
		char* mEndDataPtr = mEncodedBuffer;
	};
}
