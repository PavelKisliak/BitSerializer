/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

/// <summary>
/// Registers a map of strings equivalents for enum type.
/// </summary>
/// <example><code>
/// REGISTER_ENUM_MAP(YOUR_ENUM_TYPE)
///	{
///		{ YOUR_ENUM_TYPE::Apple, "Apple" },
///		{ YOUR_ENUM_TYPE::Orange, "Orange" }
/// }
/// END_ENUM_MAP()
/// </code></example>
#define REGISTER_ENUM_MAP(enumType) namespace { \
	static const bool registration_##enumType = ::BitSerializer::Convert::Detail::ConvertEnum::Register<enumType>(
#define END_ENUM_MAP() ); }

namespace BitSerializer::Convert::Detail {

template <typename TEnum>
class EnumDescriptor
{
public:
	EnumDescriptor() = default;
	EnumDescriptor(TEnum value, const char* name)
		: mValue(value)
		, mStrName(name)
	{ }

	template <typename TSym>
	[[nodiscard]] bool Equals(const std::basic_string_view<TSym>& str_value) const
	{
		return std::equal(str_value.cbegin(), str_value.cend(), mStrName.cbegin(), mStrName.cend(), [](const TSym lhs, const char rhs) {
			return std::tolower(static_cast<int>(lhs)) == std::tolower(static_cast<int>(rhs));
		});
	}

	[[nodiscard]] bool Equals(TEnum enum_value) const noexcept {
		return enum_value == mValue;
	}

	template <typename TSym, typename TAllocator>
	void GetName(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& ret_Str) const
	{
		ret_Str.clear();
		ret_Str.append(mStrName.cbegin(), mStrName.cend());
	}

	[[nodiscard]] TEnum GetEnum() const noexcept {
		return mValue;
	}

private:
	TEnum mValue;
	std::string_view mStrName;
};

class ConvertEnum
{
public:
	template <typename TEnum>
	using enum_descriptors = std::vector<EnumDescriptor<TEnum>>;

	template <typename TEnum>
	static const enum_descriptors<TEnum>& GetDescriptors() noexcept
	{
		const auto& descriptors = GetDescriptorsImpl<TEnum>();
		// Make sure, that type is registered
		assert(!descriptors.empty());
		return const_cast<enum_descriptors<TEnum>&>(descriptors);
	}

	template <typename TEnum>
	static bool Register(const std::initializer_list<std::pair<TEnum, const char*>>& descriptors)
	{
		auto& staticDescriptors = GetDescriptorsImpl<TEnum>();
		// Check is that type was already registered
		if (!staticDescriptors.empty())
			return true;

		staticDescriptors.reserve(descriptors.size());
		for (const auto& descriptor : descriptors) {
			staticDescriptors.emplace_back(descriptor.first, descriptor.second);
		}
		return true;
	}

	template <typename TEnum, typename TSym, typename TAllocator>
	static void ToString(TEnum val, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& ret_Str)
	{
		const auto& descriptors = GetDescriptors<TEnum>();
		auto it = std::find_if(descriptors.cbegin(), descriptors.cend(), [val](const EnumDescriptor<TEnum>& descr) {
			return descr.Equals(val);
		});
		if (it == descriptors.end())
			throw std::out_of_range("Invalid argument");

		it->GetName(ret_Str);
	}

	template <typename TEnum, typename TSym>
	static void FromString(const std::basic_string_view<TSym>& str, TEnum& ret_Val)
	{
		const auto& descriptors = GetDescriptors<TEnum>();
		auto it = std::find_if(descriptors.cbegin(), descriptors.cend(), [&str](const EnumDescriptor<TEnum>& descr) {
			return descr.Equals(str);
		});
		if (it == descriptors.end())
			throw std::out_of_range("Invalid argument");

		ret_Val = it->GetEnum();
	}

private:
	template <typename TEnum>
	static enum_descriptors<TEnum>& GetDescriptorsImpl() noexcept
	{
		static enum_descriptors<TEnum> descriptors;
		return descriptors;
	}
};

}
