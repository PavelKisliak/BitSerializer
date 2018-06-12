/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "serialization_detail/serialization_base_types.h"
#include "serialization_detail/serialization_stl_containers.h"
#include "serialization_detail/serialization_stl_types.h"

namespace BitSerializer
{
	/// <summary>
	/// Loads the object from one of archive supported data type (strings, binary data).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input array.</param>
	template <typename TMediaArchive, typename T, typename TInput, std::enable_if_t<!is_input_stream_v<TInput>, int> = 0>
	inline void LoadObject(T& object, const TInput& input)
	{
		if constexpr (is_archive_support_input_data_type<TMediaArchive::input_archive_type, TInput>::value)
		{
			typename TMediaArchive::input_archive_type archive(input);
			Serialize(archive, object);
		}
		else {
			static_assert(false, "BitSerializer. The archive doesn't support loading from provided data type.");
		}
	}

	/// <summary>
	/// Loads the object from stream (should be supported in the archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input stream.</param>
	template <typename TMediaArchive, typename T, typename TStreamElem>
	inline void LoadObject(T& object, std::basic_istream<TStreamElem, std::char_traits<TStreamElem>>& input)
	{
		if constexpr (is_archive_support_input_data_type<TMediaArchive::input_archive_type, std::basic_istream<TStreamElem, std::char_traits<TStreamElem>>>::value)
		{
			typename TMediaArchive::input_archive_type archive(input);
			Serialize(archive, object);
		}
		else
		{
			if constexpr (std::is_same_v<TStreamElem, char>)
				static_assert(false, "BitSerializer. The archive doesn't support loading from stream based on ANSI char element.");
			else
				static_assert(false, "BitSerializer. The archive doesn't support loading from stream based on wide string element.");
		}
	}

	/// <summary>
	/// Saves the object to one of archive supported data type (strings, binary data).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output array.</param>
	template <typename TMediaArchive, typename T, typename TOutput, std::enable_if_t<!is_output_stream_v<TOutput>, int> = 0>
	inline void SaveObject(T& object, TOutput& output)
	{
		if constexpr (is_archive_support_output_data_type<TMediaArchive::output_archive_type, TOutput>::value)
		{
			typename TMediaArchive::output_archive_type archive(output);
			Serialize(archive, object);
		}
		else {
			static_assert(false, "BitSerializer. The archive doesn't support save to provided data type.");
		}
	}

	/// <summary>
	/// Saves the object to stream (should be supported in the archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output stream.</param>
	template <typename TMediaArchive, typename T, typename TStreamElem>
	inline void SaveObject(T& object, std::basic_ostream<TStreamElem, std::char_traits<TStreamElem>>& output)
	{
		if constexpr (is_archive_support_output_data_type<TMediaArchive::output_archive_type, std::basic_ostream<TStreamElem, std::char_traits<TStreamElem>>>::value)
		{
			typename TMediaArchive::output_archive_type archive(output);
			Serialize(archive, object);
		}
		else
		{
			if constexpr (std::is_same_v<TChar, char>)
				static_assert(false, "BitSerializer. The archive doesn't support save to stream based on ANSI char element.");
			else
				static_assert(false, "BitSerializer. The archive doesn't support save to stream based on wide string element.");
		}
	}

	/// <summary>
	/// Saves the object to preferred output type.
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <returns>The output string or binary array</returns>
	template <typename TMediaArchive, typename T, typename TOutput = typename TMediaArchive::preferred_output_format>
	inline TOutput SaveObject(T& object)
	{
		typename TMediaArchive::preferred_output_format output;
		SaveObject<TMediaArchive>(object, output);
		return output;
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
		using preferred_stream_char_type = typename TMediaArchive::preferred_stream_char_type;
		std::basic_ifstream<preferred_stream_char_type, std::char_traits<preferred_stream_char_type>> stream;
		stream.open(std::forward<TString>(path), std::ios::in | std::ios::binary);
		if (stream.is_open())
			LoadObjectFromStream<TMediaArchive>(object, stream);
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
		using preferred_stream_char_type = typename TMediaArchive::preferred_stream_char_type;
		std::basic_ofstream<preferred_stream_char_type, std::char_traits<preferred_stream_char_type>> stream;
		stream.open(std::forward<TString>(path), std::ios::out | std::ios::binary);
		if (stream.is_open())
			SaveObjectToStream<TMediaArchive>(object, stream);
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
