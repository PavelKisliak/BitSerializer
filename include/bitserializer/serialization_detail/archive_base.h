/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <tuple>
#include "bitserializer/serialization_detail/serialization_context.h"
#include "bitserializer/conversion_detail/convert_enum.h"

namespace BitSerializer
{
	/**
	 * @brief Defines the current serialization mode: loading or saving.
	 */
	enum class SerializeMode
	{
		Save,
		Load
	};

	/**
	 * @brief Enumeration of all supported archive types.
	 */
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

	/**
	 * @brief List of supported key types for an archive.
	 */
	template <class ...KeyTypes>
	using TSupportedKeyTypes = std::tuple<KeyTypes...>;

	/**
	 * @brief Base class for implementing low-level archive structures such a key-value map (object) or sequence (array).
	 *
	 * The implementation must have a specific set of serialization methods that depend on the structure of the format.
	 * For example, JSON may have object and array scopes with different allowed operations.
	 */
	template <SerializeMode TMode>
	class TArchiveScope
	{
	public:
		explicit TArchiveScope(SerializationContext& serializationContext) noexcept
			: mSerializationContext(serializationContext)
		{
		}

		TArchiveScope(const TArchiveScope&) = delete;
		TArchiveScope& operator=(const TArchiveScope&) = delete;
		TArchiveScope& operator=(TArchiveScope&&) noexcept = default;

		static constexpr SerializeMode GetMode() noexcept { return TMode; }
		static constexpr bool IsSaving() noexcept { return TMode == SerializeMode::Save; }
		static constexpr bool IsLoading() noexcept { return TMode == SerializeMode::Load; }

		[[nodiscard]] SerializationContext& GetContext() const noexcept { return mSerializationContext; }
		[[nodiscard]] const SerializationOptions& GetOptions() const noexcept { return mSerializationContext.GetOptions(); }

	protected:
		~TArchiveScope() = default;
		TArchiveScope(TArchiveScope&&) noexcept = default;

	private:
		SerializationContext& mSerializationContext;
	};

	/**
	 * @brief Base class for archive implementations.
	 *
	 * @tparam TArchiveTraits Traits class defining archive-specific settings.
	 * @tparam TInputArchive Implementation of input archive.
	 * @tparam TOutputArchive Implementation of output archive.
	 */
	template <typename TArchiveTraits, class TInputArchive, class TOutputArchive>
	class TArchiveBase : public TArchiveTraits
	{
	public:
		using input_archive_type = TInputArchive;
		using output_archive_type = TOutputArchive;
	};

	/**
	 * @brief Wrapper that holds a reference to a base class object.
	 *
	 * Used to simplify the serialization of base objects.
	 *
	 * @tparam TBase Base class type.
	 */
	template<class TBase>
	struct BaseObject
	{
		/**
		 * @brief Constructs the wrapper with a reference to a derived object.
		 *
		 * @tparam TDerived Derived class type.
		 * @param[in] object Reference to the derived object.
		 */
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
		/**
		 * @brief Converts a value from one type to another based on specified policies.
		 *
		 * @tparam TSource Source type.
		 * @tparam TTarget Target type.
		 * @param[in] sourceValue Value to convert.
		 * @param[out] targetValue Converted value will be stored here.
		 * @param[in] mismatchedTypesPolicy Policy for handling mismatched types.
		 * @param[in] overflowNumberPolicy Policy for handling numeric overflows.
		 * @return true if conversion succeeded, false otherwise.
		 * @throws SerializationException Depending on policy.
		 */
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
				throw SerializationException(SerializationErrorCode::ParsingError, "Unknown error while converting value");
			}
			return false;
		}
	}
}
