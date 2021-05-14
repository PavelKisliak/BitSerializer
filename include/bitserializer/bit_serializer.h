/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <fstream>
#include "serialization_detail/serialization_base_types.h"
#include "serialization_detail/key_value_proxy.h"
#include "serialization_detail/validators.h"
#include "serialization_detail/errors_handling.h"

namespace BitSerializer
{
	struct Version
	{
		static constexpr uint8_t Major = 0;
		static constexpr uint8_t Minor = 10;
		static constexpr uint8_t Maintenance = 0;

		static constexpr uint32_t Full = Major * 100 + Minor * 10 + Maintenance;
	};

	/// <summary>
	/// Loads the object from one of archive supported data type (strings, binary data).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input array.</param>
	template <typename TArchive, typename T, typename TInput, std::enable_if_t<!is_input_stream_v<TInput>, int> = 0>
	static void LoadObject(T&& object, const TInput& input)
	{
		constexpr auto hasInputDataTypeSupport = is_archive_support_input_data_type_v<typename TArchive::input_archive_type, TInput>;
		static_assert(hasInputDataTypeSupport, "BitSerializer. The archive doesn't support loading from passed data type.");

		if constexpr (hasInputDataTypeSupport)
		{
			Context.OnStartSerialization();
			typename TArchive::input_archive_type archive(input);
			KeyValueProxy::SplitAndSerialize(archive, std::forward<T>(object));
			archive.Finalize();
		}
	}

	/// <summary>
	/// Loads the object from stream (archive should have support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="input">The input stream.</param>
	template <typename TArchive, typename T, typename TStreamElem>
	static void LoadObject(T&& object, std::basic_istream<TStreamElem, std::char_traits<TStreamElem>>& input)
	{
		constexpr auto hasInputDataTypeSupport = is_archive_support_input_data_type_v<typename TArchive::input_archive_type, std::basic_istream<TStreamElem, std::char_traits<TStreamElem>>>;
		static_assert(hasInputDataTypeSupport, "BitSerializer. The archive does not support loading from passed stream type.");

		if constexpr (hasInputDataTypeSupport)
		{
			Context.OnStartSerialization();
			typename TArchive::input_archive_type archive(input);
			KeyValueProxy::SplitAndSerialize(archive, std::forward<T>(object));
			archive.Finalize();
		}
	}

	/// <summary>
	/// Saves the object to one of archive supported data type (strings, binary data).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output array.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	template <typename TArchive, typename T, typename TOutput, std::enable_if_t<!is_output_stream_v<TOutput>, int> = 0>
	static void SaveObject(T&& object, TOutput& output, const SerializationOptions& serializationOptions = {})
	{
		constexpr auto hasOutputDataTypeSupport = is_archive_support_output_data_type_v<typename TArchive::output_archive_type, TOutput>;
		static_assert(hasOutputDataTypeSupport, "BitSerializer. The archive does not support save to passed stream type.");

		if constexpr (hasOutputDataTypeSupport)
		{
			Context.OnStartSerialization();
			typename TArchive::output_archive_type archive(output, serializationOptions);
			KeyValueProxy::SplitAndSerialize(archive, std::forward<T>(object));
			archive.Finalize();
		}
	}

	/// <summary>
	/// Saves the object to stream (archive should have support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="output">The output stream.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	template <typename TArchive, typename T, typename TStreamElem>
	static void SaveObject(T&& object, std::basic_ostream<TStreamElem, std::char_traits<TStreamElem>>& output, const SerializationOptions& serializationOptions = {})
	{
		constexpr auto hasOutputDataTypeSupport = is_archive_support_output_data_type_v<typename TArchive::output_archive_type, std::basic_ostream<TStreamElem, std::char_traits<TStreamElem>>>;
		static_assert(hasOutputDataTypeSupport, "BitSerializer. The archive does not support save to passed stream type.");

		if constexpr (hasOutputDataTypeSupport)
		{
			Context.OnStartSerialization();
			typename TArchive::output_archive_type archive(output, serializationOptions);
			KeyValueProxy::SplitAndSerialize(archive, std::forward<T>(object));
			archive.Finalize();
		}
	}

	/// <summary>
	/// Saves the object to preferred output type.
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	/// <returns>The output string or binary array that is used in archive by default.</returns>
	template <typename TArchive, typename T, typename TOutput = typename TArchive::preferred_output_format>
	static TOutput SaveObject(T&& object, const SerializationOptions& serializationOptions = {})
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
	template <typename TArchive, typename T, typename TString>
	static void LoadObjectFromFile(T&& object, TString&& path)
	{
		using preferred_stream_char_type = typename TArchive::preferred_stream_char_type;
		std::basic_ifstream<preferred_stream_char_type, std::char_traits<preferred_stream_char_type>> stream;
		stream.open(std::forward<TString>(path), std::ios::in | std::ios::binary);
		if (stream.is_open())
			LoadObject<TArchive>(std::forward<T>(object), stream);
		else
			throw SerializationException(SerializationErrorCode::InputOutputError, "File not found: " + Convert::ToString(std::forward<TString>(path)));
	}

	/// <summary>
	/// Saves the object to file (archive should have support serialization to stream).
	/// </summary>
	/// <param name="object">The serializing object.</param>
	/// <param name="path">The file path.</param>
	/// <param name="serializationOptions">The serialization options.</param>
	template <typename TArchive, typename T, typename TString>
	static void SaveObjectToFile(T&& object, TString&& path, const SerializationOptions& serializationOptions = {})
	{
		using preferred_stream_char_type = typename TArchive::preferred_stream_char_type;
		std::basic_ofstream<preferred_stream_char_type, std::char_traits<preferred_stream_char_type>> stream;
		stream.open(std::forward<TString>(path), std::ios::out | std::ios::binary);
		if (stream.is_open())
			SaveObject<TArchive>(std::forward<T>(object), stream, serializationOptions);
		else
			throw SerializationException(SerializationErrorCode::InputOutputError, "Could not open file: " + Convert::ToString(std::forward<TString>(path)));
	}

} // namespace BitSerializer


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
