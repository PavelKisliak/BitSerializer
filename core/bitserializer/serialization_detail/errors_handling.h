/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <exception>
#include <string>
#include "../string_conversion.h"

namespace BitSerializer {

	/// <summary>
	/// Serialization error code
	/// </summary>
	enum class SerializationErrorCode
	{
		ParsingError
	};

	namespace Convert::Detail
	{
		static const bool _SerializationErrorCode = ConvertEnum::Register<SerializationErrorCode>(
		{
			{ SerializationErrorCode::ParsingError,	"Parsing error" }
		});
	}

	/// <summary>
	/// Serialization exception
	/// </summary>
	/// <seealso cref="std::exception" />
	class SerializationException : public std::exception
	{
	public:
		SerializationException(const SerializationErrorCode errorCode, const std::string& message)
			: mErrorCode(errorCode)
			, mMessage((Convert::ToString(errorCode) + ": " + message))
		{ }

		SerializationErrorCode GetErrorCode() const noexcept
		{
			return mErrorCode;
		}

		const char* what() const noexcept override
		{
			return mMessage.c_str();
		}

	private:
		SerializationErrorCode mErrorCode;
		std::string mMessage;
	};

}	// namespace BitSerializer
