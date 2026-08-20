/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <iosfwd>
#include <iostream>
#include <optional>
#include <type_traits>
#include "bitserializer/export.h"
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/object_traits.h"


namespace BitSerializer::Json {
namespace Detail {

/**
 * @brief Json archive traits.
 */
struct JsonArchiveTraits  // NOLINT(cppcoreguidelines-special-member-functions)
{
	static constexpr ArchiveType archive_type = ArchiveType::Json;
	using key_type = std::string;
	using supported_key_types = TSupportedKeyTypes<key_type, std::string_view>;
	using string_view_type = std::string_view;
	using preferred_output_type = std::basic_string<char, std::char_traits<char>>;
	using raw_type = RawPayload<std::string, ArchiveType::Json>;
	static constexpr char path_separator = '/';
	static constexpr bool is_binary = false;
	static constexpr bool require_array_size = false;
	static constexpr bool require_map_size = false;

protected:
	~JsonArchiveTraits() = default;
};

enum class ValueType
{
	Null,
	Boolean,
	UnsignedInteger,
	SignedInteger,
	Float,
	String,
	Array,
	Object
};

class BITSERIALIZER_API IJsonWriter
{
public:
	virtual ~IJsonWriter() = default;

	template <typename T>
	void WriteValue(T value)
	{
		// Some integer types don't have fixed-size convertible types
		static_assert(!std::is_same_v<compatible_fixed_t<T>, void>);
		WriteValue(static_cast<compatible_fixed_t<T>>(value));
	}

	virtual void WriteValue(std::nullptr_t) = 0;

	virtual void WriteValue(bool value) = 0;

	virtual void WriteValue(uint8_t value) = 0;
	virtual void WriteValue(uint16_t value) = 0;
	virtual void WriteValue(uint32_t value) = 0;
	virtual void WriteValue(uint64_t value) = 0;

	virtual void WriteValue(int8_t value) = 0;
	virtual void WriteValue(int16_t value) = 0;
	virtual void WriteValue(int32_t value) = 0;
	virtual void WriteValue(int64_t value) = 0;

	virtual void WriteValue(float value) = 0;
	virtual void WriteValue(double value) = 0;
	virtual void WriteValue(long double value) = 0;

	virtual void WriteValue(const char* value) = 0;	// For avoid conflict with overload for boolean
	virtual void WriteValue(std::string_view value) = 0;

	virtual void WriteValue(JsonArchiveTraits::raw_type& value) = 0;

	virtual void WriteValueSeparator() = 0;

	virtual void BeginArray() = 0;
	virtual void EndArray(bool hasElements) = 0;

	virtual void BeginObject() = 0;
	virtual void WriteKey(std::string_view key) = 0;
	virtual void EndObject(bool hasElements) = 0;
};

class BITSERIALIZER_API IJsonReader
{
public:
	virtual ~IJsonReader() = default;

	[[nodiscard]] virtual size_t GetPosition() const noexcept = 0;
	virtual void SetPosition(size_t pos) = 0;
	[[nodiscard]] virtual size_t GetLineNumber() const noexcept = 0;
	[[nodiscard]] virtual bool IsEnd() const = 0;

	virtual void ReadKey(std::string_view& key) = 0;

	template <typename T>
	bool ReadValue(T& value)
	{
		static_assert(!std::is_same_v<compatible_fixed_t<T>, void>);
		compatible_fixed_t<T> temp;
		if (ReadValue(temp))
		{
			value = static_cast<T>(temp);
			return true;
		}
		return false;
	}

	virtual bool ReadValue(std::nullptr_t&) = 0;
	virtual bool ReadValue(bool& value) = 0;

	virtual bool ReadValue(uint8_t& value) = 0;
	virtual bool ReadValue(uint16_t& value) = 0;
	virtual bool ReadValue(uint32_t& value) = 0;
	virtual bool ReadValue(uint64_t& value) = 0;

	virtual bool ReadValue(char& value) = 0;
	virtual bool ReadValue(int8_t& value) = 0;
	virtual bool ReadValue(int16_t& value) = 0;
	virtual bool ReadValue(int32_t& value) = 0;
	virtual bool ReadValue(int64_t& value) = 0;

