/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <type_traits>
#include <optional>
#include <variant>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/media_archive_base.h"

// External dependency (RapidJson)
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <rapidjson/stream.h>
#include "rapidjson/encodings.h"
#include "rapidjson/error/en.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

namespace BitSerializer::Json::RapidJson {
namespace Detail {

/// <summary>
/// The traits of JSON archive based on RapidJson
/// </summary>
template <class TEncoding>
struct RapidJsonArchiveTraits
{
	using key_type = std::basic_string<typename TEncoding::Ch, std::char_traits<typename TEncoding::Ch>>;
	using supported_key_types = SupportedKeyTypes<const typename TEncoding::Ch*, key_type>;
	using preferred_output_format = std::basic_string<typename TEncoding::Ch, std::char_traits<typename TEncoding::Ch>>;
	using preferred_stream_char_type = typename TEncoding::Ch;
	static const char path_separator = '/';

protected:
	~RapidJsonArchiveTraits() = default;
};

// Forward declarations
template <SerializeMode TMode, class TEncoding, class TAllocator>
class RapidJsonObjectScope;

/// <summary>
/// Base class of JSON scope
/// </summary>
/// <seealso cref="MediaArchiveBase" />
template <class TEncoding>
class RapidJsonScopeBase : public RapidJsonArchiveTraits<TEncoding>
{
public:
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using key_type_view = std::basic_string_view<typename TEncoding::Ch>;

	RapidJsonScopeBase(const RapidJsonNode* node, RapidJsonScopeBase<TEncoding>* parent = nullptr, key_type_view parentKey = {})
		: mNode(const_cast<RapidJsonNode*>(node))
		, mParent(parent)
		, mParentKey(parentKey)
	{ }

	RapidJsonScopeBase(const RapidJsonScopeBase&) = delete;
	RapidJsonScopeBase& operator=(const RapidJsonScopeBase&) = delete;

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer). Unicode symbols encode to UTF-8.
	/// </summary>
	virtual std::string GetPath() const
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

	static bool LoadValue(const RapidJsonNode& jsonValue, bool& value)
	{
		if (jsonValue.IsBool()) {
			value = jsonValue.GetBool();
			return true;
		}
		return false;
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	static bool LoadValue(const RapidJsonNode& jsonValue, T& value)
	{
		if (!jsonValue.IsNumber())
			return false;

		if constexpr (std::is_integral_v<T>)
		{
			if constexpr (std::is_same_v<T, int64_t>) {
				value = jsonValue.GetInt64();
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				value = jsonValue.GetUint64();
			}
			else {
				value = static_cast<T>(jsonValue.GetInt());
			}
		}
		else {
			value = static_cast<T>(jsonValue.GetDouble());
		}
		return true;
	}

	template <typename TSym, typename TAllocator>
	static bool LoadValue(const RapidJsonNode& jsonValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if (!jsonValue.IsString())
			return false;

		if constexpr (std::is_same_v<TSym, typename RapidJsonNode::EncodingType::Ch>)
			value = jsonValue.GetString();
		else
			value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(jsonValue.GetString());
		return true;
	}

	template <typename TSym, typename TAllocator, typename TRapidAllocator>
	static RapidJsonNode MakeRapidJsonNodeFromString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value, TRapidAllocator& allocator)
	{
		using TargetSymType = typename TEncoding::Ch;
		if constexpr (std::is_same_v<TSym, TargetSymType>)
			return RapidJsonNode(value.data(), static_cast<rapidjson::SizeType>(value.size()), allocator);
		else {
			const auto str = Convert::To<std::basic_string<TargetSymType, std::char_traits<TargetSymType>>>(value);
			return RapidJsonNode(str.data(), static_cast<rapidjson::SizeType>(str.size()), allocator);
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
class RapidJsonArrayScope final : public ArchiveScope<TMode>, public RapidJsonScopeBase<TEncoding>
{
public:
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using iterator = typename RapidJsonNode::ValueIterator;
	using key_type_view = std::basic_string_view<typename TEncoding::Ch>;

	RapidJsonArrayScope(const RapidJsonNode* node, TAllocator& allocator, RapidJsonScopeBase<TEncoding>* parent = nullptr, key_type_view parentKey = {})
		: RapidJsonScopeBase<TEncoding>(node, parent, parentKey)
		, mAllocator(allocator)
		, mValueIt(this->mNode->GetArray().Begin())
	{
		assert(this->mNode->IsArray());
	}

	/// <summary>
	/// Returns the size of stored elements (for arrays and objects).
	/// </summary>
	size_t GetSize() const {
		return this->mNode->Capacity();
	}

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer). Unicode symbols encode to UTF-8.
	/// </summary>
	std::string GetPath() const override
	{
		int64_t index;
		if constexpr (TMode == SerializeMode::Load)
			index = std::distance(this->mNode->Begin(), mValueIt);
		else
			index = this->mNode->GetArray().Size();
		return RapidJsonScopeBase<TEncoding>::GetPath()
			+ RapidJsonArchiveTraits<TEncoding>::path_separator
			+ Convert::ToString(index == 0 ? 0 : index - 1);
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = NextElement();
			if (jsonValue != nullptr)
				this->LoadValue(*jsonValue, value);
		}
		else {
			SaveJsonValue(RapidJsonNode(value));
		}
	}

	template <typename TSym, typename TStrAllocator>
	void SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			auto* jsonValue = NextElement();
			if (jsonValue != nullptr)
				this->LoadValue(*jsonValue, value);
		}
		else {
			SaveJsonValue(RapidJsonScopeBase<TEncoding>::MakeRapidJsonNodeFromString(value, mAllocator));
		}
	}

