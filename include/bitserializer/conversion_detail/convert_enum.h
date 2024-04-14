/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <stdexcept>


/// <summary>
/// Registers a map of strings equivalents for enum type.
/// </summary>
/// <example><code>
/// REGISTER_ENUM(YOUR_ENUM_TYPE, {
///		{ YOUR_ENUM_TYPE::Apple, "Apple" },
///		{ YOUR_ENUM_TYPE::Orange, "Orange" }
/// })
/// </code></example>
#define REGISTER_ENUM(enumType, ...) namespace { \
	static const bool registration_##enumType = ::BitSerializer::Convert::Detail::EnumRegistry<enumType>::Register(__VA_ARGS__); \
}

/// <summary>
/// Declares I/O streams operators for enum (register the enum via REGISTER_ENUM macro).
/// </summary>
#define DECLARE_ENUM_STREAM_OPS(enum2Type) \
template <typename TSym> \
std::basic_ostream<TSym, std::char_traits<TSym>>& operator<<(std::basic_ostream<TSym, std::char_traits<TSym>>& stream, enum2Type value) \
{ \
	std::basic_string<TSym, std::char_traits<TSym>> str; \
	BitSerializer::Convert::Detail::EnumRegistry<enum2Type>::ToString(value, str); \
	return stream << str; \
} \
template <class TSym, class TTraits = std::char_traits<TSym>> \
std::basic_istream<TSym, TTraits>& operator>>(std::basic_istream<TSym, TTraits>& stream, enum2Type& value) \
{ \
	TSym sym; std::basic_string<TSym, TTraits> str; \
	for (stream >> sym; !stream.eof() && !std::isspace(sym); sym = stream.get()) { \
		str.push_back(sym); } \
	BitSerializer::Convert::Detail::EnumRegistry<enum2Type>::FromString(std::basic_string_view<TSym>(str), value); \
	return stream; \
} \


namespace BitSerializer::Convert::Detail
{
	template <typename TEnum>
	class EnumMetadata
	{
	public:
		TEnum Value{};
		std::string_view Name;

		EnumMetadata() = default;
		EnumMetadata(TEnum value, const char* name) noexcept
			: Value(value), Name(name)
		{ }
	};


	template <typename TEnum>
	class EnumRegistry
	{
	public:
		template <size_t Size>
		static bool Register(const EnumMetadata<TEnum>(&descriptors)[Size])
		{
			// Check is that type was already registered
			if (mBeginIt != nullptr) {
				return false;
			}

			static EnumMetadata<TEnum> descriptors_[Size];
			std::memcpy(descriptors_, descriptors, sizeof(EnumMetadata<TEnum>) * Size);
			mBeginIt = descriptors_;
			mEndIt = descriptors_ + Size;
			return true;
		}

		static const EnumMetadata<TEnum>& GetEnumMetadata(TEnum val)
		{
			const auto it = std::find_if(mBeginIt, mEndIt, [val](const EnumMetadata<TEnum>& metadata) {
				return metadata.Value == val;
			});
			if (it == cend()) {
				throw std::invalid_argument("Enum with passed value is not registered");
			}
			return *it;
		}

		template <typename TSym>
		static const EnumMetadata<TEnum>& GetEnumMetadata(std::basic_string_view<TSym> name)
		{
			const auto it = std::find_if(mBeginIt, mEndIt, [name](const auto& metadata) {
				return std::equal(name.cbegin(), name.cend(), metadata.Name.cbegin(), metadata.Name.cend(), [](const TSym lhs, const char rhs) {
					return std::tolower(static_cast<int>(lhs)) == std::tolower(rhs);
				});
			});
			if (it == cend()) {
				throw std::invalid_argument("Enum with passed name is not registered");
			}
			return *it;
		}

		template <typename TSym, typename TAllocator>
		static void ToString(TEnum val, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& ret_Str)
		{
			const auto& metadata = GetEnumMetadata(val);
			ret_Str.append(metadata.Name.cbegin(), metadata.Name.cend());
		}

		template <typename TSym>
		static void FromString(std::basic_string_view<TSym> str, TEnum& ret_Val)
		{
			ret_Val = GetEnumMetadata(str).Value;
		}

		[[nodiscard]] static size_t size() noexcept {
			return cend() - cbegin();
		}

		[[nodiscard]] static const EnumMetadata<TEnum>* cbegin() noexcept {
			return mBeginIt;
		}

		[[nodiscard]] static const EnumMetadata<TEnum>* cend() noexcept {
			return mEndIt;
		}

	private:
		static inline EnumMetadata<TEnum>* mBeginIt = nullptr;
		static inline EnumMetadata<TEnum>* mEndIt = nullptr;
	};
}