	virtual bool ReadValue(float& value) = 0;
	virtual bool ReadValue(double& value) = 0;
	virtual bool ReadValue(long double& value) = 0;

	virtual bool ReadValue(std::string_view& value) = 0;

	virtual bool ReadValue(JsonArchiveTraits::raw_type& value) = 0;

	[[nodiscard]] virtual ValueType ReadValueType() = 0;

	virtual void ReadValueSeparator() = 0;

	virtual void SkipValue() = 0;

	virtual bool OpenArray() = 0;
	virtual bool IsArrayEnd() = 0;
	virtual void CloseArray(bool expectedComma) = 0;

	virtual bool OpenObject() = 0;
	virtual bool IsObjectEnd() = 0;
	virtual void CloseObject(bool expectedComma) = 0;
};

//-----------------------------------------------------------------------------
// Json writers
//-----------------------------------------------------------------------------

// Forward declarations
template <class TWriter>
class CJsonWriteObjectScope;

/**
 * @brief Json scope for writing arrays (sequential values).
 */
template <class TWriter>
class CJsonWriteArrayScope final : public JsonArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	CJsonWriteArrayScope(TWriter* msgPackWriter, SerializationContext& serializationContext) noexcept
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mJsonWriter(msgPackWriter)
	{ }

	~CJsonWriteArrayScope()
	{
		mJsonWriter->EndArray(mIndex);
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T> || std::is_same_v<T, std::string_view> || std::is_same_v<T, raw_type>, int> = 0>
	bool SerializeValue(T& value)
	{
		if (mIndex)
		{
			mJsonWriter->WriteValueSeparator();
		}
		mJsonWriter->WriteValue(value);
		++mIndex;
		return true;
	}

	[[nodiscard]] std::optional<CJsonWriteArrayScope<TWriter>> OpenArrayScope(size_t)
	{
		if (mIndex)
		{
			mJsonWriter->WriteValueSeparator();
		}
		mJsonWriter->BeginArray();
		++mIndex;
		return std::make_optional<CJsonWriteArrayScope<TWriter>>(mJsonWriter, GetContext());
	}

	[[nodiscard]] std::optional<CJsonWriteObjectScope<TWriter>> OpenObjectScope(size_t)
	{
		if (mIndex)
		{
			mJsonWriter->WriteValueSeparator();
		}
		mJsonWriter->BeginObject();
		++mIndex;
		return std::make_optional<CJsonWriteObjectScope<TWriter>>(mJsonWriter, GetContext());
	}

private:
	TWriter* mJsonWriter;
	size_t mIndex = 0;
};


/**
 * @brief Json scope for writing objects (key-value pairs).
 */
template <class TWriter>
class CJsonWriteObjectScope final : public JsonArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	CJsonWriteObjectScope(TWriter* msgPackWriter, SerializationContext& serializationContext) noexcept
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mJsonWriter(msgPackWriter)
	{ }

	~CJsonWriteObjectScope()
	{
		mJsonWriter->EndObject(mIndex);
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T> || std::is_same_v<T, string_view_type> || std::is_same_v<T, raw_type>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if (mIndex)
		{
			mJsonWriter->WriteValueSeparator();
		}
		mJsonWriter->WriteKey(std::string_view(key));
		mJsonWriter->WriteValue(value);
		++mIndex;
		return true;
	}

	template <typename TKey>
	std::optional<CJsonWriteArrayScope<TWriter>> OpenArrayScope(TKey&& key, size_t)
	{
		if (mIndex)
		{
			mJsonWriter->WriteValueSeparator();
		}
		mJsonWriter->WriteKey(std::string_view(key));
		mJsonWriter->BeginArray();
		++mIndex;
		return std::make_optional<CJsonWriteArrayScope<TWriter>>(mJsonWriter, GetContext());
	}

	template <typename TKey>
	[[nodiscard]] std::optional<CJsonWriteObjectScope<TWriter>> OpenObjectScope(TKey&& key, size_t)
	{
		if (mIndex)
		{
			mJsonWriter->WriteValueSeparator();
		}
		mJsonWriter->WriteKey(std::string_view(key));
		mJsonWriter->BeginObject();
		++mIndex;
		return std::make_optional<CJsonWriteObjectScope<TWriter>>(mJsonWriter, GetContext());
	}

private:
	TWriter* mJsonWriter;
	size_t mIndex = 0;
};


/**
 * @brief Json root scope for writing data (can write array or object).
 */
class BITSERIALIZER_API JsonWriteRootScope final : public JsonArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	JsonWriteRootScope(std::string& outputData, SerializationContext& serializationContext);
	JsonWriteRootScope(std::ostream& outputStream, SerializationContext& serializationContext);
	~JsonWriteRootScope();

