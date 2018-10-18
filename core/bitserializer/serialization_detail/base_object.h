/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>

namespace BitSerializer {

/// <summary>
/// The wrapper class that keeps a reference to a base user object, used to simplify the serialization of base objects.
/// </summary>
template<class TBase>
struct BaseObject
{
	template <typename TDerived>
	explicit BaseObject(TDerived& object) noexcept
		: Object(object)
	{
		static_assert(std::is_base_of_v<TBase, TDerived>,
			"BitSerializer. The template parameter 'TBase' should be a base type of passed object.");
	}

	TBase& Object;
};

}	// namespace BitSerializer
