﻿/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include <type_traits>
#include <functional>
#include "bitserializer/export.h"
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"
#include "bitserializer/serialization_detail/errors_handling.h"
#include "bitserializer/serialization_detail/object_traits.h"


namespace BitSerializer::MsgPack {
namespace Detail {

using BitSerializer::Detail::CBinTimestamp;

/**
 * @brief MsgPack archive traits.
 */
struct MsgPackArchiveTraits  // NOLINT(cppcoreguidelines-special-member-functions)
{
	static constexpr ArchiveType archive_type = ArchiveType::MsgPack;
	using key_type = std::string;
	using supported_key_types = TSupportedKeyTypes<key_type, std::string_view, int64_t, uint64_t, float, double, CBinTimestamp>;
	using string_view_type = std::string_view;
	using preferred_output_format = std::basic_string<char, std::char_traits<char>>;
	using preferred_stream_char_type = char;
	static constexpr char path_separator = '/';
	static constexpr bool is_binary = true;
	static constexpr bool require_array_size = false;
	static constexpr bool require_map_size = false;

protected:
	~MsgPackArchiveTraits() = default;
};

enum class ValueType
{
	Unknown,
	Nil,
	Boolean,
	UnsignedInteger,
	SignedInteger,
	Float,
	Double,
	String,
	Array,
	BinaryArray,
	Map,
	Ext,
	Timestamp
};

/**
 * @brief Stores the current key in the archive, supports all variant of keys from specified tuple.
 */
template <typename TTuple>
class CVariableKey
{
public:
	CVariableKey() = default;
	~CVariableKey() = default;

	CVariableKey(CVariableKey&&) = delete;
	CVariableKey& operator=(CVariableKey&&) = delete;

	CVariableKey(const CVariableKey&) = delete;
	CVariableKey& operator=(const CVariableKey&) = delete;

	operator bool() const noexcept {
		return mLast != nullptr;
	}

	template <typename T>
	bool operator==(const T& value) const noexcept
	{
		if constexpr (std::is_integral_v<T> && (std::is_unsigned_v<T> || std::is_signed_v<T>))
		{
			if (auto& refUnsigned = std::get<uint64_t>(mTuple); mLast == &refUnsigned) {
				return value >= 0 && refUnsigned == static_cast<std::make_unsigned_t<T>>(value);
			}
			if (auto& refSigned = std::get<int64_t>(mTuple); mLast == &refSigned)
			{
				if constexpr (std::is_signed_v<T>) {
					return refSigned == value;
				}
				else {
					return value <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()) && refSigned == static_cast<int64_t>(value);
				}
			}
			return false;
		}
		else
		{
			auto& ref = std::get<T>(mTuple);
			return mLast == &ref && ref == value;
		}
	}

	bool operator==(const std::string& value) const noexcept
	{
		auto& ref = std::get<std::string_view>(mTuple);
		return mLast == &ref && ref == value;
	}

	template <typename T, size_t ArraySize>
	bool operator==(T(&value)[ArraySize]) const noexcept
	{
		const auto& ref = std::get<std::string_view>(mTuple);
		return mLast == &ref && ref == value;
	}

	template <typename T>
	void Set(T&& key)
	{
		auto& ref = std::get<T>(mTuple);
		ref = std::forward<T>(key);
		mLast = &ref;
	}

	template <typename T>
	T& GetValueRef() noexcept
	{
		auto& ref = std::get<T>(mTuple);
		mLast = &ref;
		return ref;
	}

	[[nodiscard]] std::string ToString() const
	{
		if (mLast != nullptr)
		{
			std::string result;
			const auto convertFn = [last = mLast, &result](auto&& value)
			{
				if (&value == last) {
					result = Convert::ToString(value);
				}
			};
			std::apply([&convertFn](auto&&... args) { (convertFn(args), ...); }, mTuple);
			return result;
		}
		return {};
	}

	void Reset() noexcept {
		mLast = nullptr;
	}

private:
	TTuple mTuple;
	void* mLast = nullptr;
};

using MsgPackVariableKey = CVariableKey<MsgPackArchiveTraits::supported_key_types>;
using PathResolver = std::function<std::string()>;

class BITSERIALIZER_API IMsgPackWriter
{
public:
	virtual ~IMsgPackWriter() = default;

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

