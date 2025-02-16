/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <chrono>
#include <type_traits>
#include "bitserializer/serialization_detail/archive_traits.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::chrono::time_point (wall clock types) as ISO 8601 string in format: YYYY-MM-DDThh:mm:ss[.SSS]Z.
	/// </summary>
	template <typename TArchive, typename TKey, typename TClock, typename TDuration, std::enable_if_t<!TClock::is_steady, int> = 0>
	bool Serialize(TArchive& archive, TKey&& key, std::chrono::time_point<TClock, TDuration>& tpValue)
	{
		if constexpr (can_serialize_value_with_key_v<TArchive, Detail::CBinTimestamp, TKey>)
		{
			// Serialize as binary timestamp
			Detail::CBinTimestamp timestamp;
			if constexpr (TArchive::IsLoading())
			{
				return archive.SerializeValue(std::forward<TKey>(key), timestamp)
					&& Detail::ConvertByPolicy(timestamp, tpValue, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
			}
			else
			{
				return Detail::ConvertByPolicy(tpValue, timestamp, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy)
					&& archive.SerializeValue(std::forward<TKey>(key), timestamp);
			}
		}
		else
		{
			// Serialize as ISO 8601
			if constexpr (TArchive::IsLoading())
			{
				std::string isoDate;
				if (Serialize(archive, std::forward<TKey>(key), isoDate)) {
					return Detail::ConvertByPolicy(isoDate, tpValue, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
				}
				return false;
			}
			else
			{
				std::string isoDate = Convert::ToString(tpValue);
				return Serialize(archive, std::forward<TKey>(key), isoDate);
			}
		}
	}

	/// <summary>
	/// Serializes std::chrono::time_point (wall clock types) as ISO 8601 string in format: YYYY-MM-DDThh:mm:ss[.SSS]Z.
	/// </summary>
	template<typename TArchive, typename TClock, typename TDuration, std::enable_if_t<!TClock::is_steady, int> = 0>
	bool Serialize(TArchive& archive, std::chrono::time_point<TClock, TDuration>& tpValue)
	{
		if constexpr (can_serialize_value_v<TArchive, Detail::CBinTimestamp>)
		{
			// Serialize as binary timestamp
			Detail::CBinTimestamp timestamp;
			if constexpr (TArchive::IsLoading())
			{
				return archive.SerializeValue(timestamp)
					&& Detail::ConvertByPolicy(timestamp, tpValue, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
			}
			else
			{
				return Detail::ConvertByPolicy(tpValue, timestamp, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy)
					&& archive.SerializeValue(timestamp);
			}
		}
		else
		{
			// Serialize as ISO 8601
			if constexpr (TArchive::IsLoading())
			{
				std::string isoDate;
				if (Serialize(archive, isoDate)) {
					return Detail::ConvertByPolicy(isoDate, tpValue, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
				}
				return false;
			}
			else
			{
				std::string isoDate = Convert::ToString(tpValue);
				return Serialize(archive, isoDate);
			}
		}
	}

	/// <summary>
	/// Serializes std::chrono::duration as ISO 8601 string in format: PnWnDTnHnMnS.
	/// </summary>
	template <typename TArchive, typename TKey, typename TRep, typename TPeriod>
	bool Serialize(TArchive& archive, TKey&& key, std::chrono::duration<TRep, TPeriod>& value)
	{
		if constexpr (can_serialize_value_with_key_v<TArchive, Detail::CBinTimestamp, TKey>)
		{
			// Serialize as binary timestamp
			Detail::CBinTimestamp timestamp;
			if constexpr (TArchive::IsLoading())
			{
				return archive.SerializeValue(std::forward<TKey>(key), timestamp)
					&& Detail::ConvertByPolicy(timestamp, value, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
			}
			else
			{
				return Detail::ConvertByPolicy(value, timestamp, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy)
					&& archive.SerializeValue(std::forward<TKey>(key), timestamp);
			}
		}
		else
		{
			// Serialize as ISO 8601
			if constexpr (TArchive::IsLoading())
			{
				std::string isoDate;
				if (Serialize(archive, std::forward<TKey>(key), isoDate)) {
					return Detail::ConvertByPolicy(isoDate, value, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
				}
				return false;
			}
			else
			{
				std::string isoDate = Convert::ToString(value);
				return Serialize(archive, std::forward<TKey>(key), isoDate);
			}
		}
	}

	/// <summary>
	/// Serializes std::chrono::duration as ISO 8601 string in format: PnWnDTnHnMnS.
	/// </summary>
	template<typename TArchive, typename TRep, typename TPeriod>
	bool Serialize(TArchive& archive, std::chrono::duration<TRep, TPeriod>& value)
	{
		if constexpr (can_serialize_value_v<TArchive, Detail::CBinTimestamp>)
		{
			// Serialize as binary timestamp
			Detail::CBinTimestamp timestamp;
			if constexpr (TArchive::IsLoading())
			{
				return archive.SerializeValue(timestamp)
					&& Detail::ConvertByPolicy(timestamp, value, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
			}
			else
			{
				return Detail::ConvertByPolicy(value, timestamp, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy)
					&& archive.SerializeValue(timestamp);
			}
		}
		else
		{
			// Serialize as ISO 8601
			if constexpr (TArchive::IsLoading())
			{
				std::string isoDate;
				if (Serialize(archive, isoDate)) {
					return Detail::ConvertByPolicy(isoDate, value, archive.GetOptions().mismatchedTypesPolicy, archive.GetOptions().overflowNumberPolicy);
				}
				return false;
			}
			else
			{
				std::string isoDate = Convert::ToString(value);
				return Serialize(archive, isoDate);
			}
		}
	}
}
