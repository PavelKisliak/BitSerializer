/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "bitserializer/convert.h"

namespace BitSerializer
{
	/**
	 * @brief Error codes used for reporting serialization and parsing errors.
	 */
	enum class SerializationErrorCode
	{
		InvalidOptions,         ///< Invalid or unsupported serialization options.
		ParsingError,           ///< Error occurred during parsing input data.
		InputOutputError,       ///< Input/output operation failed.
		UnsupportedEncoding,    ///< The specified encoding is not supported.
		UtfEncodingError,       ///< UTF encoding or decoding failed.
		OutOfRange,             ///< Value is out of valid range.
		Overflow,               ///< Value overflowed the target type.
		MismatchedTypes,        ///< Attempted to deserialize into an incompatible type.
		FailedValidation,       ///< Validation failed for one or more fields.
		UnregisteredEnum        ///< Attempted to serialize/deserialize an unregistered enum.
	};

	BITSERIALIZER_REGISTER_ENUM(SerializationErrorCode, {
		{ SerializationErrorCode::InvalidOptions, "Invalid options" },
		{ SerializationErrorCode::ParsingError, "Parsing error" },
		{ SerializationErrorCode::InputOutputError, "Input/output error" },
		{ SerializationErrorCode::UnsupportedEncoding, "Unsupported encoding" },
		{ SerializationErrorCode::UtfEncodingError, "UTF encoding error" },
		{ SerializationErrorCode::OutOfRange, "Out of range" },
		{ SerializationErrorCode::Overflow, "Overflow" },
		{ SerializationErrorCode::MismatchedTypes, "Mismatched types" },
		{ SerializationErrorCode::FailedValidation, "Failed validation" },
		{ SerializationErrorCode::UnregisteredEnum, "Unregistered enum" }
	})

	/// @brief Collection of validation error messages for a single field.
	using ValidationErrors = std::vector<std::string>;

	/// @brief Map of fields with validation error messages.
	using ValidationMap = std::map<std::string, ValidationErrors>;

	/**
	 * @brief Base exception class for all serialization-related errors.
	 *
	 * This exception is thrown when a serialization, deserialization, or validation error occurs.
	 */
	class SerializationException : public std::runtime_error
	{
	public:
		/**
		 * @brief Constructs an exception with only an error code.
		 *
		 * @param errorCode The error code describing the failure.
		 */
		explicit SerializationException(const SerializationErrorCode errorCode)
			: std::runtime_error(Convert::ToString(errorCode))
			, mErrorCode(errorCode)
		{
		}

		/**
		 * @brief Constructs an exception with an error code and a custom message.
		 *
		 * @param errorCode The error code describing the failure.
		 * @param message   A human-readable error message.
		 */
		SerializationException(const SerializationErrorCode errorCode, const char* message)
			: std::runtime_error(Convert::ToString(errorCode) + ": " + message)
			, mErrorCode(errorCode)
		{
		}

		/**
		 * @brief Constructs an exception with an error code and a custom message.
		 *
		 * @param errorCode The error code describing the failure.
		 * @param message   A human-readable error message.
		 */
		SerializationException(const SerializationErrorCode errorCode, const std::string& message)
			: SerializationException(errorCode, message.c_str())
		{
		}

		/**
		 * @brief Retrieves the error code associated with this exception.
		 *
		 * @return The error code describing the failure.
		 */
		[[nodiscard]]
		SerializationErrorCode GetErrorCode() const noexcept
		{
			return mErrorCode;
		}

	private:
		SerializationErrorCode mErrorCode;
	};

	/**
	 * @brief Exception thrown when data parsing fails.
	 *
	 * Contains additional context such as line and offset where the error occurred.
	 */
	class ParsingException : public SerializationException
	{
	public:
		/**
		 * @brief Constructs a parsing exception with a message and location.
		 *
		 * @param message  A description of the parsing error.
		 * @param line     Line number where the error occurred (optional).
		 * @param offset   Character offset where the error occurred (optional).
		 */
		explicit ParsingException(const std::string& message, size_t line = 0, size_t offset = 0)
			: SerializationException(SerializationErrorCode::ParsingError, message)
			, Line(line)
			, Offset(offset)
		{
		}

		const size_t Line;   ///< Line number where parsing failed (0-based if used).
		const size_t Offset; ///< Character offset where parsing failed (0-based).
	};

	/**
	 * @brief Exception thrown when validation of deserialized data fails.
	 *
	 * Contains a map of fields with validation error messages.
	 */
	class ValidationException : public SerializationException
	{
	public:
		/**
		 * @brief Constructs a validation exception with detailed error map.
		 *
		 * @param validationErrors Map of fields with validation error messages.
		 */
		explicit ValidationException(ValidationMap&& validationErrors)
			: SerializationException(SerializationErrorCode::FailedValidation)
			, mValidationMap(std::move(validationErrors))
		{
		}

		/**
		 * @brief Gets the validation error map.
		 *
		 * @return A const reference to the validation error map.
		 */
		[[nodiscard]] const ValidationMap& GetValidationErrors() const noexcept
		{
			return mValidationMap;
		}

		/**
		 * @brief Extracts the validation error map by moving it.
		 *
		 * @return The validation error map.
		 */
		[[nodiscard]] ValidationMap&& TakeValidationErrors() noexcept
		{
			return std::move(mValidationMap);
		}

	private:
		ValidationMap mValidationMap;
	};

} // namespace BitSerializer
