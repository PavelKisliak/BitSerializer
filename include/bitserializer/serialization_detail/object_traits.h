/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>
#include <string>
#include <optional>
#include "bitserializer/serialization_detail/archive_base.h"

namespace BitSerializer {

	/**
	 * @brief Checks whether a class has an internal `Serialize()` method.
	 */
	template <typename T>
	struct has_serialize_method
	{
	private:
		template <typename U>
		static decltype(std::declval<U>().Serialize(std::declval<TArchiveScope<SerializeMode::Load>&>()), std::true_type()) test(int);

		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<T>(0));
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool has_serialize_method_v = has_serialize_method<T>::value;

	/**
	 * @brief Checks whether a globally defined `SerializeObject()` function exists for a type.
	 */
	template <typename T>
	struct has_global_serialize_object
	{
	private:
		template <typename TObject>
		static std::enable_if_t<std::is_same_v<decltype(SerializeObject(std::declval<TArchiveScope<SerializeMode::Load>&>(), std::declval<TObject&>())), void>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<T>(0));
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool has_global_serialize_object_v = has_global_serialize_object<T>::value;

	/**
	 * @brief Checks whether a globally defined `SerializeArray()` function exists for a type.
	 */
	template <typename T>
	struct has_global_serialize_array
	{
	private:
		template <typename TObject>
		static std::enable_if_t<std::is_same_v<decltype(SerializeArray(std::declval<TArchiveScope<SerializeMode::Load>&>(), std::declval<TObject&>())), void>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<T>(0));
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool has_global_serialize_array_v = has_global_serialize_array<T>::value;

	/**
	 * @brief Checks whether a container has a `size()` method.
	 */
	template <typename T>
	struct has_size
	{
	private:
		template <typename U>
		static decltype(std::is_integral_v<decltype(std::declval<U>().size())>, std::true_type()) test(int);

		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<T>(0));
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool has_size_v = has_size<T>::value;

	/**
	 * @brief Returns the number of elements in a `std::tuple`.
	 */
	template<typename... TArgs>
	constexpr size_t size(const std::tuple<TArgs...>&) noexcept
	{
		return std::tuple_size_v<std::tuple<TArgs...>>;
	}

	/**
	 * @brief Checks whether a global `size(const T&)` function is defined for a given type.
	 */
	template <typename T>
	struct has_global_size
	{
	private:
		template <typename U>
		static std::enable_if_t<std::is_integral_v<decltype(size(std::declval<const U&>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<T>(0));
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool has_global_size_v = has_global_size<T>::value;

	/**
	 * @brief Checks whether a container has a `reserve()` method.
	 */
	template <typename T>
	struct has_reserve
	{
	private:
		template <typename U>
		static decltype(std::declval<U>().reserve(std::declval<size_t>()), std::true_type()) test(int);

		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<T>(0));
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool has_reserve_v = has_reserve<T>::value;

	/**
	 * @brief Checks whether a type supports enumeration via `begin()` and `end()` methods.
	 */
	template <typename TContainer>
	struct is_enumerable
	{
	private:
		template <typename T>
		static std::enable_if_t<
			std::is_convertible_v<decltype(std::declval<T>().begin().operator*()), typename T::value_type>
			&& std::is_convertible_v<decltype(std::declval<T>().end().operator*()), typename T::value_type>, std::true_type> test(int);

	template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<TContainer>(0));
		enum { value = type::value };
	};

	template <typename TContainer>
	constexpr bool is_enumerable_v = is_enumerable<TContainer>::value;

	/**
	 * @brief Checks whether a type is an enumerable container of a specific element type.
	 */
	template <typename TContainer, typename TValue>
	struct is_enumerable_of
	{
	private:
		template <typename T, typename U>
		static std::enable_if_t<is_enumerable_v<T> && std::is_same_v<typename T::value_type, U>, std::true_type> test(int);

		template <typename, typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<TContainer, TValue>(0));
		enum { value = type::value };
	};

	template <typename TContainer, typename TValue>
	constexpr bool is_enumerable_of_v = is_enumerable_of<TContainer, TValue>::value;

	/**
	 * @brief Checks whether a container holds single-byte integer types.
	 */
	template <typename TContainer>
	constexpr auto is_binary_container =
		is_enumerable_of_v<TContainer, char> ||
		is_enumerable_of_v<TContainer, signed char> ||
		is_enumerable_of_v<TContainer, unsigned char>;

	/**
	 * @brief Gets the size of a container using available methods.
	 */
	template <typename TContainer>
	constexpr size_t GetContainerSize(const TContainer& container)
	{
		if constexpr (has_global_size_v<TContainer>) {
			return size(container);
		}
		else if constexpr (has_size_v<TContainer>) {
			return container.size();
		}
		else {
			return std::distance(std::begin(container), std::end(container));
		}
	}

	/**
	 * @brief Visitor used to count the number of fields in a serializable object.
	 */
	template <typename TArchive>
	class FieldsCountVisitor
	{
	public:
		explicit FieldsCountVisitor(const TArchive& archive) noexcept : Archive(archive) {}

		template <typename TValue>
		size_t Count(TValue& obj)
		{
			constexpr auto hasSerializeMethod = has_serialize_method_v<TValue>;
			constexpr auto hasGlobalSerializeObject = has_global_serialize_object_v<TValue>;
			static_assert(hasSerializeMethod || hasGlobalSerializeObject, "BitSerializer. Cannot count number of object fields");

			// Internal Serialize() method has higher priority than global one
			if constexpr (hasSerializeMethod) {
				obj.Serialize(*this);
			}
			else if constexpr (hasGlobalSerializeObject) {
				SerializeObject(*this, obj);
			}
			return Size;
		}

