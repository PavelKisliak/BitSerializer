/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"

namespace BitSerializer
{
	namespace Detail
	{
		/**
		 * @brief Configuration that customizes the archive stub behavior.
		 *
		 * @tparam TChar          Character type used for keys and strings.
		 * @tparam TArchiveType   Archive type advertised by the stub.
		 * @tparam THasTimestamp  Whether the stub supports binary timestamps.
		 */
		template <typename TChar, ArchiveType TArchiveType, bool THasTimestamp>
		struct ArchiveStubConfig
		{
			using char_type = TChar;
			using key_type = std::basic_string<TChar>;
			using string_view_type = std::basic_string_view<TChar>;

			static constexpr ArchiveType archive_type = TArchiveType;
			static constexpr bool is_binary = (TArchiveType == ArchiveType::MsgPack);
			static constexpr bool has_timestamp = THasTimestamp;
		};

		// Forward declarations
		template <typename TConfig>
		class TestIoData;

		/**
		 * @brief Pointer to an I/O data node.
		 */
		template <typename TConfig>
		using TestIoDataPtr = std::shared_ptr<TestIoData<TConfig>>;

		/**
		 * @brief Represents an object node in the I/O data tree.
		 */
		template <typename TConfig>
		class TestIoDataObject : public std::map<typename TConfig::key_type, TestIoDataPtr<TConfig>>
		{
		};

		template <typename TConfig>
		using TestIoDataObjectPtr = std::shared_ptr<TestIoDataObject<TConfig>>;

		/**
		 * @brief Represents an array node in the I/O data tree.
		 */
		template <typename TConfig>
		class TestIoDataArray : public std::vector<TestIoDataPtr<TConfig>>
		{
		public:
			explicit TestIoDataArray(const std::size_t expectedSize)
			{
				this->reserve(expectedSize);
			}
		};

		template <typename TConfig>
		using TestIoDataArrayPtr = std::shared_ptr<TestIoDataArray<TConfig>>;

		/**
		 * @brief Unified I/O data type that can represent various value types.
		 */
		template <typename TConfig>
		using TestIoDataVariant = std::conditional_t<TConfig::has_timestamp,
			std::variant<std::nullptr_t, bool, int64_t, uint64_t, double, typename TConfig::key_type, CBinTimestamp, TestIoDataObjectPtr<TConfig>, TestIoDataArrayPtr<TConfig>>,
			std::variant<std::nullptr_t, bool, int64_t, uint64_t, double, typename TConfig::key_type, TestIoDataObjectPtr<TConfig>, TestIoDataArrayPtr<TConfig>>>;

		template <typename TConfig>
		class TestIoData : public TestIoDataVariant<TConfig>
		{
		};

		/**
		 * @brief Root container for I/O data used during serialization tests.
		 */
		template <typename TConfig>
		class TestIoDataRoot
		{
		public:
			TestIoDataRoot()
				: data(std::make_shared<TestIoData<TConfig>>())
			{
			}

			TestIoDataPtr<TConfig> data;
		};

		/**
		 * @brief Traits defining properties of the archive stub.
		 */
		template <typename TConfig>
		struct ArchiveStubTraits : public TConfig
		{
			using supported_key_types = TSupportedKeyTypes<typename TConfig::key_type>;
			using preferred_output_type = TestIoDataRoot<TConfig>;
			static constexpr char path_separator = '/';

		protected:
			~ArchiveStubTraits() = default;
		};

		// Forward declarations
		template <typename TConfig, SerializeMode TMode>
		class ArchiveStubObjectScope;

		template <typename TConfig, SerializeMode TMode>
		class ArchiveStubArrayScope;

		/**
		 * @brief Base class for archive scopes used in test stubs.
		 */
		template <typename TConfig>
		class ArchiveStubScopeBase : public ArchiveStubTraits<TConfig>
		{
		public:
			using typename ArchiveStubTraits<TConfig>::key_type;
			using typename ArchiveStubTraits<TConfig>::string_view_type;
			using ArchiveStubTraits<TConfig>::path_separator;

			explicit ArchiveStubScopeBase(TestIoDataPtr<TConfig> node, ArchiveStubScopeBase* parent = nullptr, key_type parentKey = key_type())
				: mNode(std::move(node))
				, mParent(parent)
				, mParentKey(std::move(parentKey))
			{
			}

