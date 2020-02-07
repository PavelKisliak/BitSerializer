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
struct PugiXmlArchiveTraits
{
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

	static const char path_separator = '/';

protected:
	~PugiXmlArchiveTraits() = default;
};

namespace PugiXmlExtensions
{
	inline pugi::xml_node AppendChild(pugi::xml_node& node, const PugiXmlArchiveTraits::key_type& key) {
		return node.append_child(key.c_str());
	}

	inline pugi::xml_node AppendChild(pugi::xml_node& node, const pugi::char_t* key) {
		return node.append_child(key);
	}

	inline pugi::xml_node GetChild(pugi::xml_node& node, const PugiXmlArchiveTraits::key_type& key) {
		return node.child(key.c_str());
	}

	inline pugi::xml_node GetChild(pugi::xml_node& node, const pugi::char_t* key) {
		return node.child(key);
	}

	inline pugi::xml_attribute AppendAttribute(pugi::xml_node& node, const PugiXmlArchiveTraits::key_type& key) {
		return node.append_attribute(key.c_str());
	}

	inline pugi::xml_attribute AppendAttribute(pugi::xml_node& node, const pugi::char_t* key) {
		return node.append_attribute(key);
	}

	inline pugi::xml_attribute GetAttribute(pugi::xml_node& node, const PugiXmlArchiveTraits::key_type& key) {
		return node.attribute(key.c_str());
	}

	inline pugi::xml_attribute GetAttribute(pugi::xml_node& node, const pugi::char_t* key) {
		return node.attribute(key);
	}

	template <typename T>
	void LoadValue(const pugi::xml_node& node, T& value)
	{
		if constexpr (std::is_same_v<T, bool>)
		{
			value = node.text().as_bool();
		}
		else if constexpr (std::is_integral_v<T>)
		{
			if constexpr (std::is_same_v<T, int64_t>)
				value = node.text().as_llong();
			else if constexpr (std::is_same_v<T, uint64_t>)
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
	void SaveValue(const pugi::xml_node& node, T& value) {
		node.text().set(value);
	}

	inline void SaveValue(const pugi::xml_node& node, const pugi::char_t* value) {
		node.text().set(value);
	}

	template <typename TSym, typename TStrAllocator>
	void SaveValue(const pugi::xml_node& node, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, pugi::char_t>)
			node.text().set(value.c_str());
		else
			node.text().set(Convert::To<pugi::string_t>(value).c_str());
	}

	[[nodiscard]] inline std::string GetPath(const pugi::xml_node& node)
	{
#ifdef PUGIXML_WCHAR_MODE
		return Convert::ToString(node.path());
#else
		return node.path();
#endif
	}
} // namespace PugiXmlExtensions


// Forward declarations
template <SerializeMode TMode>
class PugiXmlObjectScope;

/// <summary>
/// XML scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
template <SerializeMode TMode>
class PugiXmlArrayScope final : public ArchiveScope<TMode>, public PugiXmlArchiveTraits
{
public:
	explicit PugiXmlArrayScope(const pugi::xml_node& node)
		: mNode(node)
		, mValueIt(mNode.begin())
	{ }

	/// <summary>
	/// Gets the current path in XML. Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] std::string GetPath() const {
		return PugiXmlExtensions::GetPath(mNode);
	}

	/// <summary>
	/// Returns the size of stored elements (for arrays and objects).
	/// </summary>
	[[nodiscard]] size_t GetSize() const {
		return std::distance(mNode.begin(), mNode.end());
	}

	template<typename T>
	void SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (mValueIt != mValueIt->end())
			{
				PugiXmlExtensions::LoadValue(*mValueIt, value);
				++mValueIt;
			}
		}
		else
		{
			auto child = mNode.append_child(GetKeyByValueType<T>());
			if (!child.empty())
				PugiXmlExtensions::SaveValue(child, value);
		}
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
			auto node = mNode.append_child(PUGIXML_TEXT("array"));
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
	static constexpr const pugi::char_t* GetKeyByValueType() noexcept
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

	pugi::xml_node mNode;
	pugi::xml_node_iterator mValueIt;
};


