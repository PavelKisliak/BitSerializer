/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>
#include <optional>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/archive_base.h"

// Definition of macros 'U' causes conflict with c4core template argument (see: blob.hpp) which uses in YAML archive implementation
#define _TURN_OFF_PLATFORM_STRING

// External dependency (C++ REST SDK)
#include "cpprest/json.h"

namespace BitSerializer::Json::CppRest {
namespace Detail {

/// <summary>
/// The traits of JSON archive based on C++ REST SDK
/// </summary>
struct JsonArchiveTraits
{
	static constexpr ArchiveType archive_type = ArchiveType::Json;
#ifdef _UTF16_STRINGS
	using key_type = std::wstring;
	using supported_key_types = TSupportedKeyTypes<std::wstring>;
#else
	using key_type = std::string;
	using supported_key_types = TSupportedKeyTypes<std::string>;
#endif
	using preferred_output_format = std::string;
	using preferred_stream_char_type = char;
	static constexpr char path_separator = '/';

protected:
	~JsonArchiveTraits() = default;
};

// Forward declarations
template <SerializeMode TMode>
class JsonObjectScope;

/// <summary>
/// Base class of JSON scope
/// </summary>
/// <seealso cref="TArchiveBase" />
class JsonScopeBase : public JsonArchiveTraits
{
public:
	using key_type_view = std::basic_string_view<key_type::value_type>;

	explicit JsonScopeBase(web::json::value* node, JsonScopeBase* parent = nullptr, key_type_view parentKey = {})
		: mNode(node)
		, mParent(parent)
		, mParentKey(parentKey)
	{ }

	JsonScopeBase(const JsonScopeBase&) = delete;
	JsonScopeBase& operator=(const JsonScopeBase&) = delete;

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer).
	/// </summary>
	[[nodiscard]] virtual std::string GetPath() const
	{
		const std::string localPath = mParentKey.empty()
			? std::string()
			: path_separator + Convert::ToString(mParentKey);
		return mParent == nullptr ? localPath : mParent->GetPath() + localPath;
	}

protected:
	~JsonScopeBase() = default;
	JsonScopeBase(JsonScopeBase&&) = default;
	JsonScopeBase& operator=(JsonScopeBase&&) = default;

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	static bool LoadFundamentalValue(const web::json::value& jsonValue, T& value)
	{
		if (!jsonValue.is_number())
			return false;

		if constexpr (std::is_integral_v<T>)
		{
			if constexpr (std::is_same_v<T, int64_t>) {
				value = jsonValue.as_number().to_int64();
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				value = jsonValue.as_number().to_uint64();
			}
			else {
				value = static_cast<T>(jsonValue.as_integer());
			}
		}
		else
		{
			value = static_cast<T>(jsonValue.as_double());
		}
		return true;
	}

	template <typename TSym, typename TAllocator>
	static bool LoadString(const web::json::value& jsonValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if (!jsonValue.is_string())
			return false;

		if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
			value = jsonValue.as_string();
		else
			value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(jsonValue.as_string());
		return true;
	}

	web::json::value* mNode;
	JsonScopeBase* mParent;
	key_type_view mParentKey;
};


/// <summary>
/// JSON scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="JsonScopeBase" />
template <SerializeMode TMode>
class JsonArrayScope final : public TArchiveScope<TMode>, public JsonScopeBase
{
public:
	JsonArrayScope(web::json::value* node, JsonScopeBase* parent = nullptr, key_type_view parentKey = {})
		: JsonScopeBase(node, parent, parentKey)
		, mSize(node == nullptr ? 0 : node->size())
		, mIndex(0)
	{
		assert(mNode->is_array());
	}

	[[nodiscard]] size_t GetSize() const {
		return mNode->size();
	}

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer).
	/// </summary>
	[[nodiscard]] std::string GetPath() const override
	{
		const auto index = mIndex == 0 ? 0 : mIndex - 1;
		return JsonScopeBase::GetPath() + path_separator + Convert::ToString(index);
	}

	void SerializeValue(bool& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mIndex < mSize) {
				const auto& jsonValue = (*mNode)[mIndex++];
				if (jsonValue.is_boolean())
					value = jsonValue.as_bool();
			}
		}
		else
		{
			SaveJsonValue(web::json::value(value));
		}
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load)	{
			if (mIndex < mSize)
				LoadFundamentalValue((*mNode)[mIndex++], value);
		}
		else {
			SaveJsonValue(web::json::value(value));
		}
	}

	template <typename TSym, typename TAllocator>
	void SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			if (mIndex < mSize)
				LoadString((*mNode)[mIndex++], value);
		}
		else
		{
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				SaveJsonValue(web::json::value(value));
			else
				SaveJsonValue(web::json::value(Convert::To<utility::string_t>(value)));
		}
	}

	std::optional<JsonObjectScope<TMode>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mIndex < mSize)
			{
				auto& jsonValue = (*mNode)[mIndex++];
				if (jsonValue.is_object())
					return std::make_optional<JsonObjectScope<TMode>>(&jsonValue, this);
			}
			return std::nullopt;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(web::json::value::object());
			return std::make_optional<JsonObjectScope<TMode>>(&jsonValue, this);
		}
	}

	std::optional<JsonArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mIndex < mSize) {
				auto& jsonValue = (*mNode)[mIndex++];
				if (jsonValue.is_array())
					return std::make_optional<JsonArrayScope<TMode>>(&jsonValue, this);
			}
			return std::nullopt;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(web::json::value::array(arraySize));
			return std::make_optional<JsonArrayScope<TMode>>(&jsonValue, this);
		}
	}

