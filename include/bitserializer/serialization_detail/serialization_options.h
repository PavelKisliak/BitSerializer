/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>
#include "bitserializer/conversion_detail/convert_utf.h"

namespace BitSerializer
{
	/// <summary>
	/// Contains a set of options which give a control over formatting output text (for text based archives).
	/// Some options cannot be applicable to all types of archive, in that case it will be ignored.
	/// </summary>
	struct FormatOptions
	{
		/// <summary>
		/// Determines that output text archive should be formatted.
		/// </summary>
		bool enableFormat = false;

		/// <summary>
		/// Character for padding, must be whitespace ' ' or '\t'.
		/// </summary>
		char paddingChar = '\t';

		/// <summary>
		/// The number of characters for padding each level.
		/// </summary>
		uint16_t paddingCharNum = 1;
	};

	/// <summary>
	/// Contains a set of options for output stream.
	/// Some options cannot be applicable to all types of archive, in that case it will be ignored.
	/// </summary>
	struct StreamOptions
	{
		/// <summary>
		/// Determines that BOM (Byte Order Mark) should be written to output stream (applicable for text based formats).
		/// </summary>
		bool writeBom = true;

		/// <summary>
		/// The encoding for output stream (applicable for formats which based on UTF encoded text).
		/// </summary>
		Convert::Utf::UtfType encoding = Convert::Utf::UtfType::Utf8;
	};

	/// <summary>
	/// Policy for case when size of target type is not enough for loading value.
	/// </summary>
	enum class OverflowNumberPolicy
	{
		/// <summary>
		/// Value will be skipped, but can be handled by Required() validator.
		/// </summary>
		Skip,
		/// <summary>
		/// Will be thrown SerializationException with error code `SerializationErrorCode::Overflow` when size of target type is not enough for loading value.
		/// </summary>
		ThrowError
	};

	/// <summary>
	/// Policy for case when a type from the archive (source format, like JSON) does not match to the target value.
	/// </summary>
	enum class MismatchedTypesPolicy
	{
		/// <summary>
		/// Value will be skipped, but can be handled by Required() validator.
		/// </summary>
		Skip,
		/// <summary>
		/// Will be thrown SerializationException with error code `SerializationErrorCode::MismatchedTypes`.
		/// </summary>
		ThrowError
	};

	/// <summary>
	/// Contains a set of serialization options.
	/// Some options cannot be applicable to all types of archive, in that case it will be ignored.
	/// </summary>
	struct SerializationOptions
	{
		/// <summary>
		/// Contains a set of options which give a control over formatting output text (for text based archives).
		/// </summary>
		FormatOptions formatOptions;

		/// <summary>
		/// Contains a set of options for output stream.
		/// </summary>
		StreamOptions streamOptions;

		/// <summary>
		/// Policy for case when size of target type is not enough for loading number.
		/// For example, when loading number is 500 but target type is char.
		/// </summary>
		/// <seealso cref="OverflowNumberPolicy" />
		OverflowNumberPolicy overflowNumberPolicy = OverflowNumberPolicy::ThrowError;

		/// <summary>
		/// Policy for case when type of target field does not match the value being loaded.
		/// For example, when loading string, but target type is number.
		/// </summary>
		/// <seealso cref="MismatchedTypesPolicy" />
		MismatchedTypesPolicy mismatchedTypesPolicy = MismatchedTypesPolicy::ThrowError;

		/// <summary>
		/// Policy for handle UTF encoding/decoding errors (for example, when UTF sequence is invalid).
		/// </summary>
		/// <seealso cref="Convert::Utf::UtfEncodingErrorPolicy" />
		Convert::Utf::UtfEncodingErrorPolicy utfEncodingErrorPolicy = Convert::Utf::UtfEncodingErrorPolicy::ThrowError;

		/// <summary>
		/// The maximum number of validation errors that will be collected before an exception is thrown ().
		///	The default value is "0", which means unlimited quantity. Number of errors for each particular field is unlimited in any case.
		/// </summary>
		uint32_t maxValidationErrors = 0;

		/// <summary>
		/// Values separator, currently used only for CSV format (allowed: ',', ';', '\t', ' ', '|').
		/// </summary>
		char valuesSeparator = ',';
	};
}
