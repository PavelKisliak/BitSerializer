/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/object_traits.h"

namespace BitSerializer
{
	/**
	 * @brief Determines if a given type derives from `TArchiveScope`.
	 */
	template <typename T>
	struct is_archive_scope
	{
		constexpr static bool value = std::is_base_of_v<TArchiveScope<SerializeMode::Load>, T> ||
			std::is_base_of_v<TArchiveScope<SerializeMode::Save>, T>;
	};

	template <typename T>
	constexpr bool is_archive_scope_v = is_archive_scope<T>::value;

	/**
	 * @brief Determines if an archive supports a specific input data type.
	 *
	 * @tparam TArchive The archive type being tested.
	 * @tparam TInput   The input data type (e.g., string, stream).
	 */
	template <typename TArchive, typename TInput>
	struct is_archive_support_input_data_type
	{
		constexpr static bool value = is_input_stream_v<TInput>
			? std::is_constructible_v<TArchive, TInput&, SerializationContext&>
			: std::is_constructible_v<TArchive, const TInput&, SerializationContext&>;
	};

	template <typename TArchive, typename TInput>
	constexpr bool is_archive_support_input_data_type_v = is_archive_support_input_data_type<TArchive, TInput>::value;

	/**
	 * @brief Determines if an archive supports a specific output data type.
	 *
	 * @tparam TArchive The archive type being tested.
	 * @tparam TOutput  The output data type (e.g., string, stream).
	 */
	template <typename TArchive, typename TOutput>
	struct is_archive_support_output_data_type
	{
		constexpr static bool value = std::is_constructible_v<TArchive, TOutput&, SerializationContext&>;
	};

	template <typename TArchive, typename TOutput>
	constexpr bool is_archive_support_output_data_type_v = is_archive_support_output_data_type<TArchive, TOutput>::value;

	/**
	 * @brief Determines if a value (number or string) can be serialized using the specified archive.
	 *
	 * Detects whether the archive has a method matching:
	 * `bool SerializeValue(TValue&)`.
	 *
	 * @tparam TArchive The archive type.
	 * @tparam TValue   The value type to serialize.
	 */
	template <typename TArchive, typename TValue>
	struct can_serialize_value
	{
	private:
		template <typename TObj, typename TVal>
		static std::enable_if_t<std::is_same_v<bool, decltype(std::declval<TObj>().SerializeValue(std::declval<TVal&>()))>, std::true_type> test(int);

