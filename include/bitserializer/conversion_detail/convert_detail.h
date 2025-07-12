/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/conversion_detail/convert_utf.h"
#include "bitserializer/conversion_detail/object_traits.h"

namespace BitSerializer::Convert::Detail
{
	/**
	 * @brief Converts any UTF string to another UTF encoding format.
	 *
	 * Supports conversion between UTF-8, UTF-16, and UTF-32 encoded strings.
	 *
	 * @param in Input string view.
	 * @param out Output string object.
	 * @throws std::invalid_argument If the source string contains invalid UTF sequences.
	 */
	template <typename TInSym, typename TOutSym, typename TAllocator>
	void To(std::basic_string_view<TInSym> in, std::basic_string<TOutSym, std::char_traits<TOutSym>, TAllocator>& out)
	{
		if (!Utf::Transcode(in, out, Utf::UtfEncodingErrorPolicy::ThrowError)) 	{
			throw std::invalid_argument("The source string contains an invalid UTF sequence");
		}
	}

	//-----------------------------------------------------------------------------

	/**
	 * @brief Converts classes and unions to any UTF string.
	 *
	 * Classes can have external overloads of this function or internal convert method(s) like below:
	 * - Member functions: `ToString()`, `ToU16String()`, `ToU32String()`
	 * - Global function: `to_string(const T&)`
	 *
	 * Instead of these return types also can be used `std::basic_string<>` with custom allocator.
	 * Not all of these methods are required, but to avoid performance losses (for transcoding),
	 * it is recommended to implement conversion methods for most commonly used types.
	 */
	template <class T, typename TSym, typename TAllocator, std::enable_if_t<(has_any_internal_ToString_v<T> || has_global_to_string_v<T, std::basic_string<char, std::char_traits<char>>>), int> = 0>
	void To(const T& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		constexpr auto hasGlobalToString = has_global_to_string_v<T, std::basic_string<char, std::char_traits<char>>>;
		constexpr auto hasInternalToString = has_internal_ToString_v<T, std::basic_string<char, std::char_traits<char>>>;
		constexpr auto hasInternalToUtf16String = has_internal_ToString_v<T, std::u16string>;
		constexpr auto hasInternalToUtf32String = has_internal_ToString_v<T, std::u32string>;

		static_assert(hasGlobalToString || hasInternalToString || hasInternalToUtf16String || hasInternalToUtf32String);

		if constexpr (sizeof(TSym) == sizeof(char))
		{
			// The best path - use conversion methods with the same char type
			if constexpr (hasInternalToString) {
				out.append(in.ToString());
			} else if constexpr (hasGlobalToString) {
				out.append(to_string(in));
			// Otherwise use conversion methods for other encodings with transcoding to UTF-8
			} else if constexpr (hasInternalToUtf16String) {
				const auto utf16str = in.ToU16String();
				Utf::Utf8::Encode(utf16str.cbegin(), utf16str.cend(), out);
			} else if constexpr (hasInternalToUtf32String) {
				const auto utf32str = in.ToU32String();
				Utf::Utf8::Encode(utf32str.cbegin(), utf32str.cend(), out);
			}
		}
		else if constexpr (sizeof(TSym) == sizeof(char16_t))
		{
			// The best path - use conversion methods with the same char type
			if constexpr (hasInternalToUtf16String) {
				out.append(in.ToU16String());
			// Otherwise use conversion methods for other encodings with transcoding to UTF-16
			} else if constexpr (hasInternalToUtf32String) {
				const auto utf32Str = in.ToU32String();
				Utf::Utf16::Encode(utf32Str.cbegin(), utf32Str.cend(), out);
			} else if constexpr (hasInternalToString) {
				const auto utf8Str = in.ToString();
				Utf::Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
			} else if constexpr (hasGlobalToString) {
				const auto utf8Str = to_string(in);
				Utf::Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
			}
		}
		else if constexpr (sizeof(TSym) == sizeof(char32_t))
		{
			// The best path - use conversion methods with the same char type
			if constexpr (hasInternalToUtf32String) {
				out.append(in.ToU32String());
			}
			// Otherwise use conversion methods for other encodings with transcoding to UTF-32
			else if constexpr (hasInternalToUtf16String) {
				const auto utf16Str = in.ToU16String();
				Utf::Utf16::Decode(utf16Str.cbegin(), utf16Str.cend(), out);
			} else if constexpr (hasInternalToString) {
				auto utf8Str = in.ToString();
				Utf::Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
			} else if constexpr (hasGlobalToString) {
				auto utf8Str = to_string(in);
				Utf::Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
			}
		}
	}

