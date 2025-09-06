/*******************************************************************************
* Copyright (C) 2020-2025 by Artsiom Marozau, Pavel Kisliak                    *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <iosfwd>
#include <optional>
#include <type_traits>
#include <variant>

#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/errors_handling.h"

// External dependency (Rapid YAML)
#include "c4/format.hpp"
#include "ryml/ryml.hpp"
#include "ryml/ryml_std.hpp"

namespace BitSerializer::Yaml::RapidYaml {
	namespace Detail {

		/**
		 * @brief YAML archive traits (based on RapidYaml).
		 */
		class RapidYamlArchiveTraits  // NOLINT(cppcoreguidelines-special-member-functions)
		{
		public:
			static constexpr ArchiveType archive_type = ArchiveType::Yaml;
			using key_type = std::string;
			using supported_key_types = TSupportedKeyTypes<const char*, key_type>;
			using string_view_type = std::string_view;
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

		/**
		 * @brief Converts `std::string_view` to `c4::csubstr`
		 */
		inline c4::csubstr to_csubstr(std::string_view str)
		{
			return { str.data(), str.size() };
		}

		/**
		 * @brief Base class of YAML scope.
		 */
		class RapidYamlScopeBase : public RapidYamlArchiveTraits
		{
		public:
			using RapidYamlNode = ryml::NodeRef;
			using key_type_view = std::basic_string_view<key_type::value_type>;

			RapidYamlScopeBase() = default;
			RapidYamlScopeBase(const RapidYamlNode& node, RapidYamlScopeBase* parent, key_type_view parentKey) noexcept
				: mNode(node)
				, mParent(parent)
				, mParentKey(parentKey)
			{ }

			RapidYamlScopeBase(const RapidYamlScopeBase&) = delete;
			RapidYamlScopeBase(RapidYamlScopeBase&&) = delete;
			RapidYamlScopeBase& operator=(const RapidYamlScopeBase&) = delete;
			RapidYamlScopeBase& operator=(RapidYamlScopeBase&&) = delete;

			/**
			 * @brief Gets the current path in YAML.
			 */
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
				return BitSerializer::Detail::ConvertByPolicy(str, value, serializationOptions.mismatchedTypesPolicy, serializationOptions.overflowNumberPolicy);
			}

			static bool LoadValue(const RapidYamlNode& yamlValue, string_view_type& value)
			{
				if (!yamlValue.is_val() && !yamlValue.is_keyval()) {
					return false;
				}

				if (IsNullYamlValue(yamlValue.val())) {
					return false;
				}

				const auto str = yamlValue.val();
				value = string_view_type(str.data(), str.size());
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
			RapidYamlScopeBase* mParent = nullptr;
			key_type_view mParentKey;
		};

		/**
		 * @brief YAML scope for serializing arrays (sequential values).
		 */
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

			/**
			 * @brief Returns the estimated number of items to load (for reserving the size of containers).
			 */
			[[nodiscard]]
			size_t GetEstimatedSize() const {
				return mSize;
			}

			/**
			 * @brief Returns `true` when there are no more values to load.
			 */
			[[nodiscard]]
			bool IsEnd() const
			{
				static_assert(TMode == SerializeMode::Load);
				return mIndex == mSize;
			}

			/**
			 * @brief Gets the current path in YAML.
			 */
			[[nodiscard]]
			std::string GetPath() const override
			{
				return RapidYamlScopeBase::GetPath() + path_separator + Convert::ToString(mIndex);
			}

			template <typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool SerializeValue(T& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < GetEstimatedSize()) {
						return LoadValue(LoadNextItem(), value, this->GetOptions());
					}
					return false;
				}
				else
				{
					auto yamlValue = mNode.append_child();
					SaveValue(yamlValue, value);
					mIndex++;
					return true;
				}
			}

			bool SerializeValue(string_view_type& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mIndex < GetEstimatedSize()) {
						return LoadValue(LoadNextItem(), value);
					}
					return false;
				}
				else
				{
					auto yamlValue = mNode.append_child();
					yamlValue << c4::csubstr(value.data(), value.size());
					mIndex++;
					return true;
				}
			}

			/**
			 * @brief Opens a nested object scope.
			 */
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
						// NULL value from the source YAML is excluded from MismatchedTypesPolicy processing
						if (!IsNullYamlValue(yamlValue.val())) {
							HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
						}
					}
					return std::nullopt;
				}
				else
				{
					auto yamlValue = mNode.append_child();
					yamlValue |= ryml::MAP;
					mIndex++;
					return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), this);
				}
			}

			/**
			 * @brief Opens a nested array scope.
			 */
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
						// NULL value from the source YAML is excluded from MismatchedTypesPolicy processing
						if (!IsNullYamlValue(yamlValue.val())) {
							HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
						}
					}
					return std::nullopt;
				}
				else
				{
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


		/**
		 * @brief YAML scope for serializing objects (key-value pairs).
		 */
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

			/**
			 * @brief Returns the estimated number of items to load (for reserving the size of containers).
			 */
			[[nodiscard]]
			size_t GetEstimatedSize() const {
				return mNode.num_children();
			}

			/**
			 * @brief Enumerates all keys in the current object.
			 *
			 * @tparam TCallback Callback function type.
			 * @param fn Callback to invoke for each key.
			 */
			template <typename TCallback>
			void VisitKeys(const TCallback& fn)
			{
				for (const auto& keyVal : this->mNode)
				{
					const auto key = keyVal.key();
					fn(std::string_view(key.data(), key.size()));
				}
			}

			template <typename TKey, typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
			bool SerializeValue(const TKey& key, T& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode.find_child(to_csubstr(key));
#if defined RYML_VERSION_MAJOR && RYML_VERSION_MAJOR >= 0 && RYML_VERSION_MINOR >= 7
					return yamlValue.invalid() ? false : LoadValue(yamlValue, value, this->GetOptions());
#else
					return yamlValue.valid() ? LoadValue(yamlValue, value, this->GetOptions()) : false;
#endif
				}
				else
				{
					auto yamlValue = mNode.append_child();
					yamlValue << ryml::key(key);
					SaveValue(yamlValue, value);
					return true;
				}
			}

			template <typename TKey>
			bool SerializeValue(const TKey& key, string_view_type& value)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode.find_child(to_csubstr(key));
