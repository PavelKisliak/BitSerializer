/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
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
		Convert::UtfType encoding = Convert::UtfType::Utf8;
	};

	/// <summary>
	/// Policy for case when size of target type is not enough for loading number.
	/// For example, when loading number is 500 but target type is char.
	/// </summary>
	enum class OverflowNumberPolicy
	{
		// Value will be skipped, but can be handled by Required() validator.
		Skip,
		// Will be thrown exception `SerializationErrorCode::Overflow` when size of target type is not enough for loading value.
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
		OverflowNumberPolicy overflowNumberPolicy = OverflowNumberPolicy::ThrowError;
	};
}
