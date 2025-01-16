/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <optional>
#include <sstream>
#include <type_traits>
#include <variant>
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/errors_handling.h"

// External dependency (PugiXml)
#include "pugixml.hpp"

namespace BitSerializer::Xml::PugiXml {
namespace Detail {

/// <summary>
/// The traits of XML archive based on PugiXml
/// </summary>
struct PugiXmlArchiveTraits
{
	static constexpr ArchiveType archive_type = ArchiveType::Xml;
	// Key type is related to PugiXml's definition PUGIXML_WCHAR_MODE
#ifdef PUGIXML_WCHAR_MODE
	using key_type = std::wstring;
	using supported_key_types = TSupportedKeyTypes<key_type, const wchar_t*>;
	using string_view_type = std::basic_string_view<wchar_t>;
#else
	using key_type = std::string;
	using supported_key_types = TSupportedKeyTypes<key_type, const char*>;
	using string_view_type = std::basic_string_view<char>;
#endif

	using preferred_output_format = std::string;
	using preferred_stream_char_type = char;
	static constexpr char path_separator = '/';
	static constexpr bool is_binary = false;

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

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
	bool LoadValue(const pugi::xml_node& node, T& value, const SerializationOptions& serializationOptions)
	{
		try
		{
			// Empty node is treated as Null
			if (const auto strValue = node.text().as_string(nullptr))
			{
				value = Convert::To<T>(strValue);
				return true;
			}
			return false;
		}
		catch (const std::out_of_range&)
		{
			if (serializationOptions.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::Overflow,
					std::string("The size of target field is not sufficient to deserialize number: ") + node.text().as_string());
			}
		}
		catch (...)
		{
			if (serializationOptions.mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					std::string("The type of target field does not match the value being loaded: ") + node.text().as_string());
			}
		}
		return false;
	}

	inline bool LoadValue(const pugi::xml_node& node, std::nullptr_t&, const SerializationOptions&) {
		return node.empty();
	}

	inline bool LoadValue(const pugi::xml_node& node, PugiXmlArchiveTraits::string_view_type& value, const SerializationOptions&)
	{
		// Empty node is treated as Null
		if (const auto strValue = node.text().as_string(nullptr))
		{
			value = strValue;
			return true;
		}
		return false;
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
	void SaveValue(const pugi::xml_node& node, const T& value) {
		node.text().set(value);
	}

	inline void SaveValue(const pugi::xml_node&, const std::nullptr_t&) {}

	inline void SaveValue(const pugi::xml_node& node, const pugi::char_t* value) {
		node.text().set(value);
	}

	inline void SaveValue(const pugi::xml_node& node, PugiXmlArchiveTraits::string_view_type& value)
	{
		node.text().set(value.data(), value.size());
	}

	[[nodiscard]] inline std::string GetPath(const pugi::xml_node& node)
	{
#ifdef PUGIXML_WCHAR_MODE
		return Convert::ToString(node.path());
#else
		return node.path();
#endif
	}

	inline void HandleMismatchedTypesPolicy(MismatchedTypesPolicy mismatchedTypesPolicy)
	{
		if (mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
		{
			throw SerializationException(SerializationErrorCode::MismatchedTypes,
				"The type of target field does not match the value being loaded");
		}
	}
}


// Forward declarations
template <SerializeMode TMode>
class PugiXmlObjectScope;

/// <summary>
/// XML scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
template <SerializeMode TMode>
class PugiXmlArrayScope final : public TArchiveScope<TMode>, public PugiXmlArchiveTraits
{
public:
	explicit PugiXmlArrayScope(const pugi::xml_node& node, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, mNode(node)
		, mValueIt(mNode.begin())
	{ }

	/// <summary>
	/// Gets the current path in XML. Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] std::string GetPath() const {
		return PugiXmlExtensions::GetPath(mNode);
	}

	/// <summary>
	/// Returns the estimated number of items to load (for reserving the size of containers).
	/// </summary>
	[[nodiscard]] size_t GetEstimatedSize() const {
		return std::distance(mNode.begin(), mNode.end());
	}

	/// <summary>
	/// Returns `true` when all no more values to load.
	/// </summary>
	[[nodiscard]]
	bool IsEnd() const
	{
		static_assert(TMode == SerializeMode::Load);
		return mValueIt == mNode.end();
	}

	template<typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>
		|| std::is_same_v<T, string_view_type>, int> = 0>
	bool SerializeValue(T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			return PugiXmlExtensions::LoadValue(LoadNextItem(), value, this->GetOptions());
		}
		else
		{
			auto child = mNode.append_child("value");
			if (!child.empty())
			{
				PugiXmlExtensions::SaveValue(child, value);
				return true;
			}
		}
		return false;
	}

	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (auto xmlNode = LoadNextItem())
			{
				if (xmlNode.first_child().type() == pugi::node_element)
				{
					return std::make_optional<PugiXmlArrayScope<TMode>>(xmlNode, TArchiveScope<TMode>::GetContext());
				}
				PugiXmlExtensions::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
			return std::nullopt;
		}
		else
		{
			auto node = mNode.append_child(PUGIXML_TEXT("array"));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node, TArchiveScope<TMode>::GetContext());
		}
	}

	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (auto xmlNode = LoadNextItem())
			{
				if (xmlNode.first_child().type() == pugi::node_element)
				{
					return std::make_optional<PugiXmlObjectScope<TMode>>(xmlNode, TArchiveScope<TMode>::GetContext());
				}
				PugiXmlExtensions::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
			return std::nullopt;
		}
		else
		{
			auto node = mNode.append_child(PUGIXML_TEXT("object"));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(node, TArchiveScope<TMode>::GetContext());
		}
	}