	virtual void WriteValue(const char* value) = 0;	// For avoid conflict with overload for boolean
	virtual void WriteValue(std::string_view value) = 0;
	void WriteValue(const MsgPackArchiveTraits::key_type& value) {
		WriteValue(std::string_view(value.data(), value.size()));
	}

	virtual void WriteValue(const CBinTimestamp& timestamp) = 0;

	virtual void BeginArray(size_t arraySize) = 0;
	virtual void BeginMap(size_t mapSize) = 0;

	virtual void BeginBinary(size_t binarySize) = 0;
	virtual void WriteBinary(char byte) = 0;
};

class BITSERIALIZER_API IMsgPackReader
{
public:
	virtual ~IMsgPackReader() = default;

	[[nodiscard]] virtual size_t GetPosition() const noexcept = 0;
	virtual void SetPosition(size_t pos) = 0;
	[[nodiscard]] virtual ValueType ReadValueType() = 0;
	[[nodiscard]] virtual bool IsEnd() const = 0;

	template <typename T>
	bool ReadValue(T& value)
	{
		// Some integer types don't have fixed-size convertible types
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

	virtual bool ReadValue(std::string_view& value) = 0;

	virtual bool ReadValue(CBinTimestamp& timestamp) = 0;

	virtual bool ReadArraySize(size_t& arraySize) = 0;
	virtual bool ReadMapSize(size_t& mapSize) = 0;

	virtual bool ReadBinarySize(size_t& binarySize) = 0;
	virtual char ReadBinary() = 0;

	virtual void SkipValue() = 0;
};

//-----------------------------------------------------------------------------
// MsgPack writers
//-----------------------------------------------------------------------------

// Forward declarations
template <class TWriter>
class CMsgPackWriteObjectScope;

/**
 * @brief MsgPack scope for writing binary arrays.
 */
template <class TWriter>
class CMsgPackWriteBinaryScope final : public MsgPackArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	CMsgPackWriteBinaryScope(size_t arraySize, TWriter* msgPackWriter, SerializationContext& serializationContext) noexcept
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mMsgPackWriter(msgPackWriter)
		, mSize(arraySize)
	{ }

	template <typename T, std::enable_if_t<std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>, int> = 0>
	bool SerializeValue(T& value)
	{
		if (mIndex == mSize)
		{
			throw SerializationException(SerializationErrorCode::OutOfRange, "Attempt to write more bytes than was declared for that binary array");
		}

		mMsgPackWriter->WriteBinary(static_cast<char>(value));
		++mIndex;
		return true;
	}

private:
	TWriter* mMsgPackWriter;
	size_t mSize;
	size_t mIndex = 0;
};


/**
 * @brief MsgPack scope for writing arrays (sequential values).
 */
template <class TWriter>
class CMsgPackWriteArrayScope final : public MsgPackArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	CMsgPackWriteArrayScope(size_t arraySize, TWriter* msgPackWriter, SerializationContext& serializationContext) noexcept
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mMsgPackWriter(msgPackWriter)
		, mSize(arraySize)
	{ }

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>
		|| std::is_same_v<T, std::string_view> || std::is_same_v<T, CBinTimestamp>, int> = 0>
	bool SerializeValue(T& value)
	{
		CheckEnd();
		mMsgPackWriter->WriteValue(value);
		++mIndex;
		return true;
	}

	[[nodiscard]] std::optional<CMsgPackWriteArrayScope<TWriter>> OpenArrayScope(size_t arraySize)
	{
		CheckEnd();
		mMsgPackWriter->BeginArray(arraySize);
		++mIndex;
		return std::make_optional<CMsgPackWriteArrayScope<TWriter>>(arraySize, mMsgPackWriter, GetContext());
	}

	[[nodiscard]] std::optional<CMsgPackWriteObjectScope<TWriter>> OpenObjectScope(size_t mapSize)
	{
		CheckEnd();
		mMsgPackWriter->BeginMap(mapSize);
		++mIndex;
		return std::make_optional<CMsgPackWriteObjectScope<TWriter>>(mapSize, mMsgPackWriter, GetContext());
	}

