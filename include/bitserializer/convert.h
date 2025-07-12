/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include "bitserializer/config.h"
#include "bitserializer/conversion_detail/convert_fundamental.h"
#include "bitserializer/conversion_detail/convert_detail.h"
#include "bitserializer/conversion_detail/convert_enum.h"
#if BITSERIALIZER_HAS_FILESYSTEM
#include "bitserializer/conversion_detail/convert_filesystem.h"
#endif
#include "bitserializer/conversion_detail/convert_chrono.h"
#include "bitserializer/conversion_detail/convert_traits.h"

// Suppress C4702 - unreachable code
#pragma warning(push)
#pragma warning(disable: 4702)

namespace BitSerializer::Convert
{
	/**
	 * @brief Determines if conversion from `TIn` to `TOut` is supported.
	 *
	 * @tparam TIn Source type.
	 * @tparam TOut Target type.
	 * @return True if conversion is supported; false otherwise.
	 */
	template <typename TIn, typename TOut>
	constexpr bool IsConvertible()
	{
		return Detail::is_convert_supported_v<TIn, TOut>;
	}

	/**
	 * @brief Generic function for converting a value to any supported target type.
	 *
	 * @tparam TOut Target type.
	 * @tparam TIn Source type.
	 * @tparam TInitArgs Optional variadic arguments for constructing TOut.
	 * @param[in] value Source value to convert.
	 * @param[in] initArgs Optional arguments for constructing the output type (e.g., allocator or existing string).
	 * @return Converted value of type TOut.
	 * @throw std::out_of_range If conversion overflows target type.
	 * @throw std::invalid_argument If input format is invalid.
	 * @note This function handles both direct and indirect conversions (e.g., string_view to string, numeric types, etc.).
	 */
	template <typename TOut, typename TIn, typename... TInitArgs, std::enable_if_t<std::is_constructible_v<TOut, TInitArgs...>, int> = 0>
	TOut To(TIn&& value, TInitArgs... initArgs)
	{
		constexpr bool isSame = std::is_same_v<TOut, std::decay_t<TIn>>;
		constexpr bool isConvertible = IsConvertible<TIn, TOut>();

		// You may need to implement internal or external conversion functions for your types
		static_assert(isSame || isConvertible, "BitSerializer::Convert. Converting these types is not supported.");

		// Convert to the same type
		if constexpr (isSame && sizeof...(TInitArgs) == 0) {
			return std::forward<TIn>(value);
		}
		else if constexpr (isConvertible)
		{
			TOut result(std::forward<TInitArgs>(initArgs)...);

			using namespace Detail;
			if constexpr (is_convertible_to_string_view_v<TIn>) {
				// String types like std::basic_string and c-strings must be converted to string_view
				To(ToStringView(value), result);
			}
			else {
				To(std::forward<TIn>(value), result);
			}
			return result;
		}
		else
		{
			// Will never be reached due to static_assert (defined for analyzers)
			return {};
		}
	}

	/**
	 * @brief Converts a value to `std::string` (syntax sugar for `To<std::string>()`).
	 *
	 * @tparam TIn Source type.
	 * @param[in] value Input value to convert.
	 * @param[in] initArgs Arguments used to construct the output type (e.g., allocator or existing string).
	 * @return UTF-8 encoded string.
	 * @throw std::out_of_range If conversion overflows target type.
	 * @throw std::invalid_argument If input format is invalid.
	 */
	template <typename TIn, typename... TInitArgs, std::enable_if_t<std::is_constructible_v<std::string, TInitArgs...>, int> = 0>
	std::string ToString(TIn&& value, TInitArgs... initArgs)
	{
		return To<std::string>(std::forward<TIn>(value), std::forward<TInitArgs>(initArgs)...);
	}

	/**
	 * @brief Converts a value to `std::wstring` (syntax sugar for To<std::wstring>()).
	 *
	 * @tparam TIn Source type.
	 * @param[in] value Input value to convert.
	 * @param[in] initArgs Optional arguments for constructing the output type (e.g., allocator or existing string).
	 * @return Wide string.
	 * @throw std::out_of_range If conversion overflows target type.
	 * @throw std::invalid_argument If input format is invalid.
	 */
	template <typename TIn, typename... TInitArgs, std::enable_if_t<std::is_constructible_v<std::wstring, TInitArgs...>, int> = 0>
	std::wstring ToWString(TIn&& value, TInitArgs... initArgs)
	{
		return To<std::wstring>(std::forward<TIn>(value), std::forward<TInitArgs>(initArgs)...);
	}

	/**
	 * @brief Generic function for converting a value to any supported target type without throwing exceptions.
	 *
	 * @tparam TOut Target type.
	 * @tparam TIn Source type.
	 * @tparam TInitArgs Variadic arguments for constructing TOut.
	 * @param[in] value Source value to convert.
	 * @param[in] initArgs Optional arguments for constructing the output type (e.g., allocator or existing string).
	 * @return `optional<TOut>` containing the result, or empty if an error occurs.
	 */
	template <typename TOut, typename TIn, typename... TInitArgs, std::enable_if_t<std::is_constructible_v<TOut, TInitArgs...>, int> = 0>
	std::optional<TOut> TryTo(TIn&& value, TInitArgs... initArgs) noexcept
	{
		try
		{
			return To<TOut>(std::forward<TIn>(value), std::forward<TInitArgs>(initArgs)...);
		}
		catch (...)
		{
			return std::nullopt;
		}
	}

} // namespace BitSerializer::Convert

#pragma warning(pop)
