/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <string>
#include <map>
#include <variant>
#include <optional>
#include <type_traits>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/media_archive_base.h"


namespace BitSerializer {
namespace Detail {

/// <summary>
/// The input/output format for archive stub
/// </summary>
class TestIoData;
class TestIoDataObject : public std::map<std::wstring, TestIoData> { };
class TestIoDataArray : public std::vector<TestIoData> {
public:
	TestIoDataArray(size_t size)
		: std::vector<TestIoData>(size)	{ }
};
class TestIoData : public std::variant<bool, int64_t, double, std::wstring, TestIoDataObject, TestIoDataArray> { };

/// <summary>
/// The traits of archive stub 
/// </summary>
struct ArchiveStubTraits
{
	using key_type = std::wstring;
	using supported_key_types = SupportedKeyTypes<std::wstring>;
	using preferred_output_format = TestIoData;
	static constexpr char path_separator = '/';

protected:
	~ArchiveStubTraits() = default;
};

// Forward declarations
template <SerializeMode TMode>
class ArchiveStubObjectScope;


/// <summary>
/// Base class of archive stub
/// </summary>
/// <seealso cref="MediaArchiveBase" />
class ArchiveStubScopeBase : public ArchiveStubTraits
{
public:
	explicit ArchiveStubScopeBase(const TestIoData* node, ArchiveStubScopeBase* parent = nullptr, const key_type& parentKey = key_type())
		: mNode(const_cast<TestIoData*>(node))
		, mParent(parent)
		, mParentKey(parentKey)
	{ }

	/// <summary>
	/// Returns the size of stored elements (for arrays and objects).
	/// </summary>
	/// <returns></returns>
	[[nodiscard]] size_t GetSize() const {
		if (std::holds_alternative<TestIoDataObject>(*mNode)) {
			return std::get<TestIoDataObject>(*mNode).size();
		}
		if (std::holds_alternative<TestIoDataArray>(*mNode)) {
			return std::get<TestIoDataArray>(*mNode).size();
		}
		return 0;
	}

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

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool LoadFundamentalValue(const TestIoData& ioData, T& value)
	{
		if constexpr (std::is_integral_v<T>)
		{
			if (std::holds_alternative<int64_t>(ioData))
			{
				value = static_cast<T>(std::get<int64_t>(ioData));
				return true;
			}
		}
		else
		{
			if (std::holds_alternative<double>(ioData))
			{
				value = static_cast<T>(std::get<double>(ioData));
				return true;
			}
		}
		return false;
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SaveFundamentalValue(TestIoData& ioData, T& value)
	{
		if constexpr (std::is_integral_v<T>)
			ioData.emplace<int64_t>(value);
		else if constexpr (std::is_floating_point_v<T>)
			ioData.emplace<double>(value);
	}

	template <typename TSym, typename TAllocator>
	bool LoadString(const TestIoData& ioData, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if (!std::holds_alternative<std::wstring>(ioData))
			return false;

		if constexpr (std::is_same_v<TSym, std::wstring::value_type>)
			value = std::get<std::wstring>(ioData);
		else
			value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(std::get<std::wstring>(ioData));
		return true;
	}

	template <typename TSym, typename TAllocator>
	void SaveString(TestIoData& ioData, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, std::wstring::value_type>)
			ioData.emplace<std::wstring>(value);
		else
			ioData.emplace<std::wstring>(Convert::FromString<std::wstring>(value));
	}

	TestIoData* mNode;
	ArchiveStubScopeBase* mParent;
	key_type mParentKey;
};

/// <summary>
/// Scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="ArchiveStubScopeBase" />
template <SerializeMode TMode>
class ArchiveStubArrayScope final : public ArchiveScope<TMode>, public ArchiveStubScopeBase
{
public:
	explicit ArchiveStubArrayScope(const TestIoData* node, ArchiveStubScopeBase* parent = nullptr, const key_type& parentKey = key_type())
		: ArchiveStubScopeBase(node, parent, parentKey)
		, mIndex(0)
	{
		assert(std::holds_alternative<TestIoDataArray>(*mNode));
	}

	/// <summary>
	/// Gets the current path
	/// </summary>
	[[nodiscard]] std::string GetPath() const override
	{
		auto index = mIndex == 0 ? 0 : mIndex - 1;
		return ArchiveStubScopeBase::GetPath() + path_separator + Convert::ToString(index);
	}

	template <typename TSym, typename TAllocator>
	void SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		TestIoData& ioData = NextElement();
		if constexpr (TMode == SerializeMode::Load)
			LoadString(ioData, value);
		else
			SaveString(ioData, value);
	}

