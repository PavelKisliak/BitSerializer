/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <cctype>
#include <cstring>
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
	BitSerializer::Convert::Detail::To(value, str); \
	return stream << str; \
} \
template <class TSym, class TTraits = std::char_traits<TSym>> \
std::basic_istream<TSym, TTraits>& operator>>(std::basic_istream<TSym, TTraits>& stream, enum2Type& value) \
{ \
	TSym sym; std::basic_string<TSym, TTraits> str; \
	for (stream >> sym; !stream.eof() && !std::isspace(sym); sym = stream.get()) { \
		str.push_back(sym); } \
	BitSerializer::Convert::Detail::To(std::basic_string_view<TSym>(str), value); \
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
			// Check that this type has not yet been registered
			if (mBeginIt != nullptr) {
				return false;
			}

			static EnumMetadata<TEnum> descriptors_[Size];
			std::memcpy(descriptors_, descriptors, sizeof(EnumMetadata<TEnum>) * Size);
			mBeginIt = descriptors_;
			mEndIt = descriptors_ + Size;
			return true;
		}

		static bool IsRegistered() noexcept
		{
			return mBeginIt != mEndIt;
		}

		static const EnumMetadata<TEnum>* GetEnumMetadata(TEnum val) noexcept
		{
			for (auto it = mBeginIt; it != mEndIt; ++it)
			{
				if (it->Value == val)
				{
					return it;
				}
			}
			return nullptr;
		}

		template <typename TSym>
		static const EnumMetadata<TEnum>* GetEnumMetadata(std::basic_string_view<TSym> name)
		{
			const auto nameSize = name.size();
			for (auto it = mBeginIt; it != mEndIt; ++it)
			{
				if (it->Name.size() == nameSize)
				{
					bool isMatched = true;
					for (size_t i = 0; i < nameSize; ++i)
					{
						if (std::tolower(static_cast<int>(it->Name[i])) != std::tolower(name[i]))
						{
							isMatched = false;
							break;
						}
					}
					if (isMatched)
					{
						return it;
					}
				}
			}
			return nullptr;
		}

		[[nodiscard]] static size_t size() noexcept {
			return mEndIt - mBeginIt;
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

	//------------------------------------------------------------------------------

	/// <summary>
	/// Converts any UTF string to enum types.
	/// </summary>
	template <typename T, typename TSym, std::enable_if_t<std::is_enum_v<T>, int> = 0>
	void To(std::basic_string_view<TSym> in, T& out)
	{
		if (auto metadata = EnumRegistry<T>::GetEnumMetadata(in))
		{
			out = metadata->Value;
			return;
		}
		throw std::invalid_argument("Enum with passed name is not registered");
	}

	/// <summary>
	/// Converts enum types to any UTF string
	/// </summary>
	template <typename T, typename TSym, typename TAllocator, std::enable_if_t<std::is_enum_v<T>, int> = 0>
	void To(T val, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& ret_Str)
	{
		if (auto metadata = EnumRegistry<T>::GetEnumMetadata(val))
		{
			ret_Str.append(metadata->Name.cbegin(), metadata->Name.cend());
			return;
		}
		throw std::invalid_argument("Enum with passed value is not registered");
	}
}
