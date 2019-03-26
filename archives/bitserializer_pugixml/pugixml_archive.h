/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <sstream>
#include <type_traits>
#include <optional>
#include <variant>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/media_archive_base.h"

// External dependency (PugiXml)
#include "pugixml.hpp"

namespace BitSerializer::Xml::PugiXml {
namespace Detail {

/// <summary>
/// The traits of XML archive based on PugiXml
/// </summary>
class PugiXmlArchiveTraits
{
public:
	// Character type is related to PugiXml's definition PUGIXML_WCHAR_MODE
#ifdef PUGIXML_WCHAR_MODE
	using key_type = std::wstring;
	using supported_key_types = SupportedKeyTypes<key_type, const wchar_t*>;
	using preferred_output_format = std::wstring;
	using preferred_stream_char_type = wchar_t;
#else
	using key_type = std::string;
	using supported_key_types = SupportedKeyTypes<key_type, const char*>;
	using preferred_output_format = std::string;
	using preferred_stream_char_type = char;
#endif

	static const wchar_t path_separator	= L'/';
};

// Forward declarations
template <SerializeMode TMode>
class PugiXmlObjectScope;

/// <summary>
/// Base class of XML scope
/// </summary>
/// <seealso cref="MediaArchiveBase" />
class PugiXmlScopeBase : public PugiXmlArchiveTraits
{
public:
	explicit PugiXmlScopeBase(const pugi::xml_node& node)
		: mNode(node)
	{ }

	virtual ~PugiXmlScopeBase() = default;

	/// <summary>
	/// Gets the current path in XML.
	/// </summary>
	/// <returns></returns>
	virtual std::wstring GetPath() const
	{
		return Convert::ToWString(mNode.path());
	}

protected:
	pugi::xml_node AppendChild(const key_type& key) {
		return mNode.append_child(key.c_str());
	}

	pugi::xml_node AppendChild(const pugi::char_t* key) {
		return mNode.append_child(key);
	}

	pugi::xml_node GetChild(const key_type& key) const {
		return mNode.child(key.c_str());
	}

	pugi::xml_node GetChild(const pugi::char_t* key) const {
		return mNode.child(key);
	}

	template <typename T>
	void LoadValue(const pugi::xml_node& node, T& value) {
		if constexpr (std::is_same_v<T, bool>) {
			value = node.text().as_bool();
		}
		else if constexpr (std::is_integral_v<T>)
		{
			if constexpr (std::is_same_v<T, long long>)
				value = node.text().as_llong();
			else if constexpr (std::is_same_v<T, unsigned long>)
				value = node.text().as_ullong();
			else if constexpr (std::is_unsigned_v<T>)
				value = static_cast<T>(node.text().as_uint());
			else
				value = static_cast<T>(node.text().as_int());
		}
		else
		{
			if constexpr (std::is_same_v<T, float>)
				value = node.text().as_float();
			else if constexpr (std::is_same_v<T, double>)
				value = node.text().as_double();
		}
	}

	template <typename TSym, typename TStrAllocator>
	void LoadValue(const pugi::xml_node& node, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, pugi::char_t>)
			value = node.text().as_string();
		else
			value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>>(node.text().as_string());
	}

	template <typename T>
	static void SaveValue(const pugi::xml_node& node, T& value) {
		node.text().set(value);
	}

	static void SaveValue(const pugi::xml_node& node, const pugi::char_t* value) {
		node.text().set(value);
	}

	template <typename TSym, typename TStrAllocator>
	static void SaveValue(const pugi::xml_node& node, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value) {
		if (std::is_same_v<TSym, pugi::char_t>)
			node.text().set(value.c_str());
		else
			node.text().set(Convert::To<pugi::string_t>(value).c_str());
	}

	template <SerializeMode TMode>
	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScopeImpl(const key_type& key)
	{
		if constexpr (TMode == SerializeMode::Load) {
			auto child = mNode.child(key.c_str());
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child);
		}
		else
		{
			auto child = mNode.append_child(key.c_str());
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child);
		}
	}

	template <SerializeMode TMode>
	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScopeImpl(const pugi::char_t* key)
	{
		if constexpr (TMode == SerializeMode::Load) {
			auto child = mNode.child(key);
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child);
		}
		else
		{
			auto child = mNode.append_child(key);
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child);
		}
	}

	pugi::xml_node mNode;
};


/// <summary>
/// XML scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
template <SerializeMode TMode>
class PugiXmlArrayScope : public ArchiveScope<TMode>, public PugiXmlScopeBase
{
public:
	explicit PugiXmlArrayScope(const pugi::xml_node& node)
		: PugiXmlScopeBase(node)
		, mValueIt(mNode.begin())
	{ }

	/// <summary>
	/// Returns the size of stored elements (for arrays and objects).
	/// </summary>
	size_t GetSize() const {
		return std::distance(mNode.begin(), mNode.end());
	}