			/**
			 * @brief Gets the current path within the serialized object graph.
			 */
			[[nodiscard]] virtual std::string GetPath() const
			{
				const std::string localPath = mParentKey.empty()
					? Convert::ToString(mParentKey)
					: path_separator + Convert::ToString(mParentKey);

				return mParent == nullptr ? localPath : mParent->GetPath() + localPath;
			}

		protected:
			~ArchiveStubScopeBase() = default;

			/**
			 * @brief Returns the number of elements stored in this node.
			 */
			[[nodiscard]] std::size_t GetSize() const
			{
				if (std::holds_alternative<TestIoDataObjectPtr<TConfig>>(*mNode)) {
					return std::get<TestIoDataObjectPtr<TConfig>>(*mNode)->size();
				}
				if (std::holds_alternative<TestIoDataArrayPtr<TConfig>>(*mNode)) {
					return std::get<TestIoDataArrayPtr<TConfig>>(*mNode)->size();
				}
				return 0;
			}

			/**
			 * @brief Loads a fundamental value from I/O data with type conversion policy.
			 */
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool LoadFundamentalValue(const TestIoData<TConfig>& ioData, T& value, const SerializationOptions& options)
			{
				if (std::holds_alternative<std::nullptr_t>(ioData)) {
					return std::is_null_pointer_v<T>;
				}

				using Detail::ConvertByPolicy;

				if constexpr (std::is_integral_v<T>)
				{
					if (std::holds_alternative<int64_t>(ioData)) {
						return ConvertByPolicy(std::get<int64_t>(ioData), value, options.mismatchedTypesPolicy, options.overflowNumberPolicy);
					}
					if (std::holds_alternative<uint64_t>(ioData)) {
						return ConvertByPolicy(std::get<uint64_t>(ioData), value, options.mismatchedTypesPolicy, options.overflowNumberPolicy);
					}
					if (std::holds_alternative<bool>(ioData)) {
						return ConvertByPolicy(std::get<bool>(ioData), value, options.mismatchedTypesPolicy, options.overflowNumberPolicy);
					}
				}
				else if constexpr (std::is_floating_point_v<T>)
				{
					if (std::holds_alternative<double>(ioData)) {
						return ConvertByPolicy(std::get<double>(ioData), value, options.mismatchedTypesPolicy, options.overflowNumberPolicy);
					}
				}

				// NULL value deserialized from the archive is excluded from MismatchedTypesPolicy processing
				if (!std::holds_alternative<std::nullptr_t>(ioData) && options.mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"The type of target field does not match the value being loaded");
				}
				return false;
			}

			/**
			 * @brief Saves a fundamental value into I/O data.
			 */
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			void SaveFundamentalValue(TestIoData<TConfig>& ioData, T& value)
			{
				if constexpr (std::is_same_v<T, bool>) {
					ioData.template emplace<bool>(value);
				}
				else if constexpr (std::is_integral_v<T>)
				{
					if constexpr (std::is_signed_v<T>) {
						ioData.template emplace<int64_t>(value);
					}
					else {
						ioData.template emplace<uint64_t>(value);
					}
				}
				else if constexpr (std::is_floating_point_v<T>) {
					ioData.template emplace<double>(value);
				}
				else if constexpr (std::is_null_pointer_v<T>) {
					ioData.template emplace<std::nullptr_t>(value);
				}
			}

			/**
			 * @brief Loads a string value from I/O data.
			 */
			static bool LoadString(const TestIoData<TConfig>& ioData, string_view_type& value, const SerializationOptions& options)
			{
				if (std::holds_alternative<key_type>(ioData))
				{
					value = std::get<key_type>(ioData);
					return true;
				}

				// NULL value deserialized from the archive is excluded from MismatchedTypesPolicy processing
				if (!std::holds_alternative<std::nullptr_t>(ioData) && options.mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"The type of target field does not match the value being loaded");
				}
				return false;
			}

			/**
			 * @brief Saves a string value into I/O data.
			 */
			static void SaveString(TestIoData<TConfig>& ioData, string_view_type& value)
			{
				ioData.template emplace<key_type>(value);
			}

