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

namespace BitSerializer
{
	namespace Detail
	{
		/**
		 * @brief Represents input/output data structure for archive stub.
		 */
		class TestIoData;
		using TestIoDataPtr = std::shared_ptr<TestIoData>;

		/**
		 * @brief Represents an object node in the I/O data tree.
		 */
		class TestIoDataObject : public std::map<std::wstring, TestIoDataPtr> {};

		using TestIoDataObjectPtr = std::shared_ptr<TestIoDataObject>;

		/**
		 * @brief Represents an array node in the I/O data tree.
		 */
		class TestIoDataArray : public std::vector<TestIoDataPtr> {
		public:
			explicit TestIoDataArray(const size_t expectedSize) {
				reserve(expectedSize);
			}
		};

		using TestIoDataArrayPtr = std::shared_ptr<TestIoDataArray>;

		/**
		 * @brief Unified I/O data type that can represent various value types.
		 */
		class TestIoData
			: public std::variant<std::nullptr_t, bool, int64_t, uint64_t, double, std::wstring, TestIoDataObjectPtr, TestIoDataArrayPtr>
		{
		};

		/**
		 * @brief Root container for I/O data used during serialization tests.
		 */
		class TestIoDataRoot
		{
		public:
			TestIoDataRoot()
				: Data(std::make_shared<TestIoData>())
			{
			}

			TestIoDataPtr Data;
		};

		/**
		 * @brief Traits defining properties of the archive stub.
		 */
		struct ArchiveStubTraits
		{
			static constexpr ArchiveType archive_type = ArchiveType::Json;
			using key_type = std::wstring;
			using supported_key_types = TSupportedKeyTypes<std::wstring>;
			using string_view_type = std::basic_string_view<wchar_t>;
			using preferred_output_format = TestIoDataRoot;
			static constexpr char path_separator = '/';
			static constexpr bool is_binary = false;

		protected:
			~ArchiveStubTraits() = default;
		};

		// Forward declarations
		template <SerializeMode TMode>
		class ArchiveStubObjectScope;

		/**
		 * @brief Base class for archive scopes used in test stubs.
		 */
		class ArchiveStubScopeBase : public ArchiveStubTraits
		{
		public:
			explicit ArchiveStubScopeBase(TestIoDataPtr node, ArchiveStubScopeBase* parent = nullptr, key_type parentKey = key_type())
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
			[[nodiscard]] size_t GetSize() const
			{
				if (std::holds_alternative<TestIoDataObjectPtr>(*mNode)) {
					return std::get<TestIoDataObjectPtr>(*mNode)->size();
				}
				if (std::holds_alternative<TestIoDataArrayPtr>(*mNode)) {
					return std::get<TestIoDataArrayPtr>(*mNode)->size();
				}
				return 0;
			}

			/**
			 * @brief Loads a fundamental value from I/O data with type conversion policy.
			 */
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool LoadFundamentalValue(const TestIoData& ioData, T& value, const SerializationOptions& options)
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

				if (options.mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
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
			void SaveFundamentalValue(TestIoData& ioData, T& value)
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
			 * @brief Loads a string value from I/O data.
			 */
			static bool LoadString(const TestIoData& ioData, string_view_type& value)
			{
				if (!std::holds_alternative<key_type>(ioData)) {
					return false;
				}
				value = std::get<key_type>(ioData);
				return true;
			}

			/**
			 * @brief Saves a string value into I/O data.
			 */
			static void SaveString(TestIoData& ioData, string_view_type& value)
			{
				ioData.emplace<key_type>(value);
			}

			TestIoDataPtr mNode;
			ArchiveStubScopeBase* mParent;
			key_type mParentKey;
		};

		/**
		 * @brief Scope for handling arrays (sequences of values without keys).
		 */
		template <SerializeMode TMode>
		class ArchiveStubArrayScope final : public TArchiveScope<TMode>, public ArchiveStubScopeBase
		{
		public:
			ArchiveStubArrayScope(TestIoDataPtr node, SerializationContext& context, ArchiveStubScopeBase* parent = nullptr, const key_type& parentKey = key_type())
				: TArchiveScope<TMode>(context)
				, ArchiveStubScopeBase(std::move(node), parent, parentKey)
			{
				assert(std::holds_alternative<TestIoDataArrayPtr>(*mNode));
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
				return ArchiveStubScopeBase::GetPath() + path_separator + Convert::ToString(mIndex);
			}