	template<typename T>
	void SerializeValue(T& value) {
		SerializeValueImpl(GetKeyByValueType<T>(), value);
	}

	template <typename TSym, typename TAllocator>
	void SerializeString(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value) {
		SerializeValueImpl(GetKeyByValueType<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(), value);
	}

	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mValueIt != mValueIt->end())
			{
				auto arrayScope = std::make_optional<PugiXmlArrayScope<TMode>>(*mValueIt);
				++mValueIt;
				return arrayScope;
			}
			return std::nullopt;
		}
		else
		{
			auto node = AppendChild(PUGIXML_TEXT("array"));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
	}

	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mValueIt != mValueIt->end())
			{
				auto objectScope = std::make_optional<PugiXmlObjectScope<TMode>>(*mValueIt);
				++mValueIt;
				return objectScope;
			}
			return std::nullopt;
		}
		else
		{
			auto node = mNode.append_child(PUGIXML_TEXT("object"));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(node);
		}
	}

protected:
	template <typename T>
	void SerializeValueImpl(const pugi::char_t* key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mValueIt != mValueIt->end())
			{
				LoadValue(*mValueIt, value);
				++mValueIt;
			}
		}
		else
		{
			auto child = AppendChild(key);
			if (!child.empty())
				SaveValue(child, value);
		}
	}

	template <typename T>
	static const pugi::char_t* GetKeyByValueType()
	{
		if constexpr (std::is_same_v<T, bool>)
			return PUGIXML_TEXT("bool");

		if constexpr (std::is_same_v<T, int8_t>)
			return PUGIXML_TEXT("byte");
		if constexpr (std::is_same_v<T, uint8_t>)
			return PUGIXML_TEXT("unsignedByte");

		if constexpr (std::is_same_v<T, int16_t>)
			return PUGIXML_TEXT("short");
		if constexpr (std::is_same_v<T, uint16_t>)
			return PUGIXML_TEXT("unsignedShort");

		if constexpr (std::is_same_v<T, int32_t>)
			return PUGIXML_TEXT("int");
		if constexpr (std::is_same_v<T, uint32_t>)
			return PUGIXML_TEXT("unsignedInt");

		if constexpr (std::is_same_v<T, int64_t>)
			return PUGIXML_TEXT("long");
		if constexpr (std::is_same_v<T, uint64_t>)
			return PUGIXML_TEXT("unsignedLong");

		if constexpr (std::is_same_v<T, float>)
			return PUGIXML_TEXT("float");
		if constexpr (std::is_same_v<T, double>)
			return PUGIXML_TEXT("double");

		return PUGIXML_TEXT("string");
	}

	pugi::xml_node_iterator mValueIt;
};


/// <summary>
/// Constant iterator of the keys.
/// </summary>
class key_const_iterator
{
	template <SerializeMode TMode>
	friend class PugiXmlObjectScope;

	pugi::xml_node_iterator mNodeIt;

	key_const_iterator(pugi::xml_node_iterator it)
		: mNodeIt(it) { }

public:
	bool operator==(const key_const_iterator& rhs) const {
		return this->mNodeIt == rhs.mNodeIt;
	}
	bool operator!=(const key_const_iterator& rhs) const {
		return this->mNodeIt != rhs.mNodeIt;
	}

	key_const_iterator& operator++() {
		++mNodeIt;
		return *this;
	}

	const PugiXmlArchiveTraits::key_type::value_type* operator*() const {
		return mNodeIt->name();
	}
};


/// <summary>
/// XML scope for serializing objects (list of values with keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
template <SerializeMode TMode>
class PugiXmlObjectScope : public ArchiveScope<TMode>, public PugiXmlScopeBase
{
public:
	explicit PugiXmlObjectScope(const pugi::xml_node& node)
		: PugiXmlScopeBase(node)
	{
		assert(mNode.type() == pugi::node_element);
	}

	key_const_iterator cbegin() const {
		return key_const_iterator(mNode.begin());
	}

	key_const_iterator cend() const {
		return key_const_iterator(mNode.end());
	}

	bool SerializeValue(const key_type& key, bool& value) {
		return SerializeImpl(key, value);
	}
	bool SerializeValue(const pugi::char_t* key, bool& value) {
		return SerializeImpl(key, value);
	}

	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool SerializeValue(const key_type& key, T& value) {
		return SerializeImpl(key, value);
	}
	template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool SerializeValue(const wchar_t* key, T& value) {
		return SerializeImpl(key, value);
	}

