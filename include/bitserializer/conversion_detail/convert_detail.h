/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "convert_utf.h"
#include "object_traits.h"
#include "convert_enum.h"

namespace BitSerializer::Convert::Detail
{
	/// <summary>
	/// Converts any UTF string to any other UTF format.
	/// </summary>
	template <typename TInSym, typename TOutSym, typename TAllocator>
	void To(std::basic_string_view<TInSym> in, std::basic_string<TOutSym, std::char_traits<TOutSym>, TAllocator>& out)
	{
		// Assign to the same char type
		if constexpr (sizeof(TInSym) == sizeof(TOutSym)) {
			out.append(std::cbegin(in), std::cend(in));
		}
		// Decode from UTF-8 (to UTF-16 or UTF-32)
		else if constexpr (sizeof(TInSym) == sizeof(char)) {
			out.reserve(out.size() + in.size());
			Utf8::Decode(in.cbegin(), in.cend(), out);
		}
		// Encode to UTF-8 (from UTF-16 or UTF-32)
		else if constexpr (sizeof(TOutSym) == sizeof(char)) {
			out.reserve(out.size() + in.size() * 2);
			Utf8::Encode(in.cbegin(), in.cend(), out);
		}
		// Decode from Utf-16 to Utf-32
		else if constexpr (sizeof(TInSym) == sizeof(char16_t)) {
			out.reserve(out.size() + in.size());
			Utf16Le::Decode(in.cbegin(), in.cend(), out);
		}
		// Encode to Utf-16 from Utf-32
		else if constexpr (sizeof(TInSym) == sizeof(char32_t)) {
			out.reserve(out.size() + in.size());
			Utf16Le::Encode(in.cbegin(), in.cend(), out);
		}
	}

	//------------------------------------------------------------------------------

	/// <summary>
	/// Converts any UTF string to enum types.
	/// </summary>
	template <typename T, typename TSym, std::enable_if_t<std::is_enum_v<T>, int> = 0>
	void To(std::basic_string_view<TSym> in, T& out) {
		EnumRegistry<T>::FromString(in, out);
	}

	/// <summary>
	/// Converts enum types to any UTF string
	/// </summary>
	template <typename T, typename TSym, typename TAllocator, std::enable_if_t<std::is_enum_v<T>, int> = 0>
	void To(T val, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& ret_Str) {
		EnumRegistry<T>::ToString(val, ret_Str);
	}

	//-----------------------------------------------------------------------------

