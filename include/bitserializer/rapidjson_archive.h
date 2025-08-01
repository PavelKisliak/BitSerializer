﻿/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
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

#ifdef GetObject
// see https://github.com/Tencent/rapidjson/issues/1448
// a former included windows.h might have defined a macro called GetObject, which affects
// GetObject defined here. This ensures the macro does not get applied
#pragma push_macro("GetObject")
#define RAPIDJSON_WINDOWS_GETOBJECT_WORKAROUND_APPLIED
#undef GetObject
#endif

namespace BitSerializer::Json::RapidJson {
namespace Detail {

template <typename TSym>
using RapidJsonEncoding = typename rapidjson::UTF8<TSym>;

/**
 * @brief JSON archive traits (based on RapidJson).
 */
template <class TEncoding = RapidJsonEncoding<char>>
struct RapidJsonArchiveTraits  // NOLINT(cppcoreguidelines-special-member-functions)
{
	static constexpr ArchiveType archive_type = ArchiveType::Json;
	using key_type = std::basic_string<typename TEncoding::Ch, std::char_traits<typename TEncoding::Ch>>;
	using supported_key_types = TSupportedKeyTypes<const typename TEncoding::Ch*, key_type>;
	using string_view_type = std::basic_string_view<typename TEncoding::Ch>;
	using preferred_output_format = std::basic_string<char, std::char_traits<char>>;
	using preferred_stream_char_type = char;
	static constexpr char path_separator = '/';
	static constexpr bool is_binary = false;

protected:
	~RapidJsonArchiveTraits() = default;
};

// Forward declarations
template <SerializeMode TMode, class TEncoding, class TAllocator>
class RapidJsonObjectScope;


/**
 * @brief Base class of JSON scope.
 */
template <class TEncoding>
class RapidJsonScopeBase : public RapidJsonArchiveTraits<TEncoding>
{
public:
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using key_type_view = std::basic_string_view<typename TEncoding::Ch>;
	using string_view_type = typename RapidJsonArchiveTraits<TEncoding>::string_view_type;

	RapidJsonScopeBase(RapidJsonNode* node, RapidJsonScopeBase<TEncoding>* parent = nullptr, key_type_view parentKey = {})
		: mNode(node)
		, mParent(parent)
		, mParentKey(parentKey)
	{ }

	RapidJsonScopeBase(const RapidJsonScopeBase&) = delete;
	RapidJsonScopeBase& operator=(const RapidJsonScopeBase&) = delete;

