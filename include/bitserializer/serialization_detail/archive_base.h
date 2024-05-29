/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include "bitserializer/serialization_detail/serialization_context.h"
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
	Csv,
	MsgPack
};

REGISTER_ENUM(ArchiveType, {
	{ ArchiveType::Json, "Json" },
	{ ArchiveType::Xml, "Xml" },
	{ ArchiveType::Yaml, "Yaml" },
	{ ArchiveType::Csv, "Csv" },
	{ ArchiveType::MsgPack, "MsgPack" }
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
	/// Converts types according to policy.
	/// </summary>
	template <typename TSource, typename TTarget>
	bool ConvertByPolicy(TSource&& sourceValue, TTarget& targetValue, MismatchedTypesPolicy mismatchedTypesPolicy, OverflowNumberPolicy overflowNumberPolicy)
	{
		try
		{
			if constexpr (Convert::IsConvertible<TSource, TTarget>())
			{
				targetValue = Convert::To<TTarget>(std::forward<TSource>(sourceValue));
				return true;
			}
			else
			{
				if (mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"The target field type does not match the value being loaded");
				}
			}
		}
		catch (const std::invalid_argument&)
		{
			if (mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
			{
				if constexpr (std::is_enum_v<TSource>)
				{
					if (Convert::Detail::EnumRegistry<TSource>::IsRegistered())
					{
						throw SerializationException(SerializationErrorCode::UnregisteredEnum,
							"Enum value (" + Convert::ToString(static_cast<std::underlying_type_t<TSource>>(sourceValue)) + ") is invalid or not registered");
					}
					throw SerializationException(SerializationErrorCode::UnregisteredEnum);
				}
				else
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"The target field type does not match the value being loaded");
				}
			}
		}
		catch (const std::out_of_range&)
		{
			if (overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::Overflow,
					"The target field range is insufficient for the value being loaded");
			}
		}
		catch (...) {
			throw SerializationException(SerializationErrorCode::ParsingError, "Unknown error when convert value");
		}
		return false;
	}
}

}
