/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/conversion_detail/convert_filesystem.h"

namespace BitSerializer
{
	/// <summary>
	/// Serializes std::filesystem::path.
	/// </summary>
	template <class TArchive, typename TKey>
	bool Serialize(TArchive& archive, TKey&& key, std::filesystem::path& value)
	{
		std::string strPath;
		if constexpr (TArchive::IsLoading())
		{
			if (Serialize(archive, std::forward<TKey>(key), strPath))
			{
				Convert::Detail::To(std::string_view(strPath), value);
				return true;
			}
			return false;
		}
		else
		{
			Convert::Detail::To(value, strPath);
			return Serialize(archive, std::forward<TKey>(key), strPath);
		}
	}

	template<typename TArchive>
	bool Serialize(TArchive& archive, std::filesystem::path& value)
	{
		std::string strPath;
		if constexpr (TArchive::IsLoading())
		{
			if (Serialize(archive, strPath))
			{
				Convert::Detail::To(std::string_view(strPath), value);
				return true;
			}
			return false;
		}
		else
		{
			Convert::Detail::To(value, strPath);
			return Serialize(archive, strPath);
		}
	}
}
