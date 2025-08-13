/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/serialization_detail/object_traits.h"
#include "bitserializer/convert.h"

namespace BitSerializer::Validate
{
	/**
	 * @brief Validates that field is deserialized.
	 */
	class Required
	{
	public:
		/**
		 * @param errorMessage Custom message to show if validation fails.
		 */
		constexpr explicit Required(const char* errorMessage = "Value is required") noexcept
			: mErrorMessage(errorMessage)
		{ }

		template <class TValue>
		std::optional<std::string> operator()(const TValue&, bool isLoaded) const
		{
			if (isLoaded) {
				return std::nullopt;
			}

			return mErrorMessage;
		}

	private:
		const char* mErrorMessage;
	};

	/**
	 * @brief Validates that a numeric value is within the specified range.
	 *
	 * Can be applied for any type that has operators '<' and '>' (e.g. `std::chrono` types).
	 */
	template <class TValue>
	class Range
	{
	public:
		/**
		 * @param min Minimum allowed value (inclusive).
		 * @param max Maximum allowed value (inclusive).
		 * @param errorMessage Optional custom error message.
		 */
		constexpr Range(const TValue& min, const TValue& max, const char* errorMessage = nullptr) noexcept(std::is_nothrow_copy_constructible_v<TValue>)
			: mMin(min)
			, mMax(max)
			, mErrorMessage(errorMessage)
		{ }

		std::optional<std::string> operator()(const TValue& value, bool isLoaded) const
		{
			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded) {
				return std::nullopt;
			}

			if (value < mMin || value > mMax)
			{
				if (mErrorMessage) {
					return mErrorMessage;
				}
				return "Must be between " + Convert::ToString(mMin) + " and " + Convert::ToString(mMax) + " (inclusive)";
			}

			return std::nullopt;
		}

