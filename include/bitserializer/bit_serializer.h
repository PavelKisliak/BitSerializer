/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <fstream>
#include <iterator>
#include "bitserializer/serialization_detail/serialization_base_types.h"
#include "bitserializer/serialization_detail/serialization_dispatch.h"
#include "bitserializer/serialization_detail/serialization_context.h"
#include "bitserializer/validate.h"
#include "bitserializer/refine.h"

namespace BitSerializer
{
	/**
	 * @brief Default serialization options.
	 *
	 * These options are used as a fallback when no explicit configuration is provided.
	 * It's recommended to configure these at startup time for application-specific needs.
	 */
	inline SerializationOptions DefaultOptions;

	/**
	 * @brief Loads an object from a supported data type (UTF-8 string or binary array depending on archive type).
	 *
	 * @tparam TArchive   The archive type that handles serialization/deserialization (e.g. JsonArchive, MsgPackArchive).
	 * @tparam TValue     The type of the object to be deserialized.
	 * @tparam TInput     The type of the input data (`std::string` or `std::string_view`).
	 * @param[in] object  The object to be deserialized from the input data.
	 * @param[in] input   The input data.
	 * @param[in] options The serialization options.
	 */
	template <typename TArchive, typename TValue, typename TInput, std::enable_if_t<!is_input_stream_v<TInput>, int> = 0>
	static void LoadObject(TValue&& object, const TInput& input, const SerializationOptions& options = DefaultOptions)
	{
		constexpr auto hasInputDataTypeSupport = is_archive_support_input_data_type_v<typename TArchive::input_archive_type, TInput>;
		static_assert(hasInputDataTypeSupport, "BitSerializer. The archive does not support loading from the provided data type.");

		if constexpr (hasInputDataTypeSupport)
		{
			SerializationContext context(options);
			typename TArchive::input_archive_type archive(input, context);
			Detail::Dispatch(archive, std::forward<TValue>(object));
			archive.Finalize();
			context.OnFinishSerialization();
		}
	}

	/**
	 * @brief Loads an object from a stream.
	 *
	 * @tparam TArchive    The archive type that handles serialization/deserialization (e.g. JsonArchive, MsgPackArchive).
	 * @tparam TValue      The type of the object to be deserialized.
	 * @tparam TChar       The character type of the input stream (currently all archives only support streams with `char` type).
	 * @param[in] object   The object to be deserialized from the input stream.
	 * @param[in] input    The input stream (any implementation of `std::istream` that supports `seekg()` operation).
	 * @param[in] options  The serialization options.
	 */
	template <typename TArchive, typename TValue, typename TChar>
	static void LoadObject(TValue&& object, std::basic_istream<TChar, std::char_traits<TChar>>& input, const SerializationOptions& options = DefaultOptions)
	{
		constexpr auto hasInputDataTypeSupport = is_archive_support_input_data_type_v<typename TArchive::input_archive_type, std::basic_istream<TChar, std::char_traits<TChar>>>;
		if constexpr (hasInputDataTypeSupport)
		{
			SerializationContext context(options);
			typename TArchive::input_archive_type archive(input, context);
			Detail::Dispatch(archive, std::forward<TValue>(object));
			archive.Finalize();
			context.OnFinishSerialization();
		}
		else
		{
			constexpr bool isSupportedChar = std::is_same_v<TChar, typename TArchive::preferred_output_type::value_type>;
			static_assert(isSupportedChar, "BitSerializer. The archive does not support the specified stream type.");

			if constexpr (isSupportedChar)
			{
				// Fallback: load entire stream into memory and parse from string
				typename TArchive::preferred_output_type inputData{
					std::istreambuf_iterator<TChar>(input), std::istreambuf_iterator<TChar>()
				};

				// Validate encoding (only UTF-8 is supported)
				size_t dataOffset;
				const auto utfType = Convert::Utf::DetectEncoding(std::string_view(inputData.data(), inputData.size()), dataOffset);
				if (utfType != Convert::Utf::UtfType::Utf8)
				{
					const auto encodingName = Convert::TryTo<std::string>(utfType);
					throw SerializationException(SerializationErrorCode::UnsupportedEncoding,
						"The archive does not support encoding: " + encodingName.value_or("unknown"));
				}

				// Skip BOM if present and delegate to string-based LoadObject
				auto inputView = static_cast<typename TArchive::string_view_type>(inputData);
				if (dataOffset > 0) {
					inputView = inputView.substr(dataOffset);
				}
				LoadObject<TArchive>(std::forward<TValue>(object), inputView, options);
			}
		}
	}

