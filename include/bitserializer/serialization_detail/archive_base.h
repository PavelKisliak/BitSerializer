/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include <limits>
#include "serialization_context.h"
#include "bitserializer/conversion_detail/convert_enum.h"

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
	Xml,
	Yaml,
	Csv
};

REGISTER_ENUM(ArchiveType, {
	{ ArchiveType::Json, "Json" },
	{ ArchiveType::Xml, "Xml" },
	{ ArchiveType::Yaml, "Yaml" },
	{ ArchiveType::Csv, "Csv" }
})

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
	explicit TArchiveScope(SerializationContext& serializationContext) noexcept
		: mSerializationContext(serializationContext)
	{ }

	TArchiveScope(const TArchiveScope&) = delete;
	TArchiveScope& operator=(const TArchiveScope&) = delete;

	static constexpr SerializeMode GetMode() noexcept	{ return TMode; }
	static constexpr bool IsSaving() noexcept			{ return TMode == SerializeMode::Save; }
	static constexpr bool IsLoading() noexcept			{ return TMode == SerializeMode::Load; }

	[[nodiscard]] SerializationContext& GetContext() const noexcept			{ return mSerializationContext; }
	[[nodiscard]] const SerializationOptions& GetOptions() const noexcept	{ return mSerializationContext.GetOptions(); }

protected:
	~TArchiveScope() = default;
	TArchiveScope(TArchiveScope&&) = default;
	TArchiveScope& operator=(TArchiveScope&&) = default;

	SerializationContext& mSerializationContext;
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


/// <summary>
/// The wrapper class that keeps a reference to a base user object, used to simplify the serialization of base objects.
/// </summary>
template<class TBase>
struct BaseObject
{
	template <typename TDerived>
	explicit BaseObject(TDerived& object) noexcept
		: Object(object)
	{
		static_assert(std::is_base_of_v<TBase, TDerived>,
			"BitSerializer. The template parameter 'TBase' should be a base type of passed object.");
	}

	TBase& Object;
};


namespace Detail
{
	/// <summary>
	/// Casts numbers according to policy.
	/// </summary>
	template <typename TSource, typename TTarget, std::enable_if_t<std::is_arithmetic_v<TSource> && std::is_arithmetic_v<TTarget>, int> = 0>
	bool SafeNumberCast(TSource sourceValue, TTarget& targetValue, OverflowNumberPolicy overflowNumberPolicy)
	{
		bool result = true;
		if constexpr (std::is_same_v<TSource, TTarget>)
		{
			targetValue = sourceValue;
		}
		else if constexpr (std::is_floating_point_v<TSource>)
		{
			if constexpr (std::is_floating_point_v<TTarget>)
			{
				if (result = sizeof(TTarget) > sizeof(TSource)
					|| (sourceValue >= std::numeric_limits<TTarget>::lowest() && sourceValue <= std::numeric_limits<TTarget>::max()); result)
				{
					targetValue = static_cast<TTarget>(sourceValue);
				}
			}
			else {
				// The number with floating point cannot be converted to an integer without lost precision
				result = false;
			}
		}
		else if constexpr (std::is_same_v<bool, TSource> || std::is_same_v<bool, TTarget>)
		{
			auto value = static_cast<TTarget>(sourceValue);
			if (result = static_cast<TSource>(value) == sourceValue; result) {
				targetValue = value;
			}
		}
		else
		{
			auto value = static_cast<TTarget>(sourceValue);
			result = (static_cast<TSource>(value) == sourceValue) && !((value > 0 && sourceValue < 0) || (value < 0 && sourceValue > 0));
			if (result) { 
				targetValue = value;
			}
		}

		if (!result)
		{
			if (overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::Overflow,
					std::string("The size of target field is not sufficient to deserialize number " + Convert::ToString(sourceValue)));
			}
		}
		return result;
	}
}

}
