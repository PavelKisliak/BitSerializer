/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <array>
#include <cstddef>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace BitSerializer::Detail
{
	/**
	 * @brief Traits for serializable types.
	 *
	 * Users specialize this for their types to register them for name-based serialization.
	 *
	 * @par Example:
	 * @code
	 * namespace BitSerializer::Detail {
	 *     template <>
	 *     struct SerializableTypeTraits<MyStruct> {
	 *         static constexpr std::string_view Name = "MyStruct";
	 *         static MyStruct* Create() { return new MyStruct; }
	 *     };
	 * }
	 * @endcode
	 */
	template <class T, class = void>
	struct SerializableTypeTraits
	{
		static constexpr std::string_view Name{};
		static T* Create() = delete;
	};

	/**
	 * @brief Checks whether a type has a registered name via `BITSERIALIZER_REGISTER_TYPE`.
	 *
	 */
	template <class T>
	inline constexpr bool is_type_registered_v = !SerializableTypeTraits<T>::Name.empty();

	/**
	 * @brief Base type registry for a set of types.
	 *
	 * Provides compile-time mapping from type name to a registry entry (name + index).
	 *
	 * @par Example:
	 * @code
	 * using MyRegistry = TypeRegistry<User, Admin, Guest>;
	 * @endcode
	 */
	template <class... Types>
	class TypeRegistry
	{
	public:
		struct Entry
		{
			std::string_view name;
			size_t index = 0;
		};

		static constexpr size_t Size() noexcept
		{
			return mEntries.size();
		}

		// Find entry by name
		static constexpr const Entry* Find(std::string_view name) noexcept
		{
			for (const auto& e : mEntries)
			{
				if (e.name == name) {
					return &e;
				}
			}
			return nullptr;
		}

		// Find entry by index
		static constexpr const Entry* FindByIndex(size_t index) noexcept
		{
			if (index < mEntries.size()) {
				return &mEntries[index];
			}
			return nullptr;
		}

		// Check if type name is registered
		static constexpr bool Contains(std::string_view name) noexcept
		{
			return Find(name) != nullptr;
		}

		// Helper to emplace variant alternative by runtime index
		template <class TVariant>
		static void EmplaceByIndex(TVariant& variant, size_t index)
		{
			EmplaceByIndexImpl(variant, index, std::index_sequence_for<Types...>{});
		}

		// Every type must have a registered name: the active type of name-based variant
		// is resolved by that name at runtime, so an unregistered type could not be serialized.
		static_assert(std::conjunction_v<std::bool_constant<!SerializableTypeTraits<Types>::Name.empty()>...>,
			"BitSerializer. Name-based variant/polymorphic serialization requires every type to be registered with BITSERIALIZER_REGISTER_TYPE(Type, \"Name\")");

	private:
		// Emplace a variant alternative at the given runtime index
		template <class TVariant, size_t... I>
		static void EmplaceByIndexImpl(TVariant& variant, size_t index, std::index_sequence<I...>)
		{
			// Array of function pointers is used to avoid MSVC fold-expression issues
			using EmplaceFunc = void (*)(TVariant&);
			constexpr std::array<EmplaceFunc, sizeof...(Types)> emplaceFuncs = {
				[](TVariant& v) { v.template emplace<std::tuple_element_t<I, std::tuple<Types...>>>(); }...
			};

			if (index < emplaceFuncs.size())
			{
				emplaceFuncs[index](variant);
			}
		}

		template <size_t... I>
		static constexpr std::array<Entry, sizeof...(Types)> MakeEntries(std::index_sequence<I...>)
		{
			return std::array<Entry, sizeof...(Types)>{
				Entry{ SerializableTypeTraits<std::tuple_element_t<I, std::tuple<Types...>>>::Name, I }...
			};
		}

		static constexpr std::array<Entry, sizeof...(Types)> mEntries = MakeEntries(std::index_sequence_for<Types...>{});
	};

	/**
	 * @brief Register a type for name-based serialization.
	 *
	 * Must be used at global (file) namespace scope - it cannot appear inside a function or another namespace.
	 * Safe to use in both headers and .cpp files (no ODR issues, since all members are inline).
	 * The registration must be visible (via #include) in every translation unit that uses name-based
	 * serialization (VariantAsNamed) for the type, and each type must be registered exactly once in the program.
	 *
	 * Usage:
	 * @code
	 * BITSERIALIZER_REGISTER_TYPE(User, "User");
	 * BITSERIALIZER_REGISTER_TYPE(Admin, "Admin");
	 * @endcode
	 */
#define BITSERIALIZER_REGISTER_TYPE(type, name) \
	namespace BitSerializer::Detail { \
		template <> \
		struct SerializableTypeTraits<type> { \
			static constexpr std::string_view Name = name; \
			static type* Create() { return new type; } \
		}; \
	}

} // namespace BitSerializer::Detail
