/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <assert.h>
#include <memory>
#include <type_traits>
#include <variant>
#include "..\serialization_detail\media_archive_base.h"

// External dependency (C++ REST SDK)
#include "cpprest\json.h"

namespace BitSerializer {
namespace Detail {

// Forward declarations
class JsonObjectScope;

/// <summary>
///  Base class of JSON archive
/// </summary>
/// <seealso cref="MediaArchiveBase" />
class JsonArchiveBase : public MediaArchiveBase
{
public:
	using key_type = utility::string_t;
	using archive_format = utility::string_t;
	using input_stream = utility::istream_t;
	using output_stream = utility::ostream_t;

	JsonArchiveBase(const web::json::value* node, SerializeState state = SerializeState::Idle)
		: MediaArchiveBase(state)
		, mNode(const_cast<web::json::value*>(node))
	{ }

	inline size_t GetSize() const {
		return mNode->size();
	}

protected:
	web::json::value* mNode;
};


/// <summary>
/// Implementation of MediaArchive for serializing not-named objects into JSON array.
/// </summary>
/// <seealso cref="JsonArchiveBase" />
class JsonArrayScope : public JsonArchiveBase
{
public:
	JsonArrayScope(const web::json::value* node, SerializeState state = SerializeState::Idle)
		: JsonArchiveBase(node, state)
		, mIndex(0)
	{
		assert(mNode->is_array());
	}

	//------------------------------------------------------------------------------
	// Serialize string types
	//------------------------------------------------------------------------------
	template <typename TSym, typename TAllocator>
	inline void	LoadString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		auto& jsonValue = LoadJsonValue();
		if (jsonValue.is_string())
		{
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				value = jsonValue.as_string();
			else
				value = Convert::ToString(jsonValue.as_string());
		}
	}

	template <typename TSym, typename TAllocator>
	inline void SaveString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
			SaveJsonValue(web::json::value(value));
		else
			SaveJsonValue(web::json::value(Convert::FromString<utility::string_t>(value)));
	}

	//------------------------------------------------------------------------------
	// Serialize fundamental types
	//------------------------------------------------------------------------------
	inline void LoadValue(bool& value)
	{
		auto& jsonValue = LoadJsonValue();
		if (jsonValue.is_boolean())
			value = jsonValue.as_bool();
	}

	inline void SaveValue(bool value) {
		SaveJsonValue(web::json::value(value));
	}

	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	inline void LoadValue(T& value)
	{
		auto& jsonValue = LoadJsonValue();
		if (jsonValue.is_integer())
			value = static_cast<T>(jsonValue.as_integer());
	}

	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	inline void LoadValue(T& value)
	{
		auto& jsonValue = LoadJsonValue();
		if (jsonValue.is_number())
			value = static_cast<T>(jsonValue.as_double());
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	inline void SaveValue(T value) {
		SaveJsonValue(web::json::value(value));
	}

	//------------------------------------------------------------------------------
	// Serialize classes
	//------------------------------------------------------------------------------
	std::unique_ptr<JsonObjectScope> OpenScopeForLoadObject()
	{
		auto& jsonValue = LoadJsonValue();
		if (jsonValue.is_object())
		{
			decltype(auto) node = const_cast<web::json::value&>(jsonValue);
			return std::make_unique<JsonObjectScope>(&node, mSerializeState);
		}
		return nullptr;
	}

	std::unique_ptr<JsonObjectScope> OpenScopeForSaveObject()
	{
		auto& jsonValue = SaveJsonValue(web::json::value::object());
		return std::make_unique<JsonObjectScope>(&jsonValue, mSerializeState);
	}

	//------------------------------------------------------------------------------
	// Serialize arrays
	//------------------------------------------------------------------------------
	inline std::unique_ptr<JsonArrayScope> OpenScopeForLoadArray()
	{
		auto& jsonValue = LoadJsonValue();
		if (jsonValue.is_array())
			return std::make_unique<JsonArrayScope>(&jsonValue, mSerializeState);
		return nullptr;
	}

	inline std::unique_ptr<JsonArrayScope> OpenScopeForSaveArray(size_t arraySize)
	{
		auto& jsonValue = SaveJsonValue(web::json::value::array(arraySize));
		return std::make_unique<JsonArrayScope>(&jsonValue, mSerializeState);
	}

protected:
	inline const web::json::value& LoadJsonValue()
	{
		assert(mSerializeState == SerializeState::Load);
		assert(mIndex < GetSize());
		return mNode->at(mIndex++);
	}

	inline web::json::value& SaveJsonValue(web::json::value&& jsonValue)
	{
		assert(mSerializeState == SerializeState::Save);
		assert(mIndex < GetSize());
		return (*mNode)[mIndex++] = jsonValue;
	}

private:
	size_t mIndex;
};


/// <summary>
/// Implementation of MediaArchive for serializing named objects into JSON.
/// </summary>
/// <seealso cref="JsonArchiveBase" />
class JsonObjectScope : public JsonArchiveBase
{
public:
	JsonObjectScope(const web::json::value* node, SerializeState state = SerializeState::Idle)
		: JsonArchiveBase(node, state)
	{ };

