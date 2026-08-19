/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <variant>

#include "bitserializer/convert.h"
#include "bitserializer/key_value.h"
#include "bitserializer/serialization_options.h"
#include "bitserializer/serialization_detail/errors_handling.h"

namespace BitSerializer
{
	namespace Detail
	{
		template <size_t TIndex = 0, typename TArchive, typename TKey, typename... TArgs>
		void SerializeVariantAlternative(TArchive& archive, const TKey& key, std::variant<TArgs...>& value, size_t activeIndex)
		{
			if constexpr (TIndex < sizeof...(TArgs))
			{
				if (activeIndex == TIndex)
				{
					if (value.index() != TIndex)
					{
						value.template emplace<TIndex>();
					}
					Serialize(archive, key, std::get<TIndex>(value));
				}
				else
				{
					SerializeVariantAlternative<TIndex + 1>(archive, key, value, activeIndex);
				}
			}
			else if (archive.GetOptions().mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					"Variant index is out of range for target type: " + Convert::ToString(activeIndex));
			}
		}
	}

	/**
	 * @brief A wrapper that explicitly requests tagged serialization of `std::variant`.
	 *
	 * Serializes the variant as an object with `index` and `value` fields,
	 * preserving the type information. This is useful when the default
	 * serialization behavior for `std::variant` may change in the future,
	 * and you want to explicitly opt in to the tagged format.
	 *
	 * @par Example:
	 * @code
	 * std::variant<int, std::string, CUser> data;
	 * archive << KeyValue("data", VariantAsTagged(data));
	 * @endcode
	 */
	template <typename T>
	struct VariantAsTagged
	{
		explicit VariantAsTagged(T& v) noexcept : value(v) {}
		T& value;
	};

	/**
	 * @brief Serializes `VariantAsTagged<std::variant<...>>` as an object with `index` and `value` fields.
	 *
	 * @note This representation requires object support in the target archive.
	 * Flat archives such as CSV may not support nested alternatives.
	 */
	template <typename TArchive, typename... TArgs>
	void SerializeObject(TArchive& archive, VariantAsTagged<std::variant<TArgs...>> taggedVariant)
	{
		SerializeObject(archive, taggedVariant.value);
	}

	/**
	 * @brief Serializes `std::variant` as an object with `index` and `value` fields.
	 *
	 * @note This representation requires object support in the target archive.
	 * Flat archives such as CSV may not support nested alternatives.
	 */
	template <typename TArchive, typename... TArgs>
	void SerializeObject(TArchive& archive, std::variant<TArgs...>& value)
	{
		using index_type = size_t;

		if constexpr (TArchive::IsLoading())
		{
			const auto indexName = Convert::To<typename TArchive::key_type>("index");
			const auto valueName = Convert::To<typename TArchive::key_type>("value");

			index_type activeIndex = 0;
			if (Serialize(archive, indexName, activeIndex))
			{
				Detail::SerializeVariantAlternative(archive, valueName, value, activeIndex);
			}
		}
		else
		{
			if (value.valueless_by_exception())
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					"Cannot serialize std::variant in valueless_by_exception state");
			}

			index_type activeIndex = value.index();
			archive << KeyValue("index", activeIndex);
			std::visit([&archive](auto& activeValue) {
				archive << KeyValue("value", activeValue);
			}, value);
		}
	}
}