	JsonWriteRootScope(JsonWriteRootScope&&) = delete;
	JsonWriteRootScope& operator=(JsonWriteRootScope&&) = delete;
	JsonWriteRootScope(const JsonWriteRootScope&) = delete;
	JsonWriteRootScope& operator=(const JsonWriteRootScope&) = delete;

	/**
	 * @brief Gets the current path in Json.
	 */
	[[nodiscard]] static constexpr std::string_view GetPath() noexcept
	{
		return {};
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T> || std::is_same_v<T, string_view_type> || std::is_same_v<T, raw_type>, int> = 0>
	bool SerializeValue(T& value)
	{
		mJsonWriter->WriteValue(value);
		return true;
	}

	[[nodiscard]] std::optional<CJsonWriteArrayScope<IJsonWriter>> OpenArrayScope(size_t) const
	{
		mJsonWriter->BeginArray();
		return std::make_optional<CJsonWriteArrayScope<IJsonWriter>>(mJsonWriter, GetContext());
	}

	[[nodiscard]] std::optional<CJsonWriteObjectScope<IJsonWriter>> OpenObjectScope(size_t) const
	{
		mJsonWriter->BeginObject();
		return std::make_optional<CJsonWriteObjectScope<IJsonWriter>>(mJsonWriter, GetContext());
	}

	static constexpr void Finalize() noexcept { /* Not required */ }

private:
	IJsonWriter* mJsonWriter = nullptr;
};


//-----------------------------------------------------------------------------
// Json readers
//-----------------------------------------------------------------------------

// Forward declarations
template <class TReader> class CJsonReadObjectScope;


class CJsonReadScopeBase : public JsonArchiveTraits
{
public:
	CJsonReadScopeBase(CJsonReadScopeBase* parentScope = nullptr) noexcept
		: mParentScope(parentScope)
	{ }

	CJsonReadScopeBase(const CJsonReadScopeBase&) = delete;
	CJsonReadScopeBase(CJsonReadScopeBase&&) noexcept = delete;
	CJsonReadScopeBase& operator=(const CJsonReadScopeBase&) = delete;
	CJsonReadScopeBase& operator=(CJsonReadScopeBase&&) noexcept = default;

	/**
	 * @brief Gets the current path in Json.
	 */
	[[nodiscard]] virtual std::string GetPath() const
	{
		std::string path = mParentScope ? mParentScope->GetPath() : std::string();
		return path;
	}

	virtual void OnFinishChildScope() {}

protected:
	~CJsonReadScopeBase()
	{
		if (mParentScope) {
			mParentScope->OnFinishChildScope();
		}
	}

private:
	CJsonReadScopeBase* mParentScope;
};


/**
 * @brief Json scope for reading arrays (sequential values).
 */
template <class TReader>
class CJsonReadArrayScope final : public CJsonReadScopeBase, public TArchiveScope<SerializeMode::Load>
{
public:
	CJsonReadArrayScope(TReader* msgPackReader, SerializationContext& serializationContext, CJsonReadScopeBase* parentScope = nullptr) noexcept
		: CJsonReadScopeBase(parentScope)
		, TArchiveScope<SerializeMode::Load>(serializationContext)
		, mJsonReader(msgPackReader)
	{ }

	~CJsonReadArrayScope()
	{
		if (!GetContext().IsStackUnwinding())
		{
			mJsonReader->CloseArray(!!mIndex);
		}
	}

