/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <type_traits>
#include <string>
#include <optional>
#include "archive_base.h"

namespace BitSerializer {

/// <summary>
/// Checks that the class is serializable - has internal Serialize() method.
/// </summary>
template <typename T>
struct has_serialize_method
{
private:
	template <typename U>
	static decltype(std::declval<U>().Serialize(std::declval<TArchiveScope<SerializeMode::Load>&()>), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_serialize_method_v = has_serialize_method<T>::value;


/// <summary>
/// Checks that the class is serializable - has globally defined SerializeObject() function.
/// </summary>
template <typename T>
struct has_global_serialize_object
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_same_v<decltype(SerializeObject(std::declval<TArchiveScope<SerializeMode::Load>&>(), std::declval<TObj&>())), void>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_global_serialize_object_v = has_global_serialize_object<T>::value;


/// <summary>
/// Checks that the class is serializable - has globally defined SerializeArray() function.
/// </summary>
template <typename T>
struct has_global_serialize_array
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_same_v<decltype(SerializeArray(std::declval<TArchiveScope<SerializeMode::Load>&>(), std::declval<TObj&>())), void>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_global_serialize_array_v = has_global_serialize_array<T>::value;


/// <summary>
/// Checks that the container has size() method.
/// </summary>
template <typename T>
struct has_size
{
private:
	template <typename U>
	static decltype(std::declval<U>().size(), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_size_v = has_size<T>::value;


/// <summary>
/// Checks that the global function `size(const T&)` is defined for the passed object type.
/// </summary>
template <typename T>
struct has_global_size
{
private:
	template <typename U>
	static std::enable_if_t<std::is_same_v<decltype(size(std::declval<const U&>())), size_t>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_global_size_v = has_global_size<T>::value;


/// <summary>
/// Checks that the container has reserve() method.
/// </summary>
template <typename T>
struct has_reserve
{
private:
	template <typename U>
	static decltype(std::declval<U>().reserve(std::declval<size_t>()), std::true_type()) test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool has_reserve_v = has_reserve<T>::value;


/// <summary>
/// Checks that the passed object type can be enumerated (by checking existence of begin() and end() methods).
/// </summary>
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
	typedef decltype(test<TContainer>(0)) type;
	enum { value = type::value };
};

template <typename TContainer>
constexpr bool is_enumerable_v = is_enumerable<TContainer>::value;


/// <summary>
/// Gets the size of the container.
/// </summary>
template <typename TContainer>
constexpr size_t GetContainerSize(const TContainer& cont)
{
	if constexpr (has_size_v<TContainer>) {
		return size(cont);
	}
	else {
		return std::distance(std::begin(cont), std::end(cont));
	}
}


/// <summary>
/// Visitor for count number of fields in a serialization object.
///	Supported internal `Serialize()` or external `SerializeObject()` with serialization in the form `archive << KeyValue("value", value)`.
/// </summary>
template <typename TArchive>
class FieldsCountVisitor
{
public:
	explicit FieldsCountVisitor(const TArchive& archive) noexcept
		: Archive(archive)
	{ }

	template <typename TValue>
	size_t Count(TValue& obj)
	{
		constexpr auto hasSerializeMethod = has_serialize_method_v<TValue>;
		constexpr auto hasGlobalSerializeObject = has_global_serialize_object_v<TValue>;
		static_assert(hasSerializeMethod || hasGlobalSerializeObject, "BitSerializer. Cannot count number of object fields");

		// Globally defined functions have higher priority over internal ones
		if constexpr (hasGlobalSerializeObject) {
			SerializeObject(*this, obj);
		}
		else if constexpr (hasSerializeMethod) {
			obj.Serialize(*this);
		}
		return Size;
	}

	static constexpr ArchiveType archive_type = TArchive::archive_type;
	using key_type = typename TArchive::key_type;
	static constexpr bool is_binary = TArchive::is_binary;

	static constexpr auto GetMode() noexcept { return TArchive::GetMode(); }
	static constexpr auto IsSaving() noexcept { return TArchive::IsSaving(); }
	static constexpr auto IsLoading() noexcept { return TArchive::IsLoading(); }

	[[nodiscard]] SerializationContext& GetContext() const noexcept { return Archive->GetContext(); }
	[[nodiscard]] const SerializationOptions& GetOptions() const noexcept { return Archive->GetOptions(); }

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


/// <summary>
/// Counts number of fields in a serialization object or size of map (for types like `std::map`, `std::unordered_map`, etc).
///	Always returns zero when loading or when target format of archive is text.
/// </summary>
/// <returns>The number of fields or zero when target format of archive is text</returns>
template <typename TArchive, typename TValue>
size_t CountMapObjectFields(const TArchive& archive, TValue& obj)
{
	if constexpr (TArchive::IsLoading() || !TArchive::is_binary)
	{
		// Text formats do not require counting number of fields
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

/// <summary>
/// Checks that it is input stream.
/// </summary>
template <typename T>
struct is_input_stream
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_base_of_v<std::basic_istream<typename TObj::char_type, std::char_traits<typename TObj::char_type>>, TObj>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool is_input_stream_v = is_input_stream<T>::value;


/// <summary>
/// Checks that it is output stream.
/// </summary>
template <typename T>
struct is_output_stream
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_base_of_v<std::basic_ostream<typename TObj::char_type, std::char_traits<typename TObj::char_type>>, TObj>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T>
constexpr bool is_output_stream_v = is_output_stream<T>::value;


/// <summary>
/// Checks that it is a validator
/// </summary>
template <typename T, typename TValue>
struct is_validator
{
private:
	template <typename TObj>
	static std::enable_if_t<std::is_same_v<std::optional<std::string>,
		decltype(std::declval<TObj>().operator()(std::declval<const TValue&>(), std::declval<const bool>()))>, std::true_type> test(int);

	template <typename>
	static std::false_type test(...);

public:
	typedef decltype(test<T>(0)) type;
	enum { value = type::value };
};

template <typename T, typename TValue>
constexpr bool is_validator_v = is_validator<T, TValue>::value;

}
