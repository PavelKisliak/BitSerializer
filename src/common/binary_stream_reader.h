/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <iosfwd>
#include <optional>
#include <string_view>

namespace BitSerializer::Detail
{
	/**
	 * @brief A helper class for reading binary data from an input stream.
	 *
	 * This class provides buffered access to an input stream, allowing efficient reading of bytes, peeking,
	 * and random access within the current buffer. It's used internally when working with binary archives.
	 */
	class CBinaryStreamReader
	{
	public:
		/// @brief Size of the internal read buffer in bytes.
		static constexpr size_t chunk_size = 256u;

		/**
		 * @brief Constructs a new binary stream reader for the specified input stream.
		 *
		 * @param inputStream Reference to the input stream to read from.
		 */
		explicit CBinaryStreamReader(std::istream& inputStream);

		// Disable copy and move operations
		CBinaryStreamReader(const CBinaryStreamReader&) = delete;
		CBinaryStreamReader(CBinaryStreamReader&&) = delete;
		CBinaryStreamReader& operator=(const CBinaryStreamReader&) = delete;
		CBinaryStreamReader& operator=(CBinaryStreamReader&&) = delete;
		~CBinaryStreamReader() = default;

		/**
		 * @brief Checks if the end of the stream has been reached.
		 *
		 * @return true if there are no more bytes available for reading; false otherwise.
		 */
		[[nodiscard]] bool IsEnd() const noexcept;

		/**
		 * @brief Checks if an error occurred during reading.
		 *
		 * @return true if the stream has failed; false otherwise.
		 */
		[[nodiscard]] bool IsFailed() const noexcept;

		/**
		 * @brief Gets the current position in the stream (in bytes).
		 *
		 * @return The number of bytes read so far.
		 */
		[[nodiscard]] size_t GetPosition() const noexcept;

		/**
		 * @brief Moves the stream pointer to the specified position.
		 * @param pos Position to move to (in bytes from the beginning of the stream).
		 * @return true on success, false if the position is invalid or cannot be set.
		 */
		bool SetPosition(size_t pos);

		/**
		 * @brief Peeks at the next byte without incrementing the current position.
		 *
		 * @return An optional containing the byte if available; `std::nullopt` if EOF or failure.
		 */
		[[nodiscard]] std::optional<char> PeekByte();

		/**
		 * @brief Increments the current position by one byte.
		 */
		void GotoNextByte();

		/**
		 * @brief Reads the next byte and increments the current position.
		 *
		 * @return An optional containing the byte if available; `std::nullopt` if EOF or failure.
		 */
		[[nodiscard]] std::optional<char> ReadByte();

		/**
		 * @brief Attempts to read exactly specified number of bytes as a contiguous block.
		 *
		 * @param blockSize The number of bytes to read. Must not exceed the internal buffer size (e.g., `chunk_size`).
		 * @return A `string_view` referencing the read data, or an empty view if the requested number of bytes is not available.
		 */
		[[nodiscard]] std::string_view ReadExactly(size_t blockSize);

		/**
		 * @brief Reads the requested block, if the returned block is smaller than the requested one, you need to call the method again.
		 *
		 * @param maxBytesToRead The maximum number of bytes to read.
		 * @return A `string_view` referencing the read data, or an empty view if there is no more data.
		 */
		[[nodiscard]] std::string_view ReadUpTo(size_t maxBytesToRead);

	private:
		/**
		 * @brief Fills the internal buffer with the next chunk of data from the stream.
		 *
		 * @return true if more data was read; false if the end of the stream was reached or an error occurred.
		 */
		bool ReadNextChunk();

		std::istream& mStream;
		char mBuffer[chunk_size];
		const char* const mEndBufferPtr = mBuffer + chunk_size;
		char* mStartDataPtr = mBuffer;
		char* mEndDataPtr = mBuffer;
		size_t mStreamPos = 0;
	};

} // namespace BitSerializer::Detail