	[[nodiscard]] std::optional<CMsgPackWriteBinaryScope<TWriter>> OpenBinaryScope(size_t binarySize)
	{
		CheckEnd();
		mMsgPackWriter->BeginBinary(binarySize);
		++mIndex;
		return std::make_optional<CMsgPackWriteBinaryScope<TWriter>>(binarySize, mMsgPackWriter, GetContext());
	}

private:
	void CheckEnd() const
	{
		if (mIndex == mSize)
		{
			throw SerializationException(SerializationErrorCode::OutOfRange, "Attempt to write more elements than was stated for that array");
		}
	}

	TWriter* mMsgPackWriter;
	size_t mSize;
	size_t mIndex = 0;
};


/**
 * @brief MsgPack scope for writing objects (key-value pairs).
 */
template <class TWriter>
class CMsgPackWriteObjectScope final : public MsgPackArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	CMsgPackWriteObjectScope(size_t mapSize, TWriter* msgPackWriter, SerializationContext& serializationContext) noexcept
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mMsgPackWriter(msgPackWriter)
		, mSize(mapSize)
	{ }

	template <typename TKey, typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>
		|| std::is_same_v<T, string_view_type> || std::is_same_v<T, CBinTimestamp>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		CheckEnd();
		mMsgPackWriter->WriteValue(key);
		mMsgPackWriter->WriteValue(value);
		++mIndex;
		return true;
	}

	template <typename TKey>
	std::optional<CMsgPackWriteArrayScope<TWriter>> OpenArrayScope(TKey&& key, size_t arraySize)
	{
		CheckEnd();
		mMsgPackWriter->WriteValue(key);
		mMsgPackWriter->BeginArray(arraySize);
		++mIndex;
		return std::make_optional<CMsgPackWriteArrayScope<TWriter>>(arraySize, mMsgPackWriter, GetContext());
	}

	template <typename TKey>
	[[nodiscard]] std::optional<CMsgPackWriteObjectScope<TWriter>> OpenObjectScope(TKey&& key, size_t mapSize)
	{
		CheckEnd();
		mMsgPackWriter->WriteValue(key);
		mMsgPackWriter->BeginMap(mapSize);
		++mIndex;
		return std::make_optional<CMsgPackWriteObjectScope<TWriter>>(mapSize, mMsgPackWriter, GetContext());
	}

	template <typename TKey>
	[[nodiscard]] std::optional<CMsgPackWriteBinaryScope<TWriter>> OpenBinaryScope(TKey&& key, size_t binarySize)
	{
		CheckEnd();
		mMsgPackWriter->WriteValue(key);
		mMsgPackWriter->BeginBinary(binarySize);
		++mIndex;
		return std::make_optional<CMsgPackWriteBinaryScope<TWriter>>(binarySize, mMsgPackWriter, GetContext());
	}

private:
	void CheckEnd() const
	{
		if (mIndex == mSize)
		{
			throw SerializationException(SerializationErrorCode::OutOfRange, "Attempt to write more items than was stated for that map");
		}
	}

	TWriter* mMsgPackWriter;
	size_t mSize;
	size_t mIndex = 0;
};


/**
 * @brief MsgPack root scope for writing data (can write array or object).
 */
