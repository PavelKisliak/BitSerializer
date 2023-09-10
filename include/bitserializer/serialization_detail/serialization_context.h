/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "serialization_options.h"
#include "errors_handling.h"

namespace BitSerializer
{
	/// <summary>
	/// Serialization context - stores all necessary information about current serialization session (options, validation errors).
	/// </summary>
	class SerializationContext
	{
	public:
		explicit SerializationContext(const SerializationOptions& serializationOptions)
			: mSerializationOptions(serializationOptions)
		{ }

		[[nodiscard]] const SerializationOptions& GetOptions() const noexcept {
			return mSerializationOptions;
		}

		void AddValidationError(std::string path, std::string errorMsg)
		{
			if (const auto it = mErrorsMap.find(path); it == mErrorsMap.end()) {
				mErrorsMap.emplace(std::move(path), ValidationErrors{ std::move(errorMsg) });
			}
			else {
				it->second.push_back(std::move(errorMsg));
			}
		}

		void OnFinishSerialization()
		{
			if (!mErrorsMap.empty()) {
				throw ValidationException(std::move(mErrorsMap));
			}
		}

	private:
		ValidationMap mErrorsMap;
		const SerializationOptions& mSerializationOptions;
	};
}
