/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <optional>
#include <type_traits>
#include <variant>
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/errors_handling.h"

// External dependency (RapidJson)
#include "rapidjson/document.h"
#include "rapidjson/encodings.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stream.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"

namespace BitSerializer::Json::RapidJson {
namespace Detail {

/// <summary>
/// The traits of JSON archive based on RapidJson
/// </summary>
template <class TEncoding>
struct RapidJsonArchiveTraits
{
	static constexpr ArchiveType archive_type = ArchiveType::Json;
	using key_type = std::basic_string<typename TEncoding::Ch, std::char_traits<typename TEncoding::Ch>>;
	using supported_key_types = TSupportedKeyTypes<const typename TEncoding::Ch*, key_type>;
	using preferred_output_format = std::basic_string<char, std::char_traits<char>>;
	using preferred_stream_char_type = char;
	static constexpr char path_separator = '/';

protected:
	~RapidJsonArchiveTraits() = default;
};

// Forward declarations
template <SerializeMode TMode, class TEncoding, class TAllocator>
class RapidJsonObjectScope;

/// <summary>
/// Base class of JSON scope
/// </summary>
/// <seealso cref="TArchiveBase" />
template <class TEncoding>
class RapidJsonScopeBase : public RapidJsonArchiveTraits<TEncoding>
{
public:
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using key_type_view = std::basic_string_view<typename TEncoding::Ch>;

	RapidJsonScopeBase(RapidJsonNode* node, RapidJsonScopeBase<TEncoding>* parent = nullptr, key_type_view parentKey = {})
		: mNode(node)
		, mParent(parent)
		, mParentKey(parentKey)
	{ }

	RapidJsonScopeBase(const RapidJsonScopeBase&) = delete;
	RapidJsonScopeBase& operator=(const RapidJsonScopeBase&) = delete;

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer). Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] virtual std::string GetPath() const
	{
		const std::string localPath = mParentKey.empty()
			? std::string()
			: RapidJsonArchiveTraits<TEncoding>::path_separator + Convert::ToString(mParentKey);
		return mParent == nullptr ? localPath : mParent->GetPath() + localPath;
	}

protected:
	~RapidJsonScopeBase() = default;
	RapidJsonScopeBase(RapidJsonScopeBase&&) = default;
	RapidJsonScopeBase& operator=(RapidJsonScopeBase&&) = default;

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool LoadValue(const RapidJsonNode& jsonValue, T& value, const SerializationOptions& serializationOptions)
	{
		// Null value from JSON is excluded from MismatchedTypesPolicy processing
		if (jsonValue.IsNull()) {
			return std::is_null_pointer_v<T>;
		}

		using BitSerializer::Detail::SafeNumberCast;
		if constexpr (std::is_integral_v<T>)
		{
			if (jsonValue.IsNumber())
			{
				if (jsonValue.IsInt64()) {
					return SafeNumberCast(jsonValue.GetInt64(), value, serializationOptions.overflowNumberPolicy);
				}
				if (jsonValue.IsUint64()) {
					return SafeNumberCast(jsonValue.GetUint64(), value, serializationOptions.overflowNumberPolicy);
				}
				if (jsonValue.IsDouble()) {
					return SafeNumberCast(jsonValue.GetDouble(), value, serializationOptions.overflowNumberPolicy);
				}
			}
			if (jsonValue.IsBool()) {
				return SafeNumberCast(jsonValue.GetBool(), value, serializationOptions.overflowNumberPolicy);
			}
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			if (jsonValue.IsNumber()) {
				return SafeNumberCast(jsonValue.GetDouble(), value, serializationOptions.overflowNumberPolicy);
			}
		}

		HandleMismatchedTypesPolicy(serializationOptions.mismatchedTypesPolicy);
		return false;
	}

	template <typename TSym, typename TAllocator>
	bool LoadValue(const RapidJsonNode& jsonValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value, const SerializationOptions& serializationOptions)
	{
		if (!jsonValue.IsString())
		{
			HandleMismatchedTypesPolicy(serializationOptions.mismatchedTypesPolicy);
			return false;
		}

		if constexpr (std::is_same_v<TSym, typename RapidJsonNode::EncodingType::Ch>)
			value = jsonValue.GetString();
		else
			value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(jsonValue.GetString());
		return true;
	}

