/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include "bitserializer/serialization_detail/serialization_base_types.h"

namespace BitSerializer
{
	/**
	 * @brief Serializes `std::unique_ptr`.
	 */
	template <class TArchive, typename TKey, class TValue>
	bool Serialize(TArchive& archive, TKey&& key, std::unique_ptr<TValue>& ptr)
	{
		if constexpr (TArchive::IsLoading())
		{
			if (!ptr) {
				ptr = std::make_unique<TValue>();
			}
			if (Serialize(archive, std::forward<TKey>(key), *ptr)) {
				return true;
			}
			ptr.reset();
			return false;
		}
		else
		{
			if (ptr) {
				return Serialize(archive, std::forward<TKey>(key), *ptr);
			}

			std::nullptr_t nullValue = nullptr;
			return Serialize(archive, std::forward<TKey>(key), nullValue);
		}
	}

	template<typename TArchive, typename TValue>
	bool Serialize(TArchive& archive, std::unique_ptr<TValue>& ptr)
	{
		if constexpr (TArchive::IsLoading())
		{
			if (!ptr) {
				ptr = std::make_unique<TValue>();
			}
			if (Serialize(archive, *ptr)) {
				return true;
			}
			ptr.reset();
			return false;
		}
		else
		{
			if (ptr) {
				return Serialize(archive, *ptr);
			}

			std::nullptr_t nullValue = nullptr;
			return Serialize(archive, nullValue);
		}
	}

	/**
	 * @brief Serializes `std::shared_ptr`.
	 */
	template <class TArchive, typename TKey, class TValue>
	bool Serialize(TArchive& archive, TKey&& key, std::shared_ptr<TValue>& ptr)
	{
		if constexpr (TArchive::IsLoading())
		{
			if (!ptr) {
				ptr = std::make_shared<TValue>();
			}
			if (Serialize(archive, std::forward<TKey>(key), *ptr)) {
				return true;
			}
			ptr.reset();
			return false;
		}
		else
		{
			if (ptr) {
				return Serialize(archive, std::forward<TKey>(key), *ptr);
			}

			std::nullptr_t nullValue = nullptr;
			return Serialize(archive, std::forward<TKey>(key), nullValue);
		}
	}

	template<typename TArchive, typename TValue>
	bool Serialize(TArchive& archive, std::shared_ptr<TValue>& ptr)
	{
		if constexpr (TArchive::IsLoading())
		{
			if (!ptr) {
				ptr = std::make_shared<TValue>();
			}
			if (Serialize(archive, *ptr)) {
				return true;
			}
			ptr.reset();
			return false;
		}
		else
		{
			if (ptr) {
				return Serialize(archive, *ptr);
			}

			std::nullptr_t nullValue = nullptr;
			return Serialize(archive, nullValue);
		}
	}
}
