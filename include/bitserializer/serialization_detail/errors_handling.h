/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <exception>
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
	/// <seealso cref="std::exception" />
	class SerializationException : public std::exception
	{
	public:
		SerializationException(const SerializationErrorCode errorCode, const char* message)
			: mErrorCode(errorCode)
			, mMessage(Convert::ToString(errorCode) + ": " + message)
		{ }

		SerializationException(const SerializationErrorCode errorCode, const std::string& message)
			: SerializationException(errorCode, message.c_str())
		{ }

		[[nodiscard]]
		SerializationErrorCode GetErrorCode() const noexcept
		{
			return mErrorCode;
		}

		[[nodiscard]]
		const char* what() const noexcept override
		{
			return mMessage.c_str();
		}

	private:
		SerializationErrorCode mErrorCode;
		std::string mMessage;
	};

}
