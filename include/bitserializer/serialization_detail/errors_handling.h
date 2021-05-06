/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include <string>
#include "bitserializer/convert.h"

namespace BitSerializer {

	/// <summary>
	/// Serialization error code
	/// </summary>
	enum class SerializationErrorCode
	{
		ParsingError,
		InputOutputError,
		UnsupportedEncoding
	};

	REGISTER_ENUM_MAP(SerializationErrorCode)
	{
		{ SerializationErrorCode::ParsingError, "Parsing error" },
		{ SerializationErrorCode::InputOutputError, "Input/output error" },
		{ SerializationErrorCode::UnsupportedEncoding, "Unsupported encoding" }
	}
	END_ENUM_MAP()

	/// <summary>
	/// Serialization exception
	/// </summary>
	/// <seealso cref="std::runtime_error" />
	class SerializationException : public std::runtime_error
	{
	public:
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

}