	template <typename TSym, typename TAllocator, typename TRapidAllocator>
	RapidJsonNode MakeRapidJsonNodeFromString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value, TRapidAllocator& allocator)
	{
		using TargetSymType = typename TEncoding::Ch;
		if constexpr (std::is_same_v<TSym, TargetSymType>)
			return RapidJsonNode(value.data(), static_cast<rapidjson::SizeType>(value.size()), allocator);
		else {
			const auto str = Convert::To<std::basic_string<TargetSymType, std::char_traits<TargetSymType>>>(value);
			return RapidJsonNode(str.data(), static_cast<rapidjson::SizeType>(str.size()), allocator);
		}
	}

	static void HandleMismatchedTypesPolicy(MismatchedTypesPolicy mismatchedTypesPolicy)
	{
		if (mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
		{
			throw SerializationException(SerializationErrorCode::MismatchedTypes,
				"The type of target field does not match the value being loaded");
		}
	}

	RapidJsonNode* mNode;
	RapidJsonScopeBase* mParent;
	key_type_view mParentKey;
};


/// <summary>
/// JSON scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
template <SerializeMode TMode, class TEncoding, class TAllocator>
class RapidJsonArrayScope final : public TArchiveScope<TMode>, public RapidJsonScopeBase<TEncoding>
{
public:
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using iterator = typename RapidJsonNode::ValueIterator;
	using key_type_view = std::basic_string_view<typename TEncoding::Ch>;

	RapidJsonArrayScope(RapidJsonNode* node, TAllocator& allocator, SerializationContext& serializationContext, RapidJsonScopeBase<TEncoding>* parent = nullptr, key_type_view parentKey = {})
		: TArchiveScope<TMode>(serializationContext)
		, RapidJsonScopeBase<TEncoding>(node, parent, parentKey)
		, mAllocator(allocator)
		, mValueIt(this->mNode->GetArray().Begin())
	{
		assert(this->mNode->IsArray());
	}

	/// <summary>
	/// Returns the estimated number of items to load (for reserving the size of containers).
	/// </summary>
	[[nodiscard]] size_t GetEstimatedSize() const {
		return this->mNode->Capacity();
	}

	/// <summary>
	/// Returns `true` when all no more values to load.
	/// </summary>
	bool IsEnd()
	{
		static_assert(TMode == SerializeMode::Load);
		return mValueIt == this->mNode->GetArray().End();
	}

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer). Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] std::string GetPath() const override
	{
		int64_t index;
		if constexpr (TMode == SerializeMode::Load)
			index = std::distance(this->mNode->Begin(), mValueIt);
		else
			index = this->mNode->GetArray().Size();
		return RapidJsonScopeBase<TEncoding>::GetPath() + RapidJsonArchiveTraits<TEncoding>::path_separator + Convert::ToString(index);
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>, int> = 0>
	bool SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return this->LoadValue(LoadNextItem(), value, this->GetOptions());
		}
		else
		{
			SaveJsonValue(value);
			return true;
		}
	}

	template <typename TSym, typename TStrAllocator>
	bool SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return this->LoadValue(LoadNextItem(), value, this->GetOptions());
		}
		else 
		{
			SaveJsonValue(RapidJsonScopeBase<TEncoding>::MakeRapidJsonNodeFromString(value, mAllocator));
			return true;
		}
	}

	std::optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadNextItem();
			if (jsonValue.IsObject()) {
				return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(&jsonValue, mAllocator, this->GetContext(), this);
			}
			return std::nullopt;
		}
		else
		{
			SaveJsonValue(RapidJsonNode(rapidjson::kObjectType));
			auto& lastJsonValue = (*this->mNode)[this->mNode->Size() - 1];
			return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(&lastJsonValue, mAllocator, this->GetContext(), this);
		}
	}

	std::optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadNextItem();
			if (jsonValue.IsArray()) {
				return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(&jsonValue, mAllocator, this->GetContext(), this);
			}
			return std::nullopt;
		}
		else
		{
			auto rapidJsonArray = RapidJsonNode(rapidjson::kArrayType);
			rapidJsonArray.Reserve(static_cast<rapidjson::SizeType>(arraySize), mAllocator);
			SaveJsonValue(std::move(rapidJsonArray));
			auto& lastJsonValue = (*this->mNode)[this->mNode->Size() - 1];
			return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(&lastJsonValue, mAllocator, this->GetContext(), this);
		}
	}