	std::optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = NextElement();
			if (jsonValue != nullptr && jsonValue->IsObject())
				return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(jsonValue, mAllocator, this);
			return std::nullopt;
		}
		else
		{
			SaveJsonValue(RapidJsonNode(rapidjson::kObjectType));
			auto& lastJsonValue = (*this->mNode)[this->mNode->Size() - 1];
			return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(&lastJsonValue, mAllocator, this);
		}
	}

	std::optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = NextElement();
			if (jsonValue != nullptr && jsonValue->IsArray())
				return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(jsonValue, mAllocator, this);
			return std::nullopt;
		}
		else
		{
			auto rapidJsonArray = RapidJsonNode(rapidjson::kArrayType);
			rapidJsonArray.Reserve(static_cast<rapidjson::SizeType>(arraySize), mAllocator);
			SaveJsonValue(std::move(rapidJsonArray));
			auto& lastJsonValue = (*this->mNode)[this->mNode->Size() - 1];
			return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(&lastJsonValue, mAllocator, this);
		}
	}

protected:
	const RapidJsonNode* NextElement()
	{
		if (mValueIt == this->mNode->End())
			return nullptr;
		const auto& jsonValue = *mValueIt;
		++mValueIt;
		return &jsonValue;
	}

	void SaveJsonValue(RapidJsonNode&& jsonValue) const
	{
		assert(this->mNode->Size() < this->mNode->Capacity());
		this->mNode->PushBack(jsonValue.Move(), mAllocator);
	}

	TAllocator& mAllocator;
	iterator mValueIt;
};

/// <summary>
/// Constant iterator of the keys.
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
class RapidJsonObjectScope final : public ArchiveScope<TMode>, public RapidJsonScopeBase<TEncoding>
{
public:
	using RapidJsonNode = rapidjson::GenericValue<TEncoding>;
	using key_type = typename RapidJsonArchiveTraits<TEncoding>::key_type;
	using key_type_view = std::basic_string_view<typename TEncoding::Ch>;

	RapidJsonObjectScope(const RapidJsonNode* node, TAllocator& allocator, RapidJsonScopeBase<TEncoding>* parent = nullptr, key_type_view parentKey = {})
		: RapidJsonScopeBase<TEncoding>(node, parent, parentKey)
		, mAllocator(allocator)
	{
		assert(this->mNode->IsObject());
	};

	key_const_iterator<TEncoding> cbegin() const {
		return key_const_iterator<TEncoding>(this->mNode->GetObject().begin());
	}