protected:
	web::json::value& SaveJsonValue(web::json::value&& jsonValue)
	{
		assert(mIndex < GetSize());
		return (*mNode)[mIndex++] = std::move(jsonValue);
	}

private:
	size_t mSize;
	size_t mIndex;
};


/// <summary>
/// Constant iterator for keys.
/// </summary>
class key_const_iterator
{
	template <SerializeMode TMode>
	friend class JsonObjectScope;

	web::json::object::const_iterator mJsonIt;

	key_const_iterator(web::json::object::const_iterator&& it)
		: mJsonIt(it) { }

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

	const JsonScopeBase::key_type& operator*() const {
		return mJsonIt->first;
	}
};


/// <summary>
/// JSON scope for serializing objects (list of values with keys).
/// </summary>
/// <seealso cref="JsonScopeBase" />
template <SerializeMode TMode>
class JsonObjectScope final : public TArchiveScope<TMode>, public JsonScopeBase
{
public:
	explicit JsonObjectScope(web::json::value* node, JsonScopeBase* parent = nullptr, key_type_view parentKey = {})
		: JsonScopeBase(node, parent, parentKey)
	{
		assert(mNode->is_object());
	}

	[[nodiscard]] key_const_iterator cbegin() const {
		return key_const_iterator(mNode->as_object().cbegin());
	}

	[[nodiscard]] key_const_iterator cend() const {
		return key_const_iterator(mNode->as_object().cend());
	}

	bool SerializeValue(const key_type& key, bool& value) const
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr && jsonValue->is_boolean())
			{
				value = jsonValue->as_bool();
				return true;
			}
			return false;
		}
		else
		{
			SaveJsonValue(key, web::json::value::boolean(value));
			return true;
		}
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool SerializeValue(const key_type& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			return jsonValue == nullptr ? false : LoadFundamentalValue(*jsonValue, value);
		}
		else
		{
			SaveJsonValue(key, web::json::value::number(value));
			return true;
		}
	}

	template <typename TSym, typename TAllocator>
	bool SerializeValue(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			return jsonValue == nullptr ? false : LoadString(*jsonValue, value);
		}
		else
		{
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				SaveJsonValue(key, web::json::value(value));
			else
				SaveJsonValue(key, web::json::value(Convert::To<utility::string_t>(value)));
			return true;
		}
	}

	std::optional<JsonObjectScope<TMode>> OpenObjectScope(const key_type& key)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr && jsonValue->is_object())
			{
				decltype(auto) node = const_cast<web::json::value*>(jsonValue);
				return std::make_optional<JsonObjectScope<TMode>>(node, this, key);
			}
			return std::nullopt;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(key, web::json::value::object());
			return std::make_optional<JsonObjectScope<TMode>>(&jsonValue, this, key);
		}
	}

	std::optional<JsonArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr && jsonValue->is_array())
				return std::make_optional<JsonArrayScope<TMode>>(jsonValue, this, key);
			return std::nullopt;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(key, web::json::value::array(arraySize));
			return std::make_optional<JsonArrayScope<TMode>>(&jsonValue, this, key);
		}
	}

protected:
	web::json::value* LoadJsonValue(const key_type& key) const
	{
		auto& jObject = mNode->as_object();
		auto it = std::find_if(jObject.begin(), jObject.end(), [&key](const auto& p) { return p.first == key; });
		return it == jObject.end() ? nullptr : &it->second;
	}

	web::json::value& SaveJsonValue(const key_type& key, web::json::value&& jsonValue) const
	{
		// Checks that object was not saved previously under the same key
		assert(mNode->as_object().find(key) == mNode->as_object().end());

		return (*mNode)[key] = std::move(jsonValue);
	}
};

