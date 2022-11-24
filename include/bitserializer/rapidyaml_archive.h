/*******************************************************************************
* Copyright (C) 2020-2022 by Artsiom Marozau, Pavel Kisliak                    *
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
			bool LoadValue(const RapidYamlNode& yamlValue, T& value, const SerializationOptions& serializationOptions)
			{
				if (!yamlValue.is_val() && !yamlValue.is_keyval()) {
					return false;
				}

				if (IsNullYamlValue(yamlValue.val())) {
					return std::is_null_pointer_v<T>;
				}

				const auto str = std::string_view(yamlValue.val().data(), yamlValue.val().size());
				try
				{
					if constexpr (!std::is_null_pointer_v<T>)
					{
						value = Convert::To<T>(str);
						return true;
					}
					throw std::exception();
				}
				catch (const std::out_of_range&)
				{
					if (serializationOptions.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
					{
						throw SerializationException(SerializationErrorCode::Overflow,
							std::string("The size of target field is not sufficient to deserialize number: ").append(str));
					}
				}
				catch (...)
				{
					if (serializationOptions.mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
					{
						throw SerializationException(SerializationErrorCode::MismatchedTypes,
							std::string("The type of target field does not match the value being loaded: ").append(str));
					}
				}
				return false;
			}

			template <typename TSym, typename TAllocator>
			static bool LoadValue(const RapidYamlNode& yamlValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value, const SerializationOptions& serializationOptions)
			{
				if (!yamlValue.is_val() && !yamlValue.is_keyval())
					return false;

				if (IsNullYamlValue(yamlValue.val())) {
					return false;
				}

				const auto str = yamlValue.val();
				if constexpr (std::is_same_v<TSym, std::string::value_type>)
					value.assign(str.data(), str.size());
				else {
					value = Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(std::string_view(str.data(), str.size()));
				}
				return true;
			}

			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			static void SaveValue(RapidYamlNode& yamlValue, T& value)
			{
				if constexpr (std::is_null_pointer_v<T>) {
					yamlValue << nullValue;
				} else if constexpr (std::is_floating_point_v<T>) {
					yamlValue << c4::fmt::real(value, std::numeric_limits<T>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
				} else if constexpr (std::is_same_v<T, char>) {
					// Need to extend size of type for prevent save as character
					yamlValue << static_cast<int16_t>(value);
				} else {
					yamlValue << value;
				}
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
				return str.data() == nullptr ||
					std::equal(str.begin(), str.end(), std::cbegin(nullValueAlt), std::cend(nullValueAlt) - 1) ||
					std::equal(str.begin(), str.end(), std::cbegin(nullValue), std::cend(nullValue) - 1,
						[](int lhs, int rhs) {
							return std::tolower(lhs) == rhs;
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
			RapidYamlArrayScope(const RapidYamlNode& node, SerializationContext& serializationContext, size_t size, RapidYamlScopeBase* parent = nullptr, key_type_view parentKey = {})
				: TArchiveScope<TMode>(serializationContext)
				, RapidYamlScopeBase(node, parent, parentKey)
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
				return RapidYamlScopeBase::GetPath() + path_separator + Convert::ToString(mIndex);
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
						return LoadValue(LoadNextItem(), value, this->GetOptions());
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
						return yamlValue.is_map() ? std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), this) : std::nullopt;
					}
					return std::nullopt;
				}
				else
				{
					assert(mIndex < GetEstimatedSize());
					auto yamlValue = mNode.append_child();
					yamlValue |= ryml::MAP;
					mIndex++;
					return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), this);
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
						return yamlValue.is_seq() ? std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), yamlValue.num_children(), this) : std::nullopt;
					}
					return std::nullopt;
				}
				else
				{
					assert(mIndex < GetEstimatedSize());
					auto yamlValue = mNode.append_child();
					yamlValue |= ryml::SEQ;
					mIndex++;
					return std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), arraySize, this);
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
			RapidYamlObjectScope(const RapidYamlNode& node, SerializationContext& serializationContext, RapidYamlScopeBase* parent = nullptr, key_type_view parentKey = {})
				: TArchiveScope<TMode>(serializationContext)
				, RapidYamlScopeBase(node, parent, parentKey)
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
					return yamlValue.valid() ? LoadValue(yamlValue, value, this->GetOptions()) : false;
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
						return yamlValue.is_map() ? std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), this, key) : std::nullopt;
					return std::nullopt;
				}
				else
				{
					assert(!mNode.find_child(c4::to_csubstr(key)).valid());
					auto yamlValue = mNode.append_child();
					yamlValue << c4::yml::key(key);
					yamlValue |= ryml::MAP;
					return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), this, key);
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
						return yamlValue.is_seq() ? std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), yamlValue.num_children(), this, key) : std::nullopt;
					return std::nullopt;
				}
				else
				{
					assert(!mNode.find_child(c4::to_csubstr(key)).valid());
					auto yamlValue = mNode.append_child();
					yamlValue << c4::yml::key(key);
					yamlValue |= ryml::SEQ;
					return std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), arraySize, this, key);
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
			template <typename T>
			struct ryml_has_parse_in_arena
			{
			private:
				template <typename U>
				static decltype(std::declval<U>().parse_in_arena(std::declval<c4::csubstr>(), std::declval<c4::csubstr>()), void(), std::true_type()) test(int);

				template <typename>
				static std::false_type test(...);

			public:
				typedef decltype(test<T>(0)) type;
				enum { value = type::value };
			};

		public:
			RapidYamlRootScope(const RapidYamlRootScope&) = delete;
			RapidYamlRootScope& operator=(const RapidYamlRootScope&) = delete;

			RapidYamlRootScope(const char* inputStr, SerializationContext& serializationContext)
				: TArchiveScope<TMode>(serializationContext)
				, RapidYamlScopeBase(mRootNode)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
				Parse<c4::yml::Parser>(inputStr);
			}

			RapidYamlRootScope(const std::string& inputStr, SerializationContext& serializationContext)
				: TArchiveScope<TMode>(serializationContext)
				, RapidYamlScopeBase(mRootNode)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
				Parse<c4::yml::Parser>(inputStr);
			}

			RapidYamlRootScope(std::string& outputStr, SerializationContext& serializationContext)
				: TArchiveScope<TMode>(serializationContext)
				, RapidYamlScopeBase(mRootNode)
				, mOutput(&outputStr)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
			}

			RapidYamlRootScope(std::istream& inputStream, SerializationContext& serializationContext)
				: TArchiveScope<TMode>(serializationContext)
				, RapidYamlScopeBase(mRootNode)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");

				const auto utfType = Convert::DetectEncoding(inputStream);
				if (utfType != Convert::UtfType::Utf8) {
					throw SerializationException(SerializationErrorCode::UnsupportedEncoding, "The archive does not support encoding: " + Convert::ToString(utfType));
				}

				// ToDo: base library does not support std::stream (check in new versions)
				const std::string inputStr(std::istreambuf_iterator<char>(inputStream), {});
				Parse<c4::yml::Parser>(inputStr);
			}

			RapidYamlRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
				: TArchiveScope<TMode>(serializationContext)
				, RapidYamlScopeBase(mRootNode)
				, mOutput(&outputStream)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
			}

			/// <summary>
			/// Returns root node as object type in YAML.
			/// </summary>
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope()
			{
				if constexpr (TMode == SerializeMode::Load) {
					return mRootNode.is_map() ? std::make_optional<RapidYamlObjectScope<TMode>>(mRootNode, TArchiveScope<TMode>::GetContext()) : std::nullopt;
				}
				else
				{
					mRootNode |= ryml::MAP;
					return std::make_optional<RapidYamlObjectScope<TMode>>(mRootNode, TArchiveScope<TMode>::GetContext());
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
						? std::make_optional<RapidYamlArrayScope<TMode>>(mRootNode, TArchiveScope<TMode>::GetContext(), mRootNode.num_children())
						: std::nullopt;
				}
				else
				{
					mRootNode |= ryml::SEQ;
					return std::make_optional<RapidYamlArrayScope<TMode>>(mRootNode, TArchiveScope<TMode>::GetContext(), arraySize);
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
						auto& options = TArchiveScope<TMode>::GetOptions();
						if constexpr (std::is_same_v<T, std::string*>) {
							*arg = ryml::emitrs<std::string>(mTree);
						}
						else if constexpr (std::is_same_v<T, std::ostream*>)
						{
							if (options.streamOptions.writeBom) {
								arg->write(Convert::Utf8::bom, sizeof Convert::Utf8::bom);
							}
							*arg << mTree;
						}
					}, mOutput);
					mOutput = nullptr;
				}
			}

		private:
			RapidYamlRootScope(RapidYamlRootScope&&) = default;
			RapidYamlRootScope& operator=(RapidYamlRootScope&&) = default;

			template <typename T>
			void Parse(std::string_view inputStr)
			{
				if constexpr (ryml_has_parse_in_arena<T>::value)
				{
					T parser(ryml::Callbacks(nullptr, nullptr, nullptr, &RapidYamlRootScope::ErrorCallback));
					mTree = parser.parse_in_arena({}, c4::csubstr(inputStr.data(), inputStr.size()));
				}
				else
				{
					// For keep compatibility with old versions of RapidYaml library
					if (c4::yml::get_callbacks().m_error != &RapidYamlRootScope::ErrorCallback)
					{
						ryml::set_callbacks(ryml::Callbacks(nullptr, nullptr, nullptr, &RapidYamlRootScope::ErrorCallback));
						c4::set_error_flags(c4::get_error_flags() | c4::ON_ERROR_CALLBACK);
					}
					c4::yml::parse(c4::csubstr(inputStr.data(), inputStr.size()), &mTree);
				}
				mRootNode = mTree.rootref();
			}

			static void ErrorCallback(const char* msg, size_t length, ryml::Location location, [[maybe_unused]] void* user_data)
			{
				throw ParsingException({ msg, msg + length }, location.line);
			}

			ryml::Tree mTree;
			RapidYamlNode mRootNode = mTree.rootref();
			std::variant<std::nullptr_t, std::string*, std::ostream*> mOutput;
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
