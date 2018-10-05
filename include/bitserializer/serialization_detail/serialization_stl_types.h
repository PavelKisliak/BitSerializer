/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>
#include <tuple>
#include "key_value.h"
#include "media_archive_base.h"

namespace BitSerializer {

//-----------------------------------------------------------------------------
// Serialize std::pair
//-----------------------------------------------------------------------------
namespace Detail
{
	template<typename TFirst, typename TSecond>
	class PairSerializer
	{
	public:
		using value_type = std::pair<TFirst, TSecond>;

		PairSerializer(value_type& pair)
			: value(pair)
		{ }

		template <class TArchive>
		inline void Serialize(TArchive& archive)
		{
			using noConstKeyType = std::remove_const_t<value_type::first_type>;
			archive << MakeKeyValue("key", const_cast<noConstKeyType&>(value.first));
			archive << MakeKeyValue("value", value.second);
		}

		value_type& value;
	};
}	// namespace Detail

template<typename TArchive, typename TFirst, typename TSecond>
inline bool Serialize(TArchive& archive, const typename TArchive::key_type& key, std::pair<TFirst, TSecond>& pair)
{
	auto pairSerializer = Detail::PairSerializer<TFirst, TSecond>(pair);
	return Serialize(archive, key, pairSerializer);
}

template<typename TArchive, typename TFirst, typename TSecond>
inline void Serialize(TArchive& archive, std::pair<TFirst, TSecond>& pair)
{
	auto pairSerializer = Detail::PairSerializer<TFirst, TSecond>(pair);
	Serialize(archive, pairSerializer);
}

}	// namespace BitSerializer
