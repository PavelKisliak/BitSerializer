/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <type_traits>

#include "convert_fundamental.h"
#include "object_traits.h"
#include "convert_enum.h"

namespace BitSerializer::Convert::Detail {

//------------------------------------------------------------------------------
// Convert to the same type (string to string)
//------------------------------------------------------------------------------
inline void FromString(const std::string& in_str, std::string& ret_Str)
{
	ret_Str = in_str;
}

inline void FromString(const std::wstring& in_str, std::wstring& ret_Str)
{
	ret_Str = in_str;
}

inline void ToString(const std::string& in_str, std::string& ret_Str)
{
	ret_Str = in_str;
}

inline void ToString(const std::wstring& in_str, std::wstring& ret_Str)
{
	ret_Str = in_str;
}

//------------------------------------------------------------------------------
// Convert std::string to std::wstring and vice versa (without using locale)
//------------------------------------------------------------------------------
inline void ToString(const std::wstring& in_str, std::string& ret_Str)
{
	ret_Str.append(in_str.begin(), in_str.end());
}
inline void ToString(const std::string& in_str, std::wstring& ret_Str)
{
	ret_Str.append(in_str.begin(), in_str.end());
}
inline void FromString(const std::wstring& in_str, std::string& ret_Str)
{
	ret_Str.append(in_str.begin(), in_str.end());
}
inline void FromString(const std::string& in_str, std::wstring& ret_Str)
{
	ret_Str.append(in_str.begin(), in_str.end());
}

//------------------------------------------------------------------------------
// Convert enumeration types
// Need to register your enumeration types, see detail in class ConvertEnum.
//------------------------------------------------------------------------------
template <typename T, typename TSym, typename TAllocator, std::enable_if_t<std::is_enum_v<T>, int> = 0>
inline bool ToString(T val, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& ret_Str)
{
	return ConvertEnum::ToString<T>(val, ret_Str);
}
template <typename T, typename TSym, typename TAllocator, std::enable_if_t<std::is_enum_v<T>, int> = 0>
inline bool FromString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& str, T& ret_Val)
{
	return ConvertEnum::FromString<T>(str, ret_Val);
}

//------------------------------------------------------------------------------
// Convert fundamental types
//------------------------------------------------------------------------------
template <typename T, typename TSym, typename TAllocator, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
inline void ToString(T val, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& ret_Str)
{
	Fundamental::ToString(val, ret_Str);
}
template <typename T, typename TSym, typename TAllocator, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
inline void FromString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& str, T& ret_Val)
{
	Fundamental::FromString(str, ret_Val);
}

//-----------------------------------------------------------------------------
// Convert classes and unions (convert methods should be implemented in concrete classes)
//-----------------------------------------------------------------------------
template <class T, typename TAllocator, std::enable_if_t<(std::is_class_v<T> || std::is_union_v<T>), int> = 0>
inline void ToString(const T& classRef, std::basic_string<char, std::char_traits<char>, TAllocator>& ret_Str)
{
	if constexpr (!has_to_string_v<T, std::basic_string<char, std::char_traits<char>, TAllocator>>)
		static_assert(false, "Class should has public constant methods ToString() or static in namespace BitSerializer::Convert::Detail.");
	else
		ret_Str = classRef.ToString();
}
template <class T, typename TAllocator, std::enable_if_t<(std::is_class_v<T> || std::is_union_v<T>), int> = 0>
inline void ToString(const T& classRef, std::basic_string<wchar_t, std::char_traits<wchar_t>, TAllocator>& ret_Str)
{
	if constexpr (!has_to_string_v<T, std::basic_string<wchar_t, std::char_traits<wchar_t>, TAllocator>>)
		static_assert(false, "Class should has public constant methods ToWString() or static in namespace BitSerializer::Convert::Detail.");
	else
		ret_Str = classRef.ToWString();
}

template <class T, typename TSym, typename TAllocator, std::enable_if_t<(std::is_class_v<T> || std::is_union_v<T>), int> = 0>
inline void FromString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& str, T& ret_Val)
{
	if constexpr (!has_from_string_v<T, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>)
		static_assert(false, "Class should has public method FromString() or static in namespace BitSerializer::Convert::Detail.");
	else
		ret_Val.FromString(str);
}

}	// BitSerializer::Convert::Detail
