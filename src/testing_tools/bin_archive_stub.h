/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <string>
#include <map>
#include <memory>
#include <variant>
#include <optional>
#include <type_traits>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/archive_base.h"

namespace BitSerializer {
	namespace Detail {

		/**
		 * @brief Input/output data structure used by the binary archive stub.
		 */
		class BinTestIoData;
		using BinTestIoDataPtr = std::shared_ptr<BinTestIoData>;

		/**
		 * @brief Represents an object node in the binary archive I/O data tree.
		 */
		class BinTestIoDataObject : public std::map<std::string, BinTestIoDataPtr> {};

		using BinTestIoDataObjectPtr = std::shared_ptr<BinTestIoDataObject>;

		/**
		 * @brief Represents an array node in the binary archive I/O data tree.
		 */
		class BinTestIoDataArray : public std::vector<BinTestIoDataPtr>
		{
		public:
			explicit BinTestIoDataArray(const size_t expectedSize) {
				reserve(expectedSize);
			}
		};

		using BinTestIoDataArrayPtr = std::shared_ptr<BinTestIoDataArray>;

		/**
		 * @brief Unified I/O data type that can represent various value types in binary format.
		 */
		class BinTestIoData
			: public std::variant<std::nullptr_t, bool, int64_t, uint64_t, double, std::string, CBinTimestamp, BinTestIoDataObjectPtr, BinTestIoDataArrayPtr>
		{
		};

		/**
		 * @brief Root container for binary I/O data used during serialization tests.
		 */
		class BinTestIoDataRoot
		{
		public:
			BinTestIoDataRoot()
				: Data(std::make_shared<BinTestIoData>())
			{
			}

			BinTestIoDataPtr Data;
		};

		/**
		 * @brief Traits defining properties of the binary archive stub.
		 */
		struct BinArchiveStubTraits
		{
			static constexpr ArchiveType archive_type = ArchiveType::Json;
			using key_type = std::string;
			using supported_key_types = TSupportedKeyTypes<std::string>;
			using string_view_type = std::string_view;
			using preferred_output_format = BinTestIoDataRoot;
			static constexpr char path_separator = '/';
			static constexpr bool is_binary = true;

		protected:
			~BinArchiveStubTraits() = default;
		};

		// Forward declarations
		template <SerializeMode TMode>
		class BinArchiveStubObjectScope;

		/**
		 * @brief Base class for binary archive scopes used in test stubs.
		 */
		class BinArchiveStubScopeBase : public BinArchiveStubTraits
		{
		public:
			explicit BinArchiveStubScopeBase(BinTestIoDataPtr node, BinArchiveStubScopeBase* parent = nullptr, key_type parentKey = key_type())
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
			~BinArchiveStubScopeBase() = default;

			/**
			 * @brief Returns the number of elements stored in this node.
			 */
			[[nodiscard]] size_t GetSize() const
			{
				if (std::holds_alternative<BinTestIoDataObjectPtr>(*mNode)) {
					return std::get<BinTestIoDataObjectPtr>(*mNode)->size();
				}
				if (std::holds_alternative<BinTestIoDataArrayPtr>(*mNode)) {
					return std::get<BinTestIoDataArrayPtr>(*mNode)->size();
				}
				return 0;
			}

