/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <cstdint>
#include <limits>

/// <summary>
/// Implementation detail for convert fundamental types
/// </summary>
namespace BitSerializer::Convert::Detail::Fundamental {

// Convert to string from any fundamental types
template <typename T> inline void To(T val, std::string& ret_Str)		{ ret_Str = std::to_string(val); }
template <typename T> inline void To(T val, std::wstring& ret_Str)		{ ret_Str = std::to_wstring(val); }

// Convert from unsigned decimal to string
template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_unsigned_v<T>), int> = 0>
inline void To(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& str, T& ret_Val)
{
	auto result = std::stoul(str, 0, 0);
	if (result < std::numeric_limits<T>::min() || std::numeric_limits<T>::max() < result) {
		throw std::out_of_range("argument out of range");
	}
	ret_Val = static_cast<T>(result);
}
inline void To(const std::string& str, unsigned long long& ret_Val)		{ ret_Val = std::stoull(str, 0, 0); }
inline void To(const std::wstring& str, unsigned long long& ret_Val)	{ ret_Val = std::stoull(str, 0, 0); }

// Convert from signed decimal to string
template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_signed_v<T>), int> = 0>
inline void To(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& str, T& ret_Val)
{
	auto result = std::stol(str, 0, 0);
	if (result < std::numeric_limits<T>::min() || std::numeric_limits<T>::max() < result) {
		throw std::out_of_range("argument out of range");
	}
	ret_Val = static_cast<T>(result);
}
inline void To(const std::string& str, long long& ret_Val)				{ ret_Val = std::stoll(str, 0, 0); }
inline void To(const std::wstring& str, long long& ret_Val)				{ ret_Val = std::stoll(str, 0, 0); }

// Convert from string to boolean
inline void To(const std::string& str, bool& ret_Val)					{ ret_Val = std::stoi(str) ? true : false; }
inline void To(const std::wstring& str, bool& ret_Val)					{ ret_Val = std::stoi(str) ? true : false; }

// Convert from string to number with floating point
inline void To(const std::string& str, float& ret_Val)					{ ret_Val = std::stof(str); }
inline void To(const std::wstring& str, float& ret_Val)					{ ret_Val = std::stof(str); }
inline void To(const std::string& str, double& ret_Val)					{ ret_Val = std::stod(str); }
inline void To(const std::wstring& str, double& ret_Val)				{ ret_Val = std::stod(str); }
inline void To(const std::string& str, long double& ret_Val)			{ ret_Val = std::stold(str); }
inline void To(const std::wstring& str, long double& ret_Val)			{ ret_Val = std::stold(str); }

}	// BitSerializer::Convert::Detail::Fundamental
