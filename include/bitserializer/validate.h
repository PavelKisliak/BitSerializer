/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cmath>
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

	namespace Detail
	{
		template <typename T, template <typename> class TCompareOp>
		class Compare
		{
		public:
			Compare(T threshold, const char* errorMessage = nullptr) noexcept
				: mThreshold(threshold)
				, mErrorMessage(errorMessage)
			{ }

			[[nodiscard]] std::optional<std::string> operator()(const T& value, bool isLoaded) const
			{
				if (!isLoaded) {
					return std::nullopt;
				}

				TCompareOp<T> comp;
				if (!comp(value, mThreshold)) {
					return mErrorMessage ? mErrorMessage : GenerateDefaultMessage();
				}
				return std::nullopt;
			}

		private:
			[[nodiscard]] std::string GenerateDefaultMessage() const
			{
				if constexpr (std::is_same_v<TCompareOp<T>, std::greater<T>>) {
					return "Value must be greater than " + Convert::ToString(mThreshold);
				}
				else if constexpr (std::is_same_v<TCompareOp<T>, std::greater_equal<T>>) {
					return "Value must be greater than or equal to " + Convert::ToString(mThreshold);
				}
				else if constexpr (std::is_same_v<TCompareOp<T>, std::less<T>>) {
					return "Value must be less than " + Convert::ToString(mThreshold);
				}
				else if constexpr (std::is_same_v<TCompareOp<T>, std::less_equal<T>>) {
					return "Value must be less than or equal to " + Convert::ToString(mThreshold);
				}
				//return "Validation failed";
			}

			T mThreshold;
			const char* mErrorMessage;
		};
	}

	/// Validates that the value is strictly greater than the specified threshold.
	template <typename T>
	class GreaterThan : public Detail::Compare<T, std::greater>
	{
	public:
		GreaterThan(T threshold, const char* errorMessage = nullptr) noexcept
			: Detail::Compare<T, std::greater>(threshold, errorMessage)
		{ }
	};

	/// Validates that the value is greater than or equal to the specified threshold.
	template <typename T>
	class GreaterThanOrEqual : public Detail::Compare<T, std::greater_equal>
	{
	public:
		GreaterThanOrEqual(T threshold, const char* errorMessage = nullptr) noexcept
			: Detail::Compare<T, std::greater_equal>(threshold, errorMessage)
		{ }
	};

	/// Validates that the value is strictly less than the specified threshold.
	template <typename T>
	class LessThan : public Detail::Compare<T, std::less>
	{
	public:
		LessThan(T threshold, const char* errorMessage = nullptr) noexcept
			: Detail::Compare<T, std::less>(threshold, errorMessage)
		{ }
	};

	/// Validates that the value is less than or equal to the specified threshold.
	template <typename T>
	class LessThanOrEqual : public Detail::Compare<T, std::less_equal>
	{
	public:
		LessThanOrEqual(T threshold, const char* errorMessage = nullptr) noexcept
			: Detail::Compare<T, std::less_equal>(threshold, errorMessage)
		{ }
	};

	/// Validates that the value is within the specified range.
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
	 * @brief Validates that a numeric value is a multiple of a specified divisor.
	 * 
	 * For floating-point types, uses epsilon-based comparison to handle precision issues.
	 */
	template <typename T>
	class MultipleOf
	{
	public:
		MultipleOf(T divisor, const char* errorMessage = nullptr)
			: mDivisor(divisor)
			, mErrorMessage(errorMessage)
		{
			static_assert(std::is_arithmetic_v<T>, "MultipleOfValidator supports only arithmetic types");
			if constexpr (std::is_floating_point_v<T>)
			{
				if (mDivisor <= std::numeric_limits<T>::epsilon()) {
					throw std::invalid_argument("Divisor must be greater than epsilon for floating-point types");
				}
			}
			else
			{
				if (mDivisor == T{ 0 }) {
					throw std::invalid_argument("Divisor cannot be zero");
				}
			}
		}

		[[nodiscard]] std::optional<std::string> operator()(const T& value, bool isLoaded) const
		{
			if (!isLoaded) {
				return std::nullopt;
			}

			if constexpr (std::is_integral_v<T>)
			{
				if (value % mDivisor == T{ 0 }) {
					return std::nullopt;
				}
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				// For floating-point types - epsilon-based comparison
				const T remainder = std::fmod(std::abs(value), mDivisor);
				const T epsilon = std::numeric_limits<T>::epsilon() * std::abs(value);
				if (remainder <= epsilon || std::abs(mDivisor - remainder) <= epsilon) {
					return std::nullopt;
				}
			}
			return mErrorMessage ? mErrorMessage : "Value must be a multiple of " + Convert::ToString(mDivisor);
		}

	private:
		T mDivisor;
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

	/**
	 * @brief Validates that a value is a valid UUID (RFC 4122 and RFC 9562).
	 */
	class Uuid
	{
	public:
		explicit Uuid(const char* errorMessage = "Invalid UUID format")
			: mErrorMessage(errorMessage)
		{ }

		template <typename TContainer>
		[[nodiscard]] std::optional<std::string> operator()(const TContainer& value, bool isLoaded) const
		{
			if (!isLoaded) {
				return std::nullopt;
			}

			// UUID must be exactly 36 characters: 8-4-4-4-12 + 4 hyphens
			size_t length;
			if constexpr (std::is_array_v<std::remove_reference_t<TContainer>>) {
				length = std::size(value) - 1;
			}
			else {
				length = value.size();
			}
			if (length != 36) {
				return mErrorMessage;
			}

			for (size_t i = 0; i < 36; ++i)
			{
				if (i == 8 || i == 13 || i == 18 || i == 23)
				{
					if (!IsHyphen(value[i])) {
						return mErrorMessage;
					}
				}
				else if (!IsHexDigit(value[i])) {
					return mErrorMessage;
				}
			}
			return std::nullopt;
		}

	private:
		template <typename TChar>
		static constexpr bool IsHyphen(TChar c) noexcept {
			return c == static_cast<TChar>('-');
		}

		template <typename TChar>
		static constexpr bool IsHexDigit(TChar c) noexcept
		{
			return (c >= static_cast<TChar>('0') && c <= static_cast<TChar>('9')) ||
				(c >= static_cast<TChar>('a') && c <= static_cast<TChar>('f')) ||
				(c >= static_cast<TChar>('A') && c <= static_cast<TChar>('F'));
		}

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
	typedef Validate::MaxSize MaxSize;

	[[deprecated("Moved into sub-namespace `BitSerializer::Validate`")]]
	typedef Validate::Email Email;

	[[deprecated("Moved into sub-namespace `BitSerializer::Validate`")]]
	typedef Validate::PhoneNumber PhoneNumber;
}