	/// <summary>
	/// Converts classes and unions to any UTF string.
	/// Classes can have external overloads of this function or internal convert method(s) like below:
	/// <c>
	///     std::string ToString() const;
	///     std::u16string ToU16String() const;
	///     std::u32string ToU32String() const;
	/// </c>
	/// Instead of these return types also can be used std::basic_string<> with custom allocator.
	/// Not all of these methods are required, but for avoid performance issues (for transcoding),
	/// it is recommended to implement conversion methods for most commonly used types.
	/// </summary>
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
			// At first try to use conversion methods with the same char type
			if constexpr (hasInternalToString) {
				out.append(in.ToString());
			} else if constexpr (hasGlobalToString) {
				out.append(to_string(in));
			// Otherwise use conversion methods for other encodings with transcoding to UTF-8
			} else if constexpr (hasInternalToUtf16String) {
				const auto utf16str = in.ToU16String();
				Utf8::Encode(utf16str.cbegin(), utf16str.cend(), out);
			} else if constexpr (hasInternalToUtf32String) {
				const auto utf32str = in.ToU32String();
				Utf8::Encode(utf32str.cbegin(), utf32str.cend(), out);
			}
		}
		else if constexpr (sizeof(TSym) == sizeof(char16_t))
		{
			// At first try to use conversion methods with the same char type
			if constexpr (hasInternalToUtf16String) {
				out.append(in.ToU16String());
			// Otherwise use conversion methods for other encodings with transcoding to UTF-16
			} else if constexpr (hasInternalToUtf32String) {
				const auto utf32Str = in.ToU32String();
				Utf16Le::Encode(utf32Str.cbegin(), utf32Str.cend(), out);
			} else if constexpr (hasInternalToString) {
				const auto utf8Str = in.ToString();
				Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
			} else if constexpr (hasGlobalToString) {
				const auto utf8Str = to_string(in);
				Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
			}
		}
		else if constexpr (sizeof(TSym) == sizeof(char32_t))
		{
			// At first try to use conversion methods with the same char type
			if constexpr (hasInternalToUtf32String) {
				out.append(in.ToU32String());
			}
			// Otherwise use conversion methods for other encodings with transcoding to UTF-32
			else if constexpr (hasInternalToUtf16String) {
				const auto utf16Str = in.ToU16String();
				Utf16Le::Decode(utf16Str.cbegin(), utf16Str.cend(), out);
			} else if constexpr (hasInternalToString) {
				auto utf8Str = in.ToString();
				Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
			} else if constexpr (hasGlobalToString) {
				auto utf8Str = to_string(in);
				Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
			}
		}
	}

	/// <summary>
	/// Converts any UTF strings to classes and unions.
	/// Classes can have external overloads of this function or internal convert method(s) like below:
	/// <c>
	///     void FromString(std::string_view str);
	///     void FromString(std::u16string_view str);
	///     void FromString(std::u32string_view str);
	/// </c>
	/// Not all of these methods are required, but for avoid performance issues (for transcoding),
	/// it is recommended to implement conversion methods for most commonly used types.
	/// You also could implement templated FromString method which converts any string types.
	/// </summary>
	template <class T, typename TSym, std::enable_if_t<(has_any_internal_FromString_v<T>), int> = 0>
	void To(const std::basic_string_view<TSym, std::char_traits<TSym>>& in, T& out)
	{
		constexpr auto hasInternalFromString_Utf8 = has_internal_FromString_v<T, std::basic_string_view<char, std::char_traits<char>>>;
		constexpr auto hasInternalFromString_Utf16 = has_internal_FromString_v<T, std::basic_string_view<char16_t, std::char_traits<char16_t>>>;
		constexpr auto hasInternalFromString_Utf32 = has_internal_FromString_v<T, std::basic_string_view<char32_t, std::char_traits<char32_t>>>;

		static_assert(hasInternalFromString_Utf8 || hasInternalFromString_Utf16 || hasInternalFromString_Utf32);

		if constexpr (sizeof(TSym) == sizeof(char))
		{
			// At first try to use conversion methods with the same char type
			if constexpr (hasInternalFromString_Utf8) {
				out.FromString(std::string_view(static_cast<const char*>(in.data()), in.size()));
			}
			// Otherwise use conversion methods for other encodings with transcoding
			else if constexpr (hasInternalFromString_Utf16)
			{
				std::u16string utf16Str;
				Utf16Le::Encode(in.cbegin(), in.cend(), utf16Str);
				out.FromString(utf16Str);
			}
			else if constexpr (hasInternalFromString_Utf32)
			{
				std::u32string utf32Str;
				Utf32Le::Encode(in.cbegin(), in.cend(), utf32Str);
				out.FromString(utf32Str);
			}
		}
		else if constexpr (sizeof(TSym) == sizeof(char16_t))
		{
			// At first try to use conversion methods with the same char type
			if constexpr (hasInternalFromString_Utf16) {
				out.FromString(std::u16string_view(reinterpret_cast<std::u16string_view::const_pointer>(in.data()), in.size()));
			}
			// Otherwise use conversion methods for other encodings with transcoding
			else if constexpr (hasInternalFromString_Utf8) {
				std::string utf8Str;
				Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
				out.FromString(utf8Str);
			}
			else if constexpr (hasInternalFromString_Utf32)
			{
				std::u32string utf32Str;
				Utf32Le::Encode(in.cbegin(), in.cend(), utf32Str);
				out.FromString(utf32Str);
			}
		}
		else if constexpr (sizeof(TSym) == sizeof(char32_t))
		{
			// At first try to use conversion methods with the same char type
			if constexpr (hasInternalFromString_Utf32) {
				out.FromString(std::u32string_view(static_cast<const char32_t*>(in.data()), in.size()));
			}
			// Otherwise use conversion methods for other encodings with transcoding
			if constexpr (hasInternalFromString_Utf8) {
				std::string utf8Str;
				Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
				out.FromString(utf8Str);
			}
			else if constexpr (hasInternalFromString_Utf16) {
				std::u16string utf16Str;
				Utf16Le::Encode(in.cbegin(), in.cend(), utf16Str);
				out.FromString(utf16Str);
			}
		}
	}

	//------------------------------------------------------------------------------

	/// <summary>
	/// Converts any UTF string to suitable string_view.
	/// </summary>
	template <typename TSym, typename TAllocator>
	constexpr std::basic_string_view<TSym> ToStringView(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& in) noexcept {
		return in;
	}

	/// <summary>
	/// Converts any c-string to suitable string_view.
	/// </summary>
	template <typename TSym>
	constexpr std::basic_string_view<TSym> ToStringView(const TSym* in) noexcept {
		return in;
	}
}