			/**
			 * @brief Loads a fundamental value from binary I/O data with type conversion policy.
			 */
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool LoadFundamentalValue(const BinTestIoData& ioData, T& value, const SerializationOptions& serializationOptions)
			{
				// Null value is excluded from MismatchedTypesPolicy processing
				if (std::holds_alternative<std::nullptr_t>(ioData)) {
					return std::is_null_pointer_v<T>;
				}

				using Detail::ConvertByPolicy;

				if constexpr (std::is_integral_v<T>)
				{
					if (std::holds_alternative<int64_t>(ioData)) {
						return ConvertByPolicy(std::get<int64_t>(ioData), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
					}
					if (std::holds_alternative<uint64_t>(ioData)) {
						return ConvertByPolicy(std::get<uint64_t>(ioData), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
					}
					if (std::holds_alternative<bool>(ioData)) {
						return ConvertByPolicy(std::get<bool>(ioData), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
					}
				}
				else if constexpr (std::is_floating_point_v<T>)
				{
					if (std::holds_alternative<double>(ioData)) {
						return ConvertByPolicy(std::get<double>(ioData), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
					}
				}

				if (serializationOptions.mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"The type of target field does not match the value being loaded");
				}
				return false;
			}

			/**
			 * @brief Saves a fundamental value into binary I/O data.
			 */
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			void SaveFundamentalValue(BinTestIoData& ioData, T& value)
			{
				if constexpr (std::is_same_v<T, bool>) {
					ioData.emplace<bool>(value);
				}
				else if constexpr (std::is_integral_v<T>)
				{
					if constexpr (std::is_signed_v<T>) {
						ioData.emplace<int64_t>(value);
					}
					else {
						ioData.emplace<uint64_t>(value);
					}
				}
				else if constexpr (std::is_floating_point_v<T>) {
					ioData.emplace<double>(value);
				}
				else if constexpr (std::is_null_pointer_v<T>) {
					ioData.emplace<std::nullptr_t>(value);
				}
			}

			/**
			 * @brief Loads a string value from binary I/O data.
			 */
			static bool LoadString(const BinTestIoData& ioData, string_view_type& value)
			{
				if (!std::holds_alternative<key_type>(ioData)) {
					return false;
				}
				value = std::get<key_type>(ioData);
				return true;
			}

			/**
			 * @brief Saves a string value into binary I/O data.
			 */
			static void SaveString(BinTestIoData& ioData, string_view_type& value)
			{
				ioData.emplace<key_type>(value);
			}

			BinTestIoDataPtr mNode;
			BinArchiveStubScopeBase* mParent;
			key_type mParentKey;
		};

		/**
		 * @brief Scope for handling arrays in binary format (sequence of values without keys).
		 */
		template <SerializeMode TMode>
		class BinArchiveStubArrayScope final : public TArchiveScope<TMode>, public BinArchiveStubScopeBase
		{
		public:
			BinArchiveStubArrayScope(BinTestIoDataPtr node, SerializationContext& context, BinArchiveStubScopeBase* parent = nullptr, const key_type& parentKey = key_type())
				: TArchiveScope<TMode>(context)
				, BinArchiveStubScopeBase(std::move(node), parent, parentKey)
				, mIndex(0)
			{
				assert(std::holds_alternative<BinTestIoDataArrayPtr>(*mNode));
			}

			/**
			 * @brief Returns the estimated number of items to load (for reserving containers).
			 */
			[[nodiscard]] size_t GetEstimatedSize() const
			{
				return 0;
			}

			/**
			 * @brief Gets the current path including index position in the array.
			 */
			[[nodiscard]] std::string GetPath() const override
			{
				return BinArchiveStubScopeBase::GetPath() + path_separator + Convert::ToString(mIndex);
			}

			/**
			 * @brief Checks whether all items have been processed.
			 */
			[[nodiscard]] bool IsEnd() const
			{
				static_assert(TMode == SerializeMode::Load);
				return mIndex == std::get<BinTestIoDataArrayPtr>(*mNode)->size();
			}

			/**
			 * @brief Serializes a string value at the current array position.
			 */
			bool SerializeValue(string_view_type& value)
			{
				if (BinTestIoDataPtr ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load) {
						return LoadString(*ioData, value);
					}
					else {
						SaveString(*ioData, value);
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
				if (BinTestIoDataPtr ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load) {
						return LoadFundamentalValue(*ioData, value, this->GetOptions());
					}
					else {
						SaveFundamentalValue(*ioData, value);
						return true;
					}
				}
				return false;
			}

			/**
			 * @brief Serializes a timestamp value at the current array position.
			 */
			bool SerializeValue(CBinTimestamp& value)
			{
				if (BinTestIoDataPtr ioData = LoadNextItem())
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
						ioData->emplace<CBinTimestamp>(value);
						return true;
					}
				}
				return false;
			}

			/**
			 * @brief Opens a nested object scope.
			 */
			std::optional<BinArchiveStubObjectScope<TMode>> OpenObjectScope(size_t)
			{
				if (BinTestIoDataPtr ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load)
					{
						return std::holds_alternative<BinTestIoDataObjectPtr>(*ioData)
							? std::make_optional<BinArchiveStubObjectScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this)
							: std::nullopt;
					}
					else
					{
						ioData->emplace<BinTestIoDataObjectPtr>(std::make_shared<BinTestIoDataObject>());
						return std::make_optional<BinArchiveStubObjectScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this);
					}
				}
				return std::nullopt;
			}

