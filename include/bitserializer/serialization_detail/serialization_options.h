/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>
#include "bitserializer/conversion_detail/convert_utf.h"

namespace BitSerializer
{
	/**
	 * @brief Configuration options for text formatting in output archives.
	 *
	 * These settings control how structured data is formatted when serializing into human-readable formats,
	 * such as JSON or XML. Some options may be ignored depending on the archive format being used.
	 */
	struct FormatOptions
	{
		/**
		 * @brief Enables pretty-printing (indentation and line breaks) for the output archive.
		 */
		bool enableFormat = false;

		/**
		 * @brief Character used for padding levels of indentation; must be whitespace (' ' or '\t').
		 */
		char paddingChar = '\t';

		/**
		 * @brief Number of padding characters used per indentation level.
		 */
		uint16_t paddingCharNum = 1;
	};

	/**
	 * @brief Configuration options for output stream behavior.
	 *
	 * Controls byte-level characteristics of the output stream, mostly relevant for text-based formats.
	 */
	struct StreamOptions
	{
		/**
		 * @brief Whether to write a Byte Order Mark (BOM) at the beginning of the output stream.
		 * Applicable only for UTF encodings that support BOM (UTF-8, UTF-16, etc.).
		 */
		bool writeBom = true;

		/**
		 * @brief Specifies the UTF encoding used for the output stream (only applies to text-based formats).
		 */
		Convert::Utf::UtfType encoding = Convert::Utf::UtfType::Utf8;
	};

	/**
	 * @brief Defines the policy for handling numeric overflow during deserialization.
	 *
	 * Used when the value read from the archive exceeds the capacity of the target type.
	 */
	enum class OverflowNumberPolicy
	{
		/**
		 * @brief Skips the out-of-range value silently (can be handled later by `Required()` validator).
		 */
		Skip,

		/**
		 * @brief Throws a `SerializationException` with error code `SerializationErrorCode::Overflow`
		 * if the target type cannot hold the loaded value.
		 */
		ThrowError
	};

	/**
	 * @brief Defines the policy for handling type mismatches during deserialization.
	 *
	 * Triggered when the type of value in the archive does not match the expected type in the target object.
	 */
	enum class MismatchedTypesPolicy
	{
		/**
		 * @brief Silently skips the mismatched value (can be handled later by `Required()` validator).
		 */
		Skip,

		/**
		 * @brief Throws a `SerializationException` with error code `SerializationErrorCode::MismatchedTypes`
		 * when the archive contains a value of an unexpected type.
		 */
		ThrowError
	};

	/**
	 * @brief Serialization options.
	 */
	struct SerializationOptions
	{
		/**
		 * @brief Options controlling text formatting (applies to text-based archives).
		 */
		FormatOptions formatOptions;

		/**
		 * @brief Options affecting output stream properties like encoding and BOM.
		 */
		StreamOptions streamOptions;

		/**
		 * @brief Policy for handling numeric overflows during deserialization.
		 *
		 * For example: attempting to load the number 500 into a `char`.
		 *
		 * @see OverflowNumberPolicy
		 */
		OverflowNumberPolicy overflowNumberPolicy = OverflowNumberPolicy::ThrowError;

		/**
		 * @brief Policy for handling type mismatches during deserialization.
		 *
		 * For example: expecting a number but reading a string from the archive.
		 *
		 * @see MismatchedTypesPolicy
		 */
		MismatchedTypesPolicy mismatchedTypesPolicy = MismatchedTypesPolicy::ThrowError;

		/**
		 * @brief Policy for handling UTF encoding/decoding errors.
		 *
		 * Applied when invalid UTF sequences are encountered during parsing or writing.
		 *
		 * @see Convert::Utf::UtfEncodingErrorPolicy
		 */
		Convert::Utf::UtfEncodingErrorPolicy utfEncodingErrorPolicy = Convert::Utf::UtfEncodingErrorPolicy::ThrowError;

		/**
		 * @brief Maximum number of validation errors to collect before throwing an exception (0 means no limit).
		 */
		uint32_t maxValidationErrors = 0;

		/**
		 * @brief Automatically trims whitespace from ALL string fields during deserialization.
		 */
		bool trimStringFields = false;

		/**
		 * @brief Separator character used between values in flat-file formats like CSV.
		 *
		 * Supported separators: ',', ';', '\t', ' ', '|'
		 */
		char valuesSeparator = ',';
	};
}