class BITSERIALIZER_API MsgPackWriteRootScope final : public MsgPackArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	MsgPackWriteRootScope(std::string& outputData, SerializationContext& serializationContext);
	MsgPackWriteRootScope(std::ostream& outputStream, SerializationContext& serializationContext);
	~MsgPackWriteRootScope();

	MsgPackWriteRootScope(MsgPackWriteRootScope&&) = delete;
	MsgPackWriteRootScope& operator=(MsgPackWriteRootScope&&) = delete;
	MsgPackWriteRootScope(const MsgPackWriteRootScope&) = delete;
	MsgPackWriteRootScope& operator=(const MsgPackWriteRootScope&) = delete;

	/**
	 * @brief Gets the current path in MsgPack.
	 */
	[[nodiscard]] static constexpr std::string_view GetPath() noexcept
	{
		return {};
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>
		|| std::is_same_v<T, string_view_type> || std::is_same_v<T, CBinTimestamp>, int> = 0>
	bool SerializeValue(T& value)
	{
		mMsgPackWriter->WriteValue(value);
		return true;
	}

	[[nodiscard]] std::optional<CMsgPackWriteArrayScope<IMsgPackWriter>> OpenArrayScope(size_t arraySize) const
	{
		mMsgPackWriter->BeginArray(arraySize);
		return std::make_optional<CMsgPackWriteArrayScope<IMsgPackWriter>>(arraySize, mMsgPackWriter, GetContext());
	}

	[[nodiscard]] std::optional<CMsgPackWriteObjectScope<IMsgPackWriter>> OpenObjectScope(size_t mapSize) const
	{
		mMsgPackWriter->BeginMap(mapSize);
		return std::make_optional<CMsgPackWriteObjectScope<IMsgPackWriter>>(mapSize, mMsgPackWriter, GetContext());
	}

	[[nodiscard]] std::optional<CMsgPackWriteBinaryScope<IMsgPackWriter>> OpenBinaryScope(size_t binarySize) const
	{
		mMsgPackWriter->BeginBinary(binarySize);
		return std::make_optional<CMsgPackWriteBinaryScope<IMsgPackWriter>>(binarySize, mMsgPackWriter, GetContext());
	}

	static constexpr void Finalize() noexcept { /* Not required */ }

private:
	IMsgPackWriter* mMsgPackWriter = nullptr;
};


//-----------------------------------------------------------------------------
// MsgPack readers
//-----------------------------------------------------------------------------

// Forward declarations
template <class TReader> class CMsgPackReadObjectScope;


class CMsgPackScopeBase : public MsgPackArchiveTraits
{
public:
	CMsgPackScopeBase(CMsgPackScopeBase* parentScope = nullptr) noexcept
		: mParentScope(parentScope)
	{ }

	CMsgPackScopeBase(const CMsgPackScopeBase&) = delete;
	CMsgPackScopeBase(CMsgPackScopeBase&&) noexcept = delete;
	CMsgPackScopeBase& operator=(const CMsgPackScopeBase&) = delete;
	CMsgPackScopeBase& operator=(CMsgPackScopeBase&&) noexcept = default;

	/**
	 * @brief Gets the current path in MsgPack.
	 */
	[[nodiscard]] virtual std::string GetPath() const
	{
		std::string path = mParentScope ? mParentScope->GetPath() : std::string();
		return path;
	}

	virtual void OnFinishChildScope() {}

protected:
	~CMsgPackScopeBase()
	{
		if (mParentScope) {
			mParentScope->OnFinishChildScope();
		}
	}

private:
	CMsgPackScopeBase* mParentScope;
};


/**
 * @brief MsgPack scope for reading binary arrays.
 */
template <class TReader>
class CMsgPackReadBinaryScope final : public CMsgPackScopeBase, public TArchiveScope<SerializeMode::Load>
{
public:
	CMsgPackReadBinaryScope(size_t arraySize, TReader* msgPackReader, SerializationContext& serializationContext, CMsgPackScopeBase* parentScope = nullptr) noexcept
		: CMsgPackScopeBase(parentScope)
		, TArchiveScope<SerializeMode::Load>(serializationContext)
		, mMsgPackReader(msgPackReader)
		, mSize(arraySize)
	{ }

	/**
	 * @brief Gets the current path in MsgPack.
	 */
	[[nodiscard]] std::string GetPath() const override
	{
		std::string path = CMsgPackScopeBase::GetPath();
		path.push_back(path_separator);
		path += Convert::ToString(mIndex);
		return path;
	}

	template <typename T, std::enable_if_t<std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>, int> = 0>
	bool SerializeValue(T& value)
	{
		CheckEnd();
		value = static_cast<T>(mMsgPackReader->ReadBinary());
		++mIndex;
		return true;
	}