			/**
			 * @brief Checks whether all items have been processed.
			 */
			[[nodiscard]]
			bool IsEnd() const
			{
				static_assert(TMode == SerializeMode::Load);
				return mIndex == std::get<TestIoDataArrayPtr>(*mNode)->size();
			}

			/**
			 * @brief Serializes a string value at the current array position.
			 */
			bool SerializeValue(string_view_type& value)
			{
				if (TestIoDataPtr ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load) {
						return LoadString(*ioData, value);
					}
					else
					{
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
				if (TestIoDataPtr ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load) {
						return LoadFundamentalValue(*ioData, value, this->GetOptions());
					}
					else
					{
						SaveFundamentalValue(*ioData, value);
						return true;
					}
				}
				return false;
			}

			/**
			 * @brief Opens a nested object scope.
			 */
			std::optional<ArchiveStubObjectScope<TMode>> OpenObjectScope(size_t)
			{
				if (TestIoDataPtr ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load)
					{
						return std::holds_alternative<TestIoDataObjectPtr>(*ioData)
							? std::make_optional<ArchiveStubObjectScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this)
							: std::nullopt;
					}
					else
					{
						ioData->emplace<TestIoDataObjectPtr>(std::make_shared<TestIoDataObject>());
						return std::make_optional<ArchiveStubObjectScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this);
					}
				}
				return std::nullopt;
			}