protected:
	RapidJsonNode& LoadNextItem()
	{
		static_assert(TMode == SerializeMode::Load);
		if (mValueIt != this->mNode->End())
		{
			auto& jsonValue = *mValueIt;
			++mValueIt;
			return jsonValue;
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "No more items to load");
	}

	template <typename T>
	void SaveJsonValue(T&& value) const
	{
		assert(this->mNode->Size() < this->mNode->Capacity());
		this->mNode->PushBack(std::forward<T>(value), mAllocator);
	}

	void SaveJsonValue(std::nullptr_t&) const
	{
		assert(this->mNode->Size() < this->mNode->Capacity());
		this->mNode->PushBack(RapidJsonNode(), mAllocator);
	}

	TAllocator& mAllocator;
	iterator mValueIt;
};

/// <summary>
/// Constant iterator for keys.
/// </summary>
template <class TEncoding>
class key_const_iterator
{
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using member_iterator = typename RapidJsonNode::MemberIterator;
	using char_type = typename TEncoding::Ch;

	template <SerializeMode TMode, class Encoding, class TAllocator>
	friend class RapidJsonObjectScope;

	member_iterator mJsonIt;

	key_const_iterator(member_iterator&& it)
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

	const char_type* operator*() const {
		return mJsonIt->name.GetString();
	}
};

/// <summary>
/// JSON scope for serializing objects (list of values with keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
template <SerializeMode TMode, class TEncoding, class TAllocator>
class RapidJsonObjectScope final : public TArchiveScope<TMode>, public RapidJsonScopeBase<TEncoding>
{
public:
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using key_type = typename RapidJsonArchiveTraits<TEncoding>::key_type;
	using key_type_view = std::basic_string_view<typename TEncoding::Ch>;

	RapidJsonObjectScope(RapidJsonNode* node, TAllocator& allocator, SerializationContext& serializationContext, RapidJsonScopeBase<TEncoding>* parent = nullptr, key_type_view parentKey = {})
		: TArchiveScope<TMode>(serializationContext)
		, RapidJsonScopeBase<TEncoding>(node, parent, parentKey)
		, mAllocator(allocator)
	{
		assert(this->mNode->IsObject());
	}

	[[nodiscard]] key_const_iterator<TEncoding> cbegin() const {
		return key_const_iterator<TEncoding>(this->mNode->GetObject().begin());
	}

	[[nodiscard]] key_const_iterator<TEncoding> cend() const {
		return key_const_iterator<TEncoding>(this->mNode->GetObject().end());
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = this->LoadJsonValue(std::forward<TKey>(key));
			return jsonValue == nullptr ? false : this->LoadValue(*jsonValue, value, this->GetOptions());
		}
		else {
			if constexpr (std::is_arithmetic_v<T>) {
				return SaveJsonValue(std::forward<TKey>(key), RapidJsonNode(value));
			}
			else {
				return SaveJsonValue(std::forward<TKey>(key), RapidJsonNode());
			}
		}
	}

	template <typename TKey, typename TSym, typename TStrAllocator>
	bool SerializeValue(TKey&& key, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = this->LoadJsonValue(std::forward<TKey>(key));
			return jsonValue == nullptr ? false : this->LoadValue(*jsonValue, value, this->GetOptions());
		}
		else {
			return SaveJsonValue(std::forward<TKey>(key), RapidJsonScopeBase<TEncoding>::MakeRapidJsonNodeFromString(value, mAllocator));
		}
	}

	template <typename TKey>
	std::optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>> OpenObjectScope(TKey&& key)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(std::forward<TKey>(key));
			if (jsonValue != nullptr && jsonValue->IsObject())
				return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(jsonValue, mAllocator, this->GetContext(), this, key);
			return std::nullopt;
		}
		else
		{
			SaveJsonValue(std::forward<TKey>(key), RapidJsonNode(rapidjson::kObjectType));
			auto& insertedMember = FindMember(std::forward<TKey>(key))->value;
			return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(&insertedMember, mAllocator, this->GetContext(), this, key);
		}
	}

	template <typename TKey>
	std::optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>> OpenArrayScope(TKey&& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(std::forward<TKey>(key));
			if (jsonValue != nullptr && jsonValue->IsArray())
				return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(jsonValue, mAllocator, this->GetContext(), this, key);
			return std::nullopt;
		}
		else
		{
			auto rapidJsonArray = RapidJsonNode(rapidjson::kArrayType);
			rapidJsonArray.Reserve(static_cast<rapidjson::SizeType>(arraySize), mAllocator);
			SaveJsonValue(std::forward<TKey>(key), std::move(rapidJsonArray));
			auto& insertedMember = FindMember(std::forward<TKey>(key))->value;
			return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(&insertedMember, mAllocator, this->GetContext(), this, key);
		}
	}