	//------------------------------------------------------------------------------
	// Access to keys by index
	//------------------------------------------------------------------------------
	key_type GetKeyByIndex(int index) {
		return (mNode->as_object().cbegin() + index)->first;
	}

	//------------------------------------------------------------------------------
	// Serialize string types
	//------------------------------------------------------------------------------
	template <typename TSym, typename TAllocator>
	inline void	LoadString(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		auto* jsonValue = LoadJsonValue(key);
		if (jsonValue != nullptr)
		{
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				value = jsonValue->as_string();
			else
				value = Convert::ToString(jsonValue->as_string());
		}
	}

	template <typename TSym, typename TAllocator>
	inline void SaveString(const key_type& key, const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
			SaveJsonValue(key, web::json::value(value));
		else
			SaveJsonValue(key, web::json::value(Convert::FromString<utility::string_t>(value)));
	}

	//------------------------------------------------------------------------------
	// Serialize fundamental types
	//------------------------------------------------------------------------------
	inline void LoadValue(const key_type& key, bool& value)
	{
		auto* jsonValue = LoadJsonValue(key);
		if (jsonValue != nullptr && jsonValue->is_boolean())
			value = jsonValue->as_bool();
	}

	inline void SaveValue(const key_type& key, bool value) {
		SaveJsonValue(key, web::json::value::boolean(value));
	}

	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	inline void LoadValue(const key_type& key, T& value)
	{
		auto* jsonValue = LoadJsonValue(key);
		if (jsonValue != nullptr && jsonValue->is_integer())
			value = static_cast<T>(jsonValue->as_integer());
	}

	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	inline void LoadValue(const key_type& key, T& value)
	{
		auto* jsonValue = LoadJsonValue(key);
		if (jsonValue != nullptr && jsonValue->is_number())
			value = static_cast<T>(jsonValue->as_double());
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	inline void SaveValue(const key_type& key, T value) {
		SaveJsonValue(key, web::json::value::number(value));
	}

	//------------------------------------------------------------------------------
	// Serialize classes
	//------------------------------------------------------------------------------
	inline std::unique_ptr<JsonObjectScope> OpenScopeForLoadObject(const key_type& key)
	{
		auto* jsonValue = LoadJsonValue(key);
		if (jsonValue != nullptr && jsonValue->is_object())
		{
			decltype(auto) node = const_cast<web::json::value*>(jsonValue);
			return std::make_unique<JsonObjectScope>(node, mSerializeState);
		}
		return nullptr;
	}

	inline std::unique_ptr<JsonObjectScope> OpenScopeForSaveObject(const key_type& key)
	{
		auto& jsonValue = SaveJsonValue(key, web::json::value::object());
		return std::make_unique<JsonObjectScope>(&jsonValue, mSerializeState);
	}

	//------------------------------------------------------------------------------
	// Serialize arrays
	//------------------------------------------------------------------------------
	inline std::unique_ptr<JsonArrayScope> OpenScopeForLoadArray(const key_type& key)
	{
		auto* jsonValue = LoadJsonValue(key);
		if (jsonValue != nullptr && jsonValue->is_array())
			return std::make_unique<JsonArrayScope>(jsonValue, mSerializeState);
		return nullptr;
	}

	inline std::unique_ptr<JsonArrayScope> OpenScopeForSaveArray(const key_type& key, size_t arraySize)
	{
		auto& jsonValue = SaveJsonValue(key, web::json::value::array(arraySize));
		return std::make_unique<JsonArrayScope>(&jsonValue, mSerializeState);
	}

protected:
	inline const web::json::value* LoadJsonValue(const key_type& key) const
	{
		assert(mSerializeState == SerializeState::Load);
		assert(mNode->is_object());
		const auto& jObject = mNode->as_object();
		auto it = jObject.find(key);
		return it == jObject.end() ? nullptr : &it->second;
	}

	inline web::json::value& SaveJsonValue(const key_type& key, web::json::value&& jsonValue) const
	{
		assert(mSerializeState == SerializeState::Save);
		assert(mNode->is_object());
		return (*mNode)[key] = jsonValue;
	}
};

} //namespace Detail


/// <summary>
/// Implementation of MediaArchive for serializing into JSON (root object).
/// </summary>
/// <seealso cref="Detail::JsonObjectScope" />
class JsonArchive : public Detail::JsonObjectScope
{
public:
	JsonArchive()
		: Detail::JsonObjectScope(&mRootJson)
		, mOutput(nullptr)
	{ }

	~JsonArchive()
	{
		Finish();
	}

