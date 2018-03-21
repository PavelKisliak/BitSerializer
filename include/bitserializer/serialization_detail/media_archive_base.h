/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>

namespace BitSerializer {

/// <summary>
/// Serialization mode
/// </summary>
enum class SerializeMode
{
	Save,
	Load
};

/// <summary>
/// Base class of scope in archive (lower level of media archive).
/// Implementation should have certain set of serialization methods which depending from structure of format.
/// The format (like JSON for example) can have several levels with different allowed serialization operations.
/// </summary>
template <SerializeMode TMode>
class ArchiveScope
{
public:
	static constexpr SerializeMode GetMode() noexcept	{ return TMode; }
	static constexpr bool IsSaving() noexcept			{ return TMode == SerializeMode::Save; }
	static constexpr bool IsLoading() noexcept			{ return TMode == SerializeMode::Load; }
};

/// <summary>
/// Base class of media-archive (wrapper over archive's root scope).
/// Scopes can be implemented as separate for load and save operations or all in one.
/// </summary>
template <typename TArchiveTraits, class TInputArchive, class TOutputArchive>
class MediaArchiveBase : public TArchiveTraits
{
public:
	inline TInputArchive Load(const typename TArchiveTraits::output_format& outputFormat)
	{
		return TInputArchive(outputFormat);
	}

	inline TOutputArchive Save(typename TArchiveTraits::output_format& outputFormat)
	{
		return TOutputArchive(outputFormat);
	}

	template <typename TChar>
	inline TInputArchive Load(std::basic_istream<TChar, std::char_traits<TChar>>& inputStream)
	{
		if constexpr (std::is_constructible_v<TInputArchive, std::basic_ostream<TChar, std::char_traits<TChar>>&>)
			return TInputArchive(inputStream);
		else
		{
			if constexpr (std::is_same_v<TChar, char>)
				static_assert(false, "BitSerializer. The archive doesn't support loading from stream based on ANSI char element.");
			else
				static_assert(false, "BitSerializer. The archive doesn't support loading from stream based on wide string element.");
		}
	}

	template <typename TChar>
	inline TOutputArchive Save(std::basic_ostream<TChar, std::char_traits<TChar>>& outputStream)
	{
		if constexpr (std::is_constructible_v<TOutputArchive, std::basic_ostream<TChar, std::char_traits<TChar>>&>)
			return TOutputArchive(outputStream);
		else
		{
			if constexpr (std::is_same_v<TChar, char>)
				static_assert(false, "BitSerializer. The archive doesn't support save to stream based on ANSI char element.");
			else
				static_assert(false, "BitSerializer. The archive doesn't support save to stream based on wide string element.");
		}
	}
};

}	// namespace BitSerializer
