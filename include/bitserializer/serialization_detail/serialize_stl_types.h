/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>
#include <tuple>
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

		template <class TMediaArchive>
		inline void Serialize(TMediaArchive& archive)
		{
			using noConstKeyType = std::remove_const_t<value_type::first_type>;
			::BitSerializer::Serialize(archive, U("key"), const_cast<noConstKeyType&>(value.first));
			::BitSerializer::Serialize(archive, U("value"), value.second);
		}

		value_type& value;
	};
}	// namespace Detail

template<typename TMediaArchive, typename TFirst, typename TSecond>
void Serialize(TMediaArchive& archive, const typename TMediaArchive::key_type& key, std::pair<TFirst, TSecond>& pair)
{
	Serialize(archive, key, Detail::PairSerializer<TFirst, TSecond>(pair));
}

template<typename TMediaArchive, typename TFirst, typename TSecond>
void Serialize(TMediaArchive& archive, std::pair<TFirst, TSecond>& pair)
{
	Serialize(archive, Detail::PairSerializer<TFirst, TSecond>(pair));
}

}	// namespace BitSerializer