	template <typename TSym, typename TStrAllocator>
	bool SerializeString(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value) {
		return SerializeImpl(key, value);
	}
	template <typename TSym, typename TStrAllocator>
	bool SerializeString(const pugi::char_t* key, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value) {
		return SerializeImpl(key, value);
	}

	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(const key_type& key) {
		return OpenObjectScopeImpl<TMode>(key);
	}

	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(const pugi::char_t* key) {
		return OpenObjectScopeImpl<TMode>(key);
	}

	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto node = mNode.child(key.c_str());
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
		else
		{
			auto node = mNode.append_child(key.c_str());
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
	}

	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(const pugi::char_t* key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto node = mNode.child(key);
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
		else
		{
			auto node = mNode.append_child(key);
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
	}

protected:
	template <typename TKey, typename T>
	bool SerializeImpl(TKey&& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto child = GetChild(std::forward<TKey>(key));
			if (child.empty())
				return false;
			LoadValue(child, value);
			return true;
		}
		else
		{
			auto child = AppendChild(std::forward<TKey>(key));
			if (child.empty())
				return false;
			SaveValue(child, value);
			return true;
		}
	}
};


/// <summary>
/// XML root scope (can serialize one value, array or object without key)
/// </summary>
template <SerializeMode TMode>
class PugiXmlRootScope : public ArchiveScope<TMode>, public PugiXmlArchiveTraits
{
public:
	explicit PugiXmlRootScope(const pugi::char_t* inputStr)
		: mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		const auto result = mRootXml.load_string(inputStr);
		if (!result)
			throw SerializationException(SerializationErrorCode::ParsingError, result.description());
	}

	explicit PugiXmlRootScope(const pugi::string_t& inputStr)
		: PugiXmlRootScope(inputStr.c_str())
	{}

	explicit PugiXmlRootScope(pugi::string_t& outputStr)
		: mOutput(&outputStr)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	explicit PugiXmlRootScope(std::istream& inputStream)
		: mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		const auto result = mRootXml.load(inputStream);
		if (!result)
			throw SerializationException(SerializationErrorCode::ParsingError, result.description());
	}

	explicit PugiXmlRootScope(std::wistream& inputStream)
		: mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		const auto result = mRootXml.load(inputStream);
		if (!result)
			throw SerializationException(SerializationErrorCode::ParsingError, result.description());
	}

	explicit PugiXmlRootScope(std::ostream& outputStream)
		: mOutput(&outputStream)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	explicit PugiXmlRootScope(std::wostream& outputStream)
		: mOutput(&outputStream)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	~PugiXmlRootScope()
	{
		Finish();
	}

	/// <summary>
	/// Gets the current path in XML.
	/// </summary>
	std::wstring GetPath() const
	{
		return Convert::ToWString(mRootXml.path());
	}

	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return mRootXml.first_child().empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(mRootXml.first_child());
		}
		else
		{
			auto node = mRootXml.append_child("array");
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
	}

	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto node = mRootXml.child(key.c_str());
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
		else
		{
			auto node = mRootXml.append_child(key.c_str());
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
	}

	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(const pugi::char_t* key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto node = mRootXml.child(key);
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
		else
		{
			auto node = mRootXml.append_child(key);
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
	}

	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope()
	{
		if constexpr (TMode == SerializeMode::Load) {
			return mRootXml.first_child().empty() ?  std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(mRootXml.first_child());
		}
		else
		{
			auto node = mRootXml.append_child(PUGIXML_TEXT("root"));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(node);
		}
	}

	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(const key_type& key)
	{
		if constexpr (TMode == SerializeMode::Load)	{
			auto node = mRootXml.child(key.c_str());
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(node);
		}
		else
		{
			auto node = mRootXml.append_child(key.c_str());
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(node);
		}
	}

	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(const pugi::char_t* key)
	{
		if constexpr (TMode == SerializeMode::Load) {
			auto node = mRootXml.child(key);
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(node);
		}
		else
		{
			auto node = mRootXml.append_child(key);
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(node);
		}
	}

private:
	void Finish()
	{
		if constexpr (TMode == SerializeMode::Save)
		{
			auto decl = mRootXml.prepend_child(pugi::node_declaration);
			decl.append_attribute("version") = "1.0";

			std::visit([this](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, std::string*>)
				{
					std::ostringstream stream;
					mRootXml.print(stream);
					*arg = stream.str();
				}
				else if constexpr (std::is_same_v<T, std::wstring*>)
				{
					std::wostringstream stream;
					mRootXml.print(stream);
					*arg = stream.str();
				}
				else if constexpr (std::is_same_v<T, std::ostream*> || std::is_same_v<T, std::wostream*>) {
					mRootXml.print(*arg);
				}
			}, mOutput);
			mOutput = nullptr;
		}
	}

	pugi::xml_document mRootXml;
	std::variant<std::nullptr_t, std::string*, std::wstring*, std::ostream*, std::wostream*> mOutput;
};

} //namespace Detail


/// <summary>
/// Declaration of JSON archive
/// </summary>
using XmlArchive = MediaArchiveBase<
	Detail::PugiXmlArchiveTraits,
	Detail::PugiXmlRootScope<SerializeMode::Load>,
	Detail::PugiXmlRootScope<SerializeMode::Save>>;

}	// namespace BitSerializer::Xml::PugiXml