		static constexpr ArchiveType archive_type = TArchive::archive_type;
		using key_type = typename TArchive::key_type;
		static constexpr bool is_binary = TArchive::is_binary;

		static constexpr auto GetMode() noexcept { return TArchive::GetMode(); }
		static constexpr auto IsSaving() noexcept { return TArchive::IsSaving(); }
		static constexpr auto IsLoading() noexcept { return TArchive::IsLoading(); }

		[[nodiscard]] SerializationContext& GetContext() const noexcept { return Archive.GetContext(); }
		[[nodiscard]] const SerializationOptions& GetOptions() const noexcept { return Archive.GetOptions(); }

		template <class TValue>
		FieldsCountVisitor& operator<<(TValue&&) noexcept
		{
			++Size;
			return *this;
		}

		template <class TBase>
		FieldsCountVisitor& operator<<(BaseObject<TBase>&& value) noexcept
		{
			Count(value.Object);
			return *this;
		}

	private:
		size_t Size = 0;
		const TArchive& Archive;
	};

	/**
	 * @brief Counts the number of fields in a serializable object or the size of a map-like structure.
	 *
	 * @param archive Reference to the archive instance.
	 * @param obj Reference to the object being counted.
	 * @return Number of fields, or zero if loading or text-based archive.
	 */
	template <typename TArchive, typename TValue>
	size_t CountMapObjectFields(const TArchive& archive, TValue& obj)
	{
		if constexpr (TArchive::IsLoading() || !TArchive::is_binary)
		{
			// Text formats do not require field counting
			return 0;
		}
		else
		{
			constexpr auto isEnumerable = is_enumerable_v<TValue>;
			constexpr auto hasSerializeMethod = has_serialize_method_v<TValue>;
			constexpr auto hasGlobalSerializeObject = has_global_serialize_object_v<TValue>;

			static_assert(isEnumerable || hasSerializeMethod || hasGlobalSerializeObject,
				"BitSerializer. Saving to a binary archive requires a known number of object fields.");

			if constexpr (isEnumerable) {
				return GetContainerSize(obj);
			}
			else {
				return FieldsCountVisitor<TArchive>(archive).Count(obj);
			}
		}
	}

	/**
	 * @brief Checks whether a type is an input stream.
	 */
	template <typename T>
	struct is_input_stream
	{
	private:
		template <typename TStream>
		static std::enable_if_t<std::is_base_of_v<std::basic_istream<typename TStream::char_type, std::char_traits<typename TStream::char_type>>, TStream>, std::true_type> test(int);
		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<T>(0));
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool is_input_stream_v = is_input_stream<T>::value;

	/**
	 * @brief Checks whether a type is an output stream.
	 */
	template <typename T>
	struct is_output_stream
	{
	private:
		template <typename TStream>
		static std::enable_if_t<std::is_base_of_v<std::basic_ostream<typename TStream::char_type, std::char_traits<typename TStream::char_type>>, TStream>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<T>(0));
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool is_output_stream_v = is_output_stream<T>::value;

	/**
	 * @brief Checks whether a type implements a validator for the specified value type.
	 */
	template <typename TValidator, typename TValue>
	struct is_validator
	{
	private:
		template <typename T>
		static std::enable_if_t<std::is_same_v<std::optional<std::string>,
			decltype(std::declval<const T>()(std::declval<const TValue&>(), std::declval<bool>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<TValidator>(0));
		enum { value = type::value };
	};

	template <typename TValidator, typename TValue>
	constexpr bool is_validator_v = is_validator<TValidator, TValue>::value;

	/**
	 * @brief Checks whether a type implements a refiner for the specified value type.
	 */
	template <typename TRefiner, typename TValue>
	struct is_refiner
	{
	private:
		template <typename T>
		static std::enable_if_t<std::is_same_v<void, decltype(std::declval<const T>()(std::declval<TValue&>(), std::declval<bool>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		using type = decltype(test<TRefiner>(0));
		enum { value = type::value };
	};

	template <typename TRefiner, typename TValue>
	constexpr bool is_refiner_v = is_refiner<TRefiner, TValue>::value;

	/**
	 * @brief Maps an arithmetic type to its fixed-size counterpart (e.g., int -> int32_t).
	 */
	template <typename T, typename = void>
	struct compatible_fixed;

	// Specialization for integral types (excluding bool)
	template <typename T>
	struct compatible_fixed<T, std::enable_if_t<std::is_integral_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
	{
		using source_type = std::decay_t<T>;
		using type = std::conditional_t<
			std::is_signed_v<source_type>,
			// Signed types
			std::conditional_t<sizeof(source_type) == 1, int8_t,
				std::conditional_t<sizeof(source_type) == 2, int16_t,
					std::conditional_t<sizeof(source_type) == 4, int32_t,
						std::conditional_t<sizeof(source_type) == 8, int64_t,
							void // Unsupported size
						>
					>
				>
			>,
			// Unsigned types
			std::conditional_t<sizeof(source_type) == 1, uint8_t,
				std::conditional_t<sizeof(source_type) == 2, uint16_t,
					std::conditional_t<sizeof(source_type) == 4, uint32_t,
						std::conditional_t<sizeof(source_type) == 8, uint64_t,
							void // Unsupported size
						>
					>
				>
			>
		>;
	};

	template <typename T>
	using compatible_fixed_t = typename compatible_fixed<T>::type;

} // namespace BitSerializer