		template <typename, typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TArchive, TValue>(0)) type;
		enum { value = type::value };
	};

	template <typename TArchive, typename TValue>
	constexpr bool can_serialize_value_v = can_serialize_value<TArchive, TValue>::value;

	/**
	 * @brief Determines if a value (number or string) can be serialized with a key using the specified archive.
	 *
	 * Detects whether the archive has a method matching:
	 * `bool SerializeValue(const TKey&, TValue&)`.
	 *
	 * @tparam TArchive The archive type.
	 * @tparam TValue   The value type to serialize.
	 * @tparam TKey     The key type.
	 */
	template <typename TArchive, typename TValue, typename TKey>
	struct can_serialize_value_with_key
	{
	private:
		template <typename TObj, typename TVal>
		static std::enable_if_t<std::is_same_v<bool, decltype(std::declval<TObj>().SerializeValue(std::declval<TKey>(), std::declval<TVal&>()))>, std::true_type> test(int);

		template <typename, typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TArchive, TValue>(0)) type;
		enum { value = type::value };
	};

	template <typename TArchive, typename TValue, typename TKey>
	constexpr bool can_serialize_value_with_key_v = can_serialize_value_with_key<TArchive, TValue, TKey>::value;

	/**
	 * @brief Determines if class objects can be serialized using the specified archive.
	 *
	 * Detects whether the archive provides a method matching:
	 * `TArchiveScope OpenObjectScope(size_t)` required to start serializing an object.
	 *
	 * @tparam TArchive The archive type.
	 */
	template <typename TArchive>
	struct can_serialize_object
	{
	private:
		template <typename TObj>
		static std::enable_if_t<std::is_class_v<decltype(std::declval<TObj>().OpenObjectScope(std::declval<size_t>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TArchive>(0)) type;
		enum { value = type::value };
	};

	template <typename TArchive>
	constexpr bool can_serialize_object_v = can_serialize_object<TArchive>::value;

	/**
	 * @brief Determines if class objects can be serialized with a key using the specified archive.
	 *
	 * Detects whether the archive provides a method matching:
	 * `TArchiveScope OpenObjectScope(const TKey&, size_t)` required to start serializing an object with a key.
	 *
	 * @tparam TArchive The archive type.
	 * @tparam TKey     The key type.
	 */
	template <typename TArchive, typename TKey>
	struct can_serialize_object_with_key
	{
	private:
		template <typename TObj>
		static std::enable_if_t<std::is_class_v<decltype(std::declval<TObj>().OpenObjectScope(std::declval<TKey>(), std::declval<size_t>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TArchive>(0)) type;
		enum { value = type::value };
	};

	template <typename TArchive, typename TKey>
	constexpr bool can_serialize_object_with_key_v = can_serialize_object_with_key<TArchive, TKey>::value;

	template <typename TArchive, typename TKey>
	constexpr bool is_object_scope_v = can_serialize_value_with_key<TArchive, int, TKey>::value;

	/**
	 * @brief Determines if arrays can be serialized using the specified archive.
	 *
	 * Detects whether the archive provides a method matching:
	 * `TArchiveScope OpenArrayScope(size_t)` required to start serializing an array.
	 *
	 * @tparam TArchive The archive type.
	 */
	template <typename TArchive>
	struct can_serialize_array
	{
	private:
		template <typename TObj>
		static std::enable_if_t<std::is_class_v<decltype(std::declval<TObj>().OpenArrayScope(std::declval<size_t>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TArchive>(0)) type;
		enum { value = type::value };
	};

	template <typename TArchive>
	constexpr bool can_serialize_array_v = can_serialize_array<TArchive>::value;

	/**
	 * @brief Determines if arrays can be serialized with a key using the specified archive.
	 *
	 * Detects whether the archive provides a method matching:
	 * `TArchiveScope OpenArrayScope(const TKey&, size_t)` required to start serializing an array with a key.
	 *
	 * @tparam TArchive The archive type.
	 * @tparam TKey     The key type.
	 */
	template <typename TArchive, typename TKey>
	struct can_serialize_array_with_key
	{
	private:
		template <typename TObj>
		static std::enable_if_t<std::is_class_v<decltype(std::declval<TObj>().OpenArrayScope(std::declval<TKey>(), std::declval<size_t>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TArchive>(0)) type;
		enum { value = type::value };
	};

	template <typename TArchive, typename TKey>
	constexpr bool can_serialize_array_with_key_v = can_serialize_array_with_key<TArchive, TKey>::value;

	/**
	 * @brief Determines if binary arrays can be serialized using the specified archive.
	 *
	 * Detects whether the archive provides a method matching:
	 * `TArchiveScope OpenBinaryScope(size_t)` required to start serializing binary data.
	 *
	 * @tparam TArchive The archive type.
	 */
	template <typename TArchive>
	struct can_serialize_binary
	{
	private:
		template <typename TObj>
		static std::enable_if_t<std::is_class_v<decltype(std::declval<TObj>().OpenBinaryScope(std::declval<size_t>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TArchive>(0)) type;
		enum { value = type::value };
	};

	template <typename TArchive>
	constexpr bool can_serialize_binary_v = can_serialize_binary<TArchive>::value;

	/**
	 * @brief Determines if binary arrays can be serialized with a key using the specified archive.
	 *
	 * Detects whether the archive provides a method matching:
	 * `TArchiveScope OpenBinaryScope(const TKey&, size_t)` required to start serializing binary data with a key.
	 *
	 * @tparam TArchive The archive type.
	 * @tparam TKey     The key type.
	 */
	template <typename TArchive, typename TKey>
	struct can_serialize_binary_with_key
	{
	private:
		template <typename TObj>
		static std::enable_if_t<std::is_class_v<decltype(std::declval<TObj>().OpenBinaryScope(std::declval<TKey>(), std::declval<size_t>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TArchive>(0)) type;
		enum { value = type::value };
	};

	template <typename TArchive, typename TKey>
	constexpr bool can_serialize_binary_with_key_v = can_serialize_binary_with_key<TArchive, TKey>::value;

	/**
	 * @brief Determines if the archive supports attribute serialization.
	 *
	 * Detects whether the archive provides a method matching:
	 * `TArchiveScope OpenAttributeScope()` required to start serializing attributes.
	 *
	 * @tparam TArchive The archive type.
	 */
	template <typename TArchive>
	struct can_serialize_attribute
	{
	private:
		template <typename TObj>
		static std::enable_if_t<std::is_class_v<decltype(std::declval<TObj>().OpenAttributeScope())>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<TArchive>(0)) type;
		enum { value = type::value };
	};

	template <typename TArchive>
	constexpr bool can_serialize_attribute_v = can_serialize_attribute<TArchive>::value;

	/**
	 * @brief Determines if a type is convertible to any element within a tuple.
	 *
	 * Useful when validating against multiple allowed types.
	 *
	 * @tparam T      The source type to test for convertibility.
	 * @tparam TTuple A tuple of target types to compare against.
	 */
	template <typename T, typename TTuple>
	struct is_convertible_to_one_from_tuple
	{
	private:
		template <class TElem>
		static constexpr bool testImpl()
		{
			if constexpr (std::is_same_v<std::decay_t<T>, bool> || std::is_null_pointer_v<T> || std::is_floating_point_v<T>) {
				return std::is_same_v<T, TElem>;
			}
			else {
				return std::is_convertible_v<T, TElem>;
			}
		}

		template <class TestType, size_t... Is >
		static constexpr bool test(std::index_sequence<Is...>) {
			return (testImpl<std::tuple_element_t<Is, TestType>>() || ...);
		}

	public:
		constexpr static bool value = test<TTuple>(std::make_index_sequence<std::tuple_size_v<TTuple>>{});
	};

	template <typename T, typename TTuple>
	constexpr bool is_convertible_to_one_from_tuple_v = is_convertible_to_one_from_tuple<T, TTuple>::value;

}	// namespace BitSerializer
