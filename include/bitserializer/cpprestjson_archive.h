/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/errors_handling.h"

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
	static constexpr bool is_binary = false;

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
	static bool LoadValue(const web::json::value& jsonValue, T& value, const SerializationOptions& serializationOptions)
	{
		// Null value from JSON is excluded from MismatchedTypesPolicy processing
		if (jsonValue.is_null()) {
			return std::is_null_pointer_v<T>;
		}

		using BitSerializer::Detail::SafeNumberCast;
		if constexpr (std::is_integral_v<T>)
		{
			if (jsonValue.is_number())
			{
				if (jsonValue.is_integer())
				{
					if (jsonValue.as_number().is_int64()) {
						return SafeNumberCast(jsonValue.as_number().to_int64(), value, serializationOptions.overflowNumberPolicy);
					}
					return SafeNumberCast(jsonValue.as_number().to_uint64(), value, serializationOptions.overflowNumberPolicy);
				}
			}
			else if (jsonValue.is_boolean())
			{
				return SafeNumberCast(jsonValue.as_bool(), value, serializationOptions.overflowNumberPolicy);
			}
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			if (jsonValue.is_number()) {
				return SafeNumberCast(jsonValue.as_double(), value, serializationOptions.overflowNumberPolicy);
			}
		}

		HandleMismatchedTypesPolicy(serializationOptions.mismatchedTypesPolicy);
		return false;
	}

	template <typename TSym, typename TAllocator>
	static bool LoadValue(const web::json::value& jsonValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value, const SerializationOptions& serializationOptions)
	{
		if (!jsonValue.is_string())
		{
			HandleMismatchedTypesPolicy(serializationOptions.mismatchedTypesPolicy);
			return false;
		}

		if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
			value = jsonValue.as_string();
		else
			value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(jsonValue.as_string());
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
	JsonArrayScope(web::json::value* node, SerializationContext& serializationContext, JsonScopeBase* parent = nullptr, key_type_view parentKey = {})
		: TArchiveScope<TMode>(serializationContext)
		, JsonScopeBase(node, parent, parentKey)
		, mSize(node == nullptr ? 0 : node->size())
		, mIndex(0)
	{
		assert(mNode->is_array());
	}

	/// <summary>
	/// Returns the estimated number of items to load (for reserving the size of containers).
	/// </summary>
	[[nodiscard]] size_t GetEstimatedSize() const {
		return mNode->size();
	}

	/// <summary>
	/// Returns `true` when all no more values to load.
	/// </summary>
	[[nodiscard]]
	bool IsEnd() const
	{
		static_assert(TMode == SerializeMode::Load);
		return mIndex == mSize;
	}

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer).
	/// </summary>
	[[nodiscard]] std::string GetPath() const override
	{
		return JsonScopeBase::GetPath() + path_separator + Convert::ToString(mIndex);
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>, int> = 0>
	bool SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load)	{
			return LoadValue(LoadNextItem(), value, this->GetOptions());
		}
		else
		{
			if constexpr (std::is_arithmetic_v<T>) {
				SaveJsonValue(web::json::value(value));
			}
			else {
				SaveJsonValue(web::json::value::null());
			}
			return true;
		}
	}

	template <typename TSym, typename TAllocator>
	bool SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return LoadValue(LoadNextItem(), value, this->GetOptions());
		}
		else
		{
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				SaveJsonValue(web::json::value(value));
			else
				SaveJsonValue(web::json::value(Convert::To<utility::string_t>(value)));
			return true;
		}
	}

	std::optional<JsonObjectScope<TMode>> OpenObjectScope(size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadNextItem();
			if (jsonValue.is_object()) {
				return std::make_optional<JsonObjectScope<TMode>>(&jsonValue, this->GetContext(), this);
			}
			return std::nullopt;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(web::json::value::object());
			return std::make_optional<JsonObjectScope<TMode>>(&jsonValue, this->GetContext(), this);
		}
	}

	std::optional<JsonArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadNextItem();
			if (jsonValue.is_array()) {
				return std::make_optional<JsonArrayScope<TMode>>(&jsonValue, this->GetContext(), this);
			}
			return std::nullopt;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(web::json::value::array(arraySize));
			return std::make_optional<JsonArrayScope<TMode>>(&jsonValue, this->GetContext(), this);
		}
	}

protected:
	web::json::value& LoadNextItem()
	{
		static_assert(TMode == SerializeMode::Load);
		if (mIndex < mSize) {
			return (*mNode)[mIndex++];
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "No more items to load");
	}

	web::json::value& SaveJsonValue(web::json::value&& jsonValue)
	{
		assert(mIndex < GetEstimatedSize());
		return (*mNode)[mIndex++] = std::move(jsonValue);
	}

private:
	size_t mSize;
	size_t mIndex;
};


/// <summary>
/// JSON scope for serializing objects (list of values with keys).
/// </summary>
/// <seealso cref="JsonScopeBase" />
template <SerializeMode TMode>
class JsonObjectScope final : public TArchiveScope<TMode>, public JsonScopeBase
{
public:
	explicit JsonObjectScope(web::json::value* node, SerializationContext& serializationContext, JsonScopeBase* parent = nullptr, key_type_view parentKey = {})
		: TArchiveScope<TMode>(serializationContext)
		, JsonScopeBase(node, parent, parentKey)
	{
		assert(mNode->is_object());
	}

