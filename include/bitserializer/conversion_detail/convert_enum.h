/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <cctype>
#include <cstring>
#include <stdexcept>


/**
 * @brief Registers a mapping between an enumeration type and its string representations.
 *
 * @param enumType The enumeration type to register.
 * @param map A list of {enum_value, "string"} pairs enclosed in braces.
 *
 * @par Example:
 * @code
 * enum class Fruit { Apple, Orange };
 * BITSERIALIZER_REGISTER_ENUM(Fruit, {
 *     { Fruit::Apple, "Apple" },
 *     { Fruit::Orange, "Orange" }
 * })
 * @endcode
 */
#define BITSERIALIZER_REGISTER_ENUM(enumType, ...) \
    namespace { \
        static const bool registration_##enumType = ::BitSerializer::Convert::Detail::EnumRegistry<enumType>::Register(__VA_ARGS__); \
    }

// DEPRECATED: Use BITSERIALIZER_REGISTER_ENUM instead.
#define REGISTER_ENUM(enumType, ...) \
    namespace { \
        static const bool registration_##enumType = ::BitSerializer::Convert::Detail::EnumRegistry<enumType>::RegisterDeprecated(__VA_ARGS__); \
    }

/**
 * @brief Declares stream operators (`<<` and `>>`) for an enum type.
 *
 * This macro allows use of standard I/O streams with enums after registering them using `REGISTER_ENUM`.
 *
 * @param enumType The enumeration type for which stream operators will be declared.
 *
 * @note Must be placed in the same namespace as the enum.
 *
 * @par Example:
 * @code
 * DECLARE_ENUM_STREAM_OPS(Fruit)
 *
 * std::stringstream ss;
 * Fruit fruit = Fruit::Apple;
 * ss << fruit; // Outputs: "Apple"
 * ss >> fruit; // Parses: "Orange" -> Fruit::Orange
 * @endcode
 */
#define BITSERIALIZER_DECLARE_ENUM_STREAM_OPS(enumType) \
template <typename TSym, class TTraits = std::char_traits<TSym>> \
std::basic_ostream<TSym, TTraits>& operator<<(std::basic_ostream<TSym, TTraits>& stream, enumType value) \
{ \
	std::basic_string<TSym, TTraits> str; \
	BitSerializer::Convert::Detail::To(value, str); \
	return stream << str; \
} \
template <class TSym, class TTraits = std::char_traits<TSym>> \
std::basic_istream<TSym, TTraits>& operator>>(std::basic_istream<TSym, TTraits>& stream, enumType& value) \
{ \
	TSym sym; std::basic_string<TSym, TTraits> str; \
	for (stream >> sym; !stream.eof() && !std::isspace(sym); sym = static_cast<TSym>(stream.get())) { \
		str.push_back(sym); } \
	BitSerializer::Convert::Detail::To(std::basic_string_view<TSym>(str), value); \
	return stream; \
} \

// DEPRECATED: Use BITSERIALIZER_DECLARE_ENUM_STREAM_OPS instead.
#define DECLARE_ENUM_STREAM_OPS(enumType) \
template <typename TSym, class TTraits = std::char_traits<TSym>> \
std::basic_ostream<TSym, TTraits>& operator<<(std::basic_ostream<TSym, TTraits>& stream, enumType value) \
{ \
	::BitSerializer::Convert::Detail::EnumRegistry<enumType>::DeclareStreamOpsDeprecated(); \
	std::basic_string<TSym, TTraits> str; \
	BitSerializer::Convert::Detail::To(value, str); \
	return stream << str; \
} \
template <class TSym, class TTraits = std::char_traits<TSym>> \
std::basic_istream<TSym, TTraits>& operator>>(std::basic_istream<TSym, TTraits>& stream, enumType& value) \
{ \
	TSym sym; std::basic_string<TSym, TTraits> str; \
	for (stream >> sym; !stream.eof() && !std::isspace(sym); sym = static_cast<TSym>(stream.get())) { \
		str.push_back(sym); } \
	BitSerializer::Convert::Detail::To(std::basic_string_view<TSym>(str), value); \
	return stream; \
} \


namespace BitSerializer::Convert::Detail
{
	/**
	 * @brief Holds metadata for a single enum value: its name and associated value.
	 */
	template <typename TEnum>
	class EnumMetadata
	{
	public:
		TEnum Value{};
		std::string_view Name;

		EnumMetadata() = default;
		EnumMetadata(TEnum value, const char* name) noexcept
			: Value(value), Name(name)
		{
		}
	};

	/**
	 * @brief Registry for mapping enum values to their string representations.
	 *
	 * Ensures only one mapping per enum type exists and provides lookup functionality.
	 */
	template <typename TEnum>
	class EnumRegistry
	{
	public:
		/**
		 * @brief Registers a static array of enum descriptors.
		 *
		 * @param[in] descriptors Array of enum metadata.
		 * @return True if registration succeeded, false if already registered.
		 */
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

		template <size_t Size>
		[[deprecated("REGISTER_ENUM macro is deprecated, use BITSERIALIZER_REGISTER_ENUM")]]
		static bool RegisterDeprecated(const EnumMetadata<TEnum>(&descriptors)[Size]) {
			return Register<Size>(descriptors);
		}

		[[deprecated("DECLARE_ENUM_STREAM_OPS macro is deprecated, use BITSERIALIZER_DECLARE_ENUM_STREAM_OPS")]]
		static constexpr void DeclareStreamOpsDeprecated() { }

		/**
		 * @brief Checks whether the enum has been registered.
		 */
		static bool IsRegistered() noexcept
		{
			return mBeginIt != mEndIt;
		}

		/**
		 * @brief Gets the metadata for a given enum value.
		 *
		 * @param val The enum value to look up.
		 * @return Pointer to metadata or null if not found.
		 */
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

		/**
		 * @brief Gets the metadata for a given enum name (case-insensitive match).
		 *
		 * @param name String representation of the enum.
		 * @return Pointer to metadata or null if not found.
		 */
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

		/**
		 * @brief Returns the number of registered enum values.
		 */
		[[nodiscard]] static size_t size() noexcept {
			return mEndIt - mBeginIt;
		}

		/**
		 * @brief Returns a pointer to the beginning of the registry.
		 */
		[[nodiscard]] static const EnumMetadata<TEnum>* cbegin() noexcept {
			return mBeginIt;
		}

		/**
		 * @brief Returns a pointer to the end of the registry.
		 */
		[[nodiscard]] static const EnumMetadata<TEnum>* cend() noexcept {
			return mEndIt;
		}

	private:
		static inline const EnumMetadata<TEnum>* mBeginIt = nullptr;
		static inline const EnumMetadata<TEnum>* mEndIt = nullptr;
	};

	//------------------------------------------------------------------------------

	/**
	 * @brief Converts a UTF string to an enum value.
	 *
	 * @param[in] in Input string to convert.
	 * @param[out] out Output enum value.
	 * @throws std::invalid_argument If no matching enum value was found.
	 */
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

	/**
	 * @brief Converts an enum value to a UTF string.
	 *
	 * @param[in] val Enum value to convert.
	 * @param[out] ret_Str Output string containing the enum name.
	 * @throws std::invalid_argument If the enum value is not registered.
	 */
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