	// Begin load the contents from text.
	void BeginLoad(const archive_format& text)
	{
		assert(mSerializeState == SerializeState::Idle);
		std::error_code error;
		mRootJson = web::json::value::parse(text, error);
		if (mRootJson.is_null())
		{
			// Todo: exception
		}
		else
		{
			mSerializeState = SerializeState::Load;
		}
	}

	// Begin load the contents from stream.
	void BeginLoad(input_stream& input)
	{
		assert(mSerializeState == SerializeState::Idle);
		std::error_code error;
		mRootJson = web::json::value::parse(input, error);
		if (mRootJson.is_null())
		{
			// Todo: exception
		}
		else
		{
			mSerializeState = SerializeState::Load;
		}
	}

	// Begin save to text.
	void BeginSave(archive_format& output)
	{
		assert(mSerializeState == SerializeState::Idle);
		mOutput = &output;
		mRootJson = web::json::value::object();
		mSerializeState = SerializeState::Save;
	}

	// Begin save to stream.
	void BeginSave(output_stream& output)
	{
		assert(mSerializeState == SerializeState::Idle);
		mOutput = &output;
		mRootJson = web::json::value::object();
		mSerializeState = SerializeState::Save;
	}

	void Finish()
	{
		if (IsSaving())
		{
			std::visit([this](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, archive_format*>)
					*arg = mRootJson.serialize();
				else if constexpr (std::is_same_v<T, output_stream*>)
					*arg << mRootJson;
			}, mOutput);
			mOutput = nullptr;
		}
		mSerializeState = SerializeState::Idle;
	}

	//------------------------------------------------------------------------------
	// Serialize string types (root level, without key)
	//------------------------------------------------------------------------------
	using JsonObjectScope::LoadString;
	using JsonObjectScope::SaveString;

	template <typename TSym, typename TAllocator>
	inline void	LoadString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if (mRootJson.is_string())
		{
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				value = mRootJson.as_string();
			else
				value = Convert::FromString<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(mRootJson.as_string());
		}
	}

	template <typename TSym, typename TAllocator>
	inline void SaveString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		assert(mRootJson.is_object() && mRootJson.size() == 0);
		if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
			mRootJson = web::json::value(value);
		else
			mRootJson = web::json::value(Convert::FromString<utility::string_t>(value));
	}

	//------------------------------------------------------------------------------
	// Serialize fundamental types (root level, without key)
	//------------------------------------------------------------------------------
	using JsonObjectScope::LoadValue;
	using JsonObjectScope::SaveValue;

	inline void LoadValue(bool& value)
	{
		if (mRootJson.is_boolean())
			value = mRootJson.as_bool();
	}

	inline void SaveValue(bool value)
	{
		assert(mRootJson.is_object() && mRootJson.size() == 0);
		mRootJson = web::json::value::boolean(value);
	}

	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	inline void LoadValue(T& value)
	{
		if (mRootJson.is_integer())
			value = static_cast<T>(mRootJson.as_integer());
	}

	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	inline void LoadValue(T& value)
	{
		if (mRootJson.is_number())
			value = static_cast<T>(mRootJson.as_double());
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	inline void SaveValue(T value)
	{
		assert(mRootJson.is_object() && mRootJson.size() == 0);
		mRootJson = web::json::value::number(value);
	}

	//------------------------------------------------------------------------------
	// Serialize classes (root level, without key)
	//------------------------------------------------------------------------------
	using JsonObjectScope::OpenScopeForLoadObject;
	using JsonObjectScope::OpenScopeForSaveObject;

	inline std::unique_ptr<JsonObjectScope> OpenScopeForLoadObject()
	{
		return std::make_unique<JsonObjectScope>(&mRootJson, mSerializeState);
	}

	inline std::unique_ptr<JsonObjectScope> OpenScopeForSaveObject()
	{
		return std::make_unique<JsonObjectScope>(&mRootJson, mSerializeState);
	}

	//------------------------------------------------------------------------------
	// Serialize arrays (root level, without key)
	//------------------------------------------------------------------------------
	using JsonObjectScope::OpenScopeForLoadArray;
	using JsonObjectScope::OpenScopeForSaveArray;

	inline std::unique_ptr<Detail::JsonArrayScope> OpenScopeForLoadArray()
	{
		if (mRootJson.is_array())
			return std::make_unique<Detail::JsonArrayScope>(&mRootJson, mSerializeState);
		return nullptr;
	}

	inline std::unique_ptr<Detail::JsonArrayScope> OpenScopeForSaveArray(size_t arraySize)
	{
		assert(mRootJson.is_object() && mRootJson.size() == 0);
		mRootJson = web::json::value::array(arraySize);
		return std::make_unique<Detail::JsonArrayScope>(&mRootJson, mSerializeState);
	}

private:
	web::json::value mRootJson;
	std::variant<std::nullptr_t, archive_format*, output_stream*> mOutput;
};

}	// namespace BitSerializer