	void SerializeValue(bool& value)
	{
		TestIoData& ioData = NextElement();
		if constexpr (TMode == SerializeMode::Load)
		{
			if (std::holds_alternative<bool>(ioData))
				value = std::get<bool>(ioData);
		}
		else {
			ioData.emplace<bool>(value);
		}
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SerializeValue(T& value)
	{
		TestIoData& ioData = NextElement();
		if constexpr (TMode == SerializeMode::Load)
			LoadFundamentalValue(ioData, value);
		else
			SaveFundamentalValue(ioData, value);
	}

	std::optional<ArchiveStubObjectScope<TMode>> OpenObjectScope()
	{
		TestIoData& ioData = NextElement();
		if constexpr (TMode == SerializeMode::Load)
		{
			return std::holds_alternative<TestIoDataObject>(ioData)
				? std::make_optional<ArchiveStubObjectScope<TMode>>(&ioData, this)
				: std::nullopt;
		}
		else
		{
			ioData.emplace<TestIoDataObject>(TestIoDataObject());
			return std::make_optional<ArchiveStubObjectScope<TMode>>(&ioData, this);
		}
	}

	std::optional<ArchiveStubArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		TestIoData& ioData = NextElement();
		if constexpr (TMode == SerializeMode::Load)
		{
			return std::holds_alternative<TestIoDataArray>(ioData)
				? std::make_optional<ArchiveStubArrayScope<TMode>>(&ioData, this)
				: std::nullopt;
		}
		else
		{
			ioData.emplace<TestIoDataArray>(TestIoDataArray(arraySize));
			return std::make_optional<ArchiveStubArrayScope<TMode>>(&ioData, this);
		}
	}

protected:
	TestIoData& NextElement()
	{
		assert(mIndex < GetSize());
		auto& archiveArray = std::get<TestIoDataArray>(*mNode);
		return archiveArray.at(mIndex++);
	}

private:
	size_t mIndex;
};


/// <summary>
/// Constant iterator of the keys.
/// </summary>
class key_const_iterator
{
	template <SerializeMode TMode>
	friend class ArchiveStubObjectScope;

	TestIoDataObject::const_iterator mJsonIt;

	key_const_iterator(TestIoDataObject::const_iterator it)
		: mJsonIt(std::move(it)) { }

public:
	bool operator==(const key_const_iterator& rhs) const {
		return this->mJsonIt == rhs.mJsonIt;
	}
	bool operator!=(const key_const_iterator& rhs) const {
		return this->mJsonIt != rhs.mJsonIt;
	}

	key_const_iterator& operator++() {
		++mJsonIt;
		return *this;
	}

	const ArchiveStubTraits::key_type& operator*() const {
		return mJsonIt->first;
	}
};


/// <summary>
/// Scope for serializing objects (list of values with keys).
/// </summary>
/// <seealso cref="ArchiveStubScopeBase" />
template <SerializeMode TMode>
class ArchiveStubObjectScope final : public ArchiveScope<TMode> , public ArchiveStubScopeBase
{
public:
	explicit ArchiveStubObjectScope(const TestIoData* node, ArchiveStubScopeBase* parent = nullptr, const key_type& parentKey = key_type())
		: ArchiveStubScopeBase(node, parent, parentKey)
	{
		assert(std::holds_alternative<TestIoDataObject>(*mNode));
	};

	[[nodiscard]] key_const_iterator cbegin() const {
		return key_const_iterator(GetAsObject().cbegin());
	}

	[[nodiscard]] key_const_iterator cend() const {
		return key_const_iterator(GetAsObject().cend());
	}

	template <typename TSym, typename TAllocator>
	bool SerializeValue(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* archiveValue = LoadArchiveValueByKey(key);
			return archiveValue == nullptr ? false : LoadString(*archiveValue, value);
		}
		else
		{
			auto& ioData = AddArchiveValue(key);
			SaveString(ioData, value);
			return true;
		}
	}

