/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
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
	}

	/// <summary>
	/// Serializes Unix time in the `time_t` as ISO 8601/UTC string (YYYY-MM-DDThh:mm:ssZ).
	///	Usage example: archive << MakeKeyValue("Time", CTimeRef(timeValue));
	/// </summary>
	template <typename TArchive, typename TKey>
	bool Serialize(TArchive& archive, TKey&& key, CTimeRef timeRef)
	{
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

	/// <summary>
	/// Serializes Unix time in the `time_t` as ISO 8601/UTC string (YYYY-MM-DDThh:mm:ssZ).
	///	Usage example: archive << MakeKeyValue("Time", CTimeRef(timeValue));
	/// </summary>
	template<typename TArchive>
	bool Serialize(TArchive& archive, CTimeRef timeRef)
	{
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