	/**
	 * @brief Gets the current path in JSON (RFC 6901 - JSON Pointer).
	 */
	[[nodiscard]] virtual std::string GetPath() const
	{
		const std::string localPath = mParentKey.empty()
			? std::string()
			: RapidJsonArchiveTraits<TEncoding>::path_separator + Convert::ToString(mParentKey);
		return mParent == nullptr ? localPath : mParent->GetPath() + localPath;
	}

protected:
	~RapidJsonScopeBase() = default;
	RapidJsonScopeBase(RapidJsonScopeBase&&) noexcept = default;
	RapidJsonScopeBase& operator=(RapidJsonScopeBase&&) noexcept = default;

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool LoadValue(const RapidJsonNode& jsonValue, T& value, const SerializationOptions& serializationOptions)
	{
		// Null value from JSON is excluded from MismatchedTypesPolicy processing
		if (jsonValue.IsNull()) {
			return std::is_null_pointer_v<T>;
		}

		using BitSerializer::Detail::ConvertByPolicy;
		if constexpr (std::is_integral_v<T>)
		{
			if (jsonValue.IsNumber())
			{
				if (jsonValue.IsInt64()) {
					return ConvertByPolicy(jsonValue.GetInt64(), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
				}
				if (jsonValue.IsUint64()) {
					return ConvertByPolicy(jsonValue.GetUint64(), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
				}
			}
			else if (jsonValue.IsBool())
			{
				return ConvertByPolicy(jsonValue.GetBool(), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
			}
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			if (jsonValue.IsNumber()) {
				return ConvertByPolicy(jsonValue.GetDouble(), value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
			}
		}

		HandleMismatchedTypesPolicy(serializationOptions.mismatchedTypesPolicy);
		return false;
	}

	bool LoadValue(const RapidJsonNode& jsonValue, string_view_type& value, const SerializationOptions& serializationOptions)
	{
		// Null value from JSON is excluded from MismatchedTypesPolicy processing
		if (jsonValue.IsNull()) {
			return false;
		}

		if (!jsonValue.IsString())
		{
			HandleMismatchedTypesPolicy(serializationOptions.mismatchedTypesPolicy);
			return false;
		}

		value = string_view_type(jsonValue.GetString(), jsonValue.GetStringLength());
		return true;
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


/**
 * @brief JSON scope for serializing arrays (sequential values).
 */
template <SerializeMode TMode, class TEncoding, class TAllocator>
class RapidJsonArrayScope final : public TArchiveScope<TMode>, public RapidJsonScopeBase<TEncoding>
{
public:
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using iterator = typename RapidJsonNode::ValueIterator;
	using key_type_view = std::basic_string_view<typename TEncoding::Ch>;
	using string_view_type = typename RapidJsonArchiveTraits<TEncoding>::string_view_type;

	RapidJsonArrayScope(RapidJsonNode* node, TAllocator& allocator, SerializationContext& serializationContext, RapidJsonScopeBase<TEncoding>* parent = nullptr, key_type_view parentKey = {})
		: TArchiveScope<TMode>(serializationContext)
		, RapidJsonScopeBase<TEncoding>(node, parent, parentKey)
		, mAllocator(allocator)
		, mValueIt(this->mNode->GetArray().Begin())
	{
		assert(this->mNode->IsArray());
	}

	/**
	 * @brief Returns the estimated number of items to load (for reserving the size of containers).
	 */
	[[nodiscard]] size_t GetEstimatedSize() const {
		return this->mNode->Capacity();
	}

	/**
	 * @brief Returns `true` when there are no more values to load.
	 */
	bool IsEnd()
	{
		static_assert(TMode == SerializeMode::Load);
		return mValueIt == this->mNode->GetArray().End();
	}

	/**
	 * @brief Gets the current path in JSON (RFC 6901 - JSON Pointer).
	 */
	[[nodiscard]] std::string GetPath() const override
	{
		int64_t index;
		if constexpr (TMode == SerializeMode::Load) {
			index = std::distance(this->mNode->Begin(), mValueIt);
		}
		else {
			index = this->mNode->GetArray().Size();
		}
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

	bool SerializeValue(string_view_type& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return this->LoadValue(LoadNextItem(), value, this->GetOptions());
		}
		else 
		{
			SaveJsonValue(RapidJsonNode(value.data(), static_cast<rapidjson::SizeType>(value.size()), mAllocator));
			return true;
		}
	}

	std::optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>> OpenObjectScope(size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadNextItem();
			if (jsonValue.IsObject()) {
				return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(&jsonValue, mAllocator, this->GetContext(), this);
			}
			RapidJsonScopeBase<TEncoding>::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
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
			RapidJsonScopeBase<TEncoding>::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			return std::nullopt;
		}
		else
		{
			auto rapidJsonArray = RapidJsonNode(rapidjson::kArrayType);
			if (arraySize) {
				rapidJsonArray.Reserve(static_cast<rapidjson::SizeType>(arraySize), mAllocator);
			}
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
		if constexpr (std::is_constructible_v<RapidJsonNode, T>) {
			this->mNode->PushBack(std::forward<T>(value), mAllocator);
		}
		else
		{
			// Some integer types don't have fixed-size convertible types
			const auto compatibleValue = static_cast<compatible_fixed_t<T>>(value);
			this->mNode->PushBack(compatibleValue, mAllocator);
		}
	}

	void SaveJsonValue(std::nullptr_t&) const
	{
		assert(this->mNode->Size() < this->mNode->Capacity());
		this->mNode->PushBack(RapidJsonNode(), mAllocator);
	}

	TAllocator& mAllocator;
	iterator mValueIt;
};


/**
 * @brief JSON scope for serializing objects (key-value pairs).
 */
template <SerializeMode TMode, class TEncoding, class TAllocator>
class RapidJsonObjectScope final : public TArchiveScope<TMode>, public RapidJsonScopeBase<TEncoding>
{
public:
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using key_type = typename RapidJsonArchiveTraits<TEncoding>::key_type;
	using key_type_view = std::basic_string_view<typename TEncoding::Ch>;
	using string_view_type = typename RapidJsonArchiveTraits<TEncoding>::string_view_type;
	using key_raw_ptr = const typename TEncoding::Ch*;

	RapidJsonObjectScope(RapidJsonNode* node, TAllocator& allocator, SerializationContext& serializationContext, RapidJsonScopeBase<TEncoding>* parent = nullptr, key_type_view parentKey = {})
		: TArchiveScope<TMode>(serializationContext)
		, RapidJsonScopeBase<TEncoding>(node, parent, parentKey)
		, mAllocator(allocator)
	{
		assert(this->mNode->IsObject());
	}

	[[nodiscard]] static size_t GetEstimatedSize() {
		return 0;
	}

	/**
	 * @brief Enumerates all keys in the current object.
	 *
	 * @tparam TCallback Callback function type.
	 * @param fn Callback to invoke for each key.
	 */
	template <typename TCallback>
	void VisitKeys(TCallback&& fn)
	{
		for (const auto& keyVal : this->mNode->GetObject()) {
			fn(keyVal.name.GetString());
		}
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = this->LoadJsonValue(std::forward<TKey>(key));
			return jsonValue == nullptr ? false : this->LoadValue(*jsonValue, value, this->GetOptions());
		}
		else
		{
			if constexpr (std::is_arithmetic_v<T>) {
				if constexpr (std::is_constructible_v<RapidJsonNode, T>) {
					return SaveJsonValue(std::forward<TKey>(key), RapidJsonNode(value));
				}
				else {
					// Some integer types don't have fixed-size convertible types
					return SaveJsonValue(std::forward<TKey>(key), RapidJsonNode(static_cast<compatible_fixed_t<T>>(value)));
				}
			}
			else {
				return SaveJsonValue(std::forward<TKey>(key), RapidJsonNode());
			}
		}
	}

	template <typename TKey>
	bool SerializeValue(TKey&& key, string_view_type& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = this->LoadJsonValue(std::forward<TKey>(key));
			return jsonValue == nullptr ? false : this->LoadValue(*jsonValue, value, this->GetOptions());
		}
		else {
			return SaveJsonValue(std::forward<TKey>(key), RapidJsonNode(value.data(), static_cast<rapidjson::SizeType>(value.size()), mAllocator));
		}
	}

	template <typename TKey>
	std::optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>> OpenObjectScope(TKey&& key, size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (auto* jsonValue = LoadJsonValue(std::forward<TKey>(key)))
			{
				if (jsonValue->IsObject())
				{
					return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(jsonValue, mAllocator, this->GetContext(), this, key);
				}
				RapidJsonScopeBase<TEncoding>::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
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
			if (auto* jsonValue = LoadJsonValue(std::forward<TKey>(key)))
			{
				if (jsonValue->IsArray())
				{
					return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(jsonValue, mAllocator, this->GetContext(), this, key);
				}
				RapidJsonScopeBase<TEncoding>::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
			return std::nullopt;
		}
		else
		{
			auto rapidJsonArray = RapidJsonNode(rapidjson::kArrayType);
			if (arraySize) {
				rapidJsonArray.Reserve(static_cast<rapidjson::SizeType>(arraySize), mAllocator);
			}
			SaveJsonValue(std::forward<TKey>(key), std::move(rapidJsonArray));
			auto& insertedMember = FindMember(std::forward<TKey>(key))->value;
			return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(&insertedMember, mAllocator, this->GetContext(), this, key);
		}
	}

protected:
	[[nodiscard]] typename RapidJsonNode::MemberIterator FindMember(const key_type& key) const {
		return this->mNode->GetObject().FindMember(key.c_str());
	}

	[[nodiscard]] typename RapidJsonNode::MemberIterator FindMember(key_raw_ptr key) const {
		return this->mNode->GetObject().FindMember(key);
	}

	[[nodiscard]] RapidJsonNode* LoadJsonValue(const key_type& key) const
	{
		const auto jObject = this->mNode->GetObject();
		auto it = jObject.FindMember(key.c_str());
		return it == jObject.MemberEnd() ? nullptr : &it->value;
	}

	[[nodiscard]] RapidJsonNode* LoadJsonValue(key_raw_ptr key) const
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
		this->mNode->AddMember(std::move(jsonKey), std::move(jsonValue), mAllocator);
		return true;
	}

	bool SaveJsonValue(key_raw_ptr key, RapidJsonNode&& jsonValue) const
	{
		// Checks that object was not saved previously under the same key
		assert(this->mNode->GetObject().FindMember(key) == this->mNode->GetObject().MemberEnd());

		this->mNode->AddMember(RapidJsonNode(typename RapidJsonNode::StringRefType(key)), std::move(jsonValue), mAllocator);
		return true;
	}

private:
	TAllocator& mAllocator;
};


/**
 * @brief JSON root scope for serializing data (can serialize one value, array or object without key).
 */
template <SerializeMode TMode, class TEncoding = RapidJsonEncoding<char>>
class RapidJsonRootScope final : public TArchiveScope<TMode>, public RapidJsonScopeBase<TEncoding>
{
protected:
	using RapidJsonDocument = rapidjson::GenericDocument<TEncoding>;
	using allocator_type = typename RapidJsonDocument::AllocatorType;
	using char_type = typename TEncoding::Ch;

public:
	using string_view_type = typename RapidJsonArchiveTraits<TEncoding>::string_view_type;

	RapidJsonRootScope(const std::string_view& encodedInputStr, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		if (mRootJson.Parse(encodedInputStr.data(), encodedInputStr.length()).HasParseError()) {
			throw ParsingException(rapidjson::GetParseError_En(mRootJson.GetParseError()), 0, mRootJson.GetErrorOffset());
		}
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
		if (mRootJson.ParseStream(eis).HasParseError()) {
			throw ParsingException(rapidjson::GetParseError_En(mRootJson.GetParseError()), 0, mRootJson.GetErrorOffset());
		}
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
				if constexpr (std::is_signed_v<T>)
				{
					if constexpr (sizeof(T) == sizeof(int64_t)) {
						mRootJson.SetInt64(value);
					}
					else {
						mRootJson.SetInt(value);
					}
				}
				else
				{
					if constexpr (sizeof(T) == sizeof(uint64_t)) {
						mRootJson.SetUint64(value);
					}
					else {
						mRootJson.SetUint(value);
					}
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

	bool SerializeValue(string_view_type& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return this->LoadValue(mRootJson, value, this->GetOptions());
		}
		else
		{
			mRootJson.SetString(value.data(), static_cast<rapidjson::SizeType>(value.size()), mRootJson.GetAllocator());
			return true;	// NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
		}
	}

	std::optional<RapidJsonArrayScope<TMode, TEncoding, allocator_type>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mRootJson.IsArray())
			{
				return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator(), this->GetContext());
			}
			RapidJsonScopeBase<TEncoding>::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			return std::nullopt;
		}
		else
		{
			mRootJson.SetArray();
			if (arraySize) {
				mRootJson.Reserve(static_cast<rapidjson::SizeType>(arraySize), mRootJson.GetAllocator());
			}
			return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator(), this->GetContext());
		}
	}