	bool SerializeValue(const key_type& key, bool& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* archiveValue = LoadArchiveValueByKey(key);
			if (archiveValue != nullptr && std::holds_alternative<bool>(*archiveValue))
			{
				value = std::get<bool>(*archiveValue);
				return true;
			}
			return false;
		}
		else
		{
			SaveArchiveValue<bool>(key, value);
			return true;
		}
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool SerializeValue(const key_type& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* archiveValue = LoadArchiveValueByKey(key);
			return archiveValue == nullptr ? false : LoadFundamentalValue(*archiveValue, value);
		}
		else
		{
			SaveFundamentalValue(AddArchiveValue(key), value);
			return true;
		}
	}

	std::optional<ArchiveStubObjectScope<TMode>> OpenObjectScope(const key_type& key)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* archiveValue = LoadArchiveValueByKey(key);
			if (archiveValue != nullptr && std::holds_alternative<TestIoDataObject>(*archiveValue)) {
				return std::make_optional<ArchiveStubObjectScope<TMode>>(archiveValue, this, key);
			}
			return std::nullopt;
		}
		else
		{
			TestIoData& ioData = AddArchiveValue(key);
			ioData.emplace<TestIoDataObject>(TestIoDataObject());
			return std::make_optional<ArchiveStubObjectScope<TMode>>(&ioData, this, key);
		}
	}

	std::optional<ArchiveStubArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* archiveValue = LoadArchiveValueByKey(key);
			if (archiveValue != nullptr && std::holds_alternative<TestIoDataArray>(*archiveValue))
				return std::make_optional<ArchiveStubArrayScope<TMode>>(archiveValue, this, key);
			return std::nullopt;
		}
		else
		{
			TestIoData& ioData = AddArchiveValue(key);
			ioData.emplace<TestIoDataArray>(TestIoDataArray(arraySize));
			return std::make_optional<ArchiveStubArrayScope<TMode>>(&ioData, this, key);
		}
	}

protected:
	constexpr TestIoDataObject& GetAsObject() const
	{
		return std::get<TestIoDataObject>(*mNode);
	}

	TestIoData* LoadArchiveValueByKey(const key_type& key) const
	{
		auto& archiveObject = GetAsObject();
		auto it = archiveObject.find(key);
		return it == archiveObject.end() ? nullptr : &it->second;
	}

	TestIoData& AddArchiveValue(const key_type& key) const
	{
		auto& archiveObject = GetAsObject();
		decltype(auto) result = archiveObject.emplace(key, TestIoData());
		return result.first->second;
	}

	template <class IoDataType, class SourceType>
	TestIoData& SaveArchiveValue(const key_type& key, const SourceType& value)
	{
		auto& archiveObject = GetAsObject();
		TestIoData ioData;
		ioData.emplace<IoDataType>(value);
		decltype(auto) result = archiveObject.emplace(key, std::move(ioData));
		return result.first->second;
	}
};


/// <summary>
/// Root scope (can serialize one value, array or object without key)
/// </summary>
template <SerializeMode TMode>
class ArchiveStubRootScope final : public ArchiveScope<TMode>, public ArchiveStubScopeBase
{
public:
	explicit ArchiveStubRootScope(const TestIoData& inputData)
		: ArchiveStubScopeBase(&inputData)
		, mOutputData(nullptr)
		, mInputData(&inputData)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
	}

	explicit ArchiveStubRootScope(TestIoData& outputData, const SerializationOptions& serializationOptions = {})
		: ArchiveStubScopeBase(&outputData)
		, mOutputData(&outputData)
		, mInputData(nullptr)
		, mSerializationOptions(serializationOptions)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	void Finalize()
	{
	}

	void SerializeValue(bool& value) const
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (std::holds_alternative<bool>(*mInputData)) {
				value = std::get<bool>(*mInputData);
			}
		}
		else {
			mOutputData->emplace<bool>(value);
		}
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
			LoadFundamentalValue(*mInputData, value);
		else
			SaveFundamentalValue(*mOutputData, value);
	}

	template <typename TSym, typename TAllocator>
	void SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
			LoadString(*mInputData, value);
		else
			SaveString(*mOutputData, value);
	}

	std::optional<ArchiveStubObjectScope<TMode>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load) {
			return std::holds_alternative<TestIoDataObject>(*mInputData)
				? std::make_optional<ArchiveStubObjectScope<TMode>>(mInputData)
				: std::nullopt;
		}
		else
		{
			mOutputData->emplace<TestIoDataObject>(TestIoDataObject());
			return std::make_optional<ArchiveStubObjectScope<TMode>>(mOutputData);
		}
	}

	std::optional<ArchiveStubArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return std::holds_alternative<TestIoDataArray>(*mInputData)
				? std::make_optional<ArchiveStubArrayScope<TMode>>(mInputData)
				: std::nullopt;
		}
		else
		{
			mOutputData->emplace<TestIoDataArray>(TestIoDataArray(arraySize));
			return std::make_optional<ArchiveStubArrayScope<TMode>>(mOutputData);
		}
	}

private:
	TestIoData* mOutputData;
	const TestIoData* mInputData;
	std::optional<SerializationOptions> mSerializationOptions;
};

}	// namespace Detail

/// <summary>
/// Declaration of archive stub
/// </summary>
using ArchiveStub = MediaArchiveBase<
	Detail::ArchiveStubTraits,
	Detail::ArchiveStubRootScope<SerializeMode::Load>,
	Detail::ArchiveStubRootScope<SerializeMode::Save>>;

}	// namespace BitSerializer
