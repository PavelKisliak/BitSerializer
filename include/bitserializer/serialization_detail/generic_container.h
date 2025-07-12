/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstddef>

namespace BitSerializer::Detail
{
	/**
	 * @brief Generic function for serializing containers.
	 *
	 * Supports both loading and saving operations depending on the archive mode.
	 *
	 * When loading:
	 * - Resizes the container if an estimated size is available.
	 * - Loads data into existing elements.
	 * - Emplaces new elements for any remaining items in the archive.
	 * - Finalizes the container size based on loaded items.
	 *
	 * When saving:
	 * - Iterates through all elements and serializes them.
	 *
	 * @tparam TArchive    The archive type used for serialization.
	 * @tparam TContainer  The container type to serialize.
	 * @param arrayScope   Archive scope object used for serializing array content.
	 * @param cont         Reference to the container being serialized.
	 */
	template<typename TArchive, typename TContainer>
	static void SerializeContainer(TArchive& arrayScope, TContainer& cont)
	{
		if constexpr (TArchive::IsLoading())
		{
			// Resize container when approximate size is known
			if (auto estimatedSize = arrayScope.GetEstimatedSize(); estimatedSize != 0)
			{
				cont.resize(estimatedSize);
			}

			// Load into existing elements
			size_t loadedItems = 0;
			for (auto it = cont.begin(); it != cont.end() && !arrayScope.IsEnd(); ++it, ++loadedItems)
			{
				Serialize(arrayScope, *it);
			}

			// Load remaining items
			for (; !arrayScope.IsEnd(); ++loadedItems)
			{
				Serialize(arrayScope, cont.emplace_back());
			}

			// Adjust container size (in case fewer items were loaded than expected)
			cont.resize(loadedItems);
		}
		else
		{
			// Save all elements in the container
			for (auto& value : cont)
			{
				Serialize(arrayScope, value);
			}
		}
	}

	/**
	 * @brief Accesses the internal container of a container adapter (e.g., `std::queue`, `std::stack`).
	 *
	 * Container adapters like `std::queue` or `std::stack` wrap an underlying container (e.g., `std::deque`),
	 * which is typically inaccessible directly. This helper exposes access via inheritance trickery.
	 *
	 * @tparam TContainer Type of the container adapter.
	 * @param container   Reference to the container adapter.
	 * @return Reference to the internal base container.
	 */
	template <class TContainer>
	typename TContainer::container_type& GetBaseContainer(TContainer& container)
	{
		struct Accessor : TContainer
		{
			static auto& Get(TContainer& cont) {
				return cont.*(&Accessor::c);
			}
		};
		return Accessor::Get(container);
	}

	/**
	 * @brief Accesses the internal container of a const container adapter.
	 *
	 * @tparam TContainer Type of the container adapter.
	 * @param container   Const reference to the container adapter.
	 * @return Const reference to the internal base container.
	 */
	template <class TContainer>
	const typename TContainer::container_type& GetBaseContainer(const TContainer& container)
	{
		struct Accessor : TContainer
		{
			static const auto& Get(const TContainer& cont) {
				return cont.*(&Accessor::c);
			}
		};
		return Accessor::Get(container);
	}

} // namespace BitSerializer::Detail
