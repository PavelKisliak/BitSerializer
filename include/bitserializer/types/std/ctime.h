/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <ctime>
#include "bitserializer/conversion_detail/convert_chrono.h"

namespace BitSerializer
{
	/// <summary>
	/// Wrapper that holds reference to `time_t` type, used to distinguish between time_t and integer types.
	///	Usage example: archive << MakeKeyValue("Time", CTimeRef(timeValue));
	/// </summary>
	struct CTimeRef
	{
		explicit CTimeRef(time_t& timeRef) noexcept : Time(timeRef) {}

		time_t& Time;
	};

	namespace Detail
	{
		inline bool SafeConvertIsoDate(const std::string& isoDate, CTimeRef time, const SerializationOptions& options)
		{
			try
			{
				time.Time = Convert::To<CRawTime>(isoDate);
				return true;
			}
			catch (const std::invalid_argument&)
			{
				if (options.mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"The value being loaded is not a valid ISO datetime: " + isoDate);
				}
			}
			catch (...) {
				throw SerializationException(SerializationErrorCode::ParsingError, "Unknown error when parsing datetime");
			}
			return false;
		}

		inline bool SafeConvertFromBinTimestamp(const CBinTimestamp& timestamp, CTimeRef timeRef, const SerializationOptions& options)
		{
			timeRef.Time = timestamp.Seconds;
			if (timestamp.Nanoseconds && options.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::Overflow,
					"The precision of target time_t type is not sufficient to store nanoseconds");
			}
			return true;
		}
	}

	/// <summary>
	/// Serializes Unix time in the `time_t` as ISO 8601/UTC string (YYYY-MM-DDThh:mm:ssZ) or binary (if supported by archive).
	///	Usage example: archive << MakeKeyValue("Time", CTimeRef(timeValue));
	/// </summary>
	template <typename TArchive, typename TKey>
	bool Serialize(TArchive& archive, TKey&& key, CTimeRef timeRef)
	{
		if constexpr (can_serialize_value_with_key_v<TArchive, Detail::CBinTimestamp, TKey>)
		{
			// Serialize as binary timestamp
			if constexpr (TArchive::IsLoading())
			{
				Detail::CBinTimestamp timestamp;
				return archive.SerializeValue(std::forward<TKey>(key), timestamp)
					&& SafeConvertFromBinTimestamp(timestamp, timeRef, archive.GetOptions());
			}
			else
			{
				Detail::CBinTimestamp timestamp(timeRef.Time);
				return archive.SerializeValue(std::forward<TKey>(key), timestamp);
			}
		}
		else
		{
			// Serialize as ISO 8601
			if constexpr (TArchive::IsLoading())
			{
				std::string isoDate;
				if (Serialize(archive, std::forward<TKey>(key), isoDate)) {
					return Detail::SafeConvertIsoDate(isoDate, timeRef, archive.GetOptions());
				}
				return false;
			}
			else
			{
				std::string isoDate = Convert::ToString(CRawTime(timeRef.Time));
				return Serialize(archive, std::forward<TKey>(key), isoDate);
			}
		}
	}

	/// <summary>
	/// Serializes Unix time in the `time_t` as ISO 8601/UTC string (YYYY-MM-DDThh:mm:ssZ) or binary (if supported by archive).
	///	Usage example: archive << MakeKeyValue("Time", CTimeRef(timeValue));
	/// </summary>
	template<typename TArchive>
	bool Serialize(TArchive& archive, CTimeRef timeRef)
	{
		if constexpr (can_serialize_value_v<TArchive, Detail::CBinTimestamp>)
		{
			// Serialize as binary timestamp
			if constexpr (TArchive::IsLoading())
			{
				Detail::CBinTimestamp timestamp;
				return archive.SerializeValue(timestamp)
					&& SafeConvertFromBinTimestamp(timestamp, timeRef, archive.GetOptions());
			}
			else
			{
				Detail::CBinTimestamp timestamp(timeRef.Time);
				return archive.SerializeValue(timestamp);
			}
		}
		else
		{
			// Serialize as ISO 8601
			if constexpr (TArchive::IsLoading())
			{
				std::string isoDate;
				if (Serialize(archive, isoDate)) {
					return Detail::SafeConvertIsoDate(isoDate, timeRef, archive.GetOptions());
				}
				return false;
			}
			else
			{
				std::string isoDate = Convert::ToString(CRawTime(timeRef.Time));
				return Serialize(archive, isoDate);
			}
		}
	}
}