protected:
	pugi::xml_node LoadNextItem()
	{
		static_assert(TMode == SerializeMode::Load);
		if (mValueIt != mValueIt->end())
		{
			auto xmlNode = *mValueIt;
			++mValueIt;
			return xmlNode;
		}
		throw SerializationException(SerializationErrorCode::OutOfRange, "No more items to load");
	}

	pugi::xml_node mNode;
	pugi::xml_node_iterator mValueIt;
};


/// <summary>
/// XML scope for serializing attributes (key=value pairs in the XML node)
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
template <SerializeMode TMode>
class PugiXmlAttributeScope final : public TArchiveScope<TMode>, public PugiXmlArchiveTraits
{
public:
	explicit PugiXmlAttributeScope(const pugi::xml_node& node, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, mNode(node)
	{
		assert(mNode.type() == pugi::node_element);
	}

	/// <summary>
	/// Gets the current path in XML. Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] std::string GetPath() const {
		return PugiXmlExtensions::GetPath(mNode);
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto attr = PugiXmlExtensions::GetAttribute(mNode, std::forward<TKey>(key));
			if (attr.empty()) {
				return std::is_null_pointer_v<T>;
			}

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
			else if constexpr (std::is_floating_point_v<T>)
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
			if constexpr (!std::is_null_pointer_v<T>) {
				attr.set_value(value);
			}
			return true;
		}
	}

	template <typename TKey>
	bool SerializeValue(TKey&& key, string_view_type& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto attr = PugiXmlExtensions::GetAttribute(mNode, std::forward<TKey>(key));
			if (attr.empty())
				return false;

			value = attr.as_string();
			return true;
		}
		else
		{
			auto attr = PugiXmlExtensions::AppendAttribute(mNode, std::forward<TKey>(key));
			if (attr.empty())
				return false;

			attr.set_value(value.data(), value.size());
			return true;
		}
	}

protected:
	pugi::xml_node mNode;
};


/// <summary>
/// XML scope for serializing objects (list of values with keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
template <SerializeMode TMode>
class PugiXmlObjectScope final : public TArchiveScope<TMode>, public PugiXmlArchiveTraits
{
public:
	explicit PugiXmlObjectScope(const pugi::xml_node& node, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, mNode(node)
	{
		assert(mNode.type() == pugi::node_element);
	}

	/// <summary>
	/// Returns the estimated number of items to load (for reserving the size of containers).
	/// </summary>
	[[nodiscard]] size_t GetEstimatedSize() const {
		return std::distance(mNode.begin(), mNode.end());
	}

	/// <summary>
	/// Enumerates all keys by calling a passed function.
	/// </summary>
	template <typename TCallback>
	void VisitKeys(TCallback&& fn)
	{
		for (auto& keyVal : this->mNode) {
			fn(keyVal.name());
		}
	}

	/// <summary>
	/// Gets the current path in XML. Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] std::string GetPath() const {
		return PugiXmlExtensions::GetPath(mNode);
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>
		|| std::is_same_v<T, string_view_type>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			auto child = PugiXmlExtensions::GetChild(mNode, std::forward<TKey>(key));
			if (child.empty())
				return false;
			return PugiXmlExtensions::LoadValue(child, value, this->GetOptions());
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
	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(TKey&& key, size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (auto child = PugiXmlExtensions::GetChild(mNode, std::forward<TKey>(key)))
			{
				if (child.first_child().type() == pugi::node_element)
				{
					return std::make_optional<PugiXmlObjectScope<TMode>>(child, TArchiveScope<TMode>::GetContext());
				}
				PugiXmlExtensions::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
			return std::nullopt;
		}
		else
		{
			auto child = PugiXmlExtensions::AppendChild(mNode, std::forward<TKey>(key));
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child, TArchiveScope<TMode>::GetContext());
		}
	}

