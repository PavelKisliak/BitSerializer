/*******************************************************************************
* Copyright (C) 2020 by Artsiom Marozau                                        *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/

#pragma once
#include <cassert>
#include <type_traits>
#include <optional>
#include <variant>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/media_archive_base.h"

#pragma warning(push)
#pragma warning(disable : 4996)
#include <yaml-cpp/yaml.h>
#pragma warning(pop)

namespace BitSerializer::Yaml::YamlCpp {
	namespace Detail {

		/// <summary>	
		/// If T char/unsigned char/signed char ::value will be True, otherwise False. 
		/// </summary>	
		template <typename T>
		struct is_any_char: std::bool_constant<std::is_same_v<char, T> ||
											   std::is_same_v<unsigned char, T> ||
											   std::is_same_v<signed char, T>>
		{};

		template<typename T>
		constexpr bool is_any_char_v = is_any_char<T>::value;

		/// <summary>	
		/// YAML archive traits class.
		/// </summary>
		class YamlArchiveTraits
		{
		public:
			using key_type = std::string;
			using supported_key_types = SupportedKeyTypes<std::string>;
			using preferred_output_format = std::string;
			using preferred_stream_char_type = std::ostream::char_type;
			static constexpr char path_separator = '/';
		};

		template <SerializeMode TMode>
		class YamlObjectScope;

		/// <summary>
		/// Common base class for YAML scopes.
		/// </summary>
		/// <seealso cref="YamlArchiveTraits" />
		class YamlScopeBase : public YamlArchiveTraits
		{
		public:

			/// <summary>	Constructor. </summary>
			/// <param name="node">	Node represented by current scope level. </param>
			/// <param name="parent">   	[in] (Optional) If non-null, the child node. </param>
			/// <param name="perentKey">	(Optional) Actual for child node only. </param>
			explicit YamlScopeBase(const YAML::Node& node, YamlScopeBase* parent = nullptr, const key_type& perentKey = key_type()) noexcept
				: mNode(node)
				, mParent(parent)
				, mParentKey(perentKey)
			{ }

			/// <summary>	
			/// Get node size (actual for sequence or map). 
			/// </summary>
			/// <returns> Non-zero value if node isn't scalar value, otherwise equal zero. </returns>
			[[nodiscard]]
			virtual size_t GetSize() const { return mNode.size(); }

			/// <summary>	
			/// Get current path in YAML. 
			/// </summary>
			[[nodiscard]]
			virtual std::string GetPath() const
			{
				std::string localPath = mParentKey.empty()
					? Convert::ToString(mParentKey)
					: path_separator + Convert::ToString(mParentKey);
				return mParent == nullptr ? localPath : mParent->GetPath() + localPath;
			}

		protected:
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool LoadValue(const YAML::Node& yamlValue, T& value)
			{
				if (!yamlValue.IsScalar())
					return false;
				//check T it's any char type (mean char/unsigned char or signed char)
				if constexpr (is_any_char_v<T>)
					//convert to short before saving, because yaml-cpp save char's as symbols
					value = static_cast<T>(yamlValue.as<short>());
				else
					value = yamlValue.as<T>();
				return true;
			}

			template <typename TSym, typename TAllocator>
			bool LoadValue(const YAML::Node& yamlValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
			{
				if (!yamlValue.IsScalar())
					return false;
				if constexpr (std::is_same_v<TSym, std::string::value_type>)				
					value = yamlValue.as<std::string>();
				else
					value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(yamlValue.as<std::string>());
				return true;
			}

			YAML::Node mNode;
			YamlScopeBase* mParent;
			key_type mParentKey;
		};

		/// <summary>
		/// YAML scope for serializing arrays.
		/// </summary>
		/// <seealso cref="YamlScopeBase" />
		template <SerializeMode TMode>
		class YamlArrayScope final: public ArchiveScope<TMode>, public YamlScopeBase
		{
		public:

			/// <param name="node">	Node represented by current scope level. </param>
			/// <param name="size">	Size of the node represented by current scope. </param>
			/// <param name="parent">   	[in] (Optional) If non-null, the child node. </param>
			/// <param name="perentKey">	(Optional) Actual for child node only. </param>
			explicit YamlArrayScope(const YAML::Node& node, size_t size, YamlScopeBase* parent = nullptr, const key_type& perentKey = key_type())
				: YamlScopeBase(node, parent, perentKey)
				, mSize(size)
				, mIndex(0)
			{
				assert(mNode.IsSequence());
			}

			inline void SetSize(size_t size) {
				if constexpr (TMode == SerializeMode::Save)
					mSize = size;
			}

			/// <summary>	
			/// Get node size (actual for sequence or map). 
			/// </summary>
			/// <returns> Non-zero value if node isn't scalar value, otherwise equal zero. </returns>
			[[nodiscard]]
			size_t GetSize() const override {
				return mSize;			
			}

			/// <summary>	
			/// Get current path in YAML. 
			/// </summary>
			[[nodiscard]]
			std::string GetPath() const override
			{
				const auto index = mIndex == 0 ? 0 : mIndex - 1;
				return YamlScopeBase::GetPath() + path_separator + Convert::ToString(index);
			}

			/// <summary>	
			/// Serialize single fundamental value. 
			/// </summary>
			/// <param name="value"> [in] The value of fundamental type. </param>
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			void SerializeValue(T& value)
			{
				if constexpr (TMode == SerializeMode::Load) {
					if (mIndex < GetSize())
						LoadValue(mNode[mIndex++], value);
				}
				else {
					assert(mIndex < GetSize());
					if constexpr (is_any_char_v<T>)
						mNode[mIndex++] = static_cast<short>(value);
					else
						mNode[mIndex++] = value;
				}
			}

			/// <summary>	
			/// Serialize string value. 
			/// </summary>
			/// <typeparam name="TSym">		 	Type of the symbol. </typeparam>
			/// <typeparam name="TAllocator">	Type of the allocator. </typeparam>
			/// <param name="value">	[in] The value of string type. </param>
			template <typename TSym, typename TAllocator>
			void SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
			{
				if constexpr (TMode == SerializeMode::Load) {
					if (mIndex < GetSize())
						LoadValue(mNode[mIndex++], value);
				}
				else
				{
					assert(mIndex < GetSize());
					if constexpr (std::is_same_v<TSym, std::string::value_type>)
						mNode[mIndex++] = value;
					else
						mNode[mIndex++] = Convert::To<std::string>(value);
				}
			}

			/// <summary>	
			/// Represent child node of current scope as object (map). 
			/// </summary>
			/// <returns> Child node wrapped in YamlObjectScope </returns>
			std::optional<YamlObjectScope<TMode>> OpenObjectScope()
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < GetSize())
					{
						const auto yamlValue = mNode[mIndex++];
						return yamlValue.IsMap() ? std::make_optional<YamlObjectScope<TMode>>(yamlValue, this) : std::nullopt;
					}
				}
				else
				{
					const auto yamlValue = mNode[mIndex++] = YAML::Node(YAML::NodeType::Map);
					return std::make_optional<YamlObjectScope<TMode>>(yamlValue, this);
				}
			}

			/// <summary>	
			/// Represent child node of current scope as array (sequence).
			/// </summary>
			/// <returns> Child node wrapped in YamlArrayScope </returns>
			std::optional<YamlArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < GetSize()) {
						const auto yamlValue = mNode[mIndex++];
						return yamlValue.IsSequence() ? std::make_optional<YamlArrayScope<TMode>>(yamlValue, yamlValue.size(), this) : std::nullopt;
					}
				}
				else
				{
					const auto yamlValue = mNode[mIndex++] = YAML::Node(YAML::NodeType::Sequence);
					return std::make_optional<YamlArrayScope<TMode>>(yamlValue, arraySize, this);
				}
			}

		private:
			size_t mSize;
			size_t mIndex;
		};

		/// <summary>
		/// Constant iterator of the keys.
		/// </summary>
		class key_const_iterator
		{
			template <SerializeMode TMode>
			friend class YamlObjectScope;

			YAML::const_iterator mYamlIt;

			key_const_iterator(YAML::const_iterator&& it)
				: mYamlIt(it) { }

		public:
			bool operator==(const key_const_iterator& rhs) const {
				return mYamlIt == rhs.mYamlIt;
			}
			bool operator!=(const key_const_iterator& rhs) const {
				return mYamlIt != rhs.mYamlIt;
			}

			key_const_iterator& operator++() {
				++mYamlIt;
				return *this;
			}

			YamlScopeBase::key_type operator*() const {
				return mYamlIt->first.as<YamlScopeBase::key_type>();
			}
		};

		/// <summary>
		/// YAML scope for serializing objects.
		/// </summary>
		/// <seealso cref="YamlScopeBase" />
		template <SerializeMode TMode>
		class YamlObjectScope final: public ArchiveScope<TMode>, public YamlScopeBase
		{
		public:
			/// <param name="node">	Node represented by current scope level. </param>
			/// <param name="size">	Size of the node represented by current scope. </param>
			/// <param name="parent">   	[in] (Optional) If non-null, the child node. </param>
			/// <param name="perentKey">	(Optional) Actual for child node only. </param>
			explicit YamlObjectScope(const YAML::Node& node, YamlScopeBase* parent = nullptr, const key_type& perentKey = key_type())
				: YamlScopeBase(node, parent, perentKey)
			{
				assert(mNode.IsMap());
			};

			/// <summary>	
			/// Get key_const_iterator for begining of object node. 
			/// </summary>
			[[nodiscard]]
			key_const_iterator cbegin() const {
				return key_const_iterator(mNode.begin());
			}

			/// <summary>	
			/// Get key_const_iterator for ending of object node. 
			/// </summary>
			[[nodiscard]]
			key_const_iterator cend() const {
				return key_const_iterator(mNode.end());
			}

			/// <summary>	
			/// Serialize single fundamental value. 
			/// </summary>
			/// <param name="key">  	The key of node. </param>
			/// <param name="value"> [in] The value of fundamental type. </param>
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool SerializeValue(const key_type& key, T& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode[key];
					return yamlValue.IsNull() ? false : LoadValue(yamlValue, value);
				}
				else
				{
					assert(!mNode[key]);
					//check T it's any char type (mean char/unsigned char or signed char)
					if constexpr (is_any_char_v<T>)
						//convert to short before saving, because yaml-cpp save char's as symbols
						mNode[key] = static_cast<short>(value);
					else
						mNode[key] = value;
					return true;
				}
			}

			/// <summary>	
			/// Serialize string value. 
			/// </summary>
			/// <typeparam name="TSym">		 	Type of the symbol. </typeparam>
			/// <typeparam name="TAllocator">	Type of the allocator. </typeparam>
			/// <param name="key">  	The key of node. </param>
			/// <param name="value">	[in] The value of string type. </param>
			template <typename TSym, typename TAllocator>
			bool SerializeValue(const key_type& key, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode[key];
					return yamlValue.IsNull() ? false : LoadValue(yamlValue, value);
				}
				else
				{
					assert(!mNode[key]);
					if constexpr (std::is_same_v<TSym, std::string::value_type>)
						mNode[key] = value;
					else
						mNode[key] = Convert::To<std::string>(value);
					return true;
				}
			}

			/// <summary>	
			/// Represent child node of current scope as object (map). 
			/// </summary>
			/// <returns> Child node wrapped in YamlObjectScope </returns>
			std::optional<Detail::YamlObjectScope<TMode>> OpenObjectScope(const key_type& key)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode[key];
					return yamlValue.IsMap() ? std::make_optional<YamlObjectScope<TMode>>(yamlValue, this, key) : std::nullopt;
				}
				else
				{
					const auto yamlValue = mNode[key] = YAML::Node(YAML::NodeType::Map);
					return std::make_optional<Detail::YamlObjectScope<TMode>>(yamlValue, this, key);
				}
			}

			/// <summary>	
			/// Represent child node of current scope as array (sequence). 
			/// </summary>
			/// <param name="key">			The key of node. </param>
			/// <param name="arraySize">	Size of the array. </param>
			/// <returns>	Child node wrapped in YamlArrayScope. </returns>
			std::optional<YamlArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode[key];
					return yamlValue.IsSequence() ? std::make_optional<YamlArrayScope<TMode>>(yamlValue, yamlValue.size(), this, key) : std::nullopt;
				}
				else
				{
					const auto yamlValue = mNode[key] = YAML::Node(YAML::NodeType::Sequence);
					return std::make_optional<YamlArrayScope<TMode>>(yamlValue, arraySize, this, key);
				}
			}
		};

		/// <summary>
		/// YAML root scope.
		/// </summary>
		/// <seealso cref="YamlScopeBase" />
		template <SerializeMode TMode>
		class YamlRootScope final: public ArchiveScope<TMode>, public YamlScopeBase
		{
		public:

			/// <summary>	Constructor. </summary>
			/// <param name="inputStr">	Input C-like string YAML. </param>
			explicit YamlRootScope(const char* inputStr) : YamlRootScope(std::string(inputStr)) {}

			/// <summary>	Constructor. </summary>
			/// <param name="inputStr">	Inputr YAML string. </param>
			explicit YamlRootScope(const std::string& inputStr)
				: YamlScopeBase(YAML::Node())
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");

				Init(inputStr);

			}

			/// <summary>	Constructor. </summary>
			/// <param name="outputStr">		   	[out] The output string. </param>
			/// <param name="serializationOptions">	(Optional) Options for controlling the serialization. </param>
			YamlRootScope(std::string& outputStr, const SerializationOptions& serializationOptions = {})
				: YamlScopeBase(YAML::Node())
				, mOutput(&outputStr)
				, mSerializationOptions(serializationOptions)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
			}

			/// <summary>	Constructor. </summary>
			/// <param name="inputStream">	[in] Stream to read YAML. </param>
			YamlRootScope(std::istream& inputStream)
				: YamlScopeBase(YAML::Node())
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");

				Init(inputStream);
			}

			/// <summary>	Constructor. </summary>
			/// <param name="outputStream">		   	[in,out] Stream to write YAML. </param>
			/// <param name="serializationOptions">	(Optional) Options for controlling the serialization. </param>
			YamlRootScope(std::ostream& outputStream, const SerializationOptions& serializationOptions = {})
				: YamlScopeBase(YAML::Node())
				, mOutput(&outputStream)
				, mSerializationOptions(serializationOptions)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
			}

			/// <summary>	
			/// Serialize single fundamental value. 
			/// </summary>
			/// <param name="value"> [in] The value of fundamental type. </param>
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			void SerializeValue(T& value)
			{
				if constexpr (TMode == SerializeMode::Load) {
					LoadValue(mNode, value);
				}
				else
				{
					assert(mNode.IsNull());
					//check T it's any char type (mean char/unsigned char or signed char)
					if constexpr (is_any_char_v<T>)
						//convert to short before saving, because yaml-cpp save char's as symbols
						mNode = static_cast<short>(value);
					else
						mNode = value;
				}
			}
				
			/// <summary>	
			/// Serialize string value. 
			/// </summary>
			/// <typeparam name="TSym">		 	Type of the symbol. </typeparam>
			/// <typeparam name="TAllocator">	Type of the allocator. </typeparam>
			/// <param name="value">	[in] The value of string type. </param>
			template <typename TSym, typename TAllocator>
			void SerializeValue(std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
			{
				if constexpr (TMode == SerializeMode::Load) {
					LoadValue(mNode, value);
				}
				else
				{
					assert(mNode.IsNull());
					if constexpr (std::is_same_v<TSym, std::string::value_type>)
						mNode = value;
					else
						mNode = Convert::To<std::string>(value);
				}
			}

			/// <summary>	
			/// Represent child node of current scope as object (map). 
			/// </summary>
			/// <returns> Child node wrapped in YamlObjectScope </returns>
			std::optional<YamlObjectScope<TMode>> OpenObjectScope()
			{
				if constexpr (TMode == SerializeMode::Load) {
					return mNode.IsMap() ? std::make_optional<YamlObjectScope<TMode>>(mNode) : std::nullopt;
				}
				else
				{
					assert(mNode.IsNull());
					mNode = YAML::Node(YAML::NodeType::Map);
					return std::make_optional<YamlObjectScope<TMode>>(mNode);
				}
			}

			/// <summary>	
			/// Represent child node of current scope as array (sequence). 
			/// </summary>
			/// <param name="arraySize">	Size of the array. </param>
			/// <returns>	Child node wrapped in YamlArrayScope. </returns>
			std::optional<YamlArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load) {
					return mNode.IsSequence() ? std::make_optional<YamlArrayScope<TMode>>(mNode, mNode.size()) : std::nullopt;
				}
				else
				{
					assert(mNode.IsNull());
					mNode = YAML::Node(YAML::NodeType::Sequence);
					return std::make_optional<YamlArrayScope<TMode>>(mNode, arraySize);
				}
			}

			/// <summary>	
			/// Serialize node tree to YAML 
			/// </summary>
			void Finalize()
			{
				if constexpr (TMode == SerializeMode::Save)
				{
					std::visit([this](auto&& arg) {
						using T = std::decay_t<decltype(arg)>;
						if constexpr (std::is_same_v<T, std::string*>) {
							*arg = YAML::Dump(mNode);
						}
						else if constexpr (std::is_same_v<T, std::ostream*>) {
							if (mSerializationOptions->streamOptions.writeBom) {
								*arg << Convert::Utf8::bom;
							}
							*arg << mNode;
						}					
					}, mOutput);
					mOutput = nullptr;
				}		
			}

			std::variant<std::nullptr_t, std::string*, std::ostream*> mOutput;
			std::optional<SerializationOptions> mSerializationOptions;

		private:
			template <typename T>
			void Init(T& input)
			{
				try
				{
					mNode = YAML::Load(input);
				}
				catch (YAML::ParserException& ex)
				{
					throw SerializationException(SerializationErrorCode::ParsingError, ex.msg);
				}
				catch (...)
				{
					throw SerializationException(SerializationErrorCode::ParsingError, "Unknown parsing error.");
				}
			}
		};
		
	}

	/// <summary>
	/// YAML archive based on yaml-cpp library.
	/// Supports load/save from:
	/// - <c>std::string</c>: UTF-8
	/// - <c>std::istream and std::ostream</c>: UTF-8
	/// </summary>
	using YamlArchive = MediaArchiveBase<
		Detail::YamlArchiveTraits,
		Detail::YamlRootScope<SerializeMode::Load>,
		Detail::YamlRootScope<SerializeMode::Save>>;
}