	/**
	 * @brief Returns the estimated number of items to load (for reserving the size of containers).
	 */
	[[nodiscard]] size_t GetEstimatedSize() const noexcept
	{
		return mSize;
	}

	/**
	 * @brief Returns `true` when there are no more values to load.
	 */
	[[nodiscard]] bool IsEnd() const noexcept
	{
		return mIndex == mSize;
	}

private:
	void CheckEnd() const
	{
		if (IsEnd()) {
			throw SerializationException(SerializationErrorCode::OutOfRange, "No more items to load");
		}
	}

	TReader* mMsgPackReader;
	const size_t mSize;
	size_t mIndex = 0;
};


/**
 * @brief MsgPack scope for reading arrays (sequential values).
 */
template <class TReader>
class CMsgPackReadArrayScope final : public CMsgPackScopeBase, public TArchiveScope<SerializeMode::Load>
{
public:
	CMsgPackReadArrayScope(size_t arraySize, TReader* msgPackReader, SerializationContext& serializationContext, CMsgPackScopeBase* parentScope = nullptr) noexcept
		: CMsgPackScopeBase(parentScope)
		, TArchiveScope<SerializeMode::Load>(serializationContext)
		, mMsgPackReader(msgPackReader)
		, mSize(arraySize)
	{ }

	/**
	 * @brief Gets the current path in MsgPack.
	 */
	[[nodiscard]] std::string GetPath() const override
	{
		std::string path = CMsgPackScopeBase::GetPath();
		path.push_back(path_separator);
		path += Convert::ToString(mIndex);
		return path;
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>
		|| std::is_same_v<T, string_view_type> || std::is_same_v<T, CBinTimestamp>, int> = 0>
	bool SerializeValue(T& value)
	{
		CheckEnd();
		if (mMsgPackReader->ReadValue(value))
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
		return mSize;
	}

	/**
	 * @brief Returns `true` when there are no more values to load.
	 */
	[[nodiscard]] bool IsEnd() const noexcept
	{
		return mIndex == mSize;
	}

	std::optional<CMsgPackReadArrayScope<TReader>> OpenArrayScope(size_t)
	{
		CheckEnd();
		if (size_t sz = 0; mMsgPackReader->ReadArraySize(sz))
		{
			++mIndex;
			return std::make_optional<CMsgPackReadArrayScope<TReader>>(sz, mMsgPackReader, GetContext(), this);
		}
		return std::nullopt;
	}

	std::optional<CMsgPackReadObjectScope<TReader>> OpenObjectScope(size_t)
	{
		CheckEnd();
		if (size_t sz = 0; mMsgPackReader->ReadMapSize(sz))
		{
			++mIndex;
			return std::make_optional<CMsgPackReadObjectScope<TReader>>(sz, mMsgPackReader, GetContext(), this);
		}
		return std::nullopt;
	}

	[[nodiscard]] std::optional<CMsgPackReadBinaryScope<TReader>> OpenBinaryScope(size_t) const
	{
		if (size_t sz = 0; mMsgPackReader->ReadBinarySize(sz)) {
			return std::make_optional<CMsgPackReadBinaryScope<TReader>>(sz, mMsgPackReader, GetContext());
		}
		return std::nullopt;
	}

private:
	void CheckEnd() const
	{
		if (IsEnd()) {
			throw SerializationException(SerializationErrorCode::OutOfRange, "No more items to load");
		}
	}

	TReader* mMsgPackReader;
	size_t mSize;
	size_t mIndex = 0;
};


/**
 * @brief MsgPack scope for reading objects (key-value pairs).
 */
template <class TReader>
class CMsgPackReadObjectScope final : public CMsgPackScopeBase, public TArchiveScope<SerializeMode::Load>
{
public:
	CMsgPackReadObjectScope(size_t mapSize, TReader* msgPackReader, SerializationContext& serializationContext, CMsgPackScopeBase* parentScope = nullptr) noexcept
		: CMsgPackScopeBase(parentScope)
		, TArchiveScope<SerializeMode::Load>(serializationContext)
		, mMsgPackReader(msgPackReader)
		, mStartPos(msgPackReader->GetPosition())
		, mSize(mapSize)
	{ }