	/**
	 * @brief Gets the current path in Json.
	 */
	[[nodiscard]] std::string GetPath() const override
	{
		std::string path = CJsonReadScopeBase::GetPath();
		path.push_back(path_separator);
		path += Convert::ToString(mIndex);
		return path;
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T> || std::is_same_v<T, string_view_type> || std::is_same_v<T, raw_type>, int> = 0>
	bool SerializeValue(T& value)
	{
		if (mIndex)
		{
			mJsonReader->ReadValueSeparator();
		}

		if (mJsonReader->ReadValue(value))
		{
			++mIndex;
			return true;
		}
		return false;
	}

	/**
	 * @brief Returns the estimated number of items to load (for reserving the size of containers).
	 */
	[[nodiscard]] size_t GetEstimatedSize() const noexcept
	{
		return mIndex;
	}

	/**
	 * @brief Returns `true` when there are no more values to load.
	 */
	[[nodiscard]] bool IsEnd() const
	{
		return mJsonReader->IsArrayEnd();
	}

	std::optional<CJsonReadArrayScope<TReader>> OpenArrayScope(size_t)
	{
		if (mIndex)
		{
			mJsonReader->ReadValueSeparator();
		}

		if (mJsonReader->OpenArray())
		{
			++mIndex;
			return std::make_optional<CJsonReadArrayScope<TReader>>(mJsonReader, GetContext(), this);
		}
		return std::nullopt;
	}

	std::optional<CJsonReadObjectScope<TReader>> OpenObjectScope(size_t)
	{
		if (mIndex)
		{
			mJsonReader->ReadValueSeparator();
		}

		if (mJsonReader->OpenObject())
		{
			++mIndex;
			return std::make_optional<CJsonReadObjectScope<TReader>>(mJsonReader, GetContext(), this);
		}
		return std::nullopt;
	}

private:
	TReader* mJsonReader;
	size_t mIndex = 0;
};

/**
 * @brief Json scope for reading objects (key-value pairs).
 */
template <class TReader>
class CJsonReadObjectScope final : public CJsonReadScopeBase, public TArchiveScope<SerializeMode::Load>
{
public:
	CJsonReadObjectScope(TReader* msgPackReader, SerializationContext& serializationContext, CJsonReadScopeBase* parentScope = nullptr) noexcept
		: CJsonReadScopeBase(parentScope)
		, TArchiveScope<SerializeMode::Load>(serializationContext)
		, mJsonReader(msgPackReader)
		, mStartPos(msgPackReader->GetPosition())
	{ }

	~CJsonReadObjectScope()
	{
		if (!GetContext().IsStackUnwinding())
		{
			if (!mCurrentKey.empty())
			{
				SkipCurrentKeyValue();
			}
			mJsonReader->CloseObject(!!mIndex);
		}
	}

	/**
	 * @brief Gets the current path in Json.
	 */
	[[nodiscard]] std::string GetPath() const override
	{
		std::string path = CJsonReadScopeBase::GetPath();
		if (!mCurrentKey.empty())
		{
			path.push_back(path_separator);
			path += mCurrentKey;
		}
		return path;
	}

	/**
	 * @brief Returns the estimated number of items to load (for reserving the size of containers).
	 */
	[[nodiscard]] static size_t GetEstimatedSize() noexcept { return 0; }

	/**
	 * @brief Enumerates all keys in the current object.
	 *
	 * @tparam TCallback Callback function type.
	 * @param fn Callback to invoke for each key.
	 */
	template <typename TCallback>
	void VisitKeys(TCallback&& fn)
	{
		// Rewind to start of object
		if (mStartPos != mJsonReader->GetPosition())
		{
			if (!mCurrentKey.empty())
			{
				SkipCurrentKeyValue();
			}
			mJsonReader->SetPosition(mStartPos);
		}

		while (!mJsonReader->IsObjectEnd())
		{
			ReadKey();
			fn(mCurrentKey);

			if (!mCurrentKey.empty())
			{
				SkipCurrentKeyValue();
			}
		}
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_fundamental_v<T> || std::is_null_pointer_v<T> || std::is_same_v<T, string_view_type> || std::is_same_v<T, raw_type>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if (FindValueByKey(key))
		{
			++mIndex;
			mCurrentKey = {};
			return mJsonReader->ReadValue(value);
		}
		return false;
	}

	template <typename TKey>
	std::optional<CJsonReadArrayScope<TReader>> OpenArrayScope(TKey&& key, size_t)
	{
		if (FindValueByKey(key))
		{
			if (mJsonReader->OpenArray())
			{
				return std::make_optional<CJsonReadArrayScope<TReader>>(mJsonReader, GetContext(), this);
			}
			OnFinishChildScope();
		}
		return std::nullopt;
	}

