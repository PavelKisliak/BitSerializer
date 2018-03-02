/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

namespace BitSerializer::Convert::Detail {

///------------------------------------------------------------------------------
/// Implementation detail for convert enumeration types
///
/// For able to convert your enum-types to strings and vice versa you need to register it:
/// static const bool _YOUR_ENUM = ConvertEnum::Register<YOUR_ENUM>(
///	{
///		{ YOUR_ENUM::Apple, "Apple" },
///		{ YOUR_ENUM::Orange, "Orange" },
///		...
///	};
///------------------------------------------------------------------------------
class ConvertEnum
{
public:
	template <typename TEnum>
	static bool Register(const std::initializer_list<std::pair<TEnum, const char*>> descriptors)
	{
		auto& staticDescriptors = EnumValueDescriptor<TEnum>::GetStaticDescriptors();
		// Check is that type was already registered
		if (!staticDescriptors.empty())
			return true;

		staticDescriptors.reserve(descriptors.size());
		for (const auto& descr : descriptors) {
			staticDescriptors.emplace_back(descr.first, descr.second);
		}
		return true;
	}

	template <typename TEnum, typename TSym, typename TAllocator>
	static bool ToString(TEnum val, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& ret_Str)
	{
		auto descriptors = EnumValueDescriptor<TEnum>::GetStaticDescriptors();
		// Make sure, that type is registered
		assert(!descriptors.empty());

		auto it = std::find_if(descriptors.cbegin(), descriptors.cend(), [val](const EnumValueDescriptor<TEnum>& descr) {
			return descr.Equals(val);
		});
		if (it == descriptors.end())
			return false;

		it->GetName(ret_Str);
		return true;
	}

	template <typename TEnum, typename TSym, typename TAllocator>
	static bool FromString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& str, TEnum& ret_Val)
	{
		auto descriptors = EnumValueDescriptor<TEnum>::GetStaticDescriptors();
		// Make sure, that type is registered
		assert(!descriptors.empty());

		auto it = std::find_if(descriptors.cbegin(), descriptors.cend(), [&str](const EnumValueDescriptor<TEnum>& descr) {
			return descr.Equals(str);
		});
		if (it == descriptors.end())
			return false;

		ret_Val = it->GetEnum();
		return true;
	}

private:
	template <typename TEnum>
	class EnumValueDescriptor
	{
	public:
		EnumValueDescriptor() = default;
		EnumValueDescriptor(TEnum value, const char* name)
			: mValue(value)
			, mStrName(name)
		{ }

		static std::vector<EnumValueDescriptor<TEnum>>& GetStaticDescriptors() noexcept
		{
			static std::vector<EnumValueDescriptor<TEnum>> descriptors;
			return descriptors;
		}

		template <typename TSym, typename TAllocator>
		inline bool Equals(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& str_value) const {
			return std::equal(str_value.cbegin(), str_value.cend(), mStrName.cbegin(), mStrName.cend(), [](const TSym lhs, const char rhs) {
				return std::tolower(static_cast<char>(lhs)) == std::tolower(static_cast<char>(rhs));
			});
		}

		inline bool Equals(TEnum enum_value) const noexcept {
			return enum_value == mValue;
		}

		template <typename TSym, typename TAllocator>
		inline void GetName(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& ret_Str) const {
			ret_Str.clear();
			ret_Str.append(mStrName.begin(), mStrName.end());
		}

		inline TEnum GetEnum() const noexcept {
			return mValue;
		}

	private:
		TEnum mValue;
		std::string mStrName;
	};
};

}	// namespace BitSerializer::Convert::Detail