	~CMsgPackReadObjectScope()
	{
		ResetKey();
		// Skip key/values that was not read
		for (size_t c = mIndex; c < mSize; ++c)
		{
			mMsgPackReader->SkipValue();
			mMsgPackReader->SkipValue();
			++mIndex;
		}
	}

	/**
	 * @brief Gets the current path in MsgPack.
	 */
	[[nodiscard]] std::string GetPath() const override
	{
		std::string path = CMsgPackScopeBase::GetPath();
		if (mCurrentKey)
		{
			path.push_back(path_separator);
			path += mCurrentKey.ToString();
		}
		return path;
	}

	/**
	 * @brief Returns the estimated number of items to load (for reserving the size of containers).
	 */
	[[nodiscard]] size_t GetEstimatedSize() const noexcept
	{
		return mSize;
	}

	/**
	 * @brief Enumerates all keys in the current object.
	 *
	 * @tparam TCallback Callback function type.
	 * @param fn Callback to invoke for each key.
	 */
	template <typename TCallback>
	void VisitKeys(TCallback&& fn)
	{
		ResetKey();
		mMsgPackReader->SetPosition(mStartPos);
		for (mIndex = 0; mIndex < mSize;)
		{
			ReadKey(fn);
			ResetKey();
		}
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_fundamental_v<T> || std::is_null_pointer_v<T>
		|| std::is_same_v<T, string_view_type> || std::is_same_v<T, CBinTimestamp>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if (FindValueByKey(key))
		{
			mCurrentKey.Reset();
			++mIndex;
			return mMsgPackReader->ReadValue(value);
		}
		return false;
	}

	template <typename TKey>
	std::optional<CMsgPackReadArrayScope<TReader>> OpenArrayScope(TKey&& key, size_t)
	{
		if (FindValueByKey(key))
		{
			if (size_t sz = 0; mMsgPackReader->ReadArraySize(sz)) {
				return std::make_optional<CMsgPackReadArrayScope<TReader>>(sz, mMsgPackReader, GetContext(), this);
			}
			OnFinishChildScope();
		}
		return std::nullopt;
	}

	template <typename TKey>
	std::optional<CMsgPackReadObjectScope<TReader>> OpenObjectScope(TKey&& key, size_t)
	{
		if (FindValueByKey(key))
		{
			if (size_t sz = 0; mMsgPackReader->ReadMapSize(sz)) {
				return std::make_optional<CMsgPackReadObjectScope<TReader>>(sz, mMsgPackReader, GetContext(), this);
			}
			OnFinishChildScope();
		}
		return std::nullopt;
	}

	template <typename TKey>
	std::optional<CMsgPackReadBinaryScope<TReader>> OpenBinaryScope(TKey&& key, size_t)
	{
		if (FindValueByKey(key))
		{
			if (size_t sz = 0; mMsgPackReader->ReadBinarySize(sz)) {
				return std::make_optional<CMsgPackReadBinaryScope<TReader>>(sz, mMsgPackReader, GetContext(), this);
			}
			OnFinishChildScope();
		}
		return std::nullopt;
	}

	void OnFinishChildScope() override
	{
		mCurrentKey.Reset();
		++mIndex;
	}

private:
	template <typename TCallback>
	void ReadKey(TCallback&& callback)
	{
		switch (mMsgPackReader->ReadValueType())
		{
		case ValueType::String:
			if (auto& ref = mCurrentKey.GetValueRef<std::string_view>(); mMsgPackReader->ReadValue(ref)) {
				callback(ref);
			}
			break;
		case ValueType::UnsignedInteger:
			if (auto& ref = mCurrentKey.GetValueRef<uint64_t>(); mMsgPackReader->ReadValue(ref)) {
				callback(ref);
			}
			break;
		case ValueType::SignedInteger:
			if (auto& ref = mCurrentKey.GetValueRef<int64_t>(); mMsgPackReader->ReadValue(ref)) {
				callback(ref);
			}
			break;
		case ValueType::Double:
			if (auto& ref = mCurrentKey.GetValueRef<double>(); mMsgPackReader->ReadValue(ref)) {
				callback(ref);
			}
			break;
		case ValueType::Float:
			if (auto& ref = mCurrentKey.GetValueRef<float>(); mMsgPackReader->ReadValue(ref)) {
				return callback(ref);
			}
			break;
		case ValueType::Timestamp:
			if (auto& ref = mCurrentKey.GetValueRef<CBinTimestamp>(); mMsgPackReader->ReadValue(ref)) {
				return callback(ref);
			}
			break;
		default:
			throw ParsingException("Unsupported key type");
		}
	}

