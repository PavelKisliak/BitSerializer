/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#if defined __has_include && __has_include(<version>)
#include <version>
#endif
#include <variant>
#include "bitserializer/serialization_options.h"
#include "bitserializer/serialization_detail/errors_handling.h"

namespace BitSerializer
{
	/**
	 * @brief Context object that holds shared state during a serialization or deserialization session.
	 *
	 * Contains options, validation errors, and temporary buffers used across the serialization process.
	 */
	class SerializationContext
	{
	public:
		explicit SerializationContext(const SerializationOptions& serializationOptions)
			: mSerializationOptions(serializationOptions)
		{ }

		[[nodiscard]] const SerializationOptions& GetOptions() const noexcept {
			return mSerializationOptions;
		}

		/**
		 * @brief Records a validation error associated with a specific field path.
		 *
		 * If the number of validation errors exceeds the limit specified in the options,
		 * a `ValidationException` will be thrown immediately.
		 *
		 * @param path Path to the field where the error occurred.
		 * @param errorMessage Description of the validation issue.
		 */
		void AddValidationError(std::string path, std::string errorMessage)
		{
			if (const auto it = mErrorsMap.find(path); it == mErrorsMap.end()) {
				mErrorsMap.try_emplace(std::move(path), ValidationErrors{ std::move(errorMessage) });
			}
			else {
				it->second.push_back(std::move(errorMessage));
			}

			// Immediately throw `ValidationException` when `MaxValidationErrors` is exceeded
			if (mSerializationOptions.maxValidationErrors > 0 && static_cast<size_t>(mSerializationOptions.maxValidationErrors) == mErrorsMap.size())
			{
				OnFinishSerialization();
			}
		}

		/**
		 * @brief Finalizes the serialization session and throws an exception if any validation errors were recorded.
		 *
		 * @throws ValidationException if there are any accumulated validation errors.
		 */
		void OnFinishSerialization()
		{
			if (!mErrorsMap.empty()) {
				throw ValidationException(std::move(mErrorsMap));
			}
		}

		/**
		 * @brief Retrieves a string buffer suitable for reading or writing string values.
		 *
		 * The buffer is reused within the same serialization session to reduce allocations.
		 *
		 * @tparam TString Type of string to retrieve or initialize.
		 * @return Reference to the initialized string buffer.
		 */
		template <class TString>
		constexpr TString& GetStringValueBuffer()
		{
			if (std::holds_alternative<TString>(mStringValueBuffer))
			{
				return std::get<TString>(mStringValueBuffer);
			}
			// Should be initialized only once per serialization session
			return mStringValueBuffer.emplace<TString>();
		}

		/**
		 * @brief Determines whether the current stack is being unwound due to an active exception.
		 */
		[[nodiscard]] bool IsStackUnwinding() const noexcept {
			return std::uncaught_exceptions() > mInitialUncaughtCount;
		}

	private:
		/// @brief Variant type supporting multiple string encodings.
		using StringsVariant = std::variant<std::string,
#if defined(__cpp_lib_char8_t)
			std::u8string,
#endif
			std::wstring, std::u16string, std::u32string>;

		StringsVariant mStringValueBuffer;
		ValidationMap mErrorsMap;
		const SerializationOptions& mSerializationOptions;
		int mInitialUncaughtCount = std::uncaught_exceptions();
	};
}
