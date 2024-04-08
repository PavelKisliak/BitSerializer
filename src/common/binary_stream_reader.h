/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include <istream>

namespace BitSerializer::Detail
{
	class CBinaryStreamReader
	{
	public:
		static constexpr size_t chunk_size = 256;

		explicit CBinaryStreamReader(std::istream& inputStream);
		CBinaryStreamReader(const CBinaryStreamReader&) = delete;
		CBinaryStreamReader(CBinaryStreamReader&&) = delete;
		CBinaryStreamReader& operator=(const CBinaryStreamReader&) = delete;
		CBinaryStreamReader& operator=(CBinaryStreamReader&&) = delete;
		~CBinaryStreamReader() = default;

		[[nodiscard]] bool IsEnd() const noexcept;
		[[nodiscard]] bool IsFailed() const noexcept;
		[[nodiscard]] size_t GetPosition() const noexcept;
		bool SetPosition(size_t pos);

		[[nodiscard]] std::optional<char> PeekByte();
		void GotoNextByte();
		[[nodiscard]] std::optional<char> ReadByte();
		[[nodiscard]] std::string_view ReadSolidBlock(size_t blockSize);
		[[nodiscard]] std::string_view ReadByChunks(size_t remainingSize);

	private:
		bool ReadNextChunk();

		std::istream& mStream;
		char mBuffer[chunk_size];
		const char* const mEndBufferPtr = mBuffer + chunk_size;
		char* mStartDataPtr = mBuffer;
		char* mEndDataPtr = mBuffer;
		size_t mStreamPos = 0;
	};
}