	template <typename TKey>
	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(TKey&& key, size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (auto node = PugiXmlExtensions::GetChild(mNode, std::forward<TKey>(key)))
			{
				if (node.first_child().type() == pugi::node_element)
				{
					return std::make_optional<PugiXmlArrayScope<TMode>>(node, TArchiveScope<TMode>::GetContext());
				}
				PugiXmlExtensions::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
			return std::nullopt;
		}
		else
		{
			auto node = PugiXmlExtensions::AppendChild(mNode, std::forward<TKey>(key));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node, TArchiveScope<TMode>::GetContext());
		}
	}

	std::optional<PugiXmlAttributeScope<TMode>> OpenAttributeScope()
	{
		return std::make_optional<PugiXmlAttributeScope<TMode>>(mNode, TArchiveScope<TMode>::GetContext());
	}

protected:
	pugi::xml_node mNode;
};


/// <summary>
/// XML root scope (can serialize one value, array or object without key)
/// </summary>
template <SerializeMode TMode>
class PugiXmlRootScope final : public TArchiveScope<TMode>, public PugiXmlArchiveTraits
{
public:
	PugiXmlRootScope(const std::string_view& inputStr, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		const auto result = mRootXml.load_buffer(inputStr.data(), inputStr.size(), pugi::parse_default, pugi::encoding_utf8);
		if (!result)
			throw ParsingException(result.description(), 0, result.offset);
	}

	PugiXmlRootScope(std::string& outputStr, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, mOutput(&outputStr)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	PugiXmlRootScope(std::istream& inputStream, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, mOutput(nullptr)
	{
		static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
		const auto result = mRootXml.load(inputStream);
		if (!result)
			throw ParsingException(result.description(), 0, result.offset);
	}

	PugiXmlRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
		: TArchiveScope<TMode>(serializationContext)
		, mOutput(&outputStream)
	{
		static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
	}

	/// <summary>
	/// Gets the current path in XML. Unicode symbols encode to UTF-8.
	/// </summary>
	[[nodiscard]] std::string GetPath() const {
		return PugiXmlExtensions::GetPath(mRootXml);
	}

	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (auto childNode = mRootXml.first_child())
			{
				if (childNode.type() == pugi::node_element)
				{
					return std::make_optional<PugiXmlArrayScope<TMode>>(childNode, TArchiveScope<TMode>::GetContext());
				}
				PugiXmlExtensions::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
			return std::nullopt;
		}
		else
		{
			auto node = mRootXml.append_child(PUGIXML_TEXT("array"));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node, TArchiveScope<TMode>::GetContext());
		}
	}

