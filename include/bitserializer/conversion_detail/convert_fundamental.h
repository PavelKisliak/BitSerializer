/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <charconv>
#include <limits>
#include <stdexcept>
#include <string>
#include "convert_utf.h"

namespace BitSerializer::Convert::Detail
{
	/// <summary>
	/// Converts any UTF string to integer types.
	/// </summary>
	template <typename T, typename TSym, std::enable_if_t<(std::is_integral_v<T>), int> = 0>
	void To(const std::basic_string_view<TSym>& in, T& out)
	{
		const auto* it = in.data();
		const auto* end = it + in.size();

		// ReSharper disable once CppPossiblyErroneousEmptyStatements
		for (; (it != end) && (*it == 0x20 || *it == 0x09); ++it);	// Skip spaces

		if constexpr (sizeof(TSym) == sizeof(char)) {
			// Uses from_chars only for convert only integers as GCC does not support floating types
			if (std::from_chars(it, end, out).ec != std::errc()) {
				throw std::out_of_range("Argument out of range");
			}
		}
		else {
			std::string utf8Str;
			Utf8::Encode(it, end, utf8Str);
			if (std::from_chars(utf8Str.data(), utf8Str.data() + utf8Str.size(), out).ec != std::errc()) {
				throw std::out_of_range("Argument out of range");
			}
		}
	}

	namespace _stdWrappers
	{
		template <typename T> T _fromStr(const char*) { throw; }
		template <typename T> T _fromStr(const wchar_t*) { throw; }

		template <>	inline float _fromStr<float>(const char* str) { return std::strtof(str, nullptr); }
		template <>	inline float _fromStr<float>(const wchar_t* str) { return std::wcstof(str, nullptr); }

		template <>	inline double _fromStr<double>(const char* str) { return std::strtod(str, nullptr); }
		template <>	inline double _fromStr<double>(const wchar_t* str) { return std::wcstod(str, nullptr); }

		template <>	inline long double _fromStr<long double>(const char* str) { return std::strtold(str, nullptr); }
		template <>	inline long double _fromStr<long double>(const wchar_t* str) { return std::wcstold(str, nullptr); }
	}

	/// <summary>
	/// Converts any UTF string to floating types.
	/// </summary>
	template <typename T, typename TSym, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
	void To(const std::basic_string_view<TSym>& in, T& out)
	{
		T result;
		static constexpr size_t bufSize = 64;
		const auto size = (in.size() < bufSize) ? in.size() : bufSize - 1;
		if constexpr (std::is_same_v<char, TSym>) {
			// Copy to temporary buffer for prepare null-terminated c-string
			TSym buf[bufSize];
			std::memcpy(buf, in.data(), size);
			buf[size] = 0;
			result = _stdWrappers::_fromStr<T>(buf);
		}
		else if constexpr (sizeof(TSym) == sizeof(wchar_t)) {
			wchar_t buf[bufSize];
			std::wmemcpy(buf, reinterpret_cast<wchar_t const*>(in.data()), size);
			buf[size] = 0;
			result = _stdWrappers::_fromStr<T>(buf);
		}
		else
		{
			std::string utf8Str;
			Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
			result = _stdWrappers::_fromStr<T>(utf8Str.c_str());
		}

		if (errno == ERANGE) {
			throw std::out_of_range("Argument out of range");
		}

		out = static_cast<T>(result);
	}

	/// <summary>
	/// Converts any UTF string to boolean.
	/// </summary>
	template <typename TSym>
	void To(const std::basic_string_view<TSym>& in, bool& ret_Val)
	{
		int32_t result = 0;
		To(in, result);
		
		if (result < 0) {
			throw std::out_of_range("Argument out of range");
		}

		ret_Val = result ? true : false;
	}

	//------------------------------------------------------------------------------

	/// <summary>
	/// Converts any integer types to any UTF string.
	/// </summary>
	template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_integral_v<T>), int> = 0>
	void To(const T& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		if constexpr (std::is_same_v<char, TSym>) {
			out = std::to_string(in);
		}
		else if constexpr (std::is_same_v<TSym, wchar_t>) {
			out = std::to_wstring(in);
		}
		else
		{
			const auto utf8Str = std::to_string(in);
			Utf8::Decode(utf8Str.cbegin(), utf8Str.cend(), out);
		}
	}

	namespace _formatTemplates
	{
		template <typename T> constexpr const char* _get() { throw; }
		template <typename T> constexpr const wchar_t* _getW() { throw; }

		template <>	constexpr const char* _get<float>() { return "%.6g"; }
		template <>	constexpr const wchar_t* _getW<float>() { return L"%.6g"; }

		template <>	constexpr const char* _get<double>() { return "%.15g"; }
		template <>	constexpr const wchar_t* _getW<double>() { return L"%.15g"; }

		template <>	constexpr const char* _get<long double>() { return "%.15Lg"; }
		template <>	constexpr const wchar_t* _getW<long double>() { return L"%.15Lg"; }
	}

	/// <summary>
	/// Converts any floating point types to any UTF string.
	/// </summary>
	template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
	void To(const T& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		constexpr auto bufSize = std::numeric_limits<T>::digits + 1;
		if constexpr (std::is_same_v<char, TSym>) {
			char buf[bufSize];
			const int result = snprintf(buf, bufSize, _formatTemplates::_get<T>(), in);
			if (result == -1) {
				throw std::out_of_range("Argument out of range");
			}
			out.append(std::cbegin(buf), std::cbegin(buf) + result);
		}
		else if constexpr (sizeof(TSym) == sizeof(wchar_t)) {
			wchar_t buf[bufSize];
			const int result = swprintf(buf, bufSize, _formatTemplates::_getW<T>(), in);
			if (result == -1) {
				throw std::out_of_range("Argument out of range");
			}
			out.append(std::cbegin(buf), std::cbegin(buf) + result);
		}
		else
		{
			char buf[bufSize];
			const int result = snprintf(buf, bufSize, _formatTemplates::_get<T>(), in);
			if (result == -1) {
				throw std::out_of_range("Argument out of range");
			}
			Utf8::Decode(buf, buf + result, out);
		}
	}

	/// <summary>
	/// Converts boolean type to any UTF string.
	/// </summary>
	template <class T, typename TSym, typename TAllocator>
	void To(const bool& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		out.push_back(in ? '1' : '0');
	}
}
