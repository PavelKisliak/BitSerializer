/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <ostream>
#include <string>
#include <variant>
#include "bitserializer/conversion_detail/convert_utf.h"

namespace BitSerializer::Convert::Utf
{
	/**
	 * @brief Writes UTF-encoded data to a stream with optional BOM.
	 */
	class EncodedStreamWriter
	{
	public:
		EncodedStreamWriter(std::ostream& outputStream, UtfType targetUtfType, bool addBom,
			UtfEncodingErrorPolicy encodingErrorPolicy = UtfEncodingErrorPolicy::Skip);

		template <typename TCharType>
		UtfEncodingErrorCode Write(const std::basic_string_view<TCharType>& str)
		{
			return std::visit([str, this](auto&& utfToolset) -> UtfEncodingErrorCode
			{
				if constexpr (sizeof(TCharType) == 1 && sizeof(decltype(utfToolset.second.front())) == 1)
				{
					mOutputStream.write(reinterpret_cast<const char*>(str.data()), static_cast<std::streamsize>(str.size()));
					return UtfEncodingErrorCode::Success;
				}
				else
				{
					utfToolset.second.clear();
					// NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage)
					const auto result = utfToolset.first.Encode(str.data(), str.data() + str.size(), utfToolset.second, mEncodingErrorPolicy);
					if (result)
					{
						mOutputStream.write(reinterpret_cast<const char*>(utfToolset.second.data()),
							static_cast<std::streamsize>(utfToolset.second.size() * sizeof(decltype(utfToolset.second.front()))));
						return UtfEncodingErrorCode::Success;
					}
					return result.ErrorCode;
				}
			}, mUtfToolset);
		}

		template <typename TCharType, typename TAllocator>
		UtfEncodingErrorCode Write(const std::basic_string<TCharType, std::char_traits<TCharType>, TAllocator>& str)
		{
			return Write(std::basic_string_view<TCharType>(str.data(), str.size()));
		}

		template <typename TCharType, size_t ArraySize>
		UtfEncodingErrorCode Write(const TCharType(&str)[ArraySize])
		{
			return Write(std::basic_string_view<TCharType>(std::cbegin(str), std::size(str)));
		}

	private:
		using UtfVariant = std::variant<std::pair<Utf8, std::string>, std::pair<Utf16Le, std::u16string>, std::pair<Utf16Be, std::u16string>, std::pair<Utf32Le, std::u32string>, std::pair<Utf32Be, std::u32string>>;
		std::ostream& mOutputStream;
		UtfVariant mUtfToolset;
		UtfEncodingErrorPolicy mEncodingErrorPolicy;
	};
}
