/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <fstream>
#include "serialization_detail/serialization_base_types.h"
#include "serialization_detail/serialization_stl_containers.h"
#include "serialization_detail/serialization_stl_types.h"
#include "serialization_detail/key_value.h"
#include "serialization_detail/validators.h"

namespace BitSerializer
{
	/// <summary>
	/// The serialization context, contains validation information, etc...
	/// </summary>
	thread_local static SerializationContext Context;

	/// <summary>
	/// Loads the object from one of archive supported data type (strings, binary data).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input array.</param>
	template <typename TMediaArchive, typename T, typename TInput, std::enable_if_t<!is_input_stream_v<TInput>, int> = 0>
	static void LoadObject(T& object, const TInput& input)
	{
		constexpr auto hasInputDataTypeSupport = is_archive_support_input_data_type<typename TMediaArchive::input_archive_type, TInput>::value;
		static_assert(hasInputDataTypeSupport, "BitSerializer. The archive doesn't support loading from provided data type.");

		if constexpr (hasInputDataTypeSupport)
		{
			Context.OnStartSerialization();
			typename TMediaArchive::input_archive_type archive(input);
			Serialize(archive, object);
		}
	}

	/// <summary>
	/// Loads the object from stream (should be supported in the archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input stream.</param>
	template <typename TMediaArchive, typename T, typename TStreamElem>
	static void LoadObject(T& object, std::basic_istream<TStreamElem, std::char_traits<TStreamElem>>& input)
	{
		constexpr auto hasInputDataTypeSupport = is_archive_support_input_data_type<typename TMediaArchive::input_archive_type, std::basic_istream<TStreamElem, std::char_traits<TStreamElem>>>::value;
		static_assert(hasInputDataTypeSupport, "BitSerializer. The archive doesn't support loading from this type of stream.");

		if constexpr (hasInputDataTypeSupport)
		{
			Context.OnStartSerialization();
			typename TMediaArchive::input_archive_type archive(input);
			Serialize(archive, object);
		}
	}

	/// <summary>
	/// Saves the object to one of archive supported data type (strings, binary data).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output array.</param>
	template <typename TMediaArchive, typename T, typename TOutput, std::enable_if_t<!is_output_stream_v<TOutput>, int> = 0>
	static void SaveObject(T& object, TOutput& output)
	{
		constexpr auto hasOutputDataTypeSupport = is_archive_support_output_data_type<typename TMediaArchive::output_archive_type, TOutput>::value;
		static_assert(hasOutputDataTypeSupport, "BitSerializer. The archive doesn't support save to provided data type.");

		if constexpr (hasOutputDataTypeSupport)
		{
			Context.OnStartSerialization();
			typename TMediaArchive::output_archive_type archive(output);
			Serialize(archive, object);
		}
	}

	/// <summary>
	/// Saves the object to stream (should be supported in the archive).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output stream.</param>
	template <typename TMediaArchive, typename T, typename TStreamElem>
	static void SaveObject(T& object, std::basic_ostream<TStreamElem, std::char_traits<TStreamElem>>& output)
	{
		constexpr auto hasOutputDataTypeSupport = is_archive_support_output_data_type<typename TMediaArchive::output_archive_type, std::basic_ostream<TStreamElem, std::char_traits<TStreamElem>>>::value;
		static_assert(hasOutputDataTypeSupport, "BitSerializer. The archive doesn't support save to this type of stream.");

		if constexpr (hasOutputDataTypeSupport)
		{
			Context.OnStartSerialization();
			typename TMediaArchive::output_archive_type archive(output);
			Serialize(archive, object);
		}
	}

	/// <summary>
	/// Saves the object to preferred output type.
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <returns>The output string or binary array</returns>
	template <typename TMediaArchive, typename T, typename TOutput = typename TMediaArchive::preferred_output_format>
	static TOutput SaveObject(T& object)
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
	static void LoadObjectFromFile(T& object, TString&& path)
	{
		using preferred_stream_char_type = typename TMediaArchive::preferred_stream_char_type;
		std::basic_ifstream<preferred_stream_char_type, std::char_traits<preferred_stream_char_type>> stream;
		stream.open(std::forward<TString>(path), std::ios::in | std::ios::binary);
		if (stream.is_open())
			LoadObject<TMediaArchive>(object, stream);
		else
			throw std::runtime_error("BitSerializer. The file was not found.");
	}

	/// <summary>
	/// Saves the object to file (archive should support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="path">The file path.</param>
	template <typename TMediaArchive, typename T, typename TString>
	static void SaveObjectToFile(T& object, TString&& path)
	{
		using preferred_stream_char_type = typename TMediaArchive::preferred_stream_char_type;
		std::basic_ofstream<preferred_stream_char_type, std::char_traits<preferred_stream_char_type>> stream;
		stream.open(std::forward<TString>(path), std::ios::out | std::ios::binary);
		if (stream.is_open())
			SaveObject<TMediaArchive>(object, stream);
		else
			throw std::runtime_error("BitSerializer. Could not open file.");
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
template <class TArchive, class TKey, class TValue, class... Validators, std::enable_if_t<BitSerializer::is_archive_scope_v<TArchive>, int> = 0>
static TArchive& operator<<(TArchive& archive, BitSerializer::KeyValue<TKey, TValue, Validators...>&& keyValue)
{
	constexpr auto hasSupportKeyType = BitSerializer::is_type_convertible_to_one_from_tuple_v<TKey, typename TArchive::supported_key_types>;
	static_assert(hasSupportKeyType, "BitSerializer. The archive doesn't support this key type.");

	bool result = BitSerializer::Serialize(archive, keyValue.GetKey(), keyValue.GetValue());

	// Validation when loading
	if constexpr (archive.IsLoading())
	{
		auto validationResult = keyValue.ValidateValue(result);
		if (validationResult.has_value())
		{
			auto path = archive.GetPath() + TArchive::path_separator + BitSerializer::Convert::ToWString(keyValue.GetKey());
			BitSerializer::Context.AddValidationErrors(path, std::move(*validationResult));
		}
	}
	return archive;
}

/// <summary>
/// Operator << for serialize a named value to/from the archive (with auto adaptation a key to type which is supported by archive).
/// </summary>
/// <param name="archive">The archive.</param>
/// <param name="keyValue">The serializing object with key.</param>
/// <returns></returns>
template <class TArchive, class TKey, class TValue, class... Validators, std::enable_if_t<BitSerializer::is_archive_scope_v<TArchive>, int> = 0>
static TArchive& operator<<(TArchive& archive, BitSerializer::AutoKeyValue<TKey, TValue, Validators...>&& keyValue)
{
	// Checks key type and adapts it to archive if needed
	if constexpr (std::is_convertible_v<TKey, typename TArchive::key_type>)
		archive << std::forward<BitSerializer::KeyValue<TKey, TValue, Validators...>>(keyValue);
	else
		archive << keyValue.AdaptAndMoveToBaseKeyValue<typename TArchive::key_type>();
	return archive;
}
