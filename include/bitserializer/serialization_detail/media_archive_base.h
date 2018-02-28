/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

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
	inline TInputArchive Load(typename TArchiveTraits::output_format& outputFormat)
	{
		return TInputArchive(outputFormat);
	}

	inline TInputArchive Load(typename TArchiveTraits::input_stream& inputStream)
	{
		return TInputArchive(inputStream);
	}

	inline TOutputArchive Save(typename TArchiveTraits::output_format& outputFormat)
	{
		return TOutputArchive(outputFormat);
	}

	inline TOutputArchive Save(typename TArchiveTraits::output_stream& outputStream)
	{
		return TOutputArchive(outputStream);
	}
};

}	// namespace BitSerializer
