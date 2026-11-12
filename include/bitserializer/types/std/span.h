/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

#if defined(__has_include)
#	if __has_include(<version>)
#		include <version>
#	endif
#endif

// `std::span` is a C++20 feature. This header is opt-in, but the guard keeps it safe to
// include from C++17 translation units (the header becomes a no-op).
#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L
#include <span>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::span`.
	 *
	 * Saving serializes the referenced range as an array. Loading writes into the existing
	 * span elements, so the target span must already reference enough storage and the number
	 * of loaded items must match the span size.
	 *
	 * `std::span<const T>` is save-only.
	 */
	template <typename TArchive, typename TValue, size_t Extent>
	void SerializeArray(TArchive& archive, std::span<TValue, Extent>& cont)
	{
		static_assert(!(TArchive::IsLoading() && std::is_const_v<TValue>),
			"BitSerializer. std::span<const T> is save-only; loading requires a mutable span.");
		Detail::SerializeFixedSizeArray(archive, cont.begin(), cont.end());
	}
}
#endif
