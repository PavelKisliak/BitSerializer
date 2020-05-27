/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <utility>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
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
			void Serialize(TArchive& archive)
			{
				static const auto keyName = Convert::To<typename TArchive::key_type>(L"key");
				static const auto valueName = Convert::To<typename TArchive::key_type>(L"value");

				using noConstKeyType = std::remove_const_t<typename value_type::first_type>;
				::BitSerializer::Serialize(archive, keyName, const_cast<noConstKeyType&>(value.first));
				::BitSerializer::Serialize(archive, valueName, value.second);
			}

			value_type& value;
		};
	}

	/// <summary>
	/// Serialize std::pair with key.
	/// </summary>
	template<typename TArchive, typename TKey, typename TFirst, typename TSecond>
	static bool Serialize(TArchive& archive, TKey&& key, std::pair<TFirst, TSecond>& pair)
	{
		auto pairSerializer = Detail::PairSerializer<TFirst, TSecond>(pair);
		return Serialize(archive, std::forward<TKey>(key), pairSerializer);
	}

	/// <summary>
	/// Serialize std::pair.
	/// </summary>
	template<typename TArchive, typename TFirst, typename TSecond>
	static void Serialize(TArchive& archive, std::pair<TFirst, TSecond>& pair)
	{
		auto pairSerializer = Detail::PairSerializer<TFirst, TSecond>(pair);
		Serialize(archive, pairSerializer);
	}
}