	/**
	 * @brief Saves an object to a supported data type (UTF-8 string or binary array depending on archive type).
	 *
	 * @tparam TArchive   The archive type that handles serialization/deserialization (e.g. JsonArchive, MsgPackArchive).
	 * @tparam TValue     The type of the object to be serialized.
	 * @tparam TOutput    The type of the output data (currently supported only `std::string`).
	 * @param[in] object  The object to be serialized.
	 * @param[out] output The destination for the serialized data.
	 * @param[in] options The serialization options.
	 */
	template <typename TArchive, typename TValue, typename TOutput, std::enable_if_t<!is_output_stream_v<TOutput>, int> = 0>
	static void SaveObject(TValue&& object, TOutput& output, const SerializationOptions& options = DefaultOptions)
	{
		constexpr auto hasOutputDataTypeSupport = is_archive_support_output_data_type_v<typename TArchive::output_archive_type, TOutput>;
		static_assert(hasOutputDataTypeSupport, "BitSerializer. The archive does not support saving to the provided data type.");

		if constexpr (hasOutputDataTypeSupport)
		{
			SerializationContext context(options);
			typename TArchive::output_archive_type archive(output, context);
			Detail::Dispatch(archive, std::forward<TValue>(object));
			archive.Finalize();
			context.OnFinishSerialization();
		}
	}

	/**
	 * @brief Saves an object to a stream (e.g., file or network stream).
	 *
	 * @tparam TArchive    The archive type that handles serialization/deserialization (e.g. JsonArchive, MsgPackArchive).
	 * @tparam TValue      The type of the object to be serialized.
	 * @tparam TChar       The character type of the output stream (currently all archives only support streams with `char` type).
	 * @param[in] object   The object to be serialized.
	 * @param[out] output  The destination for the serialized data (e.g., std::ofstream).
	 * @param[in] options  The serialization options.
	 */
	template <typename TArchive, typename TValue, typename TChar>
	static void SaveObject(TValue&& object, std::basic_ostream<TChar, std::char_traits<TChar>>& output, const SerializationOptions& options = DefaultOptions)
	{
		constexpr auto hasOutputDataTypeSupport = is_archive_support_output_data_type_v<typename TArchive::output_archive_type, std::basic_ostream<TChar, std::char_traits<TChar>>>;
		if constexpr (hasOutputDataTypeSupport)
		{
			SerializationContext context(options);
			typename TArchive::output_archive_type archive(output, context);
			Detail::Dispatch(archive, std::forward<TValue>(object));
			archive.Finalize();
			context.OnFinishSerialization();
		}
		else
		{
			constexpr bool isSupportedChar = std::is_same_v<TChar, typename TArchive::preferred_output_type::value_type>;
			static_assert(isSupportedChar, "BitSerializer. The archive does not support the specified stream type.");

			if constexpr (isSupportedChar)
			{
				// Validate encoding (only UTF-8 is supported in fallback)
				if (options.streamOptions.encoding != Convert::Utf::UtfType::Utf8)
				{
					const auto encodingName = Convert::TryTo<std::string>(options.streamOptions.encoding);
					throw SerializationException(SerializationErrorCode::UnsupportedEncoding,
						"The archive does not support encoding: " + encodingName.value_or("unknown"));
				}

				// Fallback: serialize to preferred_output_type, then write to stream
				auto outputData = SaveObject<TArchive>(std::forward<TValue>(object), options);
				if (options.streamOptions.writeBom) {
					output.write(Convert::Utf::Utf8::bom, sizeof(Convert::Utf::Utf8::bom));
				}
				output.write(outputData.data(), static_cast<std::streamsize>(outputData.size()));
			}
		}
	}

