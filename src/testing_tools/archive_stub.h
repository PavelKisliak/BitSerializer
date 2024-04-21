/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
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

/// <summary>
/// The input/output format for archive stub
/// </summary>
class TestIoData;
using TestIoDataPtr = std::shared_ptr<TestIoData>;

class TestIoDataObject : public std::map<std::wstring, TestIoDataPtr> { };
using TestIoDataObjectPtr = std::shared_ptr<TestIoDataObject>;

class TestIoDataArray : public std::vector<TestIoDataPtr> {
public:
	explicit TestIoDataArray(const size_t expectedSize) {
		reserve(expectedSize);
	}
};
using TestIoDataArrayPtr = std::shared_ptr<TestIoDataArray>;

class TestIoData
	: public std::variant<std::nullptr_t, bool, int64_t, uint64_t, double, std::wstring, TestIoDataObjectPtr, TestIoDataArrayPtr>
{ };

class TestIoDataRoot
{
public:
	TestIoDataRoot()
		: Data(std::make_shared<TestIoData>())
	{ }

	TestIoDataPtr Data;
};

/// <summary>
/// The traits of archive stub 
/// </summary>
struct ArchiveStubTraits
{
	static constexpr ArchiveType archive_type = ArchiveType::Json;
	using key_type = std::wstring;
	using supported_key_types = TSupportedKeyTypes<std::wstring>;
	using preferred_output_format = TestIoDataRoot;
	static constexpr char path_separator = '/';
	static constexpr bool is_binary = false;

protected:
	~ArchiveStubTraits() = default;
};

// Forward declarations
template <SerializeMode TMode>
class ArchiveStubObjectScope;


/// <summary>
/// Base class of archive stub
/// </summary>
/// <seealso cref="TArchiveBase" />
class ArchiveStubScopeBase : public ArchiveStubTraits
{
public:
	explicit ArchiveStubScopeBase(TestIoDataPtr node, ArchiveStubScopeBase* parent = nullptr, key_type parentKey = key_type())
		: mNode(std::move(node))
		, mParent(parent)
		, mParentKey(std::move(parentKey))
	{ }

	/// <summary>
	/// Gets the current path
	/// </summary>
	[[nodiscard]] virtual std::string GetPath() const
	{
		const std::string localPath = mParentKey.empty()
			? Convert::ToString(mParentKey)
			: path_separator + Convert::ToString(mParentKey);
		return mParent == nullptr ? localPath : mParent->GetPath() + localPath;
	}

protected:
	~ArchiveStubScopeBase() = default;