			/**
			 * @brief Opens a nested array scope.
			 */
			std::optional<BinArchiveStubArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if (BinTestIoDataPtr ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load)
					{
						return std::holds_alternative<BinTestIoDataArrayPtr>(*ioData)
							? std::make_optional<BinArchiveStubArrayScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this)
							: std::nullopt;
					}
					else
					{
						ioData->emplace<BinTestIoDataArrayPtr>(std::make_shared<BinTestIoDataArray>(arraySize));
						return std::make_optional<BinArchiveStubArrayScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this);
					}
				}
				return std::nullopt;
			}

		protected:
			/**
			 * @brief Loads or creates the next item in the array.
			 */
			BinTestIoDataPtr LoadNextItem()
			{
				auto& archiveArray = std::get<BinTestIoDataArrayPtr>(*mNode);
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < GetSize()) {
						return archiveArray->at(mIndex++);
					}
					throw SerializationException(SerializationErrorCode::OutOfRange, "No more items to load");
				}
				else
				{
					mIndex++;
					return archiveArray->emplace_back(std::make_shared<BinTestIoData>());
				}
			}

		private:
			size_t mIndex;
		};

		/**
		 * @brief Scope for handling objects in binary format (key-value pairs).
		 */
		template <SerializeMode TMode>
		class BinArchiveStubObjectScope final : public TArchiveScope<TMode>, public BinArchiveStubScopeBase
		{
		public:
			BinArchiveStubObjectScope(BinTestIoDataPtr node, SerializationContext& context, BinArchiveStubScopeBase* parent = nullptr, const key_type& parentKey = key_type())
				: TArchiveScope<TMode>(context)
				, BinArchiveStubScopeBase(std::move(node), parent, parentKey)
			{
				assert(std::holds_alternative<BinTestIoDataObjectPtr>(*mNode));
			}

			/**
			 * @brief Returns the estimated number of items to load (for reserving containers).
			 */
			[[nodiscard]] size_t GetEstimatedSize() const
			{
				return GetAsObject()->size();
			}

			/**
			 * @brief Enumerates all keys by invoking the provided callback.
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
					return archiveValue == nullptr ? false : LoadString(*archiveValue, value);
				}
				else
				{
					auto ioData = AddArchiveValue(key);
					SaveString(*ioData, value);
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
					return archiveValue == nullptr ? false : LoadFundamentalValue(*archiveValue, value, this->GetOptions());
				}
				else
				{
					SaveFundamentalValue(*AddArchiveValue(key), value);
					return true;
				}
			}

			/**
			 * @brief Serializes a timestamp value associated with the given key.
			 */
			bool SerializeValue(const key_type& key, CBinTimestamp& value)
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
			std::optional<BinArchiveStubObjectScope<TMode>> OpenObjectScope(const key_type& key, size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					auto archiveValue = LoadArchiveValueByKey(key);
					if (archiveValue != nullptr && std::holds_alternative<BinTestIoDataObjectPtr>(*archiveValue)) {
						return std::make_optional<BinArchiveStubObjectScope<TMode>>(archiveValue, TArchiveScope<TMode>::GetContext(), this, key);
					}
					return std::nullopt;
				}
				else
				{
					auto ioData = AddArchiveValue(key);
					ioData->template emplace<BinTestIoDataObjectPtr>(std::make_shared<BinTestIoDataObject>());
					return std::make_optional<BinArchiveStubObjectScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this, key);
				}
			}

			/**
			 * @brief Opens a nested array scope for the specified key.
			 */
			std::optional<BinArchiveStubArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					auto archiveValue = LoadArchiveValueByKey(key);
					if (archiveValue != nullptr && std::holds_alternative<BinTestIoDataArrayPtr>(*archiveValue)) {
						return std::make_optional<BinArchiveStubArrayScope<TMode>>(archiveValue, TArchiveScope<TMode>::GetContext(), this, key);
					}
					return std::nullopt;
				}
				else
				{
					BinTestIoDataPtr ioData = AddArchiveValue(key);
					ioData->emplace<BinTestIoDataArrayPtr>(std::make_shared<BinTestIoDataArray>(arraySize));
					return std::make_optional<BinArchiveStubArrayScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this, key);
				}
			}

		protected:
			[[nodiscard]] constexpr BinTestIoDataObjectPtr& GetAsObject() const
			{
				return std::get<BinTestIoDataObjectPtr>(*mNode);
			}

			[[nodiscard]] BinTestIoDataPtr LoadArchiveValueByKey(const key_type& key)
			{
				const auto archiveObject = GetAsObject();
				const auto it = archiveObject->find(key);
				return it == archiveObject->end() ? nullptr : it->second;
			}

			[[nodiscard]] BinTestIoDataPtr AddArchiveValue(const key_type& key) const
			{
				const auto archiveObject = GetAsObject();
				decltype(auto) result = archiveObject->emplace(key, std::make_shared<BinTestIoData>());
				return result.first->second;
			}

			template <class IoDataType, class SourceType>
			[[nodiscard]] BinTestIoDataPtr& SaveArchiveValue(const key_type& key, const SourceType& value)
			{
				const auto& archiveObject = GetAsObject();
				BinTestIoData ioData;
				ioData.emplace<IoDataType>(value);
				decltype(auto) result = archiveObject->emplace(key, std::make_shared<BinTestIoData>(std::move(ioData)));
				return result.first->second;
			}
		};

		/**
		 * @brief The root scope for serializing one value, array, or object without a key.
		 */
		template <SerializeMode TMode>
		class BinArchiveStubRootScope final : public TArchiveScope<TMode>, public BinArchiveStubScopeBase
		{
		public:
			BinArchiveStubRootScope(const BinTestIoDataRoot& inputData, SerializationContext& context)
				: TArchiveScope<TMode>(context)
				, BinArchiveStubScopeBase(inputData.Data)
				, mOutputData(nullptr)
				, mInputData(&inputData)
			{
				static_assert(TMode == SerializeMode::Load, "This data type can be used only in 'Load' mode.");
			}

			BinArchiveStubRootScope(BinTestIoDataRoot& outputData, SerializationContext& context)
				: TArchiveScope<TMode>(context)
				, BinArchiveStubScopeBase(outputData.Data)
				, mOutputData(&outputData)
				, mInputData(nullptr)
			{
				static_assert(TMode == SerializeMode::Save, "This data type can be used only in 'Save' mode.");
			}

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
					return LoadFundamentalValue(*mInputData->Data, value, this->GetOptions());
				}
			else
			{
					SaveFundamentalValue(*mOutputData->Data, value);
					return true;
				}
			}

			/**
			 * @brief Serializes a string value at the root level.
			 */
			template <typename TSym, typename TAllocator>
			bool SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
			{
				if constexpr (TMode == SerializeMode::Load) {
					return LoadString(*mInputData->Data, value);
				}
				else {
					SaveString(*mOutputData->Data, value);
					return true;
				}
			}

			/**
			 * @brief Serializes a timestamp value at the root level.
			 */
			bool SerializeValue(CBinTimestamp& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (std::holds_alternative<CBinTimestamp>(*mInputData->Data))
					{
						value = std::get<CBinTimestamp>(*mInputData->Data);
						return true;
					}
					return false;
				}
				else
				{
					mOutputData->Data->emplace<CBinTimestamp>(value);
					return true;
				}
			}

			/**
			 * @brief Opens a nested object scope at the root level.
			 */
			std::optional<BinArchiveStubObjectScope<TMode>> OpenObjectScope(size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					return std::holds_alternative<BinTestIoDataObjectPtr>(*mInputData->Data)
						? std::make_optional<BinArchiveStubObjectScope<TMode>>(mInputData->Data, TArchiveScope<TMode>::GetContext())
						: std::nullopt;
				}
				else
				{
					mOutputData->Data->emplace<BinTestIoDataObjectPtr>(std::make_shared<BinTestIoDataObject>());
					return std::make_optional<BinArchiveStubObjectScope<TMode>>(mOutputData->Data, TArchiveScope<TMode>::GetContext());
				}
			}

			/**
			 * @brief Opens a nested array scope at the root level.
			 */
			std::optional<BinArchiveStubArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					return std::holds_alternative<BinTestIoDataArrayPtr>(*mInputData->Data)
						? std::make_optional<BinArchiveStubArrayScope<TMode>>(mInputData->Data, TArchiveScope<TMode>::GetContext())
						: std::nullopt;
				}
				else
				{
					mOutputData->Data->emplace<BinTestIoDataArrayPtr>(std::make_shared<BinTestIoDataArray>(arraySize));
					return std::make_optional<BinArchiveStubArrayScope<TMode>>(mOutputData->Data, TArchiveScope<TMode>::GetContext());
				}
			}

		private:
			BinTestIoDataRoot* mOutputData;
			const BinTestIoDataRoot* mInputData;
		};

	} // namespace Detail

	/**
	 * @brief Declaration of the binary archive stub used in unit tests.
	 */
	using BinArchiveStub = TArchiveBase<
		Detail::BinArchiveStubTraits,
		Detail::BinArchiveStubRootScope<SerializeMode::Load>,
		Detail::BinArchiveStubRootScope<SerializeMode::Save>>;

} // namespace BitSerializer