/// <summary>
/// XML scope for serializing attributes (key=value pairs in the XML node)
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
template <SerializeMode TMode>
class PugiXmlAttributeScope final : public ArchiveScope<TMode>, public PugiXmlArchiveTraits
{
public:
	explicit PugiXmlAttributeScope(const pugi::xml_node& node)
		: mNode(node)
	{
		assert(mNode.type() == pugi::node_element);
	}

	/// <summary>
	/// Gets the current path in XML. Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] std::string GetPath() const {
		return PugiXmlExtensions::GetPath(mNode);
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto attr = PugiXmlExtensions::GetAttribute(mNode, std::forward<TKey>(key));
			if (attr.empty())
				return false;

			if constexpr (std::is_same_v<T, bool>) {
				value = attr.as_bool();
			}
			else if constexpr (std::is_integral_v<T>)
			{
				if constexpr (std::is_same_v<T, int64_t>)
					value = attr.as_llong();
				else if constexpr (std::is_same_v<T, uint64_t>)
					value = attr.as_ullong();
				else if constexpr (std::is_unsigned_v<T>)
					value = static_cast<T>(attr.as_uint());
				else
					value = static_cast<T>(attr.as_int());
			}
			else
			{
				if constexpr (std::is_same_v<T, float>)
					value = attr.as_float();
				else if constexpr (std::is_same_v<T, double>)
					value = attr.as_double();
			}
			return true;
		}
		else
		{
			auto attr = PugiXmlExtensions::AppendAttribute(mNode, std::forward<TKey>(key));
			if (attr.empty())
				return false;
			attr.set_value(value);
			return true;
		}
	}

	template <typename TKey, typename TSym, typename TStrAllocator>
	bool SerializeValue(TKey&& key, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto attr = PugiXmlExtensions::GetAttribute(mNode, std::forward<TKey>(key));
			if (attr.empty())
				return false;

			if constexpr (std::is_same_v<TSym, pugi::char_t>)
				value = attr.as_string();
			else
				value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>>(attr.as_string());
			return true;
		}
		else
		{
			auto attr = PugiXmlExtensions::AppendAttribute(mNode, std::forward<TKey>(key));
			if (attr.empty())
				return false;

			if constexpr (std::is_same_v<TSym, pugi::char_t>)
				attr.set_value(value.c_str());
			else
				attr.set_value(Convert::To<pugi::string_t>(value).c_str());
			return true;
		}
	}

protected:
	pugi::xml_node mNode;
};


/// <summary>
/// Constant iterator of the keys.
/// </summary>
class key_const_iterator
{
	template <SerializeMode TMode>
	friend class PugiXmlObjectScope;

	pugi::xml_node_iterator mNodeIt;

	key_const_iterator(pugi::xml_node_iterator&& it)
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
class PugiXmlObjectScope final : public ArchiveScope<TMode>, public PugiXmlArchiveTraits
{
public:
	explicit PugiXmlObjectScope(const pugi::xml_node& node)
		: mNode(node)
	{
		assert(mNode.type() == pugi::node_element);
	}

	[[nodiscard]] key_const_iterator cbegin() const {
		return key_const_iterator(mNode.begin());
	}

	[[nodiscard]] key_const_iterator cend() const {
		return key_const_iterator(mNode.end());
	}

	/// <summary>
	/// Gets the current path in XML. Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] std::string GetPath() const {
		return PugiXmlExtensions::GetPath(mNode);
	}

	template <typename TKey, typename T>
	bool SerializeValue(TKey&& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto child = PugiXmlExtensions::GetChild(mNode, std::forward<TKey>(key));
			if (child.empty())
				return false;
			PugiXmlExtensions::LoadValue(child, value);
			return true;
		}
		else
		{
			auto child = PugiXmlExtensions::AppendChild(mNode, std::forward<TKey>(key));
			if (child.empty())
				return false;
			PugiXmlExtensions::SaveValue(child, value);
			return true;
		}
	}

	template <typename TKey>
	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(TKey&& key)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto child = PugiXmlExtensions::GetChild(mNode, std::forward<TKey>(key));
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child);
		}
		else
		{
			auto child = PugiXmlExtensions::AppendChild(mNode, std::forward<TKey>(key));
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child);
		}
	}

	template <typename TKey>
	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(TKey&& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto node = PugiXmlExtensions::GetChild(mNode, std::forward<TKey>(key));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
		else
		{
			auto node = PugiXmlExtensions::AppendChild(mNode, std::forward<TKey>(key));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
	}

	std::optional<PugiXmlAttributeScope<TMode>> OpenAttributeScope()
	{
		return std::make_optional<PugiXmlAttributeScope<TMode>>(mNode);
	}

protected:
	pugi::xml_node mNode;
};


