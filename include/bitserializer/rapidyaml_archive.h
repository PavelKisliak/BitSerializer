/*******************************************************************************
* Copyright (C) 2020-2024 by Artsiom Marozau, Pavel Kisliak                    *
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

namespace c4::yml
{
	// Dummy
	template <typename T>
	void emitrs_yaml();

	// Detect existence of new ryml API function emitrs_yaml (ToDo: remove in one of future releases)
	template <typename T>
	struct ryml_has_emitrs_yaml
	{
	private:
		template <typename U>
		static std::enable_if_t<std::is_same_v<U, decltype(emitrs_yaml<U>(std::declval<Tree>()))>, std::true_type> test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<T>(0)) type;
		enum { value = type::value };
	};
}

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
			static constexpr bool is_binary = false;

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

			static bool LoadValue(const RapidYamlNode& yamlValue, std::string_view& value)
			{
				if (!yamlValue.is_val() && !yamlValue.is_keyval())
					return false;

				if (IsNullYamlValue(yamlValue.val())) {
					return false;
				}

				const auto str = yamlValue.val();
				value = std::string_view(str.data(), str.size());
				return true;
			}

			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			static void SaveValue(RapidYamlNode& yamlValue, T& value)
			{
				if constexpr (std::is_null_pointer_v<T>) {
					yamlValue << nullValue;
				} else if constexpr (std::is_same_v<T, bool>) {
					yamlValue << c4::fmt::boolalpha(value);
				} else if constexpr (std::is_floating_point_v<T>) {
					yamlValue << c4::fmt::real(value, std::numeric_limits<T>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
				} else if constexpr (std::is_same_v<T, char>) {
					// Need to extend size of type for prevent save as character
					yamlValue << static_cast<int16_t>(value);
				} else {
					yamlValue << value;
				}
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

			static void HandleMismatchedTypesPolicy(MismatchedTypesPolicy mismatchedTypesPolicy)
			{
				if (mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						"The type of target field does not match the value being loaded");
				}
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
			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
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

			bool SerializeValue(std::string_view& value)
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
					yamlValue << c4::csubstr(value.data(), value.size());
					mIndex++;
					return true;
				}
				return false;
			}

			/// <summary>
			/// Returns element of array as sub-object.
			/// </summary>
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope(size_t)
			{				
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < mSize)
					{
						auto yamlValue = LoadNextItem();
						if (yamlValue.is_map())
						{
							return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), this);
						}
						HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
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
						if (yamlValue.is_seq())
						{
							return std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), yamlValue.num_children(), this);
						}
						HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
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
			/// Returns the estimated number of items to load (for reserving the size of containers).
			/// </summary>
			[[nodiscard]]
			size_t GetEstimatedSize() const {
				return mNode.num_children();
			}

			/// <summary>
			/// Enumerates all keys by calling a passed function.
			/// </summary>
			template <typename TCallback>
			void VisitKeys(TCallback&& fn)
			{
				for (const auto& keyVal : this->mNode)
				{
					std::string key;
					c4::from_chars(keyVal.key(), &key);
					fn(std::move(key));
				}
			}

			/// <summary>
			/// Serialize value.
			/// </summary>
			/// <param name="key">The key of child node.</param>
			/// <param name="value">The value.</param>
			template <typename TKey, typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
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

			template <typename TKey>
			bool SerializeValue(TKey&& key, std::string_view& value)
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
					yamlValue << c4::csubstr(value.data(), value.size());
					return true;
				}
			}

			/// <summary>
			/// Returns child node as sub-object.
			/// </summary>
			/// <param name="key">The key of child node.</param>
			template <typename TKey>
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope(TKey&& key, size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode.find_child(c4::to_csubstr(key));
					if (yamlValue.valid())
					{
						if (yamlValue.is_map())
						{
							return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), this, key);
						}
						HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
					}
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
					{
						if (yamlValue.is_seq())
						{
							return std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), yamlValue.num_children(), this, key);
						}
						HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
					}
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
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope(size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mRootNode.is_map())
					{
						return std::make_optional<RapidYamlObjectScope<TMode>>(mRootNode, TArchiveScope<TMode>::GetContext());
					}
					HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
					return std::nullopt;
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
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mRootNode.is_seq())
					{
						return std::make_optional<RapidYamlArrayScope<TMode>>(mRootNode, TArchiveScope<TMode>::GetContext(), mRootNode.num_children());
					}
					HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
					return std::nullopt;
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
						if constexpr (std::is_same_v<T, std::string*>)
						{
							if constexpr (c4::yml::ryml_has_emitrs_yaml<std::string>::value)
							{
								*arg = ryml::emitrs_yaml<std::string>(mTree);
							}
							else
							{
								*arg = ryml::emitrs<std::string>(mTree);
							}
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
				T parser(ryml::Callbacks(nullptr, nullptr, nullptr, &RapidYamlRootScope::ErrorCallback));
				mTree = parser.parse_in_arena({}, c4::csubstr(inputStr.data(), inputStr.size()));
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
