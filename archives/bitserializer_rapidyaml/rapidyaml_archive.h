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
#include "bitserializer/serialization_detail/archive_base.h"

#include <ryml/ryml_std.hpp>
#include <ryml/ryml.hpp>

namespace BitSerializer::Yaml::RapidYaml {
	namespace Detail {

		/// <summary>	
		/// YAML archive traits class.
		/// </summary>
		class RapidYamlArchiveTraits
		{
		public:
			using key_type = std::string;
			using supported_key_types = TSupportedKeyTypes<std::string>;
			using preferred_output_format = std::string;
			using preferred_stream_char_type = std::ostream::char_type;
			static constexpr char path_separator = '/';
		};

		template <SerializeMode TMode>
		class RapidYamlObjectScope;

		/// <summary>
		/// Common base class for YAML scopes.
		/// </summary>
		/// <seealso cref="YamlArchiveTraits" />
		class RapidYamlScopeBase : public RapidYamlArchiveTraits
		{
		public:

			/// <summary>	Constructor. </summary>
			/// <param name="node">	Node represented by current scope level. </param>
			/// <param name="parent">   	[in] (Optional) If non-null, the child node. </param>
			/// <param name="perentKey">	(Optional) Actual for child node only. </param>
			explicit RapidYamlScopeBase(const ryml::NodeRef& node, RapidYamlScopeBase* parent = nullptr, const key_type& perentKey = key_type()) noexcept
				: mNode(node)
				, mParent(parent)
				, mParentKey(perentKey)
			{ }

			virtual ~RapidYamlScopeBase() {};

			/// <summary>	
			/// Get node size (actual for sequence or map). 
			/// </summary>
			/// <returns> Non-zero value if node isn't scalar value, otherwise equal zero. </returns>
			[[nodiscard]]
			virtual size_t GetSize() const { return mNode.num_children(); }

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
			bool LoadValue(const ryml::NodeRef& yamlValue, T& value)
			{
				if (!yamlValue.is_val() && !yamlValue.is_keyval())
					return false;
				yamlValue >> value;
				return true;
			}

			template <typename TSym, typename TAllocator>
			bool LoadValue(const ryml::NodeRef& yamlValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
			{
				if (!yamlValue.is_val() && !yamlValue.is_keyval())
					return false;
				if constexpr (std::is_same_v<TSym, std::string::value_type>)				
					yamlValue >> value;
				else
				{
					std::string tmp;
					yamlValue >> tmp;
					value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(std::move(tmp));
				}
				return true;
			}

			ryml::NodeRef mNode;
			RapidYamlScopeBase* mParent;
			key_type mParentKey;
		};

		/// <summary>
		/// YAML scope for serializing arrays.
		/// </summary>
		///	<seealso cref="YamlScopeBase" />
		template <SerializeMode TMode>
		class RapidYamlArrayScope final : public TArchiveScope<TMode>, public RapidYamlScopeBase
		{
		public:

			using iterator = ryml::NodeRef::iterator;

			/// <param name="node">	Node represented by current scope level. </param>
			/// <param name="size">	Size of the node represented by current scope. </param>
			/// <param name="parent">   	[in] (Optional) If non-null, the child node. </param>
			/// <param name="perentKey">	(Optional) Actual for child node only. </param>
			explicit RapidYamlArrayScope(const ryml::NodeRef& node, size_t size, RapidYamlScopeBase* parent = nullptr, const key_type& perentKey = key_type())
				: RapidYamlScopeBase(node, parent, perentKey)
				, mSize(size)
				, mIndex(0)
			{
				assert(mNode.is_seq());
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
				return RapidYamlScopeBase::GetPath() + path_separator + Convert::ToString(index);
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
					mNode.append_child() << value;
					mIndex++;
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
				else {
					assert(mIndex < GetSize());
					if constexpr (std::is_same_v<TSym, std::string::value_type>)
						mNode.append_child() << value;
					else
						mNode.append_child() << Convert::To<std::string>(value);
					mIndex++;
				}
			}

			/// <summary>	
			/// Represent child node of current scope as object (map). 
			/// </summary>
			/// <returns> Child node wrapped in YamlObjectScope </returns>
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope()
			{				
				if constexpr (TMode == SerializeMode::Load) {
					if (mIndex < mSize)
					{
						auto yamlValue = mNode[mIndex++];
						return yamlValue.is_map() ? std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, this) : std::nullopt;
					}			
				}
				else
				{
					assert(mIndex < GetSize());
					auto yamlValue = mNode.append_child();
					yamlValue |= ryml::MAP;
					mIndex++;
					return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, this);
				}
			}