	/**
	 * @brief Converts any UTF strings to classes and unions.
	 *
	 * Classes can have external overloads of this function or internal convert method(s) like below:
	 * - `void FromString(std::string_view)`
	 * - `void FromString(std::u16string_view)`
	 * - `void FromString(std::u32string_view)`
	 *
	 * Not all of these methods are required, but to avoid performance losses (for transcoding),
	 * it is recommended to implement conversion methods for most commonly used types.
	 * You also could implement templated FromString method which converts any string types.
	 */
	template <class T, typename TSym, std::enable_if_t<has_any_internal_FromString_v<T>, int> = 0>
	void To(const std::basic_string_view<TSym, std::char_traits<TSym>>& in, T& out)
	{
		constexpr auto hasInternalFromString_Utf8 = has_internal_FromString_v<T, std::basic_string_view<char>>;
		constexpr auto hasInternalFromString_Utf16 = has_internal_FromString_v<T, std::basic_string_view<char16_t>>;
		constexpr auto hasInternalFromString_Utf32 = has_internal_FromString_v<T, std::basic_string_view<char32_t>>;

		static_assert(hasInternalFromString_Utf8 || hasInternalFromString_Utf16 || hasInternalFromString_Utf32);

		if constexpr (sizeof(TSym) == sizeof(char))
		{
			// The best path - use conversion methods with the same char type
			if constexpr (hasInternalFromString_Utf8) {
				out.FromString(std::string_view(static_cast<const char*>(in.data()), in.size()));
			}
			// Otherwise use conversion methods for other encodings with transcoding
			else if constexpr (hasInternalFromString_Utf16)
			{
				std::u16string utf16Str;
				Utf::Utf16::Encode(in.cbegin(), in.cend(), utf16Str);
				out.FromString(utf16Str);
			}
			else if constexpr (hasInternalFromString_Utf32)
			{
				std::u32string utf32Str;
				Utf::Utf32::Encode(in.cbegin(), in.cend(), utf32Str);
				out.FromString(utf32Str);
			}
		}
		else if constexpr (sizeof(TSym) == sizeof(char16_t))
		{
			// The best path - use conversion methods with the same char type
			if constexpr (hasInternalFromString_Utf16) {
				out.FromString(std::u16string_view(reinterpret_cast<std::u16string_view::const_pointer>(in.data()), in.size()));
			}
			// Otherwise use conversion methods for other encodings with transcoding
			else if constexpr (hasInternalFromString_Utf8) {
				std::string utf8Str;
				Utf::Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
				out.FromString(utf8Str);
			}
			else if constexpr (hasInternalFromString_Utf32)
			{
				std::u32string utf32Str;
				Utf::Utf32::Encode(in.cbegin(), in.cend(), utf32Str);
				out.FromString(utf32Str);
			}
		}
		else if constexpr (sizeof(TSym) == sizeof(char32_t))
		{
			// The best path - use conversion methods with the same char type
			if constexpr (hasInternalFromString_Utf32) {
				out.FromString(std::u32string_view(static_cast<const char32_t*>(in.data()), in.size()));
			}
			// Otherwise use conversion methods for other encodings with transcoding
			if constexpr (hasInternalFromString_Utf8) {
				std::string utf8Str;
				Utf::Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
				out.FromString(utf8Str);
			}
			else if constexpr (hasInternalFromString_Utf16) {
				std::u16string utf16Str;
				Utf::Utf16::Encode(in.cbegin(), in.cend(), utf16Str);
				out.FromString(utf16Str);
			}
		}
	}

	//------------------------------------------------------------------------------

	/**
	 * @brief Converts a `basic_string` to its corresponding `string_view` type.
	 *
	 * @param in Input string.
	 * @return String view pointing to the contents of the input string.
	 */
	template <typename TSym, typename TAllocator>
	constexpr std::basic_string_view<TSym> ToStringView(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& in) noexcept {
		return in;
	}

	/**
	 * @brief Converts a C-string to a `string_view`.
	 *
	 * @param in Null-terminated input string.
	 * @return String view pointing to the input string.
	 */
	template <typename TSym>
	constexpr std::basic_string_view<TSym> ToStringView(const TSym* in) noexcept {
		return in;
	}
}
