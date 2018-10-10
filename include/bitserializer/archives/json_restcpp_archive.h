/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <memory>
#include <type_traits>
#include <variant>
#include "../serialization_detail/errors_handling.h"
#include "../serialization_detail/media_archive_base.h"

// External dependency (C++ REST SDK)
#include "cpprest/json.h"

namespace BitSerializer {
namespace Detail {

/// <summary>
/// The traits of JSON archive based on C++ REST SDK
/// </summary>
class JsonArchiveTraits
{
public:
	using key_type = utility::string_t;
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
	JsonScopeBase(JsonScopeBase&&) = default;

	JsonScopeBase(const web::json::value* node, JsonScopeBase* parent = nullptr, const key_type& perentKey = key_type())
		: mNode(const_cast<web::json::value*>(node))
		, mParent(parent)
		, mParentKey(perentKey)
	{ }

	/// <summary>
	/// Returns the size of stored elements (for arrays and objects).
	/// </summary>
	/// <returns></returns>
	inline size_t GetSize() const {
		return mNode->size();
	}

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer).
	/// </summary>
	/// <returns></returns>
	virtual std::wstring GetPath() const
	{
		std::wstring localPath = mParentKey.empty()
			? Convert::ToWString(mParentKey)
			: path_separator + Convert::ToWString(mParentKey);
		return mParent == nullptr ? localPath : mParent->GetPath() + localPath;
	}

protected:
	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool LoadFundamentalValue(const web::json::value& jsonValue, T& value)
	{
		if (!jsonValue.is_number())
			return false;

		if constexpr (std::is_integral_v<T>)
		{
			auto number = jsonValue.as_number();
			if constexpr (std::is_same_v<T, int64_t>) {
				value = number.to_int64();
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				value = number.to_uint64();
			}
			else {
				value = static_cast<T>(number.to_int32());
			}
		}
		else
		{
			value = static_cast<T>(jsonValue.as_double());
		}
		return true;
	}

	template <typename TSym, typename TAllocator>
	bool LoadString(const web::json::value& jsonValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if (!jsonValue.is_string())
			return false;

		if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
			value = jsonValue.as_string();
		else
			value = Convert::ToString(jsonValue.as_string());
		return true;
	}

	web::json::value* mNode;
	JsonScopeBase* mParent;
	key_type mParentKey;
};


/// <summary>
/// JSON scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="JsonArchiveBase" />
template <SerializeMode TMode>
class JsonArrayScope : public ArchiveScope<TMode>, public JsonScopeBase
{
public:
	JsonArrayScope(const web::json::value* node, JsonScopeBase* parent = nullptr, const key_type& perentKey = key_type())
		: JsonScopeBase(node, parent, perentKey)
		, mIndex(0)
	{
		assert(mNode->is_array());
	}

	/// <summary>
	/// Gets the current path in JSON (RFC 6901 - JSON Pointer).
	/// </summary>
	/// <returns></returns>
	std::wstring GetPath() const override
	{
		return JsonScopeBase::GetPath() + path_separator + Convert::ToWString(mIndex);
	}

	template <typename TSym, typename TAllocator>
	void SerializeString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load) {
			LoadString(LoadJsonValue(), value);
		}
		else
		{
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				SaveJsonValue(web::json::value(value));
			else
				SaveJsonValue(web::json::value(Convert::FromString<utility::string_t>(value)));
		}
	}

	void SerializeValue(bool& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadJsonValue();
			if (jsonValue.is_boolean())
				value = jsonValue.as_bool();
		}
		else
		{
			SaveJsonValue(web::json::value(value));
		}
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadJsonValue();
			LoadFundamentalValue(jsonValue, value);
		}
		else
		{
			SaveJsonValue(web::json::value(value));
		}
	}

	std::unique_ptr<JsonObjectScope<TMode>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadJsonValue();
			if (jsonValue.is_object())
			{
				decltype(auto) node = const_cast<web::json::value&>(jsonValue);
				return std::make_unique<JsonObjectScope<TMode>>(&node, this);
			}
			return nullptr;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(web::json::value::object());
			return std::make_unique<JsonObjectScope<TMode>>(&jsonValue, this);
		}
	}

	std::unique_ptr<Detail::JsonArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadJsonValue();
			return jsonValue.is_array() ? std::make_unique<JsonArrayScope<TMode>>(&jsonValue, this) : nullptr;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(web::json::value::array(arraySize));
			return std::make_unique<JsonArrayScope<TMode>>(&jsonValue, this);
		}
	}

protected:
	inline const web::json::value& LoadJsonValue()
	{
		assert(mIndex < GetSize());
		return mNode->at(mIndex++);
	}

	inline web::json::value& SaveJsonValue(web::json::value&& jsonValue)
	{
		assert(mIndex < GetSize());
		return (*mNode)[mIndex++] = jsonValue;
	}