			/**
			 * @brief Opens a nested array scope.
			 */
			std::optional<ArchiveStubArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if (TestIoDataPtr ioData = LoadNextItem())
				{
					if constexpr (TMode == SerializeMode::Load)
					{
						return std::holds_alternative<TestIoDataArrayPtr>(*ioData)
							? std::make_optional<ArchiveStubArrayScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this)
							: std::nullopt;
					}
					else
					{
						ioData->emplace<TestIoDataArrayPtr>(std::make_shared<TestIoDataArray>(arraySize));
						return std::make_optional<ArchiveStubArrayScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this);
					}
				}
				return std::nullopt;
			}

		protected:
			/**
			 * @brief Loads or creates the next item in the array.
			 */
			TestIoDataPtr LoadNextItem()
			{
				auto& archiveArray = std::get<TestIoDataArrayPtr>(*mNode);
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
					return archiveArray->emplace_back(std::make_shared<TestIoData>());
				}
			}

		private:
			size_t mIndex = 0;
		};

		/**
		 * @brief Scope for handling objects (key-value pairs).
		 */
		template <SerializeMode TMode>
		class ArchiveStubObjectScope final : public TArchiveScope<TMode>, public ArchiveStubScopeBase
		{
		public:
			ArchiveStubObjectScope(TestIoDataPtr node, SerializationContext& context, ArchiveStubScopeBase* parent = nullptr, const key_type& parentKey = key_type())
				: TArchiveScope<TMode>(context)
				, ArchiveStubScopeBase(std::move(node), parent, parentKey)
			{
				assert(std::holds_alternative<TestIoDataObjectPtr>(*mNode));
			}

			/**
			 * @brief Returns the estimated number of items to load (for reserving containers).
			 */
			[[nodiscard]] size_t GetEstimatedSize() const
			{
				return GetAsObject()->size();
			}

			/**
			 * @brief Enumerates all keys in the current object scope.
			 */
			template <typename TCallback>
			void VisitKeys(const TCallback& fn)
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
			 * @brief Opens a nested object scope for the specified key.
			 */
			std::optional<ArchiveStubObjectScope<TMode>> OpenObjectScope(const key_type& key, size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					auto archiveValue = LoadArchiveValueByKey(key);
					if (archiveValue != nullptr && std::holds_alternative<TestIoDataObjectPtr>(*archiveValue)) {
						return std::make_optional<ArchiveStubObjectScope<TMode>>(archiveValue, TArchiveScope<TMode>::GetContext(), this, key);
					}
					return std::nullopt;
				}
				else
				{
					TestIoDataPtr ioData = AddArchiveValue(key);
					ioData->template emplace<TestIoDataObjectPtr>(std::make_shared<TestIoDataObject>());
					return std::make_optional<ArchiveStubObjectScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this, key);
				}
			}

			/**
			 * @brief Opens a nested array scope for the specified key.
			 */
			std::optional<ArchiveStubArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					auto archiveValue = LoadArchiveValueByKey(key);
					if (archiveValue != nullptr && std::holds_alternative<TestIoDataArrayPtr>(*archiveValue)) {
						return std::make_optional<ArchiveStubArrayScope<TMode>>(archiveValue, TArchiveScope<TMode>::GetContext(), this, key);
					}
					return std::nullopt;
				}
				else
				{
					TestIoDataPtr ioData = AddArchiveValue(key);
					ioData->emplace<TestIoDataArrayPtr>(std::make_shared<TestIoDataArray>(arraySize));
					return std::make_optional<ArchiveStubArrayScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this, key);
				}
			}

		protected:
			[[nodiscard]] constexpr TestIoDataObjectPtr& GetAsObject() const
			{
				return std::get<TestIoDataObjectPtr>(*mNode);
			}

			[[nodiscard]] TestIoDataPtr LoadArchiveValueByKey(const key_type& key)
			{
				const auto& archiveObject = GetAsObject();
				const auto it = archiveObject->find(key);
				return it == archiveObject->end() ? nullptr : it->second;
			}

			[[nodiscard]] TestIoDataPtr AddArchiveValue(const key_type& key) const
			{
				const auto archiveObject = GetAsObject();
				decltype(auto) result = archiveObject->emplace(key, std::make_shared<TestIoData>());
				return result.first->second;
			}

			template <class IoDataType, class SourceType>
			[[nodiscard]] TestIoDataPtr& SaveArchiveValue(const key_type& key, const SourceType& value)
			{
				const auto& archiveObject = GetAsObject();
				TestIoData ioData;
				ioData.emplace<IoDataType>(value);
				decltype(auto) result = archiveObject->emplace(key, std::make_shared<TestIoData>(std::move(ioData)));
				return result.first->second;
			}
		};

		/**
		 * @brief The root scope for serializing one value, array, or object without a key.
		 */
		template <SerializeMode TMode>
		class ArchiveStubRootScope final : public TArchiveScope<TMode>, public ArchiveStubScopeBase
		{
		public:
			ArchiveStubRootScope(const TestIoDataRoot& inputData, SerializationContext& context)
				: TArchiveScope<TMode>(context)
				, ArchiveStubScopeBase(inputData.Data)
				, mOutputData(nullptr)
				, mInputData(&inputData)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
			}

			ArchiveStubRootScope(TestIoDataRoot& outputData, SerializationContext& context)
				: TArchiveScope<TMode>(context)
				, ArchiveStubScopeBase(outputData.Data)
				, mOutputData(&outputData)
				, mInputData(nullptr)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
			}

			void Finalize()
			{
			}

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

			bool SerializeValue(string_view_type& value)
			{
				if constexpr (TMode == SerializeMode::Load) {
					return LoadString(*mInputData->Data, value);
				}
				else {
					SaveString(*mOutputData->Data, value);
					return true;
				}
			}

			std::optional<ArchiveStubObjectScope<TMode>> OpenObjectScope(size_t)
			{
				if constexpr (TMode == SerializeMode::Load) {
					return std::holds_alternative<TestIoDataObjectPtr>(*mInputData->Data)
						? std::make_optional<ArchiveStubObjectScope<TMode>>(mInputData->Data, TArchiveScope<TMode>::GetContext())
						: std::nullopt;
				}
				else
				{
					mOutputData->Data->emplace<TestIoDataObjectPtr>(std::make_shared<TestIoDataObject>());
					return std::make_optional<ArchiveStubObjectScope<TMode>>(mOutputData->Data, TArchiveScope<TMode>::GetContext());
				}
			}

			std::optional<ArchiveStubArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load) {
					return std::holds_alternative<TestIoDataArrayPtr>(*mInputData->Data)
						? std::make_optional<ArchiveStubArrayScope<TMode>>(mInputData->Data, TArchiveScope<TMode>::GetContext())
						: std::nullopt;
				}
				else
				{
					mOutputData->Data->emplace<TestIoDataArrayPtr>(std::make_shared<TestIoDataArray>(arraySize));
					return std::make_optional<ArchiveStubArrayScope<TMode>>(mOutputData->Data, TArchiveScope<TMode>::GetContext());
				}
			}

		private:
			TestIoDataRoot* mOutputData;
			const TestIoDataRoot* mInputData;
		};

	} // namespace Detail

	/**
	 * @brief Declaration of the archive stub used in unit tests.
	 */
	using ArchiveStub = TArchiveBase<
		Detail::ArchiveStubTraits,
		Detail::ArchiveStubRootScope<SerializeMode::Load>,
		Detail::ArchiveStubRootScope<SerializeMode::Save>>;

} // namespace BitSerializer