	/// <summary>
	/// Returns the size of stored elements (for arrays and objects).
	/// </summary>
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

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool LoadFundamentalValue(const TestIoData& ioData, T& value, const SerializationOptions& serializationOptions)
	{
		// Null value is excluded from MismatchedTypesPolicy processing
		if (std::holds_alternative<std::nullptr_t>(ioData))
		{
			return std::is_null_pointer_v<T>;
		}

		using Detail::ConvertByPolicy;
		if constexpr (std::is_integral_v<T>)
		{
			if (std::holds_alternative<int64_t>(ioData))
			{
				return ConvertByPolicy(std::get<int64_t>(ioData), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
			}
			if (std::holds_alternative<uint64_t>(ioData))
			{
				return ConvertByPolicy(std::get<uint64_t>(ioData), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
			}
			if (std::holds_alternative<bool>(ioData))
			{
				return ConvertByPolicy(std::get<bool>(ioData), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
			}
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			if (std::holds_alternative<double>(ioData))
			{
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

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SaveFundamentalValue(TestIoData& ioData, T& value)
	{
		if constexpr (std::is_same_v<T, bool>)
			ioData.emplace<bool>(value);
		else if constexpr (std::is_integral_v<T>)
		{
			if constexpr (std::is_signed_v<T>) {
				ioData.emplace<int64_t>(value);
			}
			else {
				ioData.emplace<uint64_t>(value);
			}
		}
		else if constexpr (std::is_floating_point_v<T>)
			ioData.emplace<double>(value);
		else if constexpr (std::is_null_pointer_v<T>)
			ioData.emplace<std::nullptr_t>(value);
	}

	template <typename TSym, typename TAllocator>
	bool LoadString(const TestIoData& ioData, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if (!std::holds_alternative<key_type>(ioData))
			return false;

		if constexpr (std::is_same_v<TSym, key_type::value_type>)
			value = std::get<key_type>(ioData);
		else
			value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(std::get<key_type>(ioData));
		return true;
	}

	template <typename TSym, typename TAllocator>
	void SaveString(TestIoData& ioData, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, key_type::value_type>)
			ioData.emplace<key_type>(value);
		else
			ioData.emplace<key_type>(Convert::ToWString(value));
	}

	TestIoDataPtr mNode;
	ArchiveStubScopeBase* mParent;
	key_type mParentKey;
};

/// <summary>
/// Scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="ArchiveStubScopeBase" />
template <SerializeMode TMode>
class ArchiveStubArrayScope final : public TArchiveScope<TMode>, public ArchiveStubScopeBase
{
public:
	ArchiveStubArrayScope(TestIoDataPtr node, SerializationContext& serializationContext, ArchiveStubScopeBase* parent = nullptr, const key_type& parentKey = key_type())
		: TArchiveScope<TMode>(serializationContext)
		, ArchiveStubScopeBase(std::move(node), parent, parentKey)
		, mIndex(0)
	{
		assert(std::holds_alternative<TestIoDataArrayPtr>(*mNode));
	}

	/// <summary>
	/// Returns the estimated number of items to load (for reserving the size of containers).
	/// </summary>
	[[nodiscard]] size_t GetEstimatedSize() const
	{
		return 0;
	}

	/// <summary>
	/// Gets the current path
	/// </summary>
	[[nodiscard]] std::string GetPath() const override
	{
		return ArchiveStubScopeBase::GetPath() + path_separator + Convert::ToString(mIndex);
	}

	/// <summary>
	/// Returns `true` when all no more values to load.
	/// </summary>
	[[nodiscard]]
	bool IsEnd() const
	{
		static_assert(TMode == SerializeMode::Load);
		return mIndex == std::get<TestIoDataArrayPtr>(*mNode)->size();
	}

	template <typename TSym, typename TAllocator>
	bool SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if (TestIoDataPtr ioData = LoadNextItem())
		{
			if constexpr (TMode == SerializeMode::Load)
				return LoadString(*ioData, value);
			else {
				SaveString(*ioData, value);
				return true;
			}
		}
		return false;
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool SerializeValue(T& value)
	{
		if (TestIoDataPtr ioData = LoadNextItem())
		{
			if constexpr (TMode == SerializeMode::Load)
				return LoadFundamentalValue(*ioData, value, this->GetOptions());
			else {
				SaveFundamentalValue(*ioData, value);
				return true;
			}
		}
		return false;
	}

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
	size_t mIndex;
};


/// <summary>
/// Scope for serializing objects (list of values with keys).
/// </summary>
/// <seealso cref="ArchiveStubScopeBase" />
template <SerializeMode TMode>
class ArchiveStubObjectScope final : public TArchiveScope<TMode> , public ArchiveStubScopeBase
{
public:
	ArchiveStubObjectScope(TestIoDataPtr node, SerializationContext& serializationContext, ArchiveStubScopeBase* parent = nullptr, const key_type& parentKey = key_type())
		: TArchiveScope<TMode>(serializationContext)
		, ArchiveStubScopeBase(std::move(node), parent, parentKey)
	{
		assert(std::holds_alternative<TestIoDataObjectPtr>(*mNode));
	}

	/// <summary>
	/// Returns the estimated number of items to load (for reserving the size of containers).
	/// </summary>
	[[nodiscard]] size_t GetEstimatedSize() const
	{
		return GetAsObject()->size();
	}

	/// <summary>
	/// Enumerates all keys by calling a passed function.
	/// </summary>
	template <typename TCallback>
	void VisitKeys(TCallback&& fn)
	{
		for (auto& keyValue : *GetAsObject()) {
			fn(keyValue.first);
		}
	}

	template <typename TSym, typename TAllocator>
	bool SerializeValue(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
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
			auto ioData = AddArchiveValue(key);
			ioData->template emplace<TestIoDataObjectPtr>(std::make_shared<TestIoDataObject>());
			return std::make_optional<ArchiveStubObjectScope<TMode>>(ioData, TArchiveScope<TMode>::GetContext(), this, key);
		}
	}

	std::optional<ArchiveStubArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto archiveValue = LoadArchiveValueByKey(key);
			if (archiveValue != nullptr && std::holds_alternative<TestIoDataArrayPtr>(*archiveValue))
				return std::make_optional<ArchiveStubArrayScope<TMode>>(archiveValue, TArchiveScope<TMode>::GetContext(), this, key);
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
	constexpr TestIoDataObjectPtr& GetAsObject() const
	{
		return std::get<TestIoDataObjectPtr>(*mNode);
	}

	TestIoDataPtr LoadArchiveValueByKey(const key_type& key)
	{
		const auto& archiveObject = GetAsObject();
		const auto it = archiveObject->find(key);
		return it == archiveObject->end() ? nullptr : it->second;
	}

	TestIoDataPtr AddArchiveValue(const key_type& key) const
	{
		const auto archiveObject = GetAsObject();
		decltype(auto) result = archiveObject->emplace(key, std::make_shared<TestIoData>());
		return result.first->second;
	}

	template <class IoDataType, class SourceType>
	TestIoDataPtr& SaveArchiveValue(const key_type& key, const SourceType& value)
	{
		const auto& archiveObject = GetAsObject();
		TestIoData ioData;
		ioData.emplace<IoDataType>(value);
		decltype(auto) result = archiveObject->emplace(key, std::move(ioData));
		return result.first->second;
	}
};


/// <summary>
/// Root scope (can serialize one value, array or object without key)
/// </summary>
template <SerializeMode TMode>
class ArchiveStubRootScope final : public TArchiveScope<TMode>, public ArchiveStubScopeBase
{
public:
	ArchiveStubRootScope(const TestIoDataRoot& inputData, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, ArchiveStubScopeBase(inputData.Data)
		, mOutputData(nullptr)
		, mInputData(&inputData)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
	}

	ArchiveStubRootScope(TestIoDataRoot& outputData, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
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
		if constexpr (TMode == SerializeMode::Load)
			return LoadFundamentalValue(*mInputData->Data, value, this->GetOptions());
		else {
			SaveFundamentalValue(*mOutputData->Data, value);
			return true;
		}
	}

	template <typename TSym, typename TAllocator>
	bool SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
			return LoadString(*mInputData->Data, value);
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

}	// namespace Detail

/// <summary>
/// Declaration of archive stub
/// </summary>
using ArchiveStub = TArchiveBase<
	Detail::ArchiveStubTraits,
	Detail::ArchiveStubRootScope<SerializeMode::Load>,
	Detail::ArchiveStubRootScope<SerializeMode::Save>>;

}	// namespace BitSerializer