protected:
	typename RapidJsonNode::MemberIterator FindMember(const key_type& key) const {
		return this->mNode->GetObject().FindMember(key.c_str());
	}

	typename RapidJsonNode::MemberIterator FindMember(const wchar_t* key) const {
		return this->mNode->GetObject().FindMember(key);
	}

	RapidJsonNode* LoadJsonValue(const key_type& key) const
	{
		const auto jObject = this->mNode->GetObject();
		auto it = jObject.FindMember(key.c_str());
		return it == jObject.MemberEnd() ? nullptr : &it->value;
	}

	RapidJsonNode* LoadJsonValue(const wchar_t* key) const
	{
		const auto jObject = this->mNode->GetObject();
		const auto it = jObject.FindMember(key);
		return it == jObject.MemberEnd() ? nullptr : &it->value;
	}

	bool SaveJsonValue(const key_type& key, RapidJsonNode&& jsonValue) const
	{
		// Checks that object was not saved previously under the same key
		assert(this->mNode->GetObject().FindMember(key.c_str()) == this->mNode->GetObject().MemberEnd());

		auto jsonKey = RapidJsonNode(key.data(), static_cast<rapidjson::SizeType>(key.size()), mAllocator);
		this->mNode->AddMember(jsonKey.Move(), jsonValue.Move(), mAllocator);
		return true;
	}

	bool SaveJsonValue(const wchar_t* key, RapidJsonNode&& jsonValue) const
	{
		// Checks that object was not saved previously under the same key
		assert(this->mNode->GetObject().FindMember(key) == this->mNode->GetObject().MemberEnd());

		this->mNode->AddMember(RapidJsonNode(typename RapidJsonNode::StringRefType(key)), jsonValue.Move(), mAllocator);
		return true;
	}

	TAllocator& mAllocator;
};


/// <summary>
/// JSON root scope (can serialize one value, array or object without key)
/// </summary>
template <SerializeMode TMode, class TEncoding>
class RapidJsonRootScope final : public TArchiveScope<TMode>, public RapidJsonScopeBase<TEncoding>
{
protected:
	using RapidJsonDocument = rapidjson::GenericDocument<TEncoding>;
	using allocator_type = typename RapidJsonDocument::AllocatorType;
	using char_type = typename TEncoding::Ch;

public:
	RapidJsonRootScope(const std::string& encodedInputStr, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		if (mRootJson.Parse(encodedInputStr.data(), encodedInputStr.length()).HasParseError())
			throw ParsingException(rapidjson::GetParseError_En(mRootJson.GetParseError()), 0, mRootJson.GetErrorOffset());
	}

