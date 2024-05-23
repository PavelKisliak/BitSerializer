/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <optional>
#include "object_traits.h"
#include "bitserializer/convert.h"

namespace BitSerializer
{
	/// <summary>
	/// Validates that field is deserialized.
	/// </summary>
	class Required
	{
	public:
		template <class TValue>
		std::optional<std::string> operator() (const TValue&, bool isLoaded) const noexcept
		{
			if (isLoaded) {
				return std::nullopt;
			}

			return "This field is required";
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

		std::optional<std::string> operator() (const TValue& value, bool isLoaded) const
		{
			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded) {
				return std::nullopt;
			}

			if (value < mMin || value > mMax) {
				return "Value must be between " + Convert::ToString(mMin) + " and " + Convert::ToString(mMax);
			}

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
		std::optional<std::string> operator() (const TValue& value, bool isLoaded) const
		{
			constexpr auto hasSizeMethod = has_size_v<TValue>;
			static_assert(hasSizeMethod, "BitSerializer. The 'MinSize' validator can be applied only for types which has size() method.");

			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded) {
				return std::nullopt;
			}

			if constexpr (hasSizeMethod)
			{
				if (value.size() >= mMinSize) {
					return std::nullopt;
				}

				return "The minimum size of this field should be " + Convert::ToString(mMinSize);
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
		std::optional<std::string> operator() (const TValue& value, bool isLoaded) const
		{
			constexpr auto hasSizeMethod = has_size_v<TValue>;
			static_assert(hasSizeMethod, "BitSerializer. The 'MaxSize' validator can be applied only for types which has size() method.");

			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded) {
				return std::nullopt;
			}

			if constexpr (hasSizeMethod)
			{
				if (value.size() <= mMaxSize) {
					return std::nullopt;
				}

				return "The maximum size of this field should be not greater than " + Convert::ToString(mMaxSize);
			}
			return std::nullopt;
		}

	private:
		size_t mMaxSize;
	};

	/// <summary>
	/// Validates that string contains an email.
	/// Generally complies with the RFC standard, except: quoted parts, comments, SMTPUTF8 and IP address as domain part.
	/// </summary>
	class Email
	{
	public:
		template <typename T, std::enable_if_t<Convert::Detail::is_convertible_to_string_view_v<T>, int> = 0>
		std::optional<std::string> operator() (T&& value, bool isLoaded) const
		{
			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded) {
				return std::nullopt;
			}

			constexpr int localPartMaxSize = 64;
			constexpr int domainPartMaxSize = 255;
			constexpr int domainPartLabelMaxSize = 63;
			static constexpr uint8_t allowedLocalPartChars[127]{
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,0,35,36,37,38,39,0,0,42,43,0,45,0,47,48,49,50,51,52,53,54,55,56,57,0,0,0,61,0,63,0,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,0,0,0,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126
			};

			bool isValid = true, isLocalPart = true;
			int currentLabelSize = 0, startDomainPos = 0, lastDotPos = -1;
			const auto str = Convert::Detail::ToStringView(value);
			const auto strSize = static_cast<int>(str.size());
			using char_type = std::make_unsigned_t<typename decltype(str)::value_type>;
			for (int i = 0; i < strSize && isValid; ++i)
			{
				++currentLabelSize;
				if (const char_type ch = str[i]; ch == '.')
				{
					// The dot cannot be first, last or appear consecutively
					if (lastDotPos + 1 == i || strSize - 1 == i)
					{
						isValid = false;
					}
					lastDotPos = i;
					currentLabelSize = 0;
				}
				else
				{
					if (isLocalPart)
					{
						if (ch == '@')
						{
							isLocalPart = false;
							startDomainPos = i + 1;
							currentLabelSize = 0;
							// Check max local part size, dot cannot be last
							if (i > localPartMaxSize || lastDotPos + 1 == i)
							{
								isValid = false;
							}
						}
						else if (ch >= sizeof(allowedLocalPartChars) || allowedLocalPartChars[ch] == 0)
						{
							isValid = false;
						}
					}
					else
					{
						if (ch == '-')
						{
							// Label can't start or end with hyphen
							if (currentLabelSize == 1 || i + 1 == strSize || str[i + 1] == '.')
							{
								isValid = false;
							}
						}
						else if (ch >= '0' && ch <= '9')
						{
							// Label can't start with digit
							if (currentLabelSize == 1)
							{
								isValid = false;
							}
						}
						else if (ch < 'A' || ch > 'z' || (ch > 'Z' && ch < 'a'))
						{
							isValid = false;
						}
						// Check max label size
						if (currentLabelSize > domainPartLabelMaxSize)
						{
							isValid = false;
						}
					}
				}
			}

			// Does not contain @ sign, the domain part is missing or it is too long
			if (isLocalPart || startDomainPos == strSize || strSize - startDomainPos > domainPartMaxSize)
			{
				isValid = false;
			}

			return isValid ? std::nullopt : std::make_optional("Invalid email address");
		}
	};
}