	key_const_iterator<TEncoding> cend() const {
		return key_const_iterator<TEncoding>(this->mNode->GetObject().end());
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = this->LoadJsonValue(std::forward<TKey>(key));
			return jsonValue == nullptr ? false : this->LoadValue(*jsonValue, value);
		}
		else {
			return SaveJsonValue(std::forward<TKey>(key), RapidJsonNode(value));
		}
	}

	template <typename TKey, typename TSym, typename TStrAllocator>
	bool SerializeValue(TKey&& key, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = this->LoadJsonValue(std::forward<TKey>(key));
			return jsonValue == nullptr ? false : this->LoadValue(*jsonValue, value);
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
				return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(jsonValue, mAllocator, this, key);
			return std::nullopt;
		}
		else
		{
			SaveJsonValue(std::forward<TKey>(key), RapidJsonNode(rapidjson::kObjectType));
			auto& insertedMember = FindMember(std::forward<TKey>(key))->value;
			return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, TAllocator>>(&insertedMember, mAllocator, this, key);
		}
	}

	template <typename TKey>
	std::optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>> OpenArrayScope(TKey&& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(std::forward<TKey>(key));
			if (jsonValue != nullptr && jsonValue->IsArray())
				return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(jsonValue, mAllocator, this, key);
			return std::nullopt;
		}
		else
		{
			auto rapidJsonArray = RapidJsonNode(rapidjson::kArrayType);
			rapidJsonArray.Reserve(static_cast<rapidjson::SizeType>(arraySize), mAllocator);
			SaveJsonValue(std::forward<TKey>(key), std::move(rapidJsonArray));
			auto& insertedMember = FindMember(std::forward<TKey>(key))->value;
			return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, TAllocator>>(&insertedMember, mAllocator, this, key);
		}
	}

protected:
	auto FindMember(const key_type& key) const {
		return this->mNode->GetObject().FindMember(key.c_str());
	}

	auto FindMember(const wchar_t* key) const {
		return this->mNode->GetObject().FindMember(key);
	}

	const RapidJsonNode* LoadJsonValue(const key_type& key) const
	{
		const auto jObject = this->mNode->GetObject();
		auto it = jObject.FindMember(key.c_str());
		return it == jObject.MemberEnd() ? nullptr : &it->value;
	}

	const RapidJsonNode* LoadJsonValue(const wchar_t* key) const
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
class RapidJsonRootScope final : public ArchiveScope<TMode>, public RapidJsonScopeBase<TEncoding>
{
protected:
	using RapidJsonDocument = rapidjson::GenericDocument<TEncoding>;
	using allocator_type = typename RapidJsonDocument::AllocatorType;
	using char_type = typename TEncoding::Ch;
	using string_type = std::basic_string<char_type, std::char_traits<char_type>>;

public:
	explicit RapidJsonRootScope(const char_type* inputStr)
		: RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		if (mRootJson.Parse(inputStr).HasParseError())
			throw SerializationException(SerializationErrorCode::ParsingError, rapidjson::GetParseError_En(mRootJson.GetParseError()));
	}

	explicit RapidJsonRootScope(const string_type& inputStr)
		: RapidJsonRootScope(inputStr.c_str()) {}

	explicit RapidJsonRootScope(string_type& outputStr)
		: RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(&outputStr)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	explicit RapidJsonRootScope(std::istream& encodedInputStream)
		: RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		rapidjson::IStreamWrapper isw(encodedInputStream);
		rapidjson::AutoUTFInputStream<uint32_t, rapidjson::IStreamWrapper> eis(isw);
		if (mRootJson.ParseStream(eis).HasParseError())
			throw SerializationException(SerializationErrorCode::ParsingError, rapidjson::GetParseError_En(mRootJson.GetParseError()));
	}

	explicit RapidJsonRootScope(std::wistream& inputStream)
		: RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		rapidjson::WIStreamWrapper isw(inputStream);
		if (mRootJson.ParseStream(isw).HasParseError())
			throw SerializationException(SerializationErrorCode::ParsingError, rapidjson::GetParseError_En(mRootJson.GetParseError()));
	}

	RapidJsonRootScope(std::ostream& outputStream, const SerializationOptions& serializationOptions = {})
		: RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(&outputStream)
		, mSerializationOptions(serializationOptions)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	RapidJsonRootScope(std::wostream& outputStream, const SerializationOptions& serializationOptions = {})
		: RapidJsonScopeBase<TEncoding>(&mRootJson)
		, mOutput(&outputStream)
		, mSerializationOptions(serializationOptions)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			this->LoadValue(mRootJson, value);
		}
		else
		{
			assert(mRootJson.IsNull());
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
			else {
				mRootJson.SetDouble(value);
			}
		}
	}

	template <typename TSym, typename TAllocator>
	void SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			this->LoadValue(mRootJson, value);
		}
		else
		{
			assert(mRootJson.IsNull());
			if constexpr (std::is_same_v<TSym, char_type>)
				mRootJson.SetString(value.data(), static_cast<rapidjson::SizeType>(value.size()), mRootJson.GetAllocator());
			else {
				const auto str = Convert::To<std::basic_string<char_type, std::char_traits<char_type>>>(value);
				mRootJson.SetString(str.data(), static_cast<rapidjson::SizeType>(str.size()), mRootJson.GetAllocator());
			}
		}
	}

	std::optional<RapidJsonArrayScope<TMode, TEncoding, allocator_type>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return mRootJson.IsArray()
				? std::make_optional<RapidJsonArrayScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator())
				: std::nullopt;
		}
		else
		{
			assert(mRootJson.IsNull());
			mRootJson.SetArray().Reserve(static_cast<rapidjson::SizeType>(arraySize), mRootJson.GetAllocator());
			return std::make_optional<RapidJsonArrayScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator());
		}
	}

	std::optional<RapidJsonObjectScope<TMode, TEncoding, allocator_type>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return mRootJson.IsObject()
				? std::make_optional<RapidJsonObjectScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator())
				: std::nullopt;
		}
		else
		{
			assert(mRootJson.IsNull());
			mRootJson.SetObject();
			return std::make_optional<RapidJsonObjectScope<TMode, TEncoding, allocator_type>>(&mRootJson, mRootJson.GetAllocator());
		}
	}

	void Finalize()
	{
		if constexpr (TMode == SerializeMode::Save)
		{
			std::visit([this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, string_type*>)
				{
					using StringBuffer = rapidjson::GenericStringBuffer<TEncoding>;
					StringBuffer buffer;
					rapidjson::Writer<StringBuffer, TEncoding, TEncoding> writer(buffer);
					mRootJson.Accept(writer);
					*arg = buffer.GetString();
				}
				else if constexpr (std::is_same_v<T, std::ostream*>)
				{
					rapidjson::OStreamWrapper osw(*arg);
					using AutoOutputStream = rapidjson::AutoUTFOutputStream<uint32_t, rapidjson::OStreamWrapper>;
					AutoOutputStream eos(osw, rapidjson::UTFType::kUTF8, mSerializationOptions->streamOptions.writeBom);
					rapidjson::Writer<AutoOutputStream, TEncoding, rapidjson::AutoUTF<uint32_t>> writer(eos);
					mRootJson.Accept(writer);
				}
				else if constexpr (std::is_same_v<T, std::wostream*>)
				{
					rapidjson::WOStreamWrapper osw(*arg);
					rapidjson::Writer<rapidjson::WOStreamWrapper, TEncoding, rapidjson::UTF16<>> writer(osw);
					mRootJson.Accept(writer);
				}
			}, mOutput);
			mOutput = nullptr;
		}
	}

