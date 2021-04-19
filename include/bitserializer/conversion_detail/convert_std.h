/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <filesystem>

namespace BitSerializer::Convert::Detail
{
	/// <summary>
	/// Converts from filesystem::path to std::string
	/// </summary>
	template <typename TSym, typename TAllocator>
	void To(const std::filesystem::path& in, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& out) {
		const auto& nativePath = in.native();
		if constexpr (std::is_same_v<TSym, std::filesystem::path::string_type::value_type>) {
			out.append(nativePath);
		}
		else {
			To(ToStringView(nativePath), out);
		}
	}

	/// <summary>
	/// Converts from std::string_view to filesystem::path
	/// </summary>
	template <typename TSym>
	void To(const std::basic_string_view<TSym>& in, std::filesystem::path& out) {
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