/// <summary>
/// JSON root scope (can serialize one value, array or object without key)
/// </summary>
/// <seealso cref="JsonScopeBase" />
template <SerializeMode TMode>
class JsonRootScope final : public TArchiveScope<TMode>, public JsonScopeBase
{
public:
	explicit JsonRootScope(const std::string& inputStr)
		: JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		std::error_code error;
#ifdef _UTF16_STRINGS
		mRootJson = web::json::value::parse(utility::conversions::to_string_t(inputStr), error);
#else
		mRootJson = web::json::value::parse(inputStr, error);
#endif
		if (mRootJson.is_null()) {
			throw SerializationException(SerializationErrorCode::ParsingError, error.category().message(error.value()));
		}
	}

	explicit JsonRootScope(std::string& outputStr, const SerializationOptions& serializationOptions = {})
		: JsonScopeBase(&mRootJson)
		, mOutput(&outputStr)
		, mSerializationOptions(serializationOptions)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	explicit JsonRootScope(std::istream& inputStream)
		: JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		const auto utfType = Convert::DetectEncoding(inputStream);
		if (utfType != Convert::UtfType::Utf8) {
			throw SerializationException(SerializationErrorCode::UnsupportedEncoding, "The archive does not support encoding: " + Convert::ToString(utfType));
		}

		std::error_code error;
		mRootJson = web::json::value::parse(inputStream, error);
		if (mRootJson.is_null()) {
			throw SerializationException(SerializationErrorCode::ParsingError, error.category().message(error.value()));
		}
	}

	JsonRootScope(std::ostream& outputStream, const SerializationOptions& serializationOptions = {})
		: JsonScopeBase(&mRootJson)
		, mOutput(&outputStream)
		, mSerializationOptions(serializationOptions)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	void SerializeValue(bool& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mRootJson.is_boolean())
				value = mRootJson.as_bool();
		}
		else
		{
			assert(mRootJson.is_null());
			mRootJson = web::json::value::boolean(value);
		}
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			LoadFundamentalValue(mRootJson, value);
		}
		else
		{
			assert(mRootJson.is_null());
			mRootJson = web::json::value::number(value);
		}
	}

	template <typename TSym, typename TAllocator>
	void SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			LoadString(mRootJson, value);
		}
		else
		{
			assert(mRootJson.is_null());
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				mRootJson = web::json::value(value);
			else
				mRootJson = web::json::value(Convert::To<utility::string_t>(value));
		}
	}

	std::optional<JsonObjectScope<TMode>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load)	{
			return mRootJson.is_object() ? std::make_optional<JsonObjectScope<TMode>>(&mRootJson) : std::nullopt;
		}
		else
		{
			assert(mRootJson.is_null());
			mRootJson = web::json::value::object();
			return std::make_optional<JsonObjectScope<TMode>>(&mRootJson);
		}
	}

	std::optional<JsonArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return mRootJson.is_array() ? std::make_optional<JsonArrayScope<TMode>>(&mRootJson) : std::nullopt;
		}
		else
		{
			assert(mRootJson.is_null());
			mRootJson = web::json::value::array(arraySize);
			return std::make_optional<JsonArrayScope<TMode>>(&mRootJson);
		}
	}

	void Finalize()
	{
		if constexpr (TMode == SerializeMode::Save)
		{
			std::visit([this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				assert(mSerializationOptions);
				assert(!mSerializationOptions->formatOptions.enableFormat && "CppRestJson does not support formatting");
				if constexpr (std::is_same_v<T, std::string*>)
				{
					if constexpr (std::is_same_v<std::remove_pointer_t<T>, decltype(mRootJson.serialize())>) {
						*arg = mRootJson.serialize();
					}
					else {
						// Encode to UTF-8 (CppRestSDK does not have native methods on Windows platform)
						*arg = utility::conversions::to_utf8string(mRootJson.serialize());
					}
				}
				else if constexpr (std::is_same_v<T, std::ostream*>)
				{
					if (mSerializationOptions->streamOptions.writeBom) {
						arg->write(Convert::Utf8::bom, sizeof Convert::Utf8::bom);
					}
					mRootJson.serialize(*arg);
				}
			}, mOutput);
			mOutput = nullptr;
		}
	}

private:
	web::json::value mRootJson;
	std::variant<std::nullptr_t, std::string*, std::ostream*> mOutput;
	std::optional<SerializationOptions> mSerializationOptions;
};

}


/// <summary>
/// JSON archive based on JSON implementation from CppRestSdk library.
/// Supports load/save from:
/// - <c>std::string</c>: UTF-8
/// - <c>std::istream</c> and <c>std::ostream</c>: UTF-8
/// </summary>
/// <remarks>
/// The JSON-key type is depends from type utility::string_t defined in the CppRestSdk and it is different on Windows and *nix platforms.
/// For stay your code cross compiled you can use macros _XPLATSTR("MyKey") from CppRestSdk or
/// use MakeAutoKeyValue() but with possible small overhead for converting.
/// </remarks>
using JsonArchive = TArchiveBase<
	Detail::JsonArchiveTraits,
	Detail::JsonRootScope<SerializeMode::Load>,
	Detail::JsonRootScope<SerializeMode::Save>>;

}