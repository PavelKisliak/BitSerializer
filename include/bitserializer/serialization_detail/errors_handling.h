/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include <string>
#include "bitserializer/convert.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialization error code
	/// </summary>
	enum class SerializationErrorCode
	{
		ParsingError,
		InputOutputError,
		UnsupportedEncoding,
		OutOfRange,
		FailedValidation
	};

	REGISTER_ENUM_MAP(SerializationErrorCode)
	{
		{ SerializationErrorCode::ParsingError, "Parsing error" },
		{ SerializationErrorCode::InputOutputError, "Input/output error" },
		{ SerializationErrorCode::UnsupportedEncoding, "Unsupported encoding" },
		{ SerializationErrorCode::OutOfRange, "Out of range" },
		{ SerializationErrorCode::FailedValidation, "Failed validation" }
	}
	END_ENUM_MAP()

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