	template <typename TKey>
	bool FindValueByKey(TKey&& key)
	{
		if (mCurrentKey)
		{
			if (mCurrentKey == key) {
				return true;
			}
			ResetKey();
		}

		for (size_t c = 0; c < mSize; ++c)
		{
			if (mIndex == mSize)
			{
				mMsgPackReader->SetPosition(mStartPos);
				mIndex = 0;
			}
			ReadKey([](auto&&) {});
			if (mCurrentKey == key) {
				return true;
			}
			mMsgPackReader->SkipValue();
			++mIndex;
		}
		mCurrentKey.Reset();
		return false;
	}

	void ResetKey()
	{
		if (mCurrentKey)
		{
			mCurrentKey.Reset();
			mMsgPackReader->SkipValue();
			++mIndex;
		}
	}

	TReader* mMsgPackReader;
	size_t mStartPos;
	const size_t mSize;
	size_t mIndex = 0;
	MsgPackVariableKey mCurrentKey;
};


/**
 * @brief MsgPack root scope for reading data (can read array or object).
 */
class BITSERIALIZER_API MsgPackReadRootScope final : public MsgPackArchiveTraits, public TArchiveScope<SerializeMode::Load>
{
public:
	MsgPackReadRootScope(std::string_view inputData, SerializationContext& serializationContext);
	MsgPackReadRootScope(std::istream& inputStream, SerializationContext& serializationContext);
	~MsgPackReadRootScope();

	/**
	 * @brief Gets the current path in MsgPack.
	 */
	[[nodiscard]] static constexpr std::string_view GetPath() noexcept
	{
		return {};
	}

	template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_null_pointer_v<T>
		|| std::is_same_v<T, string_view_type> || std::is_same_v<T, CBinTimestamp>, int> = 0>
	bool SerializeValue(T& value) const
	{
		return mMsgPackReader->ReadValue(value);
	}

	[[nodiscard]] std::optional<CMsgPackReadArrayScope<IMsgPackReader>> OpenArrayScope(size_t) const
	{
		if (size_t sz = 0; mMsgPackReader->ReadArraySize(sz)) {
			return std::make_optional<CMsgPackReadArrayScope<IMsgPackReader>>(sz, mMsgPackReader, GetContext());
		}
		return std::nullopt;
	}

	[[nodiscard]] std::optional<CMsgPackReadObjectScope<IMsgPackReader>> OpenObjectScope(size_t) const
	{
		if (size_t sz = 0; mMsgPackReader->ReadMapSize(sz)) {
			return std::make_optional<CMsgPackReadObjectScope<IMsgPackReader>>(sz, mMsgPackReader, GetContext());
		}
		return std::nullopt;
	}

	[[nodiscard]] std::optional<CMsgPackReadBinaryScope<IMsgPackReader>> OpenBinaryScope(size_t) const
	{
		if (size_t sz = 0; mMsgPackReader->ReadBinarySize(sz)) {
			return std::make_optional<CMsgPackReadBinaryScope<IMsgPackReader>>(sz, mMsgPackReader, GetContext());
		}
		return std::nullopt;
	}

	static constexpr void Finalize() noexcept { /* Not required */ }

private:
	IMsgPackReader* mMsgPackReader = nullptr;
};

}


/**
 * @brief MsgPack archive.
 *
 * Supports load/save from:
 * - `std::string`
 * - `std::istream` and `std::ostream`
 */
using MsgPackArchive = TArchiveBase<
	Detail::MsgPackArchiveTraits,
	Detail::MsgPackReadRootScope,
	Detail::MsgPackWriteRootScope>;

} // namespace BitSerializer::MsgPack
