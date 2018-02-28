/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>

namespace BitSerializer {

/// <summary>
/// Implementation of helper class to simplify the serialization of base objects.
/// </summary>
template<class TBase, class TDerived>
struct BaseObjectImpl
{
	explicit BaseObjectImpl(TDerived& object) noexcept
		: Object(object)
	{ }

	TBase& Object;
};

/// <summary>
/// The helper function for making a base class wrapper (to simplify the serialization of base objects).
/// </summary>
/// <param name="object">The object.</param>
/// <returns></returns>
template<class TBase, class TDerived>
constexpr BaseObjectImpl<TBase, TDerived> BaseObject(TDerived& object) noexcept
{
	static_assert(std::is_base_of_v<TBase, TDerived>, "BitSerializer. The template parameter 'TBase' should be a base type of passed object.");
	return BaseObjectImpl<TBase, TDerived>(object);
}

}	// namespace BitSerializer
