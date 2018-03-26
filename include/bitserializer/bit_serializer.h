/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "serialization_detail\serialization_base_types.h"
#include "serialization_detail\serialization_stl_containers.h"
#include "serialization_detail\serialization_stl_types.h"

namespace BitSerializer
{
	/// <summary>
	/// Loads the object from string or binary array (depends to format of archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input array.</param>
	template <typename TMediaArchive, typename T>
	inline void LoadObject(T& object, const typename TMediaArchive::output_format& input)
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
	inline void SaveObject(T& object, typename TMediaArchive::output_format& output)
	{
		TMediaArchive archive;
		auto scope = archive.Save(output);
		Serialize(scope, object);
	}

	/// <summary>
	/// Saves the object to string or binary array (depends to format of archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <returns>The output string or binary array</returns>
	template <typename TMediaArchive, typename T>
	inline typename TMediaArchive::output_format SaveObject(T& object)
	{
		typename TMediaArchive::output_format output;
		SaveObject<TMediaArchive>(object, output);
		return output;
	}

	//-----------------------------------------------------------------------------

	/// <summary>
	/// Loads the object from stream (depends to format of archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input stream.</param>
	template <typename TMediaArchive, typename T, typename TStreamElem>
	inline void LoadObject(T& object, std::basic_istream<TStreamElem, std::char_traits<TStreamElem>>& input)
	{
		TMediaArchive archive;
		auto scope = archive.Load(input);
		Serialize(scope, object);
	}

	/// <summary>
	/// Saves the object to stream (depends to format of archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output stream.</param>
	template <typename TMediaArchive, typename T, typename TStreamElem>
	inline void SaveObject(T& object, std::basic_ostream<TStreamElem, std::char_traits<TStreamElem>>& output)
	{
		TMediaArchive archive;
		auto scope = archive.Save(output);
		Serialize(scope, object);
	}

	//-----------------------------------------------------------------------------

	/// <summary>
	/// Loads the object from file (archive should support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="path">The file path.</param>
	template <typename TMediaArchive, typename T, typename TString>
	inline void LoadObjectFromFile(T& object, TString&& path)
	{
		using stream_char_type = typename TMediaArchive::stream_char_type;
		std::basic_ifstream<stream_char_type, std::char_traits<stream_char_type>> stream;
		stream.open(std::forward<TString>(path), std::ios::in | std::ios::binary);
		if (stream.is_open())
			LoadObject<TMediaArchive>(object, stream);
		else
			throw std::runtime_error("The file '" + Convert::ToString(path) + "' was not found.");
	}

	/// <summary>
	/// Saves the object to file (archive should support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="path">The file path.</param>
	template <typename TMediaArchive, typename T, typename TString>
	inline void SaveObjectToFile(T& object, TString&& path)
	{
		using stream_char_type = typename TMediaArchive::stream_char_type;
		std::basic_ofstream<stream_char_type, std::char_traits<stream_char_type>> stream;
		stream.open(std::forward<TString>(path), std::ios::out | std::ios::binary);
		if (stream.is_open())
			SaveObject<TMediaArchive>(object, stream);
		else
			throw std::runtime_error("Could not open file: " + Convert::ToString(path));
	}

} // namespace BitSerializer


/// <summary>
/// Operator << for serialize object to/from the archive.
/// </summary>
/// <param name="archive">The archive.</param>
/// <param name="value">The serializing value.</param>
/// <returns></returns>
template <class TArchive, class TValue, std::enable_if_t<BitSerializer::is_archive_scope_v<TArchive>, int> = 0>
inline TArchive& operator<<(TArchive& archive, TValue&& value)
{
	BitSerializer::Serialize(archive, std::forward<TValue>(value));
	return archive;
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
	if constexpr (std::is_same_v<std::decay_t<TKey>, TArchive::key_type>)
		BitSerializer::Serialize(archive, keyValue.GetKey(), keyValue.GetValue());
	else
	{
		const auto archiveCompatibleKey = Convert::FromString<TArchive::key_type>(keyValue.GetKey());
		BitSerializer::Serialize(archive, archiveCompatibleKey, keyValue.GetValue());
	}
	return archive;
}
