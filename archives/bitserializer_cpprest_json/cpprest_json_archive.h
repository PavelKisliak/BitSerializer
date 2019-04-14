/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>
#include <optional>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/media_archive_base.h"

// External dependency (C++ REST SDK)
#include "cpprest/json.h"

namespace BitSerializer::Json::CppRest {
namespace Detail {

/// <summary>
/// The traits of JSON archive based on C++ REST SDK
/// </summary>
class JsonArchiveTraits
{
public:
	using key_type = utility::string_t;
	using supported_key_types = SupportedKeyTypes<utility::string_t>;
	using preferred_output_format = utility::string_t;
	using preferred_stream_char_type = utility::ostream_t::char_type;
	static const wchar_t path_separator = L'/';
};

// Forward declarations
template <SerializeMode TMode>
class JsonObjectScope;

/// <summary>
/// Base class of JSON scope
/// </summary>
/// <seealso cref="MediaArchiveBase" />
class JsonScopeBase : public JsonArchiveTraits
{
public:
	using key_type_view = std::basic_string_view<key_type::value_type>;

	explicit JsonScopeBase(const web::json::value* node, JsonScopeBase* parent = nullptr, key_type_view parentKey = {})
		: mNode(const_cast<web::json::value*>(node))
		, mParent(parent)
		, mParentKey(parentKey)
	{ }

	virtual ~JsonScopeBase() = default;

	/// <summary>
	/// Returns the size of stored elements (for arrays and objects).
	/// </summary>
	size_t GetSize() const {
		return mNode->size();
	}

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer).
	/// </summary>
	virtual std::wstring GetPath() const
	{
		const std::wstring localPath = mParentKey.empty()
			? std::wstring()
			: path_separator + Convert::ToWString(mParentKey);
		return mParent == nullptr ? localPath : mParent->GetPath() + localPath;
	}

protected:
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
class JsonArrayScope : public ArchiveScope<TMode>, public JsonScopeBase
{
protected:
	size_t mSize;
public:
	explicit JsonArrayScope(const web::json::value* node, JsonScopeBase* parent = nullptr, key_type_view parentKey = {})
		: JsonScopeBase(node, parent, parentKey)
		, mIndex(0)
	{
		if (node != nullptr)
			mSize = node->size();
		assert(mNode->is_array());
	}

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer).
	/// </summary>
	/// <returns></returns>
	std::wstring GetPath() const override
	{
		const auto index = mIndex == 0 ? 0 : mIndex - 1;
		return JsonScopeBase::GetPath() + path_separator + Convert::ToWString(index);
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
	inline web::json::value& SaveJsonValue(web::json::value&& jsonValue)
	{
		assert(mIndex < GetSize());
		return (*mNode)[mIndex++] = std::move(jsonValue);
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
class JsonObjectScope : public ArchiveScope<TMode>, public JsonScopeBase
{
public:
	explicit JsonObjectScope(const web::json::value* node, JsonScopeBase* parent = nullptr, key_type_view parentKey = {})
		: JsonScopeBase(node, parent, parentKey)
	{
		assert(mNode->is_object());
	};

	key_const_iterator cbegin() const {
		return key_const_iterator(mNode->as_object().cbegin());
	}

	key_const_iterator cend() const {
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
	inline const web::json::value* LoadJsonValue(const key_type& key) const
	{
		const auto& jObject = mNode->as_object();
		const auto it = jObject.find(key);
		return it == jObject.end() ? nullptr : &it->second;
	}

	inline web::json::value& SaveJsonValue(const key_type& key, web::json::value&& jsonValue) const
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
class JsonRootScope : public ArchiveScope<TMode>, public JsonScopeBase
{
public:
	explicit JsonRootScope(const utility::char_t* inputStr) : JsonRootScope(utility::string_t(inputStr)) {}

	explicit JsonRootScope(const utility::string_t& inputStr)
		: JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		std::error_code error;
		mRootJson = web::json::value::parse(inputStr, error);
		if (mRootJson.is_null()) {
			throw SerializationException(SerializationErrorCode::ParsingError, error.category().message(error.value()));
		}
	}

	explicit JsonRootScope(utility::string_t& outputStr)
		: JsonScopeBase(&mRootJson)
		, mOutput(&outputStr)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	explicit JsonRootScope(utility::istream_t& inputStream)
		: JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		std::error_code error;
		mRootJson = web::json::value::parse(inputStream, error);
		if (mRootJson.is_null()) {
			throw SerializationException(SerializationErrorCode::ParsingError, error.category().message(error.value()));
		}
	}

	explicit JsonRootScope(utility::ostream_t& outputStream)
		: JsonScopeBase(&mRootJson)
		, mOutput(&outputStream)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	virtual ~JsonRootScope()
	{
		Finish();
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

private:
	void Finish()
	{
		if constexpr (TMode == SerializeMode::Save)
		{
			std::visit([this](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, utility::string_t*>)
					*arg = mRootJson.serialize();
				else if constexpr (std::is_same_v<T, utility::ostream_t*>)
					*arg << mRootJson;
			}, mOutput);
			mOutput = nullptr;
		}
	}

	web::json::value mRootJson;
	std::variant<std::nullptr_t, utility::string_t*, utility::ostream_t*> mOutput;
};

} //namespace Detail


/// <summary>
/// Declaration of JSON archive
/// </summary>
using JsonArchive = MediaArchiveBase<
	Detail::JsonArchiveTraits,
	Detail::JsonRootScope<SerializeMode::Load>,
	Detail::JsonRootScope<SerializeMode::Save>>;

}	// namespace BitSerializer::Json::CppRest