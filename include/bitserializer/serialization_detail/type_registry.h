/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string_view>
#include <type_traits>
#include <utility>
#include <array>

namespace BitSerializer::Detail
{
	// Forward declaration
	template <class T, class>
	struct SerializableTypeTraits;

	template <class T>
	struct HasCreateFactory
	{
		template <class U>
		static auto Test(int) -> decltype(SerializableTypeTraits<U, void>::Create(), std::true_type{});

		template <class>
		static auto Test(...) -> std::false_type;

		static constexpr bool value = decltype(Test<T>(0))::value;
	};

	template <class T>
	struct HasEmplaceFactory
	{
		template <class U>
		static auto Test(int) -> decltype(SerializableTypeTraits<U, void>::Emplace(nullptr), std::true_type{});

		template <class>
		static auto Test(...) -> std::false_type;

		static constexpr bool value = decltype(Test<T>(0))::value;
	};

	/**
	 * @brief Traits for serializable types.
	 *
	 * Users specialize this for their types to register them for name-based serialization.
	 * No RTTI required - purely compile-time template specialization.
	 *
	 * @par Example:
	 * @code
	 * namespace BitSerializer::Detail {
	 *     template <>
	 *     struct SerializableTypeTraits<MyStruct> {
	 *         static constexpr std::string_view Name = "MyStruct";
	 *         static MyStruct* Create() { return new MyStruct; }
	 *         static void Emplace(void* storage) { new (storage) MyStruct; }
	 *     };
	 * }
	 * @endcode
	 */
	template <class T, class = void>
	struct SerializableTypeTraits
	{
		static constexpr std::string_view Name{};
		static T* Create() = delete;
		static void Emplace(void* storage) = delete;
	};

	/**
	 * @brief Base type registry for a set of types.
	 *
	 * Provides compile-time mapping from type name to factory/emplace functions.
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
			std::string_view Name;
			void* (*Factory)() = nullptr;
			void (*Emplace)(void* storage) = nullptr;
			size_t Index = 0;
		};

		static constexpr size_t Size() noexcept
		{
			return mEntries.size();
		}

		// Find entry by name
		static constexpr const Entry* Find(std::string_view name) noexcept
		{
			for (const auto& e : mEntries) {
				if (e.Name == name) {
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

	private:
		template <class T>
		static void* CreateInstance()
		{
			return SerializableTypeTraits<T>::Create();
		}

		template <class T>
		static void EmplaceInstance(void* storage)
		{
			SerializableTypeTraits<T>::Emplace(storage);
		}

		template <size_t... I>
		static constexpr std::array<Entry, sizeof...(Types)> MakeEntries(std::index_sequence<I...>)
		{
			return std::array<Entry, sizeof...(Types)>{
				Entry{
					SerializableTypeTraits<std::tuple_element_t<I, std::tuple<Types...>>>::Name,
					HasCreateFactory<std::tuple_element_t<I, std::tuple<Types...>>>::value
						? &CreateInstance<std::tuple_element_t<I, std::tuple<Types...>>>
						: nullptr,
					HasEmplaceFactory<std::tuple_element_t<I, std::tuple<Types...>>>::value
						? &EmplaceInstance<std::tuple_element_t<I, std::tuple<Types...>>>
						: nullptr,
					I
				}...
			};
		}

		static constexpr std::array<Entry, sizeof...(Types)> mEntries = MakeEntries(std::index_sequence_for<Types...>{});
	};

	/**
	 * @brief Register a type for name-based serialization.
	 *
	 * Usage:
	 * @code
	 * BITSERIALIZER_REGISTER_TYPE(User, "User");
	 * BITSERIALIZER_REGISTER_TYPE(Admin, "Admin");
	 * @endcode
	 *
	 * For abstract base classes (no factory/emplace):
	 * @code
	 * BITSERIALIZER_REGISTER_BASE_TYPE(Animal);
	 * @endcode
	 */
#define BITSERIALIZER_REGISTER_TYPE(type, name) \
	namespace BitSerializer::Detail { \
		template <> \
		struct SerializableTypeTraits<type> { \
			static constexpr std::string_view Name = name; \
			static type* Create() { return new type; } \
			static void Emplace(void* storage) { new (storage) type; } \
		}; \
	}

#define BITSERIALIZER_REGISTER_BASE_TYPE(type) \
	namespace BitSerializer::Detail { \
		template <> \
		struct SerializableTypeTraits<type> { \
			static constexpr std::string_view Name{}; \
			static type* Create() = delete; \
			static void Emplace(void* storage) = delete; \
		}; \
	}

} // namespace BitSerializer::Detail
