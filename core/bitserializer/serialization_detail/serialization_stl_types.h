/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <utility>
#include "media_archive_base.h"
#include "../string_conversion.h"

namespace BitSerializer {

//-----------------------------------------------------------------------------
// Serialize std::pair
//-----------------------------------------------------------------------------
namespace Detail
{
	template<class TFirst, class TSecond>
	class PairSerializer
	{
	public:
		using value_type = std::pair<TFirst, TSecond>;

		explicit PairSerializer(value_type& pair) noexcept
			: value(pair)
		{ }

		template<class TArchive>
		inline void Serialize(TArchive& archive)
		{
			static const auto keyName = Convert::To<typename TArchive::key_type>(L"key");
			static const auto valueName = Convert::To<typename TArchive::key_type>(L"value");

			using noConstKeyType = std::remove_const_t<typename value_type::first_type>;
			::BitSerializer::Serialize(archive, keyName, const_cast<noConstKeyType&>(value.first));
			::BitSerializer::Serialize(archive, valueName, value.second);
		}

		value_type& value;
	};
}	// namespace Detail

template<typename TArchive, typename TKey, typename TFirst, typename TSecond>
inline bool Serialize(TArchive& archive, TKey&& key, std::pair<TFirst, TSecond>& pair)
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