	/**
	 * @brief Saves an object to the preferred output type for the archive.
	 *
	 * @tparam TArchive   The archive type that handles serialization/deserialization (e.g. JsonArchive, MsgPackArchive).
	 * @tparam TValue     The type of the object to be serialized.
	 * @tparam TOutput    The preferred output type for the archive (default: typename TArchive::preferred_output_type).
	 * @param[in] object  The object to be serialized.
	 * @param[in] options The serialization options.
	 * @return The serialized data in the archive's preferred format.
	 */
	template <typename TArchive, typename TValue, typename TOutput = typename TArchive::preferred_output_type>
	static TOutput SaveObject(TValue&& object, const SerializationOptions& options = DefaultOptions)
	{
		typename TArchive::preferred_output_type output;
		SaveObject<TArchive>(std::forward<TValue>(object), output, options);
		return output;
	}

	//-----------------------------------------------------------------------------

	/**
	 * @brief Loads an object from a file.
	 *
	 * @tparam TArchive   The archive type that handles serialization/deserialization (e.g. JsonArchive, MsgPackArchive).
	 * @tparam TValue     The type of the object to be deserialized.
	 * @tparam TPath      The type of the file path (e.g., std::filesystem::path, std::string, const char*).
	 * @param[in] object  The object to be populated from the file data.
	 * @param[in] path    The file path to read from.
	 * @param[in] options The serialization options.
	 */
	template <typename TArchive, typename TValue, typename TPath>
	static void LoadObjectFromFile(TValue&& object, const TPath& path, const SerializationOptions& options = DefaultOptions)
	{
		std::basic_ifstream<typename TArchive::string_view_type::value_type> stream;
		stream.open(path, std::ifstream::in | std::ifstream::binary);
		if (stream.is_open()) {
			LoadObject<TArchive>(std::forward<TValue>(object), stream, options);
		}
		else {
			throw SerializationException(SerializationErrorCode::InputOutputError, Convert::ToString(path, "File not found: "));
		}
	}

	/**
	 * @brief Saves an object to a file.
	 *
	 * @tparam TArchive     The archive type that handles serialization/deserialization (e.g. JsonArchive, MsgPackArchive).
	 * @tparam TValue       The type of the object to be serialized.
	 * @tparam TPath        The type of the file path (e.g., std::filesystem::path, std::string, const char*).
	 * @param[in] object    The object to be written to the file.
	 * @param[in] path      The file path to write to.
	 * @param[in] options   The serialization options.
	 * @param[in] overwrite Whether to overwrite an existing file (default: false).
	 */
	template <typename TArchive, typename TValue, typename TPath>
	static void SaveObjectToFile(TValue&& object, const TPath& path, const SerializationOptions& options = DefaultOptions, bool overwrite = false)
	{
		// Check if file already exists
		if (!overwrite)
		{
			if (std::ifstream stream(path); stream.good())
			{
				throw SerializationException(SerializationErrorCode::InputOutputError, Convert::ToString(path, "File already exists: "));
			}
		}

		std::basic_ofstream<typename TArchive::string_view_type::value_type> stream;
		stream.open(path, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
		if (stream.is_open()) {
			SaveObject<TArchive>(std::forward<TValue>(object), stream, options);
		}
		else {
			throw SerializationException(SerializationErrorCode::InputOutputError, Convert::ToString(path, "Could not open file: "));
		}
	}

	/**
	 * @brief Validates that field is deserialized (see more @ref validate.h)
	 */
	using Validate::Required;

	/**
	 * @brief Provides a fallback value for missing deserialization data (see more @ref refine.h).
	 */
	using Refine::Fallback;

} // namespace BitSerializer


/**
 * @brief Global operator << for serializing objects to/from archives.
 *
 * @tparam TArchive   Archive type that supports serialization.
 * @tparam TValue     Type of the value being serialized.
 * @param[in] archive The archive instance.
 * @param[in] value   The value to serialize.
 * @return A reference to the archive.
 */
template <class TArchive, class TValue, std::enable_if_t<BitSerializer::is_archive_scope_v<TArchive>, int> = 0>
TArchive& operator<<(TArchive& archive, TValue&& value)
{
	BitSerializer::Detail::Dispatch(archive, std::forward<TValue>(value));
	return archive;
}
