/*******************************************************************************
* Copyright (C) 2018-2021 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include <vector>

namespace BitSerializer
{
	using ValidationErrors = std::vector<std::string>;
	using ValidationMap = std::map<std::string, ValidationErrors>;

	/// <summary>
	/// Serialization context - stores validation results and something other which would be need in future.
	/// </summary>
	class SerializationContext
	{
	public:
		[[nodiscard]] bool IsValid() const noexcept								{ return mErrorsMap.empty(); }
		[[nodiscard]] const ValidationMap& GetValidationErrors() const noexcept	{ return mErrorsMap; }

		void OnStartSerialization()
		{
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

	private:
		ValidationMap mErrorsMap;
	};

	/// <summary>
	/// The serialization context, contains validation information, etc...
	/// </summary>
	thread_local static SerializationContext Context;
}