			TestIoDataPtr<TConfig> mNode;
			ArchiveStubScopeBase* mParent;
			key_type mParentKey;
		};

		/**
		 * @brief Scope for handling arrays (sequences of values without keys).
		 */
		template <typename TConfig, SerializeMode TMode>
		class ArchiveStubArrayScope final : public ArchiveScope<TMode>, public ArchiveStubScopeBase<TConfig>
		{
		public:
			using typename ArchiveStubScopeBase<TConfig>::key_type;
			using typename ArchiveStubScopeBase<TConfig>::string_view_type;
			using ArchiveStubScopeBase<TConfig>::path_separator;

			ArchiveStubArrayScope(TestIoDataPtr<TConfig> node, SerializationContext& context, ArchiveStubScopeBase<TConfig>* parent = nullptr, const key_type& parentKey = key_type())
				: ArchiveScope<TMode>(context)
				, ArchiveStubScopeBase<TConfig>(std::move(node), parent, parentKey)
			{
				assert(std::holds_alternative<TestIoDataArrayPtr<TConfig>>(*this->mNode));
			}

			ArchiveStubArrayScope(ScopeUnopened, SerializationContext& context)
				: ArchiveScope<TMode>(context, ScopeUnopened{})
				, ArchiveStubScopeBase<TConfig>(nullptr)
			{
			}

			/**
			 * @brief Gets the current path including index position in the array.
			 */
			[[nodiscard]] std::string GetPath() const override
			{
				return ArchiveStubScopeBase<TConfig>::GetPath() + path_separator + Convert::ToString(mIndex);
			}

			/**
			 * @brief Checks whether all items have been processed.
			 */
			[[nodiscard]] bool IsEnd() const
			{
				static_assert(TMode == SerializeMode::Load);
				return mIndex == std::get<TestIoDataArrayPtr<TConfig>>(*this->mNode)->size();
			}

			/**
			 * @brief Serializes a string value at the current array position.
			 */
			bool SerializeValue(string_view_type& value)
			{
				if (TestIoDataPtr<TConfig> ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load) {
						return this->LoadString(*ioData, value, this->GetOptions());
					}
					else
					{
						this->SaveString(*ioData, value);
						return true;
					}
				}
				return false;
			}

			/**
			 * @brief Serializes a fundamental value at the current array position.
			 */
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool SerializeValue(T& value)
			{
				if (TestIoDataPtr<TConfig> ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load) {
						return this->LoadFundamentalValue(*ioData, value, this->GetOptions());
					}
					else
					{
						this->SaveFundamentalValue(*ioData, value);
						return true;
					}
				}
				return false;
			}

			/**
			 * @brief Serializes a timestamp value at the current array position.
			 */
			template <typename T, std::enable_if_t<TConfig::has_timestamp && std::is_same_v<T, CBinTimestamp>, int> = 0>
			bool SerializeValue(T& value)
			{
				if (TestIoDataPtr<TConfig> ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load)
					{
						if (std::holds_alternative<CBinTimestamp>(*ioData))
						{
							value = std::get<CBinTimestamp>(*ioData);
							return true;
						}
						return false;
					}
					else
					{
						ioData->template emplace<CBinTimestamp>(value);
						return true;
					}
				}
				return false;
			}

			/**
			 * @brief Opens a nested object scope.
			 */
			ArchiveStubObjectScope<TConfig, TMode> OpenObjectScope(size_t)
			{
				if (TestIoDataPtr<TConfig> ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load)
					{
						if (std::holds_alternative<TestIoDataObjectPtr<TConfig>>(*ioData)) {
							return ArchiveStubObjectScope<TConfig, TMode>(ioData, ArchiveScope<TMode>::GetContext(), this);
						}
					}
					else
					{
						ioData->template emplace<TestIoDataObjectPtr<TConfig>>(std::make_shared<TestIoDataObject<TConfig>>());
						return ArchiveStubObjectScope<TConfig, TMode>(ioData, ArchiveScope<TMode>::GetContext(), this);
					}
				}
				return ArchiveStubObjectScope<TConfig, TMode>(ScopeUnopened{}, ArchiveScope<TMode>::GetContext());
			}

			/**
			 * @brief Opens a nested array scope.
			 */
			ArchiveStubArrayScope<TConfig, TMode> OpenArrayScope(size_t arraySize)
			{
				if (TestIoDataPtr<TConfig> ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load)
					{
						if (std::holds_alternative<TestIoDataArrayPtr<TConfig>>(*ioData)) {
							return ArchiveStubArrayScope<TConfig, TMode>(ioData, ArchiveScope<TMode>::GetContext(), this);
						}
					}
					else
					{
						ioData->template emplace<TestIoDataArrayPtr<TConfig>>(std::make_shared<TestIoDataArray<TConfig>>(arraySize));
						return ArchiveStubArrayScope<TConfig, TMode>(ioData, ArchiveScope<TMode>::GetContext(), this);
					}
				}
				return ArchiveStubArrayScope<TConfig, TMode>(ScopeUnopened{}, ArchiveScope<TMode>::GetContext());
			}

		protected:
			/**
			 * @brief Loads or creates the next item in the array.
			 */
			TestIoDataPtr<TConfig> LoadNextItem()
			{
				auto& archiveArray = std::get<TestIoDataArrayPtr<TConfig>>(*this->mNode);
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < this->GetSize()) {
						return archiveArray->at(mIndex++);
					}
					throw SerializationException(SerializationErrorCode::OutOfRange, "No more items to load");
				}
				else
				{
					mIndex++;
					return archiveArray->emplace_back(std::make_shared<TestIoData<TConfig>>());
				}
			}

		private:
			std::size_t mIndex = 0;
		};

		/**
		 * @brief Scope for handling objects (key-value pairs).
		 */
		template <typename TConfig, SerializeMode TMode>
		class ArchiveStubObjectScope final : public ArchiveScope<TMode>, public ArchiveStubScopeBase<TConfig>
		{
		public:
			using typename ArchiveStubScopeBase<TConfig>::key_type;
			using typename ArchiveStubScopeBase<TConfig>::string_view_type;
			using ArchiveStubScopeBase<TConfig>::path_separator;

			ArchiveStubObjectScope(TestIoDataPtr<TConfig> node, SerializationContext& context, ArchiveStubScopeBase<TConfig>* parent = nullptr, const key_type& parentKey = key_type())
				: ArchiveScope<TMode>(context)
				, ArchiveStubScopeBase<TConfig>(std::move(node), parent, parentKey)
			{
				assert(std::holds_alternative<TestIoDataObjectPtr<TConfig>>(*this->mNode));
			}

			ArchiveStubObjectScope(ScopeUnopened, SerializationContext& context)
				: ArchiveScope<TMode>(context, ScopeUnopened{})
				, ArchiveStubScopeBase<TConfig>(nullptr)
			{
			}

			/**
			 * @brief Returns the estimated number of items to load (for reserving containers).
			 */
			[[nodiscard]] std::size_t GetEstimatedSize() const
			{
				return GetAsObject()->size();
			}

			/**
			 * @brief Enumerates all keys in the current object scope.
			 */
			template <typename TCallback>
			void VisitKeys(TCallback&& fn)
			{
				for (auto& keyValue : *GetAsObject()) {
					fn(keyValue.first);
				}
			}

			/**
			 * @brief Serializes a string value associated with the given key.
			 */
			bool SerializeValue(const key_type& key, string_view_type& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto archiveValue = LoadArchiveValueByKey(key);
					return archiveValue == nullptr ? false : this->LoadString(*archiveValue, value, this->GetOptions());
				}
				else
				{
					auto ioData = AddArchiveValue(key);
					this->SaveString(*ioData, value);
					return true;
				}
			}

			/**
			 * @brief Serializes a fundamental value associated with the given key.
			 */
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool SerializeValue(const key_type& key, T& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto archiveValue = LoadArchiveValueByKey(key);
					return archiveValue == nullptr ? false : this->LoadFundamentalValue(*archiveValue, value, this->GetOptions());
				}
				else
				{
					this->SaveFundamentalValue(*AddArchiveValue(key), value);
					return true;
				}
			}

			/**
			 * @brief Serializes a timestamp value associated with the given key.
			 */
			template <typename T, std::enable_if_t<TConfig::has_timestamp && std::is_same_v<T, CBinTimestamp>, int> = 0>
			bool SerializeValue(const key_type& key, T& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (const auto archiveValue = LoadArchiveValueByKey(key))
					{
						value = std::get<CBinTimestamp>(*archiveValue);
						return true;
					}
					return false;
				}
				else
				{
					AddArchiveValue(key)->template emplace<CBinTimestamp>(value);
					return true;
				}
			}

			/**
			 * @brief Opens a nested object scope for the specified key.
			 */
			ArchiveStubObjectScope<TConfig, TMode> OpenObjectScope(const key_type& key, size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					auto archiveValue = LoadArchiveValueByKey(key);
					if (archiveValue != nullptr && std::holds_alternative<TestIoDataObjectPtr<TConfig>>(*archiveValue)) {
						return ArchiveStubObjectScope<TConfig, TMode>(archiveValue, ArchiveScope<TMode>::GetContext(), this, key);
					}
					return ArchiveStubObjectScope<TConfig, TMode>(ScopeUnopened{}, ArchiveScope<TMode>::GetContext());
				}
				else
				{
					TestIoDataPtr<TConfig> ioData = AddArchiveValue(key);
					ioData->template emplace<TestIoDataObjectPtr<TConfig>>(std::make_shared<TestIoDataObject<TConfig>>());
					return ArchiveStubObjectScope<TConfig, TMode>(ioData, ArchiveScope<TMode>::GetContext(), this, key);
				}
			}

			/**
			 * @brief Opens a nested array scope for the specified key.
			 */
			ArchiveStubArrayScope<TConfig, TMode> OpenArrayScope(const key_type& key, size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					auto archiveValue = LoadArchiveValueByKey(key);
					if (archiveValue != nullptr && std::holds_alternative<TestIoDataArrayPtr<TConfig>>(*archiveValue)) {
						return ArchiveStubArrayScope<TConfig, TMode>(archiveValue, ArchiveScope<TMode>::GetContext(), this, key);
					}
					return ArchiveStubArrayScope<TConfig, TMode>(ScopeUnopened{}, ArchiveScope<TMode>::GetContext());
				}
				else
				{
					TestIoDataPtr<TConfig> ioData = AddArchiveValue(key);
					ioData->template emplace<TestIoDataArrayPtr<TConfig>>(std::make_shared<TestIoDataArray<TConfig>>(arraySize));
					return ArchiveStubArrayScope<TConfig, TMode>(ioData, ArchiveScope<TMode>::GetContext(), this, key);
				}
			}

		protected:
			[[nodiscard]] TestIoDataObjectPtr<TConfig>& GetAsObject() const
			{
				return std::get<TestIoDataObjectPtr<TConfig>>(*this->mNode);
			}

			[[nodiscard]] TestIoDataPtr<TConfig> LoadArchiveValueByKey(const key_type& key)
			{
				const auto& archiveObject = GetAsObject();
				const auto it = archiveObject->find(key);
				return it == archiveObject->end() ? nullptr : it->second;
			}

			[[nodiscard]] TestIoDataPtr<TConfig> AddArchiveValue(const key_type& key) const
			{
				const auto archiveObject = GetAsObject();
				decltype(auto) result = archiveObject->emplace(key, std::make_shared<TestIoData<TConfig>>());
				return result.first->second;
			}
		};

		/**
		 * @brief The root scope for serializing one value, array, or object without a key.
		 */
		template <typename TConfig, SerializeMode TMode>
		class ArchiveStubRootScope final : public ArchiveScope<TMode>, public ArchiveStubScopeBase<TConfig>
		{
		public:
			using typename ArchiveStubScopeBase<TConfig>::key_type;
			using typename ArchiveStubScopeBase<TConfig>::string_view_type;
			using ArchiveStubScopeBase<TConfig>::path_separator;

			ArchiveStubRootScope(const TestIoDataRoot<TConfig>& inputData, SerializationContext& context)
				: ArchiveScope<TMode>(context)
				, ArchiveStubScopeBase<TConfig>(inputData.data)
				, mOutputData(nullptr)
				, mInputData(&inputData)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
			}

			ArchiveStubRootScope(TestIoDataRoot<TConfig>& outputData, SerializationContext& context)
				: ArchiveScope<TMode>(context)
				, ArchiveStubScopeBase<TConfig>(outputData.data)
				, mOutputData(&outputData)
				, mInputData(nullptr)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
			}

			/**
			 * @brief Finalizes the root scope after serialization completes.
			 */
			void Finalize()
			{
			}

			/**
			 * @brief Serializes a fundamental value at the root level.
			 */
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool SerializeValue(T& value)
			{
				if constexpr (TMode == SerializeMode::Load) {
					return this->LoadFundamentalValue(*mInputData->data, value, this->GetOptions());
				}
				else
				{
					this->SaveFundamentalValue(*mOutputData->data, value);
					return true;
				}
			}

			/**
			 * @brief Serializes a string value at the root level.
			 */
			bool SerializeValue(string_view_type& value)
			{
				if constexpr (TMode == SerializeMode::Load) {
					return this->LoadString(*mInputData->data, value, this->GetOptions());
				}
				else
				{
					this->SaveString(*mOutputData->data, value);
					return true;
				}
			}

			/**
			 * @brief Serializes a timestamp value at the root level.
			 */
			template <typename T, std::enable_if_t<TConfig::has_timestamp && std::is_same_v<T, CBinTimestamp>, int> = 0>
			bool SerializeValue(T& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (std::holds_alternative<CBinTimestamp>(*mInputData->data))
					{
						value = std::get<CBinTimestamp>(*mInputData->data);
						return true;
					}
					return false;
				}
				else
				{
					mOutputData->data->template emplace<CBinTimestamp>(value);
					return true;
				}
			}

			/**
			 * @brief Opens a nested object scope at the root level.
			 */
			ArchiveStubObjectScope<TConfig, TMode> OpenObjectScope(size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (std::holds_alternative<TestIoDataObjectPtr<TConfig>>(*mInputData->data)) {
						return ArchiveStubObjectScope<TConfig, TMode>(mInputData->data, ArchiveScope<TMode>::GetContext());
					}
				}
				else
				{
					mOutputData->data->template emplace<TestIoDataObjectPtr<TConfig>>(std::make_shared<TestIoDataObject<TConfig>>());
					return ArchiveStubObjectScope<TConfig, TMode>(mOutputData->data, ArchiveScope<TMode>::GetContext());
				}
				return ArchiveStubObjectScope<TConfig, TMode>(ScopeUnopened{}, ArchiveScope<TMode>::GetContext());
			}

			/**
			 * @brief Opens a nested array scope at the root level.
			 */
			ArchiveStubArrayScope<TConfig, TMode> OpenArrayScope(size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (std::holds_alternative<TestIoDataArrayPtr<TConfig>>(*mInputData->data)) {
						return ArchiveStubArrayScope<TConfig, TMode>(mInputData->data, ArchiveScope<TMode>::GetContext());
					}
				}
				else
				{
					mOutputData->data->template emplace<TestIoDataArrayPtr<TConfig>>(std::make_shared<TestIoDataArray<TConfig>>(arraySize));
					return ArchiveStubArrayScope<TConfig, TMode>(mOutputData->data, ArchiveScope<TMode>::GetContext());
				}
				return ArchiveStubArrayScope<TConfig, TMode>(ScopeUnopened{}, ArchiveScope<TMode>::GetContext());
			}

		private:
			TestIoDataRoot<TConfig>* mOutputData;
			const TestIoDataRoot<TConfig>* mInputData;
		};

		/**
		 * @brief Configuration of the text (non-binary) archive stub.
		 */
		using ArchiveStubTextConfig = ArchiveStubConfig<wchar_t, ArchiveType::Json, false>;

		/**
		 * @brief Configuration of the binary archive stub.
		 */
		using ArchiveStubBinaryConfig = ArchiveStubConfig<char, ArchiveType::MsgPack, true>;

		// Convenience aliases for the text archive stub I/O data types
		using ArchiveStubTextIoDataPtr = TestIoDataPtr<ArchiveStubTextConfig>;
		using ArchiveStubTextIoDataObjectPtr = TestIoDataObjectPtr<ArchiveStubTextConfig>;

		// Convenience aliases for the binary archive stub I/O data types
		using ArchiveStubBinaryIoDataPtr = TestIoDataPtr<ArchiveStubBinaryConfig>;
		using ArchiveStubBinaryIoDataObjectPtr = TestIoDataObjectPtr<ArchiveStubBinaryConfig>;

	} // namespace Detail

	/**
	 * @brief Declaration of the text archive stub used in unit tests.
	 */
	using ArchiveStub = ArchiveBase<
		Detail::ArchiveStubTraits<Detail::ArchiveStubTextConfig>,
		Detail::ArchiveStubRootScope<Detail::ArchiveStubTextConfig, SerializeMode::Load>,
		Detail::ArchiveStubRootScope<Detail::ArchiveStubTextConfig, SerializeMode::Save>>;

	/**
	 * @brief Declaration of the binary archive stub used in unit tests.
	 */
	using BinArchiveStub = ArchiveBase<
		Detail::ArchiveStubTraits<Detail::ArchiveStubBinaryConfig>,
		Detail::ArchiveStubRootScope<Detail::ArchiveStubBinaryConfig, SerializeMode::Load>,
		Detail::ArchiveStubRootScope<Detail::ArchiveStubBinaryConfig, SerializeMode::Save>>;

} // namespace BitSerializer
