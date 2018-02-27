/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>

namespace BitSerializer {

template<class TBase, class TDerived>
struct BaseObjectImpl
{
	explicit BaseObjectImpl(TDerived& object) noexcept
		: Object(object)
	{ }

	TBase& Object;
};

template<class TBase, class TDerived>
constexpr BaseObjectImpl<TBase, TDerived> BaseObject(TDerived& object) noexcept
{
	static_assert(std::is_base_of_v<TBase, TDerived>, "BitSerializer. The template parameter 'TBase' should be a base type of passed object.");
	return BaseObjectImpl<TBase, TDerived>(object);
}

}	// namespace BitSerializer
