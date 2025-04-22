/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <fstream>
#include "bitserializer/serialization_detail/serialization_base_types.h"
#include "bitserializer/serialization_detail/key_value_proxy.h"
#include "bitserializer/serialization_detail/validators.h"
#include "bitserializer/serialization_detail/serialization_context.h"

namespace BitSerializer
{
	/// <summary>
	/// Default serialization options, it would be a good idea to configure them at startup time.
	/// </summary>
	static SerializationOptions DefaultOptions;

	using Validate::Required;

	/// <summary>
	/// Loads the object from one of archive supported data type (strings, binary data).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input array.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	template <typename TArchive, typename T, typename TInput, std::enable_if_t<!is_input_stream_v<TInput>, int> = 0>
	static void LoadObject(T&& object, const TInput& input, const SerializationOptions& serializationOptions = DefaultOptions)
	{
		constexpr auto hasInputDataTypeSupport = is_archive_support_input_data_type_v<typename TArchive::input_archive_type, TInput>;
		static_assert(hasInputDataTypeSupport, "BitSerializer. The archive doesn't support loading from passed data type.");

		if constexpr (hasInputDataTypeSupport)
		{
			SerializationContext context(serializationOptions);
			typename TArchive::input_archive_type archive(input, context);
			KeyValueProxy::SplitAndSerialize(archive, std::forward<T>(object));
			archive.Finalize();
			context.OnFinishSerialization();
		}
	}

	/// <summary>
	/// Loads the object from stream (archive should have support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input stream.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	template <typename TArchive, typename T, typename TStreamElem>
	static void LoadObject(T&& object, std::basic_istream<TStreamElem, std::char_traits<TStreamElem>>& input, const SerializationOptions& serializationOptions = DefaultOptions)
	{
		constexpr auto hasInputDataTypeSupport = is_archive_support_input_data_type_v<typename TArchive::input_archive_type, std::basic_istream<TStreamElem, std::char_traits<TStreamElem>>>;
		static_assert(hasInputDataTypeSupport, "BitSerializer. The archive does not support loading from passed stream type.");

		if constexpr (hasInputDataTypeSupport)
		{
			SerializationContext context(serializationOptions);
			typename TArchive::input_archive_type archive(input, context);
			KeyValueProxy::SplitAndSerialize(archive, std::forward<T>(object));
			archive.Finalize();
			context.OnFinishSerialization();
		}
	}

	/// <summary>
	/// Saves the object to one of archive supported data type (strings, binary data).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output array.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	template <typename TArchive, typename T, typename TOutput, std::enable_if_t<!is_output_stream_v<TOutput>, int> = 0>
	static void SaveObject(T&& object, TOutput& output, const SerializationOptions& serializationOptions = DefaultOptions)
	{
		constexpr auto hasOutputDataTypeSupport = is_archive_support_output_data_type_v<typename TArchive::output_archive_type, TOutput>;
		static_assert(hasOutputDataTypeSupport, "BitSerializer. The archive does not support save to passed stream type.");

		if constexpr (hasOutputDataTypeSupport)
		{
			SerializationContext context(serializationOptions);
			typename TArchive::output_archive_type archive(output, context);
			KeyValueProxy::SplitAndSerialize(archive, std::forward<T>(object));
			archive.Finalize();
			context.OnFinishSerialization();
		}
	}

	/// <summary>
	/// Saves the object to stream (archive should have support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output stream.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	template <typename TArchive, typename T, typename TStreamElem>
	static void SaveObject(T&& object, std::basic_ostream<TStreamElem, std::char_traits<TStreamElem>>& output, const SerializationOptions& serializationOptions = DefaultOptions)
	{
		constexpr auto hasOutputDataTypeSupport = is_archive_support_output_data_type_v<typename TArchive::output_archive_type, std::basic_ostream<TStreamElem, std::char_traits<TStreamElem>>>;
		static_assert(hasOutputDataTypeSupport, "BitSerializer. The archive does not support save to passed stream type.");

		if constexpr (hasOutputDataTypeSupport)
		{
			SerializationContext context(serializationOptions);
			typename TArchive::output_archive_type archive(output, context);
			KeyValueProxy::SplitAndSerialize(archive, std::forward<T>(object));
			archive.Finalize();
			context.OnFinishSerialization();
		}
	}

	/// <summary>
	/// Saves the object to preferred output type.
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	/// <returns>The output string or binary array that is used in archive by default.</returns>
	template <typename TArchive, typename T, typename TOutput = typename TArchive::preferred_output_format>
	static TOutput SaveObject(T&& object, const SerializationOptions& serializationOptions = DefaultOptions)
	{
		typename TArchive::preferred_output_format output;
		SaveObject<TArchive>(std::forward<T>(object), output, serializationOptions);
		return output;
	}

	//-----------------------------------------------------------------------------

	/// <summary>
	/// Loads the object from file (archive should have support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="path">The file path.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	template <typename TArchive, typename T, typename TPath>
	static void LoadObjectFromFile(T&& object, TPath&& path, const SerializationOptions& serializationOptions = DefaultOptions)
	{
		using preferred_stream_char_type = typename TArchive::preferred_stream_char_type;
		std::basic_ifstream<preferred_stream_char_type> stream;
		stream.open(path, std::ifstream::in | std::ifstream::binary);
		if (stream.is_open()) {
			LoadObject<TArchive>(std::forward<T>(object), stream, serializationOptions);
		}
		else {
			throw SerializationException(SerializationErrorCode::InputOutputError, Convert::ToString(std::forward<TPath>(path), "File not found: "));
		}
	}

	/// <summary>
	/// Saves the object to file (archive should have support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="path">The file path.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	/// <param name="overwrite">Overwrite if already exists.</param>
	template <typename TArchive, typename T, typename TPath>
	static void SaveObjectToFile(T&& object, TPath&& path, const SerializationOptions& serializationOptions = DefaultOptions, bool overwrite = false)
	{
		// Check if file already exists
		if (!overwrite)
		{
			if (std::ifstream stream(path); stream.good())
			{
				throw SerializationException(SerializationErrorCode::InputOutputError, Convert::ToString(std::forward<TPath>(path), "File already exists: "));
			}
		}

		using preferred_stream_char_type = typename TArchive::preferred_stream_char_type;
		std::basic_ofstream<preferred_stream_char_type> stream;
		stream.open(path, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
		if (stream.is_open()) {
			SaveObject<TArchive>(std::forward<T>(object), stream, serializationOptions);
		}
		else {
			throw SerializationException(SerializationErrorCode::InputOutputError, Convert::ToString(std::forward<TPath>(path), "Could not open file: "));
		}
	}
}


/// <summary>
/// Global operator << for serialize object from/to the archive.
/// </summary>
/// <param name="archive">The archive.</param>
/// <param name="value">The serializing value.</param>
/// <returns>The reference to archive that passed as parameter.</returns>
template <class TArchive, class TValue, std::enable_if_t<BitSerializer::is_archive_scope_v<TArchive>, int> = 0>
TArchive& operator<<(TArchive& archive, TValue&& value)
{
	BitSerializer::KeyValueProxy::SplitAndSerialize(archive, std::forward<TValue>(value));
	return archive;
}