	template <typename TKey>
	std::optional<PugiXmlArrayScope<TMode>> OpenArrayScope(TKey&& key, size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (auto node = PugiXmlExtensions::GetChild(mRootXml, std::forward<TKey>(key)))
			{
				if (node.type() == pugi::node_element)
				{
					return std::make_optional<PugiXmlArrayScope<TMode>>(node, TArchiveScope<TMode>::GetContext());
				}
				PugiXmlExtensions::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
			return std::nullopt;
		}
		else
		{
			auto node = PugiXmlExtensions::AppendChild(mRootXml, std::forward<TKey>(key));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlArrayScope<TMode>>(node, TArchiveScope<TMode>::GetContext());
		}
	}

	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (auto node = mRootXml.first_child())
			{
				if (node.type() == pugi::node_element)
				{
					return std::make_optional<PugiXmlObjectScope<TMode>>(node, TArchiveScope<TMode>::GetContext());
				}
				PugiXmlExtensions::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
			return std::nullopt;
		}
		else
		{
			auto node = mRootXml.append_child(PUGIXML_TEXT("root"));
			return node.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(node, TArchiveScope<TMode>::GetContext());
		}
	}

	template <typename TKey>
	std::optional<PugiXmlObjectScope<TMode>> OpenObjectScope(TKey&& key, size_t)
	{
		if constexpr (TMode == SerializeMode::Load)
		{
			if (auto child = PugiXmlExtensions::GetChild(mRootXml, std::forward<TKey>(key)))
			{
				if (child.type() == pugi::node_element)
				{
					return std::make_optional<PugiXmlObjectScope<TMode>>(child, TArchiveScope<TMode>::GetContext());
				}
				PugiXmlExtensions::HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
			}
			return std::nullopt;
			
		}
		else
		{
			auto child = PugiXmlExtensions::AppendChild(mRootXml, std::forward<TKey>(key));
			return child.empty() ? std::nullopt : std::make_optional<PugiXmlObjectScope<TMode>>(child, TArchiveScope<TMode>::GetContext());
		}
	}

	void Finalize()
	{
		if constexpr (TMode == SerializeMode::Save)
		{
			std::visit([this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;

				auto& options = TArchiveScope<TMode>::GetOptions();
				unsigned int flags = options.formatOptions.enableFormat ? pugi::format_indent : pugi::format_raw;
				const pugi::string_t indent(options.formatOptions.paddingCharNum, options.formatOptions.paddingChar);
				assert(!options.formatOptions.enableFormat || !indent.empty());

				if constexpr (std::is_same_v<T, std::string*>)
				{
					CXmlStringWriter xmlStringWriter(*arg);
					mRootXml.save(xmlStringWriter, indent.c_str(), flags, pugi::encoding_utf8);
				}
				else if constexpr (std::is_same_v<T, std::ostream*>)
				{
					flags |= options.streamOptions.writeBom ? pugi::format_write_bom : 0;
					mRootXml.save(*arg, indent.c_str(), flags, ToPugiUtfType(options.streamOptions.encoding));
				}
			}, mOutput);
			mOutput = nullptr;
		}
	}

private:
	static pugi::xml_encoding ToPugiUtfType(const Convert::Utf::UtfType utfType)
	{
		switch (utfType)
		{
		case Convert::Utf::UtfType::Utf8:
			return pugi::xml_encoding::encoding_utf8;
		case Convert::Utf::UtfType::Utf16le:
			return pugi::xml_encoding::encoding_utf16_le;
		case Convert::Utf::UtfType::Utf16be:
			return pugi::xml_encoding::encoding_utf16_be;
		case Convert::Utf::UtfType::Utf32le:
			return pugi::xml_encoding::encoding_utf32_le;
		case Convert::Utf::UtfType::Utf32be:
			return pugi::xml_encoding::encoding_utf32_be;
		default:
			const auto strEncodingType = Convert::TryTo<std::string>(utfType);
			throw SerializationException(SerializationErrorCode::UnsupportedEncoding,
				"The archive does not support encoding: " +
				(strEncodingType.has_value() ? strEncodingType.value() : std::to_string(static_cast<int>(utfType))));
		}
	}

	class CXmlStringWriter : public pugi::xml_writer
	{
	public:
		CXmlStringWriter(std::string& outputStr)
			: mOutputString(outputStr)
		{ }

		void write(const void* data, size_t size) override
		{
			mOutputString.append(static_cast<const PUGIXML_CHAR*>(data), size);
		}

	private:
		std::string& mOutputString;
	};

	pugi::xml_document mRootXml;
	std::variant<std::nullptr_t, std::string*, std::ostream*> mOutput;
};

}


/// <summary>
/// XML archive based on the PugiXml library.
/// Supports load/save from:
/// - <c>std::string</c>: UTF-8
/// - <c>std::istream</c> and <c>std::ostream</c>: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE
/// </summary>
/// <remarks>
/// The XML-key type is depends from global definition in the PugiXml 'PUGIXML_WCHAR_MODE' in the PugiXml, by default uses std::string.
/// For stay your code cross compiled you can use macros PUGIXML_TEXT("MyKey") from PugiXml or
/// use BitSerializer::AutoKeyValue() but with possible small overhead for converting.
/// </remarks>
using XmlArchive = TArchiveBase<
	Detail::PugiXmlArchiveTraits,
	Detail::PugiXmlRootScope<SerializeMode::Load>,
	Detail::PugiXmlRootScope<SerializeMode::Save>>;

}
