/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#if defined __has_include && __has_include(<version>)
#include <version>
#endif
#include <variant>
#include "bitserializer/serialization_detail/serialization_options.h"
#include "bitserializer/serialization_detail/errors_handling.h"

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
				mErrorsMap.try_emplace(std::move(path), ValidationErrors{ std::move(errorMsg) });
			}
			else {
				it->second.push_back(std::move(errorMsg));
			}

			// Immediately throw `ValidationException` when `MaxValidationErrors` is exceeded
			if (mSerializationOptions.maxValidationErrors > 0 && static_cast<size_t>(mSerializationOptions.maxValidationErrors) == mErrorsMap.size())
			{
				OnFinishSerialization();
			}
		}

		void OnFinishSerialization()
		{
			if (!mErrorsMap.empty()) {
				throw ValidationException(std::move(mErrorsMap));
			}
		}

		template <class TString>
		TString& GetStringValueBuffer()
		{
			if (std::holds_alternative<TString>(mStringValueBuffer))
			{
				return std::get<TString>(mStringValueBuffer);
			}
			// Should be initialized only once per serialization session
			return mStringValueBuffer.emplace<TString>();
		}

	private:
		using StringsVariant = std::variant<std::string,
#if defined(__cpp_lib_char8_t)
			std::u8string,
#endif
			std::wstring, std::u16string, std::u32string>;

		StringsVariant mStringValueBuffer;
		ValidationMap mErrorsMap;
		const SerializationOptions& mSerializationOptions;
	};
}