#if defined RYML_VERSION_MAJOR && RYML_VERSION_MAJOR >= 0 && RYML_VERSION_MINOR >= 7
					return yamlValue.invalid() ? false : LoadValue(yamlValue, value);
#else
					return yamlValue.valid() ? LoadValue(yamlValue, value) : false;
#endif
				}
				else
				{
					auto yamlValue = mNode.append_child();
					yamlValue << ryml::key(key);
					yamlValue << c4::csubstr(value.data(), value.size());
					return true;
				}
			}

			/**
			 * @brief Opens a nested object scope for the specified key.
			 */
			template <typename TKey>
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope(const TKey& key, size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode.find_child(to_csubstr(key));
#if defined RYML_VERSION_MAJOR && RYML_VERSION_MAJOR >= 0 && RYML_VERSION_MINOR >= 7
					if (!yamlValue.invalid())
#else
					if (yamlValue.valid())
#endif
					{
						if (yamlValue.is_map())
						{
							return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), this, key);
						}
						// NULL value from the source YAML is excluded from MismatchedTypesPolicy processing
						if (!IsNullYamlValue(yamlValue.val())) {
							HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
						}
					}
					return std::nullopt;
				}
				else
				{
					auto yamlValue = mNode.append_child();
					yamlValue << c4::yml::key(key);
					yamlValue |= ryml::MAP;
					return std::make_optional<RapidYamlObjectScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), this, key);
				}
			}

			/**
			 * @brief Opens a nested array scope for the specified key.
			 */
			template <typename TKey>
			std::optional<RapidYamlArrayScope<TMode>> OpenArrayScope(const TKey& key, size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					const auto yamlValue = mNode.find_child(to_csubstr(key));
#if defined RYML_VERSION_MAJOR && RYML_VERSION_MAJOR >= 0 && RYML_VERSION_MINOR >= 7
					if (!yamlValue.invalid())
#else
					if (yamlValue.valid())
#endif
					{
						if (yamlValue.is_seq())
						{
							return std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), yamlValue.num_children(), this, key);
						}
						// NULL value from the source YAML is excluded from MismatchedTypesPolicy processing
						if (!IsNullYamlValue(yamlValue.val())) {
							HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
						}
					}
					return std::nullopt;
				}
				else
				{
					auto yamlValue = mNode.append_child();
					yamlValue << c4::yml::key(key);
					yamlValue |= ryml::SEQ;
					return std::make_optional<RapidYamlArrayScope<TMode>>(yamlValue, TArchiveScope<TMode>::GetContext(), 0, this, key);
				}
			}
		};

		/**
		 * @brief YAML root scope for serializing data (can serialize array or object).
		 */
		template <SerializeMode TMode>
		class RapidYamlRootScope final: public TArchiveScope<TMode>, public RapidYamlScopeBase
		{
		public:
			RapidYamlRootScope(const RapidYamlRootScope&) = delete;
			RapidYamlRootScope& operator=(const RapidYamlRootScope&) = delete;
			RapidYamlRootScope(RapidYamlRootScope&&) = delete;
			RapidYamlRootScope& operator=(RapidYamlRootScope&&) = delete;

			RapidYamlRootScope(std::string_view inputStr, SerializationContext& serializationContext)
				: TArchiveScope<TMode>(serializationContext)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");
				Parse<c4::yml::Parser>(inputStr);
			}

			RapidYamlRootScope(std::string& outputStr, SerializationContext& serializationContext)
				: TArchiveScope<TMode>(serializationContext)
				, mOutput(&outputStr)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
				mNode = mTree.rootref();
			}

			RapidYamlRootScope(std::istream& inputStream, SerializationContext& serializationContext)
				: TArchiveScope<TMode>(serializationContext)
				, mOutput(nullptr)
			{
				static_assert(TMode == SerializeMode::Load, "BitSerializer. This data type can be used only in 'Load' mode.");

				const auto utfType = Convert::Utf::DetectEncoding(inputStream);
				if (utfType != Convert::Utf::UtfType::Utf8) {
					throw SerializationException(SerializationErrorCode::UnsupportedEncoding, "The archive does not support encoding: " + Convert::ToString(utfType));
				}

				// ToDo: base library does not support std::stream (check in new versions)
				const std::string inputStr(std::istreambuf_iterator<char>(inputStream), {});
				Parse<c4::yml::Parser>(inputStr);
			}

			RapidYamlRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
				: TArchiveScope<TMode>(serializationContext)
				, mOutput(&outputStream)
			{
				static_assert(TMode == SerializeMode::Save, "BitSerializer. This data type can be used only in 'Save' mode.");
				mNode = mTree.rootref();
			}

			~RapidYamlRootScope() = default;

			/**
			 * @brief Opens a nested object scope.
			 */
			std::optional<RapidYamlObjectScope<TMode>> OpenObjectScope(size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mNode.is_map())
					{
						return std::make_optional<RapidYamlObjectScope<TMode>>(mNode, TArchiveScope<TMode>::GetContext());
					}
					HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
					return std::nullopt;
				}
				else
				{
					mNode |= ryml::MAP;
					return std::make_optional<RapidYamlObjectScope<TMode>>(mNode, TArchiveScope<TMode>::GetContext());
				}
			}

			/**
			 * @brief Opens a nested array scope.
			 */
			std::optional<RapidYamlArrayScope<TMode>> OpenArrayScope(size_t)
			{
				if constexpr (TMode == SerializeMode::Load)
				{
					if (mNode.is_seq())
					{
						return std::make_optional<RapidYamlArrayScope<TMode>>(mNode, TArchiveScope<TMode>::GetContext(), mNode.num_children());
					}
					HandleMismatchedTypesPolicy(this->GetContext().GetOptions().mismatchedTypesPolicy);
					return std::nullopt;
				}
				else
				{
					mNode |= ryml::SEQ;
					return std::make_optional<RapidYamlArrayScope<TMode>>(mNode, TArchiveScope<TMode>::GetContext(), 0);
				}
			}

			void Finalize()
			{
				if constexpr (TMode == SerializeMode::Save)
				{
					std::visit([this](auto&& arg)
					{
						using T = std::decay_t<decltype(arg)>;
						if constexpr (std::is_same_v<T, std::string*>)
						{
							*arg = ryml::emitrs_yaml<std::string>(mTree);
						}
						else if constexpr (std::is_same_v<T, std::ostream*>)
						{
							auto& options = TArchiveScope<TMode>::GetOptions();
							if (options.streamOptions.writeBom) {
								arg->write(Convert::Utf::Utf8::bom, sizeof Convert::Utf::Utf8::bom);
							}
							*arg << mTree;
						}
					}, mOutput);
					mOutput = nullptr;
				}
			}

		private:
			template <typename TParser>
			void Parse(std::string_view inputStr)
			{
#if defined RYML_VERSION_MAJOR && RYML_VERSION_MAJOR >= 0 && RYML_VERSION_MINOR >= 7
				c4::yml::EventHandlerTree EventHandlerTree(ryml::Callbacks(nullptr, nullptr, nullptr, &RapidYamlRootScope::ErrorCallback));
				TParser parser(&EventHandlerTree);
				mTree = parse_in_arena(&parser, c4::csubstr(inputStr.data(), inputStr.size()));
#else
				TParser parser(ryml::Callbacks(nullptr, nullptr, nullptr, &RapidYamlRootScope::ErrorCallback));
				mTree = parser.parse_in_arena({}, c4::csubstr(inputStr.data(), inputStr.size()));
#endif
				mNode = mTree.rootref();
			}

			static void ErrorCallback(const char* msg, size_t length, ryml::Location location, [[maybe_unused]] void* user_data)
			{
				throw ParsingException({ msg, msg + length }, location.line);
			}

			ryml::Tree mTree;
			std::variant<std::nullptr_t, std::string*, std::ostream*> mOutput;
		};
	}

	/**
	 * @brief RapidYaml archive (based on RapidYaml library).
	 *
	 * Supports load/save from:
	 * - `std::string`: UTF-8
	 * - `std::istream` and `std::ostream`: UTF-8
	 */
	using YamlArchive = TArchiveBase<
		Detail::RapidYamlArchiveTraits,
		Detail::RapidYamlRootScope<SerializeMode::Load>,
		Detail::RapidYamlRootScope<SerializeMode::Save>>;

} // BitSerializer::Yaml::RapidYaml
