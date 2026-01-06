/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include "bitserializer/serialization_options.h"
#include "bitserializer/serialization_detail/errors_handling.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::tuple` as an array of values in the target archive.
	 *
	 * @note Cannot be used with CSV archives which do not support arrays.
	 * @note As an exceptional case for `std` type, the required overload of the `size()` function is defined in "object_traits.h".
	 */
	template<typename TArchive, typename ...TArgs>
	void SerializeArray(TArchive& arrayScope, std::tuple<TArgs...>& value)
	{
		if constexpr (TArchive::IsLoading())
		{
			try
			{
				std::apply([&arrayScope](auto&&... args) {
					((Serialize(arrayScope, args)), ...);
				}, value);
			}
			// Handle case when size of loading array LESS than tuple
			catch (const SerializationException& ex)
			{
				if (ex.GetErrorCode() == SerializationErrorCode::OutOfRange)
				{
					if (arrayScope.GetOptions().mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
					{
						throw SerializationException(SerializationErrorCode::MismatchedTypes,
							"The size of array being loaded is less than target tuple");
					}
				}
				else {
					throw;
				}
			}
			// Handle case when size of loading array LARGER than tuple
			if (!arrayScope.IsEnd() && arrayScope.GetOptions().mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					"Number of elements in the target tuple is not sufficient to load the array");
			}
		}
		else
		{
			std::apply([&arrayScope](auto&&... args) {
				((Serialize(arrayScope, args)), ...);
			}, value);
		}
	}
}
