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
	}

	/// <summary>
	/// Serializes std::chrono::time_point (wall clock types) as ISO 8601 string in format: YYYY-MM-DDThh:mm:ss[.SSS]Z.
	/// </summary>
	template <typename TArchive, typename TKey, typename TClock, typename TDuration, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	bool Serialize(TArchive& archive, TKey&& key, std::chrono::time_point<TClock, TDuration>& tpValue)
	{
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

	/// <summary>
	/// Serializes std::chrono::time_point (wall clock types) as ISO 8601 string in format: YYYY-MM-DDThh:mm:ss[.SSS]Z.
	/// </summary>
	template<typename TArchive, typename TClock, typename TDuration, std::enable_if_t<(TClock::is_steady == false), int> = 0>
	bool Serialize(TArchive& archive, std::chrono::time_point<TClock, TDuration>& tpValue)
	{
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