private:
	RapidJsonDocument mRootJson;
	std::variant<decltype(nullptr), string_type*, std::ostream*, std::wostream*> mOutput;
	std::optional<SerializationOptions> mSerializationOptions;
};

} // namespace Detail


/// <summary>
/// JSON archive with in memory encoding in UTF-8, effective for cases when JSON contains mostly ASCII symbols and rarely Unicode.
/// </summary>
using JsonUtf8Archive = MediaArchiveBase<
	Detail::RapidJsonArchiveTraits<rapidjson::UTF8<>>,
	Detail::RapidJsonRootScope<SerializeMode::Load, rapidjson::UTF8<>>,
	Detail::RapidJsonRootScope<SerializeMode::Save, rapidjson::UTF8<>>>;

/// <summary>
/// JSON archive with in memory encoding in UTF-16, effective in cases when JSON mostly in Unicode, contains localizations strings, etc.
/// </summary>
using JsonUtf16Archive = MediaArchiveBase<
	Detail::RapidJsonArchiveTraits<rapidjson::UTF16<>>,
	Detail::RapidJsonRootScope<SerializeMode::Load, rapidjson::UTF16<>>,
	Detail::RapidJsonRootScope<SerializeMode::Save, rapidjson::UTF16<>>>;

/// <summary>
/// JSON archive based on RapidJson library.
///	Default archive with in memory encoding in UTF-8 (alias of JsonUtf8Archive).
/// </summary>
using JsonArchive = JsonUtf8Archive;

} // namespace BitSerializer::Json::RapidJson