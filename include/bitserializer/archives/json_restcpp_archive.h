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

/// <summary>
/// The traits of JSON archive based on C++ REST SDK
/// </summary>
class JsonArchiveTraits
{
public:
	using key_type = utility::string_t;
	using output_format = utility::string_t;
	using input_stream = utility::istream_t;
	using output_stream = utility::ostream_t;
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

	JsonScopeBase(const web::json::value* node)
		: mNode(const_cast<web::json::value*>(node))
	{ }

	inline size_t GetSize() const {
		return mNode->size();
	}

protected:
	web::json::value* mNode;
};


/// <summary>
/// JSON scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="JsonArchiveBase" />
template <SerializeMode TMode>
class JsonArrayScope : public ArchiveScope<TMode>, public JsonScopeBase
{
public:
	JsonArrayScope(const web::json::value* node)
		: JsonScopeBase(node)
		, mIndex(0)
	{
		assert(mNode->is_array());
	}

	template <typename TSym, typename TAllocator>
	void SerializeString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
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
			if constexpr (std::is_integral_v<T>)
			{
				if (jsonValue.is_integer())
					value = static_cast<T>(jsonValue.as_integer());
			}
			else
			{
				if (jsonValue.is_number())
					value = static_cast<T>(jsonValue.as_double());
			}
		}
		else
		{
			SaveJsonValue(web::json::value(value));
		}
	}

	std::unique_ptr<JsonObjectScope<TMode>> OpenScopeForSerializeObject()
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto& jsonValue = LoadJsonValue();
			if (jsonValue.is_object())
			{
				decltype(auto) node = const_cast<web::json::value&>(jsonValue);
				return std::make_unique<JsonObjectScope<TMode>>(&node);
			}
			return nullptr;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(web::json::value::object());
			return std::make_unique<JsonObjectScope<TMode>>(&jsonValue);
		}
	}

	//------------------------------------------------------------------------------
	// Serialize arrays
	//------------------------------------------------------------------------------
	inline std::unique_ptr<JsonArrayScope<TMode>> OpenScopeForLoadArray()
	{
		auto& jsonValue = LoadJsonValue();
		if (jsonValue.is_array())
			return std::make_unique<JsonArrayScope<TMode>>(&jsonValue);
		return nullptr;
	}

	inline std::unique_ptr<JsonArrayScope<TMode>> OpenScopeForSaveArray(size_t arraySize)
	{
		auto& jsonValue = SaveJsonValue(web::json::value::array(arraySize));
		return std::make_unique<JsonArrayScope<TMode>>(&jsonValue);
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

	JsonObjectScope(const web::json::value* node)
		: JsonScopeBase(node)
	{ };

	// ToDo: handle errors when mNode is not an object

	/// <summary>
	/// Gets the key by index.
	/// </summary>
	key_type GetKeyByIndex(int index) {
		return (mNode->as_object().cbegin() + index)->first;
	}

	template <typename TSym, typename TAllocator>
	void SerializeString(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
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
		else
		{
			if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
				SaveJsonValue(key, web::json::value(value));
			else
				SaveJsonValue(key, web::json::value(Convert::FromString<utility::string_t>(value)));
		}
	}

	void SerializeValue(const key_type& key, bool& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr && jsonValue->is_boolean())
				value = jsonValue->as_bool();
		}
		else
		{
			SaveJsonValue(key, web::json::value::boolean(value));
		}
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SerializeValue(const key_type& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr)
			{
				if constexpr (std::is_integral_v<T>)
				{
					if (jsonValue->is_integer())
						value = static_cast<T>(jsonValue->as_integer());
				}
				else
				{
					if (jsonValue->is_number())
						value = static_cast<T>(jsonValue->as_double());
				}
			}
		}
		else
		{
			SaveJsonValue(key, web::json::value::number(value));
		}
	}

	std::unique_ptr<JsonObjectScope<TMode>> OpenScopeForSerializeObject(const key_type& key)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto* jsonValue = LoadJsonValue(key);
			if (jsonValue != nullptr && jsonValue->is_object())
			{
				decltype(auto) node = const_cast<web::json::value*>(jsonValue);
				return std::make_unique<JsonObjectScope<TMode>>(node);
			}
			return nullptr;
		}
		else
		{
			auto& jsonValue = SaveJsonValue(key, web::json::value::object());
			return std::make_unique<JsonObjectScope<TMode>>(&jsonValue);
		}
	}

	//------------------------------------------------------------------------------
	// Serialize arrays
	//------------------------------------------------------------------------------
	inline std::unique_ptr<JsonArrayScope<TMode>> OpenScopeForLoadArray(const key_type& key)
	{
		auto* jsonValue = LoadJsonValue(key);
		if (jsonValue != nullptr && jsonValue->is_array())
			return std::make_unique<JsonArrayScope<TMode>>(jsonValue);
		return nullptr;
	}

	inline std::unique_ptr<JsonArrayScope<TMode>> OpenScopeForSaveArray(const key_type& key, size_t arraySize)
	{
		auto& jsonValue = SaveJsonValue(key, web::json::value::array(arraySize));
		return std::make_unique<JsonArrayScope<TMode>>(&jsonValue);
	}