	[[nodiscard]] size_t GetEstimatedSize() const {
		return mNode->size();
	}

	/// <summary>
	/// Enumerates all keys by calling a passed function.
	/// </summary>
	template <typename TCallback>
	void VisitKeys(TCallback&& fn)
	{
		for (const auto& keyVal : mNode->as_object()) {
			fn(keyVal.first);
		}
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>, int> = 0>
	bool SerializeValue(const key_type& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			return jsonValue == nullptr ? false : LoadValue(*jsonValue, value, this->GetOptions());
		}
		else
		{
			if constexpr (std::is_arithmetic_v<T>) {
				SaveJsonValue(key, web::json::value(value));
			}
			else {
				SaveJsonValue(key, web::json::value());
			}
			return true;
		}
	}

	template <typename TSym, typename TAllocator>
	bool SerializeValue(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			return jsonValue == nullptr ? false : LoadValue(*jsonValue, value, this->GetOptions());
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

	std::optional<JsonObjectScope<TMode>> OpenObjectScope(const key_type& key, size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr && jsonValue->is_object())
			{
				decltype(auto) node = const_cast<web::json::value*>(jsonValue);
				return std::make_optional<JsonObjectScope<TMode>>(node, this->GetContext(), this, key);
			}
			return std::nullopt;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(key, web::json::value::object());
			return std::make_optional<JsonObjectScope<TMode>>(&jsonValue, this->GetContext(), this, key);
		}
	}

	std::optional<JsonArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr && jsonValue->is_array())
				return std::make_optional<JsonArrayScope<TMode>>(jsonValue, this->GetContext(), this, key);
			return std::nullopt;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(key, web::json::value::array(arraySize));
			return std::make_optional<JsonArrayScope<TMode>>(&jsonValue, this->GetContext(), this, key);
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
	JsonRootScope(const std::string& inputStr, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		std::error_code error;
#ifdef _UTF16_STRINGS
		mRootJson = web::json::value::parse(utility::conversions::to_string_t(inputStr), error);
#else
		mRootJson = web::json::value::parse(inputStr, error);
#endif
		if (error) {
			throw ParsingException(error.category().message(error.value()));
		}
	}

	JsonRootScope(std::string& outputStr, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, JsonScopeBase(&mRootJson)
		, mOutput(&outputStr)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	JsonRootScope(std::istream& inputStream, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		const auto utfType = Convert::DetectEncoding(inputStream);
		if (utfType != Convert::UtfType::Utf8) {
			throw SerializationException(SerializationErrorCode::UnsupportedEncoding, "The archive does not support encoding: " + Convert::ToString(utfType));
		}

		std::error_code error;
		mRootJson = web::json::value::parse(inputStream, error);
		if (error) {
			throw ParsingException(error.category().message(error.value()));
		}
	}

	JsonRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, JsonScopeBase(&mRootJson)
		, mOutput(&outputStream)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>, int> = 0>
	bool SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return LoadValue(mRootJson, value, this->GetOptions());
		}
		else
		{
			if constexpr (std::is_same_v<T, bool>) {
				mRootJson = web::json::value::boolean(value);
			}
			else if constexpr (std::is_arithmetic_v<T>) {
				mRootJson = web::json::value::number(value);
			}
			else {
				mRootJson = web::json::value::null();
			}
			return true;
		}
	}

	template <typename TSym, typename TAllocator>
	bool SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return LoadValue(mRootJson, value, this->GetOptions());
		}
		else
		{
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				mRootJson = web::json::value(value);
			else
				mRootJson = web::json::value(Convert::To<utility::string_t>(value));
			return true;
		}
	}

	std::optional<JsonObjectScope<TMode>> OpenObjectScope(size_t)
	{
		if constexpr (TMode == SerializeMode::Load)	{
			return mRootJson.is_object() ? std::make_optional<JsonObjectScope<TMode>>(&mRootJson, this->GetContext()) : std::nullopt;
		}
		else
		{
			mRootJson = web::json::value::object();
			return std::make_optional<JsonObjectScope<TMode>>(&mRootJson, this->GetContext());
		}
	}

	std::optional<JsonArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return mRootJson.is_array() ? std::make_optional<JsonArrayScope<TMode>>(&mRootJson, this->GetContext()) : std::nullopt;
		}
		else
		{
			mRootJson = web::json::value::array(arraySize);
			return std::make_optional<JsonArrayScope<TMode>>(&mRootJson, this->GetContext());
		}
	}

	void Finalize()
	{
		if constexpr (TMode == SerializeMode::Save)
		{
			std::visit([this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				assert(!this->GetOptions().formatOptions.enableFormat && "CppRestJson does not support formatting");
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
					if (this->GetOptions().streamOptions.writeBom) {
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
/// use AutoKeyValue() but with possible small overhead for converting.
/// </remarks>
using JsonArchive = TArchiveBase<
	Detail::JsonArchiveTraits,
	Detail::JsonRootScope<SerializeMode::Load>,
	Detail::JsonRootScope<SerializeMode::Save>>;

}
