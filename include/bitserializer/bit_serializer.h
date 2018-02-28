/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "serialization_detail\serialize_base_types.h"
#include "serialization_detail\serialize_stl_containers.h"
#include "serialization_detail\serialize_stl_types.h"

namespace BitSerializer
{
	/// <summary>
	/// Loads the object from string or binary array (depends to format of archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input array.</param>
	template <typename TMediaArchive, typename T>
	static inline void LoadObject(T& object, typename TMediaArchive::output_format& input)
	{
		TMediaArchive archive;
		auto scope = archive.Load(input);
		Serialize(scope, object);
	}

	/// <summary>
	/// Loads the object from stream (depends to format of archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input stream.</param>
	template <typename TMediaArchive, typename T>
	static void LoadObject(T& object, const typename TMediaArchive::input_stream& input)
	{
		TMediaArchive archive;
		auto scope = archive.Load(input);
		Serialize(scope, object);
	}

	/// <summary>
	/// Saves the object to string or binary array (depends to format of archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output array.</param>
	template <typename TMediaArchive, typename T>
	static inline void SaveObject(T& object, typename TMediaArchive::output_format& output)
	{
		TMediaArchive archive;
		auto scope = archive.Save(output);
		Serialize(scope, object);
	}

	/// <summary>
	/// Saves the object to stream (depends to format of archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output stream.</param>
	template <typename TMediaArchive, typename T>
	static void SaveObject(T& object, typename TMediaArchive::output_stream& output)
	{
		TMediaArchive archive;
		auto scope = archive.Save(output);
		Serialize(scope, object);
	}

} // namespace BitSerializer


/// <summary>
/// Operator << for serialize object to/from the archive.
/// </summary>
/// <param name="archive">The archive.</param>
/// <param name="value">The serializing value.</param>
/// <returns></returns>
template <class TArchive, class TValue, std::enable_if_t<BitSerializer::is_archive_scope_v<TArchive>, int> = 0>
inline TArchive& operator<<(TArchive& scope, TValue& value)
{
	BitSerializer::Serialize(scope, value);
	return scope;
}

/// <summary>
/// Operator << for serialize a named value to/from the archive.
/// </summary>
/// <param name="archive">The archive.</param>
/// <param name="keyValue">The serializing object with key.</param>
/// <returns></returns>
template <class TArchive, class TKey, class TValue, std::enable_if_t<BitSerializer::is_archive_scope_v<TArchive>, int> = 0>
inline TArchive& operator<<(TArchive& archive, BitSerializer::KeyValue<TKey, TValue>&& keyValue)
{
	if constexpr (std::is_same_v<TKey, TArchive::key_type>)
		BitSerializer::Serialize(archive, keyValue.GetKey(), keyValue.GetValue());
	else
	{
		const auto archiveCompatibleKey = Convert::FromString<TArchive::key_type>(keyValue.GetKey());
		BitSerializer::Serialize(archive, archiveCompatibleKey, keyValue.GetValue());
	}
	return archive;
}

/// <summary>
/// Operator << for serialize base object to/from the archive.
/// </summary>
/// <param name="archive">The archive.</param>
/// <param name="object">The serializing object.</param>
/// <returns></returns>
template <class TArchive, class TBase, class TDerived, std::enable_if_t<BitSerializer::is_archive_scope_v<TArchive>, int> = 0>
inline TArchive& operator<<(TArchive& archive, BitSerializer::BaseObjectImpl<TBase, TDerived>&& object)
{
	BitSerializer::Serialize(archive, std::move(object));
	return archive;
}
