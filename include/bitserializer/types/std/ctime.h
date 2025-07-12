/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <ctime>
#include "bitserializer/conversion_detail/convert_chrono.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"

namespace BitSerializer
{
	/**
	 * @brief A wrapper that holds a reference to a `time_t` value.
	 *
	 * Use this when need to serialize Unix timestamps as ISO datetime.
	 *
	 * @par Example:
	 * @code
	 * time_t now = std::time(nullptr);
	 * archive << KeyValue("Time", CTimeRef(now));
	 * @endcode
	 */
	struct CTimeRef
	{
		explicit CTimeRef(time_t& timeRef) noexcept : Time(timeRef) {}

		time_t& Time; ///< Reference to the underlying time value.
	};

	namespace Detail
	{
		/**
		 * @brief Safely converts an ISO date string to a `time_t` value.
		 *
		 * Handles invalid input gracefully based on the provided policy.
		 *
		 * @param[in] isoDate ISO 8601 formatted date-time string.
		 * @param[out] time Output reference to store the converted time.
		 * @param[in] options Serialization options controlling behavior on mismatched types.
		 * @return true if conversion succeeded, false otherwise.
		 * @throws SerializationException If MismatchedTypesPolicy::ThrowError is set and parsing fails.
		 */
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

	/**
	 * @brief Serializes a Unix timestamp (`time_t`) as ISO 8601/UTC string (YYYY-MM-DDThh:mm:ssZ) or binary (if supported by archive).
	 */
	template <typename TArchive, typename TKey>
	bool Serialize(TArchive& archive, TKey&& key, CTimeRef timeRef)
	{
		if constexpr (can_serialize_value_with_key_v<TArchive, Detail::CBinTimestamp, TKey>)
		{
			// Serialize as binary timestamp
			if constexpr (TArchive::IsLoading())
			{
				Detail::CBinTimestamp timestamp;
				if (archive.SerializeValue(std::forward<TKey>(key), timestamp))
				{
					// Ignore nanoseconds
					timeRef.Time = timestamp.Seconds;
					return true;
				}
				return false;
			}
			else
			{
				Detail::CBinTimestamp timestamp(timeRef.Time);
				return archive.SerializeValue(std::forward<TKey>(key), timestamp);
			}
		}
		else
		{
			// Serialize as ISO 8601 string
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

	/**
	 * @brief Serializes a Unix timestamp (`time_t`) as ISO 8601/UTC string (YYYY-MM-DDThh:mm:ssZ) or binary (if supported by archive).
	 */
	template<typename TArchive>
	bool Serialize(TArchive& archive, CTimeRef timeRef)
	{
		if constexpr (can_serialize_value_v<TArchive, Detail::CBinTimestamp>)
		{
			// Serialize as binary timestamp
			if constexpr (TArchive::IsLoading())
			{
				Detail::CBinTimestamp timestamp;
				if (archive.SerializeValue(timestamp))
				{
					// Ignore nanoseconds
					timeRef.Time = timestamp.Seconds;
					return true;
				}
				return false;
			}
			else
			{
				Detail::CBinTimestamp timestamp(timeRef.Time);
				return archive.SerializeValue(timestamp);
			}
		}
		else
		{
			// Serialize as ISO 8601 string
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
