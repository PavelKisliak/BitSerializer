/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <chrono>
#include <type_traits>

namespace BitSerializer
{
	namespace Detail
	{
		template <typename TClock, typename TDuration, std::enable_if_t<(TClock::is_steady == false), int> = 0>
		bool SafeConvertIsoDate(const std::string& isoDate, std::chrono::time_point<TClock, TDuration>& targetTimePoint, const SerializationOptions& options)
		{
			try
			{
				targetTimePoint = Convert::To<std::chrono::time_point<TClock, TDuration>>(isoDate);
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
			catch (const std::out_of_range&)
			{
				if (options.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::Overflow,
						"Target timepoint range is not sufficient to deserialize: " + isoDate);
				}
			}
			catch (...) {
				throw SerializationException(SerializationErrorCode::ParsingError, "Unknown error when parsing datetime");
			}
			return false;
		}

		template <typename TRep, typename TPeriod>
		bool SafeConvertIsoDuration(const std::string& isoDuration, std::chrono::duration<TRep, TPeriod>& targetDuration, const SerializationOptions& options)
		{
			try
			{
				targetDuration = Convert::To<std::chrono::duration<TRep, TPeriod>>(isoDuration);
				return true;
			}
			catch (const std::invalid_argument&)
			{
				if (options.mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"The value being loaded is not a valid ISO duration: " + isoDuration);
				}
			}
			catch (const std::out_of_range&)
			{
				if (options.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::Overflow,
						"Target duration range is not sufficient to deserialize: " + isoDuration);
				}
			}
			catch (...) {
				throw SerializationException(SerializationErrorCode::ParsingError, "Unknown error when parsing datetime");
			}
			return false;
		}

		template <typename TClock, typename TDuration>
		bool SafeConvertToBinTimestamp(const std::chrono::time_point<TClock, TDuration>& timePoint, CBinTimestamp& outTimestamp, const SerializationOptions& options)
		{
			try
			{
				const auto epochTime = timePoint.time_since_epoch();
				// When duration period is equal or greater than seconds
				if constexpr (std::ratio_greater_equal_v<typename TDuration::period, std::chrono::seconds::period>)
				{
					outTimestamp.Seconds = Convert::Detail::SafeDurationCast<std::chrono::seconds>(epochTime).count();
					outTimestamp.Nanoseconds = 0;
				}
				else
				{
					outTimestamp.Seconds = std::chrono::duration_cast<std::chrono::seconds>(epochTime).count();
					const auto ns = epochTime - std::chrono::duration_cast<TDuration>(std::chrono::seconds(outTimestamp.Seconds));
					outTimestamp.Nanoseconds = static_cast<uint32_t>(ns.count());
				}
				return true;
			}
			catch (const std::out_of_range&)
			{
				if (options.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::Overflow,
						"Target timestamp range is not sufficient to serialize timepoint");
				}
			}
			return false;
		}

		template <typename TClock, typename TDuration>
		bool SafeConvertFromBinTimestamp(const CBinTimestamp& timestamp, std::chrono::time_point<TClock, TDuration>& outTimePoint, const SerializationOptions& options)
		{
			try
			{
				outTimePoint = std::chrono::time_point<TClock, TDuration>(
					Convert::Detail::SafeDurationCast<TDuration>(std::chrono::seconds(timestamp.Seconds)));
				if (timestamp.Nanoseconds) {
					Convert::Detail::SafeAddDuration(outTimePoint, std::chrono::nanoseconds(timestamp.Nanoseconds));
				}
				return true;
			}
			catch (const std::out_of_range&)
			{
				if (options.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::Overflow,
						"Target timepoint range is not sufficient to deserialize binary timestamp");
				}
			}
			return false;
		}
	}

	/// <summary>
	/// Serializes std::chrono::time_point (wall clock types) as ISO 8601 string in format: YYYY-MM-DDThh:mm:ss[.SSS]Z.
	/// </summary>
	template <typename TArchive, typename TKey, typename TClock, typename TDuration, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	bool Serialize(TArchive& archive, TKey&& key, std::chrono::time_point<TClock, TDuration>& tpValue)
	{
		if constexpr (can_serialize_value_with_key_v<TArchive, Detail::CBinTimestamp, TKey>)
		{
			// Serialize as binary timestamp
			Detail::CBinTimestamp timestamp;
			if constexpr (TArchive::IsLoading())
			{
				return archive.SerializeValue(std::forward<TKey>(key), timestamp)
					&& Detail::SafeConvertFromBinTimestamp(timestamp, tpValue, archive.GetOptions());
			}
			else
			{
				return SafeConvertToBinTimestamp(tpValue, timestamp, archive.GetOptions())
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
					return Detail::SafeConvertIsoDate(isoDate, tpValue, archive.GetOptions());
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
	template<typename TArchive, typename TClock, typename TDuration, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	bool Serialize(TArchive& archive, std::chrono::time_point<TClock, TDuration>& tpValue)
	{
		if constexpr (can_serialize_value_v<TArchive, Detail::CBinTimestamp>)
		{
			// Serialize as binary timestamp
			Detail::CBinTimestamp timestamp;
			if constexpr (TArchive::IsLoading())
			{
				return archive.SerializeValue(timestamp)
					&& Detail::SafeConvertFromBinTimestamp(timestamp, tpValue, archive.GetOptions());
			}
			else
			{
				return SafeConvertToBinTimestamp(tpValue, timestamp, archive.GetOptions())
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
					return Detail::SafeConvertIsoDate(isoDate, tpValue, archive.GetOptions());
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
		if constexpr (TArchive::IsLoading())
		{
			std::string isoDate;
			if (Serialize(archive, std::forward<TKey>(key), isoDate)) {
				return Detail::SafeConvertIsoDuration(isoDate, value, archive.GetOptions());
			}
			return false;
		}
		else
		{
			std::string isoDate = Convert::ToString(value);
			return Serialize(archive, std::forward<TKey>(key), isoDate);
		}
	}

	/// <summary>
	/// Serializes std::chrono::duration as ISO 8601 string in format: PnWnDTnHnMnS.
	/// </summary>
	template<typename TArchive, typename TRep, typename TPeriod>
	bool Serialize(TArchive& archive, std::chrono::duration<TRep, TPeriod>& value)
	{
		if constexpr (TArchive::IsLoading())
		{
			std::string isoDate;
			if (Serialize(archive, isoDate)) {
				return Detail::SafeConvertIsoDuration(isoDate, value, archive.GetOptions());
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