	RapidJsonRootScope(std::string& encodedOutputStr, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(&encodedOutputStr)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	RapidJsonRootScope(std::istream& encodedInputStream, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		rapidjson::IStreamWrapper isw(encodedInputStream);
		rapidjson::AutoUTFInputStream<uint32_t, rapidjson::IStreamWrapper> eis(isw);
		if (mRootJson.ParseStream(eis).HasParseError())
			throw ParsingException(rapidjson::GetParseError_En(mRootJson.GetParseError()), 0, mRootJson.GetErrorOffset());
	}

	RapidJsonRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(&outputStream)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>, int> = 0>
	bool SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return this->LoadValue(mRootJson, value, this->GetOptions());
		}
		else
		{
			if constexpr (std::is_same_v<T, bool>) {
				mRootJson.SetBool(value);
			}
			else if constexpr (std::is_integral_v<T>)
			{
				if constexpr (std::is_same_v<T, int64_t>) {
					mRootJson.SetInt64(value);
				}
				else if constexpr (std::is_same_v<T, uint64_t>) {
					mRootJson.SetUint64(value);
				}
				else {
					mRootJson.SetInt(value);
				}
			}
			else if constexpr (std::is_floating_point_v<T>) {
				mRootJson.SetDouble(value);
			} else {
				mRootJson.SetNull();
			}
			return true;
		}
	}

	template <typename TSym, typename TAllocator>
	bool SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return this->LoadValue(mRootJson, value, this->GetOptions());
		}
		else
		{
			if constexpr (std::is_same_v<TSym, char_type>)
				mRootJson.SetString(value.data(), static_cast<rapidjson::SizeType>(value.size()), mRootJson.GetAllocator());
			else {
				const auto str = Convert::To<std::basic_string<char_type, std::char_traits<char_type>>>(value);
				mRootJson.SetString(str.data(), static_cast<rapidjson::SizeType>(str.size()), mRootJson.GetAllocator());
			}
			return true;
		}
	}

	std::optional<RapidJsonArrayScope<TMode, TEncoding, allocator_type>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return mRootJson.IsArray()
				? std::make_optional<RapidJsonArrayScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator(), this->GetContext())
				: std::nullopt;
		}
		else
		{
			mRootJson.SetArray().Reserve(static_cast<rapidjson::SizeType>(arraySize), mRootJson.GetAllocator());
			return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator(), this->GetContext());
		}
	}

	std::optional<RapidJsonObjectScope<TMode, TEncoding, allocator_type>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return mRootJson.IsObject()
				? std::make_optional<RapidJsonObjectScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator(), this->GetContext())
				: std::nullopt;
		}
		else
		{
			mRootJson.SetObject();
			return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator(), this->GetContext());
		}
	}

	void Finalize()
	{
		if constexpr (TMode == SerializeMode::Save)
		{
			std::visit([this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				auto& options = this->GetOptions();
				if constexpr (std::is_same_v<T, std::string*>)
				{
					using StringBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF8<>>;
					StringBuffer buffer;
					if (options.formatOptions.enableFormat)
					{
						rapidjson::PrettyWriter<StringBuffer, TEncoding, rapidjson::UTF8<>> writer(buffer);
						writer.SetIndent(options.formatOptions.paddingChar, options.formatOptions.paddingCharNum);
						mRootJson.Accept(writer);
					}
					else
					{
						rapidjson::Writer<StringBuffer, TEncoding, rapidjson::UTF8<>> writer(buffer);
						mRootJson.Accept(writer);
					}
					*arg = buffer.GetString();
				}
				else if constexpr (std::is_same_v<T, std::ostream*>)
				{
					rapidjson::OStreamWrapper osw(*arg);
					using AutoOutputStream = rapidjson::AutoUTFOutputStream<uint32_t, rapidjson::OStreamWrapper>;
					AutoOutputStream eos(osw, ToRapidUtfType(options.streamOptions.encoding), options.streamOptions.writeBom);
					if (options.formatOptions.enableFormat)
					{
						rapidjson::PrettyWriter<AutoOutputStream, TEncoding, rapidjson::AutoUTF<uint32_t>> writer(eos);
						writer.SetIndent(options.formatOptions.paddingChar, options.formatOptions.paddingCharNum);
						mRootJson.Accept(writer);
					}
					else
					{
						rapidjson::Writer<AutoOutputStream, TEncoding, rapidjson::AutoUTF<uint32_t>> writer(eos);
						mRootJson.Accept(writer);
					}
				}
			}, mOutput);
			mOutput = nullptr;
		}
	}

private:
	static rapidjson::UTFType ToRapidUtfType(const Convert::UtfType utfType)
	{
		switch (utfType)
		{
		case Convert::UtfType::Utf8:
			return rapidjson::UTFType::kUTF8;
		case Convert::UtfType::Utf16le:
			return rapidjson::UTFType::kUTF16LE;
		case Convert::UtfType::Utf16be:
			return rapidjson::UTFType::kUTF16BE;
		case Convert::UtfType::Utf32le:
			return rapidjson::UTFType::kUTF32LE;
		case Convert::UtfType::Utf32be:
			return rapidjson::UTFType::kUTF32BE;
		default:
			const auto strEncodingType = Convert::TryTo<std::string>(utfType);
			throw SerializationException(SerializationErrorCode::UnsupportedEncoding,
				"The archive does not support encoding: " +
					(strEncodingType.has_value() ? strEncodingType.value() : std::to_string(static_cast<int>(utfType))));
		}
	}

	RapidJsonDocument mRootJson;
	std::variant<decltype(nullptr), std::string*, std::ostream*> mOutput;
};

}


/// <summary>
/// JSON archive based on RapidJson library.
/// Supports load/save from:
/// - <c>std::string</c>: UTF-8
/// - <c>std::istream</c> and <c>std::ostream</c>: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE
/// </summary>
using JsonArchive = TArchiveBase<
	Detail::RapidJsonArchiveTraits<rapidjson::UTF8<>>,
	Detail::RapidJsonRootScope<SerializeMode::Load, rapidjson::UTF8<>>,
	Detail::RapidJsonRootScope<SerializeMode::Save, rapidjson::UTF8<>>>;

}
