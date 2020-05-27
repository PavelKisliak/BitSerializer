/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include <type_traits>

namespace BitSerializer::Detail
{
	/// <summary>
	/// Generic function for serialization sets.
	/// </summary>
	template<typename TArchive, typename TSet>
	static void SerializeSetImpl(TArchive& scope, TSet& cont)
	{
		using TValue = typename TSet::value_type;

		if constexpr (TArchive::IsLoading())
		{
			auto contSize = scope.GetSize();
			cont.clear();
			auto hint = cont.begin();
			for (size_t c = 0; c < contSize; c++)
			{
				TValue value;
				Serialize(scope, value);
				hint = cont.insert(hint, std::move(value));
			}
		}
		else
		{
			for (const TValue& elem : cont) {
				Serialize(scope, const_cast<TValue&>(elem));
			}
		}
	}
}
