/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <cstdint>

/// <summary>
/// Implementation detail for convert fundamental types
/// </summary>
namespace BitSerializer::Convert::Detail::Fundamental {

// Convert to string from any fundamental types
template <typename T> inline void ToString(T val, std::string& ret_Str)			{ ret_Str = std::to_string(val); }
template <typename T> inline void ToString(T val, std::wstring& ret_Str)		{ ret_Str = std::to_wstring(val); }

// Convert from unsigned decimal to string
inline void FromString(const std::string& str, unsigned int& ret_Val)			{ ret_Val = std::stoul(str); }
inline void FromString(const std::wstring& str, unsigned int& ret_Val)			{ ret_Val = std::stoul(str); }
inline void FromString(const std::string& str, unsigned long& ret_Val)			{ ret_Val = std::stoul(str); }
inline void FromString(const std::wstring& str, unsigned long& ret_Val)			{ ret_Val = std::stoul(str); }
inline void FromString(const std::string& str, unsigned long long& ret_Val)		{ ret_Val = std::stoull(str); }
inline void FromString(const std::wstring& str, unsigned long long& ret_Val)	{ ret_Val = std::stoull(str); }

// Convert from signed decimal to string
inline void FromString(const std::string& str, int& ret_Val)					{ ret_Val = std::stoi(str); }
inline void FromString(const std::wstring& str, int& ret_Val)					{ ret_Val = std::stoi(str); }
inline void FromString(const std::string& str, long& ret_Val)					{ ret_Val = std::stol(str); }
inline void FromString(const std::wstring& str, long& ret_Val)					{ ret_Val = std::stol(str); }
inline void FromString(const std::string& str, long long& ret_Val)				{ ret_Val = std::stoll(str); }
inline void FromString(const std::wstring& str, long long& ret_Val)				{ ret_Val = std::stoll(str); }

// Convert from string to boolean
inline void FromString(const std::string& str, bool& ret_Val)					{ ret_Val = std::stoi(str) ? true : false; }
inline void FromString(const std::wstring& str, bool& ret_Val)					{ ret_Val = std::stoi(str) ? true : false; }

// Convert from string to number with floating point
inline void FromString(const std::string& str, float& ret_Val)					{ ret_Val = std::stof(str); }
inline void FromString(const std::wstring& str, float& ret_Val)					{ ret_Val = std::stof(str); }
inline void FromString(const std::string& str, double& ret_Val)					{ ret_Val = std::stod(str); }
inline void FromString(const std::wstring& str, double& ret_Val)				{ ret_Val = std::stod(str); }
inline void FromString(const std::string& str, long double& ret_Val)			{ ret_Val = std::stold(str); }
inline void FromString(const std::wstring& str, long double& ret_Val)			{ ret_Val = std::stold(str); }

}	// BitSerializer::Convert::Detail::Fundamental
