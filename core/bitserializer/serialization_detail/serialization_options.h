/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>

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
		bool EnableFormat = false;

		/// <summary>
		/// Character for padding, must be whitespace ' ' or '\t'.
		/// </summary>
		char PaddingChar = ' ';

		/// <summary>
		/// The number of characters for padding each level.
		/// </summary>
		uint16_t PaddingCharNum = 4;
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
		bool WriteBom = true;
	};

	/// <summary>
	///  Contains a set of serialization options.
	/// Some options cannot be applicable to all types of archive, in that case it will be ignored.
	/// </summary>
	struct SerializationOptions
	{
		/// <summary>
		/// Contains a set of options which give a control over formatting output text (for text based archives).
		/// </summary>
		FormatOptions FormatOptions;

		/// <summary>
		/// Contains a set of options for output stream.
		/// </summary>
		StreamOptions StreamOptions;
	};

}	// namespace BitSerializer