	private:
		TValue mMin;
		TValue mMax;
		const char* mErrorMessage;
	};

	/**
	 * @brief Validates that the container or string size is not less than the specified minimum.
	 *
	 * The target type must have a `size()` method.
	 */
	class MinSize
	{
	public:
		/**
		 * @param minSize Minimum required size.
		 * @param errorMessage Optional custom error message.
		 */
		constexpr MinSize(const size_t minSize, const char* errorMessage = nullptr) noexcept
			: mMinSize(minSize)
			, mErrorMessage(errorMessage)
		{ }

		template <class TValue>
		std::optional<std::string> operator()(const TValue& value, bool isLoaded) const
		{
			constexpr auto hasSizeMethod = has_size_v<TValue>;
			static_assert(hasSizeMethod, "BitSerializer. 'MinSize' validator can only be used for types with a size() method.");

			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded) {
				return std::nullopt;
			}

			if constexpr (hasSizeMethod)
			{
				if (value.size() >= mMinSize) {
					return std::nullopt;
				}

				if (mErrorMessage) {
					return mErrorMessage;
				}
				return "Size must be less than " + Convert::ToString(mMinSize);
			}
		}

	private:
		size_t mMinSize;
		const char* mErrorMessage;
	};

	/**
	 * @brief Validates that the size of a container or string does not exceed the specified maximum.
	 *
	 * The target type must have a `size()` method.
	 */
	class MaxSize
	{
	public:
		/**
		 * @param maxSize Maximum allowed size.
		 * @param errorMessage Optional custom error message.
		 */
		constexpr MaxSize(const size_t maxSize, const char* errorMessage = nullptr) noexcept
			: mMaxSize(maxSize)
			, mErrorMessage(errorMessage)
		{ }

		template <class TValue>
		std::optional<std::string> operator()(const TValue& value, bool isLoaded) const
		{
			constexpr auto hasSizeMethod = has_size_v<TValue>;
			static_assert(hasSizeMethod, "BitSerializer. 'MaxSize' validator can only be used for types with a size() method.");

			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded) {
				return std::nullopt;
			}

			if constexpr (hasSizeMethod)
			{
				if (value.size() <= mMaxSize) {
					return std::nullopt;
				}

				if (mErrorMessage) {
					return mErrorMessage;
				}
				return "Size must not exceed " + Convert::ToString(mMaxSize);
			}
		}

	private:
		size_t mMaxSize;
		const char* mErrorMessage;
	};

	/**
	 * @brief Validates that a string contains a valid email address.
	 *
	 * Supports most common formats defined by RFC 5322, excluding quoted strings,
	 * comments, SMTPUTF8 extensions, and IP addresses in the domain part.
	 */
	class Email
	{
	public:
		/**
		 * @param errorMessage Optional custom error message.
		 */
		constexpr explicit Email(const char* errorMessage = "Invalid email format") noexcept
			: mErrorMessage(errorMessage)
		{ }

		template <typename T, std::enable_if_t<Convert::Detail::is_convertible_to_string_view_v<T>, int> = 0>
		std::optional<std::string> operator() (const T& value, bool isLoaded) const
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
				if (const char_type ch = static_cast<char_type>(str[i]); ch == '.')
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

			// Does not contain @ sign, the domain part is missing, or it is too long
			if (isLocalPart || startDomainPos == strSize || strSize - startDomainPos > domainPartMaxSize)
			{
				isValid = false;
			}

			return isValid ? std::nullopt : std::make_optional(mErrorMessage);
		}

	private:
		const char* mErrorMessage;
	};

	/**
	 * @brief Validates that a string matches a phone number pattern.
	 *
	 * Allows optional '+' prefix, parentheses, spaces, and dashes.
	 * Examples: "+1 (800) 123-45-67", "800 123 4567", "(800)123-4567".
	 */
	class PhoneNumber
	{
	public:
		/**
		 * @param minDigits Minimum number of digits required.
		 * @param maxDigits Maximum number of digits allowed.
		 * @param plusRequired Whether a leading '+' is mandatory.
		 * @param errorMessage Optional custom error message.
		 */
		constexpr PhoneNumber(size_t minDigits = 7, size_t maxDigits = 15, bool plusRequired = true, const char* errorMessage = nullptr) noexcept
			: mMinDigits(minDigits)
			, mMaxDigits(maxDigits)
			, mPlusRequired(plusRequired)
			, mErrorMessage(errorMessage)
		{ }

		template <typename T, std::enable_if_t<Convert::Detail::is_convertible_to_string_view_v<T>, int> = 0>
		std::optional<std::string> operator()(const T& value, bool isLoaded) const
		{
			// Automatically pass if value is not loaded. "Required" validator should be used to check this case.
			if (!isLoaded) {
				return std::nullopt;
			}

			bool hasPlus = false;
			bool inParenthesis = false;
			bool lastWasDigit = false;
			size_t digitCount = 0;
			const char* error = nullptr;

			const auto str = Convert::Detail::ToStringView(value);
			const size_t strSize = str.size();

			using char_type = std::make_unsigned_t<typename decltype(str)::value_type>;
			for (size_t i = 0; i < strSize && !error; ++i)
			{
				if (const char_type ch = static_cast<char_type>(str[i]); digitCount == 0 && ch == '+') {
					hasPlus = true;
				}
				else if (ch >= '0' && ch <= '9')
				{
					++digitCount;
					lastWasDigit = true;
				}
				else if (ch != ' ')
				{
					if (ch == '-')
					{
						if (!lastWasDigit || i + 1 == strSize) {
							error = "Dashes must be used to separate number groups";
						}
					}
					else if (ch == '(')
					{
						if (inParenthesis) {
							error = "Phone must not contain nested parentheses";
						}
						inParenthesis = true;
					}
					else if (ch == ')')
					{
						if (inParenthesis && lastWasDigit) {
							inParenthesis = false;
						}
						else {
							error = "Closing parenthesis ')' is in wrong position";
						}
					}
					else {
						error = "Phone contains invalid characters";
					}
					lastWasDigit = false;
				}
			}

			if (mPlusRequired && !hasPlus) {
				error = "Phone number must start with '+'";
			}

			if (inParenthesis) {
				error = "Missing closing parenthesis ')'";
			}

			if (error) {
				return std::make_optional<std::string>(mErrorMessage ? mErrorMessage : error);
			}

			if (digitCount < mMinDigits || digitCount > mMaxDigits)
			{
				if (mMinDigits == mMaxDigits) {
					return mErrorMessage ? mErrorMessage : std::make_optional<std::string>("Phone must contain at least " + Convert::ToString(mMinDigits) + " digits");
				}
				return mErrorMessage ? mErrorMessage : std::make_optional<std::string>("Phone must contain "
					+ Convert::ToString(mMinDigits) + " to " + Convert::ToString(mMaxDigits) + " digits");
			}
			return std::nullopt;
		}

	private:
		size_t mMinDigits;
		size_t mMaxDigits;
		bool mPlusRequired;
		const char* mErrorMessage;
	};
}


// ToDo: remove in the future versions
namespace BitSerializer
{
	template <class TValue>
	class Range : public Validate::Range<TValue>
	{
	public:
		[[deprecated("Moved into sub-namespace `BitSerializer::Validate`")]]
		constexpr Range(const TValue& min, const TValue& max, const char* errorMessage = nullptr)
			: Validate::Range<TValue>(min, max, errorMessage)
		{ }
	};

	[[deprecated("Moved into sub-namespace `BitSerializer::Validate`")]]
	typedef Validate::MinSize MinSize;

	[[deprecated("Moved into sub-namespace `BitSerializer::Validate`")]]
	typedef Validate::MinSize MaxSize;

	[[deprecated("Moved into sub-namespace `BitSerializer::Validate`")]]
	typedef Validate::Email Email;

	[[deprecated("Moved into sub-namespace `BitSerializer::Validate`")]]
	typedef Validate::PhoneNumber PhoneNumber;
}