	std::optional<RapidJsonObjectScope<TMode, TEncoding, allocator_type>> OpenObjectScope(size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mRootJson.IsObject())
			{
				return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator(), this->GetContext());
			}
			RapidJsonScopeBase<TEncoding>::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			return std::nullopt;
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
	static rapidjson::UTFType ToRapidUtfType(const Convert::Utf::UtfType utfType)
	{
		switch (utfType)
		{
		case Convert::Utf::UtfType::Utf8:
			return rapidjson::UTFType::kUTF8;
		case Convert::Utf::UtfType::Utf16le:
			return rapidjson::UTFType::kUTF16LE;
		case Convert::Utf::UtfType::Utf16be:
			return rapidjson::UTFType::kUTF16BE;
		case Convert::Utf::UtfType::Utf32le:
			return rapidjson::UTFType::kUTF32LE;
		case Convert::Utf::UtfType::Utf32be:
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


/**
 * @brief JSON archive based on RapidJson library.
 *
 * Supports load/save from:
 * - `std::string`: UTF-8
 * - `std::istream`, `std::ostream`: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE
 */
using JsonArchive = TArchiveBase<
	Detail::RapidJsonArchiveTraits<>,
	Detail::RapidJsonRootScope<SerializeMode::Load>,
	Detail::RapidJsonRootScope<SerializeMode::Save>>;

} // namespace BitSerializer::Json::RapidJson

#ifdef RAPIDJSON_WINDOWS_GETOBJECT_WORKAROUND_APPLIED
#pragma pop_macro("GetObject")
#undef RAPIDJSON_WINDOWS_GETOBJECT_WORKAROUND_APPLIED
#endif
