/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <filesystem>

namespace BitSerializer::Convert::Detail
{
	/**
	 * @brief Converts a `std::filesystem::path` to `std::string`.
	 *
	 * @param[in] in Input filesystem path to convert.
	 * @param[out] out Output string where the result will be stored.
	 */
	template <typename TSym, typename TAllocator>
	void To(const std::filesystem::path& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out)
	{
		const auto& nativePath = in.native();
		if constexpr (std::is_same_v<TSym, std::filesystem::path::string_type::value_type>) {
			out.append(nativePath);
		}
		else {
			To(ToStringView(nativePath), out);
		}
	}

	/**
	 * @brief Converts a `std::basic_string_view` to `std::filesystem::path`.
	 *
	 * @param[in] in Input string view to convert.
	 * @param[out] out Resulting filesystem path object.
	 */
	template <typename TSym>
	void To(std::basic_string_view<TSym> in, std::filesystem::path& out)
	{
		if constexpr (std::is_same_v<TSym, std::filesystem::path::string_type::value_type>) {
			out = in;
		}
		else
		{
			std::filesystem::path::string_type nativePath;
			To(in, nativePath);
			out = nativePath;
		}
	}
}
