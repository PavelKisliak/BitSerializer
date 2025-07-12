/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstring>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include "bitserializer/conversion_detail/convert_utf.h"

namespace BitSerializer::Convert::Detail
{
	namespace _stdWrappers
	{
		template <typename T> T _fromStr(const char*, char**) { throw; }
		template <typename T> T _fromStr(const wchar_t*, wchar_t**) { throw; }

		template <>	inline float _fromStr<float>(const char* str, char** out_strEnd) { return std::strtof(str, out_strEnd); }
		template <>	inline float _fromStr<float>(const wchar_t* str, wchar_t** out_strEnd) { return std::wcstof(str, out_strEnd); }

		template <>	inline double _fromStr<double>(const char* str, char** out_strEnd) { return std::strtod(str, out_strEnd); }
		template <>	inline double _fromStr<double>(const wchar_t* str, wchar_t** out_strEnd) { return std::wcstod(str, out_strEnd); }

		template <>	inline long double _fromStr<long double>(const char* str, char** out_strEnd) { return std::strtold(str, out_strEnd); }
		template <>	inline long double _fromStr<long double>(const wchar_t* str, wchar_t** out_strEnd) { return std::wcstold(str, out_strEnd); }
	}

	/**
	 * @brief Converts a UTF string to a floating-point number.
	 *
	 * @param in Input string view containing the numeric value.
	 * @param out Output variable to store the parsed value.
	 * @throws std::invalid_argument If parsing fails or input is not a valid number.
	 * @throws std::out_of_range If result exceeds representable range.
	 */
	template <typename T, typename TSym, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
	void To(std::basic_string_view<TSym> in, T& out)
	{
		T result;
		bool isNaN;
		constexpr size_t bufSize = 64;
		const size_t size = (in.size() < bufSize) ? in.size() : bufSize - 1;
		if constexpr (std::is_same_v<char, TSym>)
		{
			// Copy to temporary buffer for prepare null-terminated c-string
			char buf[bufSize];
			std::memcpy(buf, in.data(), size);
			buf[size] = 0;
			char* endPos = nullptr;
			result = _stdWrappers::_fromStr<T>(buf, &endPos);
			isNaN = buf == endPos;
		}
		else if constexpr (sizeof(TSym) == sizeof(wchar_t))
		{
			wchar_t buf[bufSize];
			std::wmemcpy(buf, reinterpret_cast<wchar_t const*>(in.data()), size);
			buf[size] = 0;
			wchar_t* endPos = nullptr;
			result = _stdWrappers::_fromStr<T>(buf, &endPos);
			isNaN = buf == endPos;
		}
		else
		{
			std::string utf8Str;
			Utf::Utf8::Encode(in.cbegin(), in.cend(), utf8Str);
			char* endPos = nullptr;
			result = _stdWrappers::_fromStr<T>(utf8Str.c_str(), &endPos);
			isNaN = utf8Str.data() == endPos;
		}

		if (result == std::numeric_limits<T>::infinity()) {
			throw std::out_of_range("Argument out of range");
		}
		if (isNaN) {
			throw std::invalid_argument("Input string is not a number");
		}
		out = result;
	}

	namespace _formatTemplates
	{
		template <typename T> const char* _get() { throw; }
		template <typename T> const wchar_t* _getW() { throw; }

		template <>	constexpr const char* _get<float>() { return "%.7g"; }
		template <>	constexpr const wchar_t* _getW<float>() { return L"%.7g"; }

		template <>	constexpr const char* _get<double>() { return "%.15g"; }
		template <>	constexpr const wchar_t* _getW<double>() { return L"%.15g"; }

		template <>	constexpr const char* _get<long double>() { return "%.15Lg"; }
		template <>	constexpr const wchar_t* _getW<long double>() { return L"%.15Lg"; }
	}

	/**
	 * @brief Converts a floating-point number to a UTF string representation.
	 *
	 * @param in Value to convert.
	 * @param out Output string object.
	 * @throws std::overflow_error If internal buffer is insufficient.
	 */
	template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
	void To(const T& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		constexpr auto bufSize = std::numeric_limits<T>::digits + 1;
		if constexpr (sizeof(TSym) == sizeof(wchar_t))
		{
			wchar_t buf[bufSize];
			const int result = swprintf(buf, bufSize, _formatTemplates::_getW<T>(), in);
			if (result < 0 || result >= bufSize) {
				throw std::overflow_error("Internal error");
			}
			out.append(std::cbegin(buf), std::cbegin(buf) + result);
		}
		else
		{
			char buf[bufSize];
			const int result = snprintf(buf, bufSize, _formatTemplates::_get<T>(), in);
			if (result < 0 || result >= bufSize) {
				throw std::overflow_error("Internal error");
			}
			if constexpr (std::is_same_v<char, TSym>) {
				out.append(std::cbegin(buf), std::cbegin(buf) + result);
			}
			else {
				Utf::Utf8::Decode(buf, buf + result, out);
			}
		}
	}
}
