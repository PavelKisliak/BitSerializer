/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include <type_traits>
#include <utility>
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/convert.h"

namespace BitSerializer::Detail
{
	/**
	 * @brief Determines if a type is convertible to any element within a tuple.
	 *
	 * Useful when validating against multiple allowed types.
	 *
	 * @tparam T      The source type to test for convertibility.
	 * @tparam TTuple A tuple of target types to compare against.
	 */
	template <typename T, typename TTuple>
	struct is_convertible_to_any_of_tuple
	{
	private:
		template <class TElem>
		static constexpr bool testImpl()
		{
			if constexpr (std::is_same_v<std::decay_t<T>, bool> || std::is_null_pointer_v<T> || std::is_floating_point_v<T>) {
				return std::is_same_v<T, TElem>;
			}
			else {
				return std::is_convertible_v<T, TElem>;
			}
		}

		template <class TestType, size_t... Is >
		static constexpr bool test(std::index_sequence<Is...>) {
			return (testImpl<std::tuple_element_t<Is, TestType>>() || ...);
		}

	public:
		constexpr static bool value = test<TTuple>(std::make_index_sequence<std::tuple_size_v<TTuple>>{});
	};

	template <typename T, typename TTuple>
	constexpr bool is_convertible_to_any_of_tuple_v = is_convertible_to_any_of_tuple<T, TTuple>::value;

	/**
	 * @brief Adapts a key to the representation supported by the archive, converting it only if needed.
	 *
	 * If the archive natively supports `TKey` (it is listed in `supported_key_types`), the key is returned
	 * by reference without any conversion. Otherwise it is converted to `TArchive::key_type` and returned
	 * by value (an owning result).
	 *
	 * Avoids the allocation/copy that an unconditional conversion to `TArchive::key_type` would incur
	 * for archives that can consume the key type directly (e.g. `std::string_view` for JSON/CSV/MsgPack).
	 *
	 * @note The returned reference (in the non-converting case) is valid only as long as `key`; pass
	 * an lvalue whose lifetime covers the whole serialization statement.
	 */
	template <typename TArchive, typename TKey>
	decltype(auto) ToArchiveKey(TKey&& key)
	{
		if constexpr (is_convertible_to_any_of_tuple_v<std::decay_t<TKey>, typename TArchive::supported_key_types>)
		{
			return std::forward<TKey>(key);
		}
		else
		{
			return Convert::To<typename TArchive::key_type>(std::forward<TKey>(key));
		}
	}
}
