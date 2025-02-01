/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <utility>

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
			cont.clear();
			auto hint = cont.begin();
			while (!scope.IsEnd())
			{
				TValue value;
				Serialize(scope, value);
				hint = cont.insert(hint, std::move(value));
			}
		}
		else
		{
			for (auto& elem : cont)
			{
				Serialize(scope, elem);
			}
		}
	}
}