	template <typename TKey>
	std::optional<CJsonReadObjectScope<TReader>> OpenObjectScope(TKey&& key, size_t)
	{
		if (FindValueByKey(key))
		{
			if (mJsonReader->OpenObject())
			{
				return std::make_optional<CJsonReadObjectScope<TReader>>(mJsonReader, GetContext(), this);
			}
			OnFinishChildScope();
		}
		return std::nullopt;
	}

	void OnFinishChildScope() override
	{
		mCurrentKey = {};
		++mIndex;
	}

private:
	void ReadKey()
	{
		if (mIndex)
		{
			mJsonReader->ReadValueSeparator();
		}
		mJsonReader->ReadKey(mCurrentKey);
	}

	bool FindValueByKey(std::string_view key)
	{
		if (!mCurrentKey.empty())
		{
			if (mCurrentKey == key) {
				return true;
			}
			SkipCurrentKeyValue();
		}

		// Start finding from the current key/value pair
		const size_t mPrevIndex = mIndex;
		do
		{
			// If the end is reached, rewind to the start of the object
			if (mJsonReader->IsObjectEnd())
			{
				// When object is empty
				if (mPrevIndex == 0)
				{
					return false;
				}
				mJsonReader->SetPosition(mStartPos);
				mIndex = 0;
			}

			ReadKey();
			if (mCurrentKey.compare(key) == 0) {
				return true;
			}
			SkipCurrentKeyValue();
		} while (mIndex != mPrevIndex);
		return false;
	}

	void SkipCurrentKeyValue()
	{
		assert(!mCurrentKey.empty());
		mCurrentKey = {};
		mJsonReader->SkipValue();
		++mIndex;
	}

	TReader* mJsonReader;
	size_t mStartPos;
	size_t mIndex = 0;
	std::string_view mCurrentKey;
};


/**
 * @brief Json root scope for reading data (can read array or object).
 */
class BITSERIALIZER_API JsonReadRootScope final : public JsonArchiveTraits, public TArchiveScope<SerializeMode::Load>
{
public:
	JsonReadRootScope(std::string_view inputData, SerializationContext& serializationContext);
	JsonReadRootScope(std::istream& inputStream, SerializationContext& serializationContext);
	~JsonReadRootScope();

	/**
	 * @brief Gets the current path in Json.
	 */
	[[nodiscard]] static constexpr std::string_view GetPath() noexcept
	{
		return {};
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T> || std::is_same_v<T, string_view_type> || std::is_same_v<T, raw_type>, int> = 0>
	bool SerializeValue(T& value) const
	{
		return mJsonReader->ReadValue(value);
	}

	[[nodiscard]] std::optional<CJsonReadArrayScope<IJsonReader>> OpenArrayScope(size_t) const
	{
		if (mJsonReader->OpenArray()) {
			return std::make_optional<CJsonReadArrayScope<IJsonReader>>(mJsonReader, GetContext());
		}
		return std::nullopt;
	}

	[[nodiscard]] std::optional<CJsonReadObjectScope<IJsonReader>> OpenObjectScope(size_t) const
	{
		if (mJsonReader->OpenObject()) {
			return std::make_optional<CJsonReadObjectScope<IJsonReader>>(mJsonReader, GetContext());
		}
		return std::nullopt;
	}

	static constexpr void Finalize() noexcept { /* Not required */ }

private:
	IJsonReader* mJsonReader = nullptr;
};

}


/**
 * @brief Json archive.
 *
 * Supports load/save from:
 * - `std::string`
 * - `std::istream` and `std::ostream`
 */
using JsonArchive = TArchiveBase<
	Detail::JsonArchiveTraits,
	Detail::JsonReadRootScope,
	Detail::JsonWriteRootScope>;

/**
 * @brief Represents an opaque JSON subtree for efficient pass-through handling.
 *
 * This type captures a JSON subtree as a DOM fragment during deserialization, avoiding conversion to C++ objects.
 * During serialization, the fragment is directly inserted without re-parsing (cannot be used with another archive).
 */
using Raw = JsonArchive::raw_type;

} // namespace BitSerializer::Json