private:
	size_t mIndex;
};


/// <summary>
/// JSON scope for serializing objects (list of values with keys).
/// </summary>
/// <seealso cref="JsonArchiveBase" />
template <SerializeMode TMode>
class JsonObjectScope : public ArchiveScope<TMode>, public JsonScopeBase
{
public:
	JsonObjectScope(JsonObjectScope&&) = default;

	JsonObjectScope(const web::json::value* node, JsonScopeBase* parent = nullptr, const key_type& perentKey = key_type())
		: JsonScopeBase(node, parent, perentKey)
	{
		assert(mNode->is_object());
	};

	/// <summary>
	/// Gets the key by index.
	/// </summary>
	key_type GetKeyByIndex(size_t index) {
		return (mNode->as_object().cbegin() + index)->first;
	}

	template <typename TSym, typename TAllocator>
	bool SerializeString(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
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
				SaveJsonValue(key, web::json::value(Convert::FromString<utility::string_t>(value)));
			return true;
		}
	}

	bool SerializeValue(const key_type& key, bool& value)
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

	std::unique_ptr<JsonObjectScope<TMode>> OpenObjectScope(const key_type& key)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr && jsonValue->is_object())
			{
				decltype(auto) node = const_cast<web::json::value*>(jsonValue);
				return std::make_unique<JsonObjectScope<TMode>>(node, this, key);
			}
			return nullptr;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(key, web::json::value::object());
			return std::make_unique<JsonObjectScope<TMode>>(&jsonValue, this, key);
		}
	}

	std::unique_ptr<Detail::JsonArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr && jsonValue->is_array())
				return std::make_unique<JsonArrayScope<TMode>>(jsonValue, this, key);
			return nullptr;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(key, web::json::value::array(arraySize));
			return std::make_unique<JsonArrayScope<TMode>>(&jsonValue, this, key);
		}
	}

protected:
	inline const web::json::value* LoadJsonValue(const key_type& key) const
	{
		const auto& jObject = mNode->as_object();
		auto it = jObject.find(key);
		return it == jObject.end() ? nullptr : &it->second;
	}

	inline web::json::value& SaveJsonValue(const key_type& key, web::json::value&& jsonValue) const
	{
		return (*mNode)[key] = jsonValue;
	}
};

/// <summary>
/// JSON root scope (can serialize one value, array or object without key)
/// </summary>
template <SerializeMode TMode>
class JsonRootScope : public ArchiveScope<TMode>, public Detail::JsonScopeBase
{
public:
	JsonRootScope(JsonRootScope&&) = default;

	JsonRootScope(const utility::string_t& outputFormat)
		: Detail::JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		std::error_code error;
		mRootJson = web::json::value::parse(outputFormat, error);
		if (mRootJson.is_null()) {
			throw SerializationException(SerializationErrorCode::ParsingError, error.category().message(error.value()));
		}
	}

	JsonRootScope(utility::string_t& outputFormat)
		: Detail::JsonScopeBase(&mRootJson)
		, mOutput(&outputFormat)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	JsonRootScope(typename utility::istream_t& input)
		: Detail::JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		std::error_code error;
		mRootJson = web::json::value::parse(input, error);
		if (mRootJson.is_null()) {
			throw SerializationException(SerializationErrorCode::ParsingError, error.category().message(error.value()));
		}
	}

	JsonRootScope(typename utility::ostream_t& output)
		: Detail::JsonScopeBase(&mRootJson)
		, mOutput(&output)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
		mOutput = &output;
	}

	~JsonRootScope()
	{
		Finish();
	}

	template <typename TSym, typename TAllocator>
	void SerializeString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
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
				mRootJson = web::json::value(Convert::FromString<utility::string_t>(value));
		}
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

	std::unique_ptr<JsonObjectScope<TMode>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load)	{
			return mRootJson.is_object() ? std::make_unique<JsonObjectScope<TMode>>(&mRootJson) : nullptr;
		}
		else
		{
			assert(mRootJson.is_null());
			mRootJson = web::json::value::object();
			return std::make_unique<JsonObjectScope<TMode>>(&mRootJson);
		}
	}

	std::unique_ptr<Detail::JsonArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load) {
			return mRootJson.is_array() ? std::make_unique<Detail::JsonArrayScope<TMode>>(&mRootJson) : nullptr;
		}
		else
		{
			assert(mRootJson.is_null());
			mRootJson = web::json::value::array(arraySize);
			return std::make_unique<Detail::JsonArrayScope<TMode>>(&mRootJson);
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

}	// namespace BitSerializer