			/// <summary>	
			/// Represent child node of current scope as array (sequence).
			/// </summary>
			/// <returns> Child node wrapped in YamlArrayScope </returns>
			std::optional<RapidYamlArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load) {
					if (mIndex < mSize)
					{
						auto yamlValue = mNode[mIndex++];
						return yamlValue.is_seq() ? std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, yamlValue.num_children(), this) : std::nullopt;
					}
				}	
				else
				{
					assert(mIndex < GetSize());
					auto yamlValue = mNode.append_child();
					yamlValue |= ryml::SEQ;
					mIndex++;
					return std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, arraySize, this);
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
			friend class RapidYamlObjectScope;

			ryml::NodeRef::const_iterator mYamlIt;

			key_const_iterator(ryml::NodeRef::const_iterator&& it)
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

			RapidYamlScopeBase::key_type operator*() const {
				std::string key;
				c4::from_chars((*mYamlIt).key(), &key);
				return key;
			}
		};

		/// <summary>
		/// YAML scope for serializing objects.
		/// </summary>
		/// <seealso cref="YamlScopeBase" />
		template <SerializeMode TMode>
		class RapidYamlObjectScope final : public TArchiveScope<TMode>, public RapidYamlScopeBase
		{
		public:
			/// <param name="node">	Node represented by current scope level. </param>
			/// <param name="size">	Size of the node represented by current scope. </param>
			/// <param name="parent">   	[in] (Optional) If non-null, the child node. </param>
			/// <param name="perentKey">	(Optional) Actual for child node only. </param>
			explicit RapidYamlObjectScope(const ryml::NodeRef& node, RapidYamlScopeBase* parent = nullptr, const key_type& perentKey = key_type())
				: RapidYamlScopeBase(node, parent, perentKey)
			{
				assert(mNode.is_map());
			};

			/// <summary>	
			/// Get key_const_iterator for beginning of object node. 
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
					const auto yamlValue = mNode.find_child(c4::to_csubstr(key));
					return yamlValue.valid() ? LoadValue(yamlValue, value) : false;
				}
				else
				{
					assert(!mNode.find_child(c4::to_csubstr(key)).valid());
					auto yamlValue = mNode.append_child();
					yamlValue << ryml::key(key) << value;
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
					const auto yamlValue = mNode.find_child(c4::to_csubstr(key));
					return yamlValue.valid() ? LoadValue(yamlValue, value) : false;
				}
				else
				{
					assert(!mNode.find_child(c4::to_csubstr(key)).valid());
					auto yamlValue = mNode.append_child();
					yamlValue << ryml::key(key);
					if constexpr (std::is_same_v<TSym, std::string::value_type>)			
						yamlValue << value;
					else
						yamlValue << Convert::To<std::string>(value);
					return true;
				}
			}

			/// <summary>	
			/// Represent child node of current scope as object (map). 
			/// </summary>
			/// <returns> Child node wrapped in YamlObjectScope </returns>
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope(const key_type& key)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode.find_child(c4::to_csubstr(key));
					if (yamlValue.valid())
						return yamlValue.is_map() ? std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, this, key) : std::nullopt;
					return std::nullopt;
				}
				else
				{
					assert(!mNode.find_child(c4::to_csubstr(key)).valid());
					auto yamlValue = mNode.append_child();
					yamlValue << c4::yml::key(key);
					yamlValue |= ryml::MAP;
					return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, this, key);
				}
			}

			/// <summary>	
			/// Represent child node of current scope as array (sequence). 
			/// </summary>
			/// <param name="key">			The key of node. </param>
			/// <param name="arraySize">	Size of the array. </param>
			/// <returns>	Child node wrapped in YamlArrayScope. </returns>
			std::optional<RapidYamlArrayScope<TMode>> OpenArrayScope(const key_type& key, size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode.find_child(c4::to_csubstr(key));
					if (yamlValue.valid())
						return yamlValue.is_seq() ? std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, yamlValue.num_children(), this, key) : std::nullopt;
					return std::nullopt;
				}
				else
				{
					assert(!mNode.find_child(c4::to_csubstr(key)).valid());
					auto yamlValue = mNode.append_child();
					yamlValue << c4::yml::key(key);
					yamlValue |= ryml::SEQ;
					return std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, arraySize, this, key);
				}
			}
		};

		//TODO: check below
		//Please note that since a ryml tree uses linear storage, the complexity of operator[]
		//is linear on the number of children of the node on which it is invoked.
		//If you use it with a large tree with many siblings at the root level, you may get a performance hit.
		//To avoid this, you can create your own accelerator structure (eg, do a single loop at the root level
		//to fill an std::map<csubstr,size_t> mapping key names to node indices; with a node index, a lookup is O(1),
		//so this way you can get O(log n) lookup from a key.)
		
		/// <summary>
		/// YAML root scope.
		/// </summary>
		/// <seealso cref="YamlScopeBase" />
		template <SerializeMode TMode>
		class RapidYamlRootScope final: public TArchiveScope<TMode>, public RapidYamlScopeBase
		{
		public:

			/// <summary>	Constructor. </summary>
			/// <param name="inputStr">	Input C-like string YAML. </param>
			explicit RapidYamlRootScope(const char* inputStr) : RapidYamlRootScope(std::string(inputStr)) {}

			/// <summary>	Constructor. </summary>
			/// <param name="inputStr">	Input YAML string. </param>
			explicit RapidYamlRootScope(const std::string& inputStr)
				: RapidYamlScopeBase(mRootNode)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");

				mTree = ryml::parse(c4::to_csubstr(inputStr));
				mRootNode = mTree.rootref();
			}

			/// <summary>	Constructor. </summary>
			/// <param name="outputStr">		   	[out] The output string. </param>
			/// <param name="serializationOptions">	(Optional) Options for controlling the serialization. </param>
			RapidYamlRootScope(std::string& outputStr, const SerializationOptions& serializationOptions = {})
				: RapidYamlScopeBase(mRootNode)
				, mOutput(&outputStr)
				, mSerializationOptions(serializationOptions)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
				mRootNode = mTree.rootref();
			}

			/// <summary>	Constructor. </summary>
			/// <param name="inputStream">	[in] Stream to read YAML. </param>
			RapidYamlRootScope(std::istream& inputStream)
				: RapidYamlScopeBase(mRootNode)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");

				const auto utfType = Convert::DetectEncoding(inputStream);
				if (utfType != Convert::UtfType::Utf8) {
					throw SerializationException(SerializationErrorCode::UnsupportedEncoding, "The archive does not support encoding: " + Convert::ToString(utfType));
				}

				//TODO: potential performance cookie monster (construct string from istream)
				std::istreambuf_iterator<char> begin(inputStream);
				std::istreambuf_iterator<char> end;
				const std::string input(begin, end);
				//be careful when use c4::to_substr(std::string const& s). Function parse(substr buf) doesn't call copy_to_arena,
				//so when input string out from scope (csubstr/substr just string view) you lost all data and parse will be failed.
				mTree = ryml::parse(c4::to_csubstr(input));
				mRootNode = mTree.rootref();
			}

			/// <summary>	Constructor. </summary>
			/// <param name="outputStream">		   	[in,out] Stream to write YAML. </param>
			/// <param name="serializationOptions">	(Optional) Options for controlling the serialization. </param>
			RapidYamlRootScope(std::ostream& outputStream, const SerializationOptions& serializationOptions = {})
				: RapidYamlScopeBase(mRootNode)
				, mOutput(&outputStream)
				, mSerializationOptions(serializationOptions)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
				mRootNode = mTree.rootref();
			}

			/// <summary>	
			/// Serialize single fundamental value. 
			/// </summary>
			/// <param name="value"> [in] The value of fundamental type. </param>
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			void SerializeValue(T& value)
			{
				if constexpr (TMode == SerializeMode::Load) {
					LoadValue(mRootNode.first_child(), value);
				}
				else
				{
					mRootNode |= ryml::SEQ;
					mRootNode.append_child() << value;
				}
			}

			//TODO: remove all SerializeValue methods from RootScope (YAML support only MAP and SEQ on the root of tree)
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
					LoadValue(mRootNode.first_child(), value);
				}
				else
				{
					mRootNode |= ryml::SEQ;
					if constexpr (std::is_same_v<TSym, std::string::value_type>)
						mRootNode.append_child() << value;
					else
						mRootNode.append_child() << Convert::To<std::string>(value);
				}
			}

			/// <summary>	
			/// Represent child node of current scope as object (map). 
			/// </summary>
			/// <returns> Child node wrapped in YamlObjectScope </returns>
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope()
			{
				if constexpr (TMode == SerializeMode::Load) {
					return mRootNode.is_map() ? std::make_optional<RapidYamlObjectScope<TMode>>(mRootNode) : std::nullopt;
				}
				else
				{
					mRootNode |= ryml::MAP;
					return std::make_optional<RapidYamlObjectScope<TMode>>(mRootNode);
				}
			}

			/// <summary>	
			/// Represent child node of current scope as array (sequence). 
			/// </summary>
			/// <param name="arraySize">	Size of the array. </param>
			/// <returns>	Child node wrapped in YamlArrayScope. </returns>

			std::optional<RapidYamlArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load) {
					return mRootNode.is_seq() ? std::make_optional<RapidYamlArrayScope<TMode>>(mRootNode, mRootNode.num_children()) : std::nullopt;
				}
				else
				{
					mRootNode |= ryml::SEQ;
					return std::make_optional<RapidYamlArrayScope<TMode>>(mRootNode, arraySize);
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
							*arg = ryml::emitrs<std::string>(mTree);
						}
						else if constexpr (std::is_same_v<T, std::ostream*>) {
							if (mSerializationOptions->streamOptions.writeBom) {
								*arg << Convert::Utf8::bom;
							};
							*arg << ryml::emitrs<std::string>(mTree);
						}					
					}, mOutput);
					mOutput = nullptr;
				}		
			}

			std::variant<std::nullptr_t, std::string*, std::ostream*> mOutput;
			std::optional<SerializationOptions> mSerializationOptions;

		private:
			ryml::Tree mTree;
			ryml::NodeRef mRootNode;
		};
		
	}



	/// <summary>
	/// YAML archive based on rapid yaml library.
	/// Supports load/save from:
	/// - <c>std::string</c>: UTF-8
	/// - <c>std::istream and std::ostream</c>: UTF-8
	/// </summary>
	using YamlArchive = TArchiveBase<
		Detail::RapidYamlArchiveTraits,
		Detail::RapidYamlRootScope<SerializeMode::Load>,
		Detail::RapidYamlRootScope<SerializeMode::Save>>;
}