protected:
	inline const web::json::value* LoadJsonValue(const typename key_type& key) const
	{
		assert(mNode->is_object());
		const auto& jObject = mNode->as_object();
		auto it = jObject.find(key);
		return it == jObject.end() ? nullptr : &it->second;
	}

	inline web::json::value& SaveJsonValue(const key_type& key, web::json::value&& jsonValue) const
	{
		assert(mNode->is_object());
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

	JsonRootScope(output_format& outputFormat)
		: Detail::JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			std::error_code error;
			mRootJson = web::json::value::parse(outputFormat, error);
			if (mRootJson.is_null())
			{
				// Todo: exception
			}
		}
		else
		{
			mOutput = &outputFormat;
			mRootJson = web::json::value::object();
		}
	}

	JsonRootScope(input_stream& input)
		: Detail::JsonScopeBase(&mRootJson)
		, mOutput(nullptr)
	{
		assert(TMode == SerializeMode::Load);
		std::error_code error;
		mRootJson = web::json::value::parse(input, error);
		if (mRootJson.is_null())
		{
			// Todo: exception
		}
	}

	JsonRootScope(output_stream& output)
		: Detail::JsonScopeBase(&mRootJson)
		, mOutput(&output)
	{
		mOutput = &output;
		mRootJson = web::json::value::object();
	}

	~JsonRootScope()
	{
		Finish();
	}

	template <typename TSym, typename TAllocator>
	void SerializeString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mRootJson.is_string())
			{
				if constexpr (std::is_same_v<TSym, utility::string_t::value_type>)
					value = mRootJson.as_string();
				else
					value = Convert::FromString<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(mRootJson.as_string());
			}
		}
		else
		{
			assert(mRootJson.is_object() && mRootJson.size() == 0);
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
			assert(mRootJson.is_object() && mRootJson.size() == 0);
			mRootJson = web::json::value::boolean(value);
		}
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	void SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if constexpr (std::is_integral_v<T>)
			{
				if (mRootJson.is_integer())
					value = static_cast<T>(mRootJson.as_integer());
			}
			else
			{
				if (mRootJson.is_number())
					value = static_cast<T>(mRootJson.as_double());
			}
		}
		else
		{
			assert(mRootJson.is_object() && mRootJson.size() == 0);
			mRootJson = web::json::value::number(value);
		}
	}

	std::unique_ptr<JsonObjectScope<TMode>> OpenScopeForSerializeObject()
	{
		return std::make_unique<JsonObjectScope<TMode>>(&mRootJson);
	}

	//------------------------------------------------------------------------------
	// Serialize arrays
	//------------------------------------------------------------------------------
	std::unique_ptr<Detail::JsonArrayScope<TMode>> OpenScopeForLoadArray()
	{
		if (mRootJson.is_array())
			return std::make_unique<Detail::JsonArrayScope<TMode>>(&mRootJson);
		return nullptr;
	}

	std::unique_ptr<Detail::JsonArrayScope<TMode>> OpenScopeForSaveArray(size_t arraySize)
	{
		assert(mRootJson.is_object() && mRootJson.size() == 0);
		mRootJson = web::json::value::array(arraySize);
		return std::make_unique<Detail::JsonArrayScope<TMode>>(&mRootJson);
	}

private:
	void Finish()
	{
		if constexpr (TMode == SerializeMode::Save)
		{
			std::visit([this](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, output_format*>)
					*arg = mRootJson.serialize();
				else if constexpr (std::is_same_v<T, output_stream*>)
					*arg << mRootJson;
			}, mOutput);
			mOutput = nullptr;
		}
	}

	web::json::value mRootJson;
	std::variant<std::nullptr_t, output_format*, output_stream*> mOutput;
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