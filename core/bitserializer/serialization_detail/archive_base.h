/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include "serialization_options.h"

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
/// The enumeration of all used output archive types.
/// </summary>
enum class ArchiveType
{
	Json,
	Xml
};

/// <summary>
/// Class for provide information about supported key types in the archive.
/// </summary>
template <class ...KeyTypes>
using TSupportedKeyTypes = std::tuple<KeyTypes...>;

/// <summary>
/// Base class of scope in archive (lower level of archive).
/// Implementation should have certain set of serialization methods which depending from structure of format.
/// The format (like JSON for example) can have several levels with different allowed serialization operations.
/// </summary>
template <SerializeMode TMode>
class TArchiveScope
{
public:
	TArchiveScope() = default;

	TArchiveScope(const TArchiveScope&) = delete;
	TArchiveScope& operator=(const TArchiveScope&) = delete;

	static constexpr SerializeMode GetMode() noexcept	{ return TMode; }
	static constexpr bool IsSaving() noexcept			{ return TMode == SerializeMode::Save; }
	static constexpr bool IsLoading() noexcept			{ return TMode == SerializeMode::Load; }

protected:
	~TArchiveScope() = default;
	TArchiveScope(TArchiveScope&&) = default;
	TArchiveScope& operator=(TArchiveScope&&) = default;
};

/// <summary>
/// Base class of archive.
/// </summary>
template <typename TArchiveTraits, class TInputArchive, class TOutputArchive>
class TArchiveBase : public TArchiveTraits
{
public:
	using input_archive_type = TInputArchive;
	using output_archive_type = TOutputArchive;
};

}
