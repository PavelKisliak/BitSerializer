/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <map>
#include <vector>

namespace BitSerializer
{
	using ValidationErrors = std::vector<std::wstring>;
	using ValidationMap = std::map<std::wstring, ValidationErrors>;

	/// <summary>
	/// Serialization context - stores validation results and something other which would be need in future.
	/// </summary>
	class SerializationContext
	{
	public:
		bool IsValid() const noexcept								{ return mErrorsMap.empty(); }
		const ValidationMap& GetValidationErrors() const noexcept	{ return mErrorsMap; }

		void OnStartSerialization()
		{
			mErrorsMap.clear();
		}

		void AddValidationErrors(const std::wstring& path, ValidationErrors&& validationList)
		{
			auto it = mErrorsMap.find(path);
			if (it == mErrorsMap.end()) {
				mErrorsMap.emplace(path, validationList);
			}
			else {
				std::move(validationList.begin(), validationList.end(), std::back_inserter(it->second));
			}
		}

	private:
		ValidationMap mErrorsMap;
	};

}	// namespace BitSerializer