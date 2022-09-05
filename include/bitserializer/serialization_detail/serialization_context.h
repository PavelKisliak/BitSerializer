/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include <vector>
#include "serialization_options.h"

namespace BitSerializer
{
	using ValidationErrors = std::vector<std::string>;
	using ValidationMap = std::map<std::string, ValidationErrors>;

	/// <summary>
	/// Serialization context - stores all necessary information about current serialization session (options, validation errors).
	/// </summary>
	class SerializationContext
	{
	public:
		[[nodiscard]] bool IsValid() const noexcept								{ return mErrorsMap.empty(); }
		[[nodiscard]] const ValidationMap& GetValidationErrors() const noexcept	{ return mErrorsMap; }

		void OnStartSerialization(const SerializationOptions* serializationOptions)
		{
			mSerializationOptions = serializationOptions;
			mErrorsMap.clear();
		}

		void AddValidationErrors(std::string&& path, ValidationErrors&& validationList)
		{
			auto it = mErrorsMap.find(path);
			if (it == mErrorsMap.end()) {
				mErrorsMap.emplace(std::move(path), std::move(validationList));
			}
			else {
				std::move(validationList.begin(), validationList.end(), std::back_inserter(it->second));
			}
		}

		const SerializationOptions* GetOptions() const noexcept {
			return mSerializationOptions;
		}

	private:
		ValidationMap mErrorsMap;
		const SerializationOptions* mSerializationOptions = nullptr;
	};

	/// <summary>
	/// The serialization context, contains validation information, etc...
	/// </summary>
	thread_local static SerializationContext Context;
}