/// <summary>
/// XML root scope (can serialize one value, array or object without key)
/// </summary>
template <SerializeMode TMode>
class PugiXmlRootScope final : public ArchiveScope<TMode>, public PugiXmlArchiveTraits
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

	explicit PugiXmlRootScope(pugi::string_t& outputStr, const SerializationOptions& serializationOptions = {})
		: mOutput(&outputStr)
		, mSerializationOptions(serializationOptions)
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

	PugiXmlRootScope(std::ostream& outputStream, const SerializationOptions& serializationOptions = {})
		: mOutput(&outputStream)
		, mSerializationOptions(serializationOptions)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	PugiXmlRootScope(std::wostream& outputStream, const SerializationOptions& serializationOptions = {})
		: mOutput(&outputStream)
		, mSerializationOptions(serializationOptions)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	/// <summary>
	/// Gets the current path in XML. Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] std::string GetPath() const {
		return PugiXmlExtensions::GetPath(mRootXml);
	}

	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return mRootXml.first_child().empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(mRootXml.first_child());
		}
		else
		{
			auto node = mRootXml.append_child(PUGIXML_TEXT("array"));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
	}

	template <typename TKey>
	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(TKey&& key, size_t arraySize)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto node = PugiXmlExtensions::GetChild(mRootXml, std::forward<TKey>(key));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node);
		}
		else
		{
			auto node = PugiXmlExtensions::AppendChild(mRootXml, std::forward<TKey>(key));
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

	template <typename TKey>
	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(TKey&& key)
	{
		if constexpr (TMode == SerializeMode::Load) {
			auto child = PugiXmlExtensions::GetChild(mRootXml, std::forward<TKey>(key));
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child);
		}
		else
		{
			auto child = PugiXmlExtensions::AppendChild(mRootXml, std::forward<TKey>(key));
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child);
		}
	}

	void Finalize()
	{
		if constexpr (TMode == SerializeMode::Save)
		{
			auto decl = mRootXml.prepend_child(pugi::node_declaration);
			decl.append_attribute(PUGIXML_TEXT("version")) = PUGIXML_TEXT("1.0");

			std::visit([this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				assert(mSerializationOptions);
				unsigned int flags = mSerializationOptions->formatOptions.enableFormat ? pugi::format_indent : 0;
				const pugi::string_t indent(mSerializationOptions->formatOptions.paddingCharNum, mSerializationOptions->formatOptions.paddingChar);
				assert(!mSerializationOptions->formatOptions.enableFormat || !indent.empty());

				if constexpr (std::is_same_v<T, std::string*>)
				{
					std::ostringstream stream;
					mRootXml.print(stream, indent.c_str(), flags);
					*arg = stream.str();
				}
				else if constexpr (std::is_same_v<T, std::wstring*>)
				{
					std::wostringstream stream;
					mRootXml.print(stream, indent.c_str(), flags);
					*arg = stream.str();
				}
				else if constexpr (std::is_same_v<T, std::ostream*> || std::is_same_v<T, std::wostream*>)
				{
					flags |= (mSerializationOptions->streamOptions.writeBom ? pugi::format_write_bom : 0);
					mRootXml.save(*arg, indent.c_str(), flags);
				}
			}, mOutput);
			mOutput = nullptr;
		}
	}

private:
	pugi::xml_document mRootXml;
	std::variant<std::nullptr_t, std::string*, std::wstring*, std::ostream*, std::wostream*> mOutput;
	std::optional<SerializationOptions> mSerializationOptions;
};

} //namespace Detail


/// <summary>
/// XML archive based on the PugiXml library.
/// Encoding in memory is depend from global definition in the PugiXml 'PUGIXML_WCHAR_MODE', by default uses UTF-8.
/// </summary>
using XmlArchive = MediaArchiveBase<
	Detail::PugiXmlArchiveTraits,
	Detail::PugiXmlRootScope<SerializeMode::Load>,
	Detail::PugiXmlRootScope<SerializeMode::Save>>;

}	// namespace BitSerializer::Xml::PugiXml