/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <optional>
#include "object_traits.h"
#include "../string_conversion.h"

namespace BitSerializer
{
	/// <summary>
	/// Validates that field is deserialized.
	/// </summary>
	class Required
	{
	public:
		template <class TValue>
		std::optional<std::wstring> operator() (const TValue& value, const bool isLoaded) const noexcept
		{
			if (isLoaded)
				return std::nullopt;

			return L"This field is required";
		}
	};


	/// <summary>
	/// Validates that field is in required range.
	/// </summary>
	template <class TValue>
	class Range
	{
	public:
		Range(const TValue& min, const TValue& max)
			: mMin(min)
			, mMax(max)
		{
		}

		std::optional<std::wstring> operator() (const TValue& value, const bool isLoaded) const
		{
			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded)
				return std::nullopt;

			if (value < mMin || value > mMax)
				return L"Value must be between " + Convert::ToWString(mMin) + L" and " + Convert::ToWString(mMax);

			return std::nullopt;
		}

	private:
		TValue mMin;
		TValue mMax;
	};


	/// <summary>
	/// Validates that size of field (string, container) is greater or equal than specified value.
	/// </summary>
	class MinSize
	{
	public:
		MinSize(const size_t minSize) noexcept
			: mMinSize(minSize)
		{
		}

		template <class TValue>
		std::optional<std::wstring> operator() (const TValue& value, const bool isLoaded) const
		{
			constexpr auto hasSizeMethod = has_size_v<TValue>;
			static_assert(hasSizeMethod, "BitSerializer. The 'MinSize' validator can be applied only for types which has size() method.");

			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded)
				return std::nullopt;

			if constexpr (hasSizeMethod)
			{
				if (value.size() >= mMinSize)
					return std::nullopt;

				return L"The minimum size of this field should be " + Convert::ToWString(mMinSize);
			}
			return std::nullopt;
		}

	private:
		size_t mMinSize;
	};

	/// <summary>
	/// Validates that size of field (string, container) is not greater than specified value.
	/// </summary>
	class MaxSize
	{
	public:
		MaxSize(const size_t maxSize) noexcept
			: mMaxSize(maxSize)
		{
		}

		template <class TValue>
		std::optional<std::wstring> operator() (const TValue& value, const bool isLoaded) const
		{
			constexpr auto hasSizeMethod = has_size_v<TValue>;
			static_assert(hasSizeMethod, "BitSerializer. The 'MaxSize' validator can be applied only for types which has size() method.");

			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded)
				return std::nullopt;

			if constexpr (hasSizeMethod)
			{
				if (value.size() <= mMaxSize)
					return std::nullopt;

				return L"The maximum size of this field should be not greater than " + Convert::ToWString(mMaxSize);
			}
			return std::nullopt;
		}

	private:
		size_t mMaxSize;
	};

}	// namespace BitSerializer