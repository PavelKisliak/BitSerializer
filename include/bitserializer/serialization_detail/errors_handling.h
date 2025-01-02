/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
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
	/// <summary>
	/// Serialization error code
	/// </summary>
	enum class SerializationErrorCode
	{
		InvalidOptions,
		ParsingError,
		InputOutputError,
		UnsupportedEncoding,
		UtfEncodingError,
		OutOfRange,
		Overflow,
		MismatchedTypes,
		FailedValidation,
		UnregisteredEnum
	};

	REGISTER_ENUM(SerializationErrorCode, {
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

	using ValidationErrors = std::vector<std::string>;
	using ValidationMap = std::map<std::string, ValidationErrors>;

	/// <summary>
	/// Serialization exception
	/// </summary>
	/// <seealso cref="std::runtime_error" />
	class SerializationException : public std::runtime_error
	{
	public:
		explicit SerializationException(const SerializationErrorCode errorCode)
			: std::runtime_error(Convert::ToString(errorCode))
			, mErrorCode(errorCode)
		{ }

		SerializationException(const SerializationErrorCode errorCode, const char* message)
			: std::runtime_error(Convert::ToString(errorCode) + ": " + message)
			, mErrorCode(errorCode)
		{ }

		SerializationException(const SerializationErrorCode errorCode, const std::string& message)
			: SerializationException(errorCode, message.c_str())
		{ }

		[[nodiscard]]
		SerializationErrorCode GetErrorCode() const noexcept
		{
			return mErrorCode;
		}

	private:
		SerializationErrorCode mErrorCode;
	};

	/// <summary>
	/// Parsing exception
	/// </summary>
	/// <seealso cref="SerializationException" />
	class ParsingException : public SerializationException
	{
	public:
		explicit ParsingException(const std::string& message, size_t line = 0, size_t offset = 0)
			: SerializationException(SerializationErrorCode::ParsingError, message)
			, Line(line)
			, Offset(offset)
		{ }

		const size_t Line;
		const size_t Offset;
	};

	/// <summary>
	/// validation exception
	/// </summary>
	/// <seealso cref="SerializationException" />
	class ValidationException : public SerializationException
	{
	public:
		ValidationException(ValidationMap&& validationErrors)
			: SerializationException(SerializationErrorCode::FailedValidation)
			, mValidationMap(std::move(validationErrors))
		{ }

		[[nodiscard]] const ValidationMap& GetValidationErrors() const noexcept
		{
			return mValidationMap;
		}

		[[nodiscard]] ValidationMap&& TakeValidationErrors() noexcept
		{
			return std::move(mValidationMap);
		}

	private:
		ValidationMap mValidationMap;
	};

}
