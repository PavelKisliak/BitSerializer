/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

namespace BitSerializer::Detail
{
	inline void ResizeStringUninitialized(std::string& str, size_t size)
	{
#if defined(__cpp_lib_string_resize_and_overwrite) && __cpp_lib_string_resize_and_overwrite >= 202110L
		str.resize_and_overwrite(size, [](char* /*data*/, size_t count) noexcept {
			return count;
		});
#else
		str.resize(size);
#endif
	}

	/**
	 * @brief One-shot byte buffer that writes directly into a caller-provided `std::string`.
	 *
	 * The writer owns the string for one output segment only. `mData` is the current write
	 * cursor and `mEnd` the end of the safe region; there is no separate logical size.
	 */
	class StringBuffer
	{
	public:
		explicit StringBuffer(std::string& output)
			: mOutput(output)
		{
			const size_t originalSize = output.size();
			ResizeStringUninitialized(mOutput, originalSize + InitialCapacity);
			mData = mOutput.data() + originalSize;
			mEnd = mOutput.data() + mOutput.size() - PushSlack;
		}

		void Ensure(size_t additional)
		{
			// mEnd is the safe end (actual end minus PushSlack). Using the signed pointer
			// difference is important: PushUnsafe may have advanced past mEnd into the
			// reserved slack, and the difference is then negative.
			if (mEnd - mData < static_cast<ptrdiff_t>(additional)) {
				Grow(additional + PushSlack);
			}
		}

		[[nodiscard]] char* Tail() noexcept { return mData; }
		void Commit(size_t count) noexcept { mData += count; }
		[[nodiscard]] size_t Size() const noexcept {
			return static_cast<size_t>(mData - mOutput.data());
		}

		/**
		 * @brief Writes one byte without a capacity check.
		 *
		 * The caller must have an active PushSlack guarantee (every checked operation
		 * leaves at least PushSlack bytes free).
		 */
		void PushUnsafe(char value) noexcept { *mData++ = value; }

		/**
		 * @brief Writes one byte, using only a cheap safe-end comparison on the hot path.
		 *
		 * A successful common-path write always leaves the PushSlack region intact.
		 */
		void Push(char value)
		{
			if (mData < mEnd) {
				*mData++ = value;
				return;
			}
			Grow(1 + PushSlack);
			*mData++ = value;
		}

		void Append(const char* data, size_t size)
		{
			if (size == 0) {
				return;
			}
			Ensure(size);
			std::memcpy(mData, data, size);
			mData += size;
		}

		void Append(std::string_view value) { Append(value.data(), value.size()); }

		void Append(char value, size_t count)
		{
			if (count == 0) {
				return;
			}
			Ensure(count);
			std::memset(mData, value, count);
			mData += count;
		}

		template <size_t N>
		void AppendLiteral(const char (&value)[N])
		{
			// N includes the terminating null, so N == 1 is an empty literal.
			if constexpr (N > 1)
			{
				Ensure(N - 1);
				std::memcpy(mData, value, N - 1);
				mData += N - 1;
			}
		}

		void Finalize()
		{
			mOutput.resize(static_cast<size_t>(mData - mOutput.data()));
		}

	private:
		// 504 bytes plus the 8-byte safe tail gives a 512-byte initial segment.
		static constexpr size_t InitialCapacity = 504;
		static constexpr size_t PushSlack = 8;

		void Grow(size_t additional)
		{
			const size_t used = static_cast<size_t>(mData - mOutput.data());
			size_t newSize = mOutput.size();
			do {
				newSize <<= 1;
			} while (newSize < used + additional);

			ResizeStringUninitialized(mOutput, newSize);
			mData = mOutput.data() + used;
			mEnd = mOutput.data() + newSize - PushSlack;
		}

		std::string& mOutput;
		char* mData;
		char* mEnd;
	};
}
