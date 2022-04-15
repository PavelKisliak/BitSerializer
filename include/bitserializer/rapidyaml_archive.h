/*******************************************************************************
* Copyright (C) 2021 by Artsiom Marozau                                        *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <type_traits>
#include <optional>
#include <variant>
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/archive_base.h"

// External dependency (Rapid YAML)
#include <c4/format.hpp>
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
			static constexpr ArchiveType archive_type = ArchiveType::Yaml;
			using key_type = std::string;
			using supported_key_types = TSupportedKeyTypes<const char*, key_type>;
			using preferred_output_format = std::string;
			using preferred_stream_char_type = std::ostream::char_type;
			static constexpr char path_separator = '/';

			static constexpr char nullValue[] = "null";
			static constexpr char nullValueAlt[] = "~";

		protected:
			~RapidYamlArchiveTraits() = default;
		};

		// Forward declarations
		template <SerializeMode TMode>
		class RapidYamlObjectScope;

		/// <summary>
		/// Common base class for YAML scopes.
		/// </summary>
		/// <seealso cref="RapidYamlArchiveTraits" />
		class RapidYamlScopeBase : public RapidYamlArchiveTraits
		{
		public:
			using RapidYamlNode = ryml::NodeRef;
			using key_type_view = std::basic_string_view<key_type::value_type>;

			explicit RapidYamlScopeBase(const RapidYamlNode& node, RapidYamlScopeBase* parent = nullptr, key_type_view parentKey = {}) noexcept
				: mNode(node)
				, mParent(parent)
				, mParentKey(parentKey)
			{ }

			RapidYamlScopeBase(const RapidYamlScopeBase&) = delete;
			RapidYamlScopeBase& operator=(const RapidYamlScopeBase&) = delete;

			/// <summary>
			/// Get current path in YAML.
			/// </summary>
			[[nodiscard]]
			virtual std::string GetPath() const
			{
				const auto localPath = mParentKey.empty()
					? Convert::ToString(mParentKey)
					: path_separator + Convert::ToString(mParentKey);
				return mParent == nullptr ? localPath : mParent->GetPath() + localPath;
			}

		protected:
			~RapidYamlScopeBase() = default;
			RapidYamlScopeBase(RapidYamlScopeBase&&) = default;
			RapidYamlScopeBase& operator=(RapidYamlScopeBase&&) = default;

			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool LoadValue(const RapidYamlNode& yamlValue, T& value)
			{
				if (!yamlValue.is_val() && !yamlValue.is_keyval())
					return false;

				const bool isNullValue = IsNullYamlValue(yamlValue.val());
				if constexpr (std::is_null_pointer_v<T>) {
					return isNullValue;
				}
				else {
					if (isNullValue)
						return false;
					try
					{
						if constexpr (std::is_same_v<T, char>)
							yamlValue >> reinterpret_cast<uint8_t&>(value);
						else
							yamlValue >> value;
					}
					catch (...) {
						return false;
					}
				}
				return true;
			}

			template <typename TSym, typename TAllocator>
			static bool LoadValue(const RapidYamlNode& yamlValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
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

			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			static void SaveValue(RapidYamlNode& yamlValue, T& value)
			{
				if constexpr (std::is_null_pointer_v<T>) {
					yamlValue << nullValue;
				} else if constexpr (std::is_floating_point_v<T>)
					yamlValue << c4::fmt::real(value, std::numeric_limits<T>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
				else if constexpr (std::is_same_v<T, char>)
					yamlValue << static_cast<uint8_t>(value);
				else
					yamlValue << value;
			}

			template <typename TSym, typename TAllocator>
			static void SaveValue(RapidYamlNode& yamlValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
			{
				if constexpr (std::is_same_v<TSym, std::string::value_type>)
					yamlValue << value;
				else
					yamlValue << Convert::To<std::string>(value);
			}

			static bool IsNullYamlValue(c4::csubstr str)
			{
				return std::equal(str.begin(), str.end(), std::cbegin(nullValueAlt), std::cend(nullValueAlt)) ||
					std::equal(str.begin(), str.end(), std::cbegin(nullValue), std::cend(nullValue),
						[](const char lhs, const char rhs) {
							return std::tolower(static_cast<int>(lhs)) == std::tolower(static_cast<int>(rhs));
				});
			}

			RapidYamlNode mNode;
			RapidYamlScopeBase* mParent;
			key_type_view mParentKey;
		};

		/// <summary>
		/// YAML scope for serializing arrays.
		/// </summary>
		///	<seealso cref="RapidYamlScopeBase" />
		template <SerializeMode TMode>
		class RapidYamlArrayScope final : public TArchiveScope<TMode>, public RapidYamlScopeBase
		{
		public:
			RapidYamlArrayScope(const RapidYamlNode& node, size_t size, RapidYamlScopeBase* parent = nullptr, key_type_view parentKey = {})
				: RapidYamlScopeBase(node, parent, parentKey)
				, mSize(size)
				, mIndex(0)
			{
				assert(mNode.is_seq());
			}

			/// <summary>
			/// Returns the estimated number of items to load (for reserving the size of containers).
			/// </summary>
			[[nodiscard]]
			size_t GetEstimatedSize() const {
				return mSize;
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
			/// Get current path in YAML.
			/// </summary>
			[[nodiscard]]
			std::string GetPath() const override
			{
				const auto index = mIndex == 0 ? 0 : mIndex - 1;
				return RapidYamlScopeBase::GetPath() + path_separator + Convert::ToString(index);
			}

			/// <summary>
			/// Serialize value.
			/// </summary>
			/// <param name="value">The value.</param>
			template <typename T>
			bool SerializeValue(T& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < GetEstimatedSize()) {
						return LoadValue(LoadNextItem(), value);
					}
				}
				else
				{
					assert(mIndex < GetEstimatedSize());
					auto yamlValue = mNode.append_child();
					SaveValue(yamlValue, value);
					mIndex++;
					return true;
				}
				return false;
			}

			/// <summary>
			/// Returns element of array as sub-object.
			/// </summary>
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope()
			{				
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < mSize)
					{
						auto yamlValue = LoadNextItem();
						return yamlValue.is_map() ? std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, this) : std::nullopt;
					}
					return std::nullopt;
				}
				else
				{
					assert(mIndex < GetEstimatedSize());
					auto yamlValue = mNode.append_child();
					yamlValue |= ryml::MAP;
					mIndex++;
					return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, this);
				}
			}

			/// <summary>
			/// Returns element of array as sub-array.
			/// </summary>
			/// <param name="arraySize">The size of array (required only for save mode).</param>
			std::optional<RapidYamlArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < mSize)
					{
						auto yamlValue = LoadNextItem();
						return yamlValue.is_seq() ? std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, yamlValue.num_children(), this) : std::nullopt;
					}
					return std::nullopt;
				}
				else
				{
					assert(mIndex < GetEstimatedSize());
					auto yamlValue = mNode.append_child();
					yamlValue |= ryml::SEQ;
					mIndex++;
					return std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, arraySize, this);
				}
			}

		private:
			auto LoadNextItem()
			{
				static_assert(TMode == SerializeMode::Load);
				if (mIndex < mSize)
				{
					return mNode[mIndex++];
				}
				throw SerializationException(SerializationErrorCode::OutOfRange, "No more items to load");
			}

			size_t mSize;
			size_t mIndex;
		};

		/// <summary>
		/// Constant iterator for keys.
		/// </summary>
		class key_const_iterator
		{
			template <SerializeMode TMode>
			friend class RapidYamlObjectScope;

			ryml::NodeRef::const_iterator mYamlIt;

			explicit key_const_iterator(ryml::NodeRef::const_iterator&& it)
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
		/// <seealso cref="RapidYamlScopeBase" />
		template <SerializeMode TMode>
		class RapidYamlObjectScope final : public TArchiveScope<TMode>, public RapidYamlScopeBase
		{
		public:
			explicit RapidYamlObjectScope(const RapidYamlNode& node, RapidYamlScopeBase* parent = nullptr, key_type_view parentKey = {})
				: RapidYamlScopeBase(node, parent, parentKey)
			{
				assert(mNode.is_map());
			}

			/// <summary>
			/// Get the begin constant iterator of node.
			/// </summary>
			[[nodiscard]]
			key_const_iterator cbegin() const {
				return key_const_iterator(mNode.begin());
			}

			/// <summary>
			/// Get the end constant iterator of node.
			/// </summary>
			[[nodiscard]]
			key_const_iterator cend() const {
				return key_const_iterator(mNode.end());
			}

			/// <summary>
			/// Serialize value.
			/// </summary>
			/// <param name="key">The key of child node.</param>
			/// <param name="value">The value.</param>
			template <typename TKey, typename T>
			bool SerializeValue(TKey&& key, T& value)
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
					SaveValue(yamlValue, value);
					return true;
				}
			}

			/// <summary>
			/// Returns child node as sub-object.
			/// </summary>
			/// <param name="key">The key of child node.</param>
			template <typename TKey>
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope(TKey&& key)
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
			/// Returns child node as sub-array.
			/// </summary>
			/// <param name="key">The key of child node.</param>
			/// <param name="arraySize">The size of array (required only for save mode).</param>
			template <typename TKey>
			std::optional<RapidYamlArrayScope<TMode>> OpenArrayScope(TKey&& key, size_t arraySize)
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

		/// <summary>
		/// YAML root scope.
		/// </summary>
		/// <seealso cref="RapidYamlScopeBase" />
		template <SerializeMode TMode>
		class RapidYamlRootScope final: public TArchiveScope<TMode>, public RapidYamlScopeBase
		{
		public:
			RapidYamlRootScope(const RapidYamlRootScope&) = delete;
			RapidYamlRootScope& operator=(const RapidYamlRootScope&) = delete;

			explicit RapidYamlRootScope(const char* inputStr)
				: RapidYamlScopeBase(mRootNode)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");

				Init();
				mTree = ryml::parse(c4::to_csubstr(inputStr));
			}

			explicit RapidYamlRootScope(const std::string& inputStr)
				: RapidYamlScopeBase(mRootNode)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");

				Init();
				mTree = ryml::parse(c4::to_csubstr(inputStr));
			}

			explicit RapidYamlRootScope(std::string& outputStr, const SerializationOptions& serializationOptions = {})
				: RapidYamlScopeBase(mRootNode)
				, mOutput(&outputStr)
				, mSerializationOptions(serializationOptions)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");

				Init();
			}

			explicit RapidYamlRootScope(std::istream& inputStream)
				: RapidYamlScopeBase(mRootNode)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");

				const auto utfType = Convert::DetectEncoding(inputStream);
				if (utfType != Convert::UtfType::Utf8) {
					throw SerializationException(SerializationErrorCode::UnsupportedEncoding, "The archive does not support encoding: " + Convert::ToString(utfType));
				}

				Init();
				// ToDo: base library does not support std::stream (check in new versions)
				const std::string input(std::istreambuf_iterator<char>(inputStream), {});
				mTree = ryml::parse(c4::to_csubstr(input));
			}

			explicit RapidYamlRootScope(std::ostream& outputStream, const SerializationOptions& serializationOptions = {})
				: RapidYamlScopeBase(mRootNode)
				, mOutput(&outputStream)
				, mSerializationOptions(serializationOptions)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");

				Init();
			}

			/// <summary>
			/// Returns root node as object type in YAML.
			/// </summary>
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
			/// Returns root node as array type in YAML.
			/// </summary>
			/// <param name="arraySize">The size of array (required only for save mode).</param>
			std::optional<RapidYamlArrayScope<TMode>> OpenArrayScope(size_t arraySize)
			{
				if constexpr (TMode == SerializeMode::Load) {
					return mRootNode.is_seq()
						? std::make_optional<RapidYamlArrayScope<TMode>>(mRootNode, mRootNode.num_children())
						: std::nullopt;
				}
				else
				{
					mRootNode |= ryml::SEQ;
					return std::make_optional<RapidYamlArrayScope<TMode>>(mRootNode, arraySize);
				}
			}

			/// <summary>
			/// Serialize node tree to YAML.
			/// </summary>
			void Finalize()
			{
				if constexpr (TMode == SerializeMode::Save)
				{
					std::visit([this](auto&& arg)
					{
						using T = std::decay_t<decltype(arg)>;
						if constexpr (std::is_same_v<T, std::string*>) {
							*arg = ryml::emitrs<std::string>(mTree);
						}
						else if constexpr (std::is_same_v<T, std::ostream*>)
						{
							if (mSerializationOptions->streamOptions.writeBom) {
								arg->write(Convert::Utf8::bom, sizeof Convert::Utf8::bom);
							}
							*arg << mTree;
						}
					}, mOutput);
					mOutput = nullptr;
				}
			}

			~RapidYamlRootScope()
			{
				ryml::set_callbacks(mPrevCallbacks);
			}

		private:
			RapidYamlRootScope(RapidYamlRootScope&&) = default;
			RapidYamlRootScope& operator=(RapidYamlRootScope&&) = default;

			void Init()
			{
				mRootNode = mTree.rootref();
				// ToDo: isn't thread safe (check in new versions of RapidYAML)
				mPrevCallbacks = c4::yml::get_callbacks();
				c4::set_error_flags(c4::ON_ERROR_CALLBACK);

				// Keep compatibility with old RYML (pre-release versions)
				using old_error_callback = void (*)(const char* msg, size_t msg_len, void* user_data);
				if constexpr (std::is_constructible_v<ryml::Callbacks, void*, ryml::pfn_allocate, ryml::pfn_free, old_error_callback>)
				{
					const ryml::Callbacks cb(this, nullptr, nullptr, &RapidYamlRootScope::OldErrorCallback);
					ryml::set_callbacks(cb);
				}
				else
				{
					const ryml::Callbacks cb(this, nullptr, nullptr, &RapidYamlRootScope::ErrorCallback);
					ryml::set_callbacks(cb);
				}
			}

			static void OldErrorCallback(const char* msg, size_t length, [[maybe_unused]]void* user_data)
			{
				throw SerializationException(SerializationErrorCode::ParsingError, { msg, msg+length });
			}

			static void ErrorCallback(const char* msg, size_t length, ryml::Location location, [[maybe_unused]] void* user_data)
			{
				throw SerializationException(SerializationErrorCode::ParsingError, { msg, msg + length });
			}

			ryml::Tree mTree;
			RapidYamlNode mRootNode;
			std::variant<std::nullptr_t, std::string*, std::ostream*> mOutput;
			std::optional<SerializationOptions> mSerializationOptions;
			ryml::Callbacks mPrevCallbacks;
		};
	}

	/// <summary>
	/// YAML archive based on Rapid YAML library.
	/// Supports load/save from:
	/// - <c>std::string</c>: UTF-8
	/// - <c>std::istream</c> and <c>std::ostream</c>: UTF-8
	/// </summary>
	using YamlArchive = TArchiveBase<
		Detail::RapidYamlArchiveTraits,
		Detail::RapidYamlRootScope<SerializeMode::Load>,
		Detail::RapidYamlRootScope<SerializeMode::Save>>;
}
