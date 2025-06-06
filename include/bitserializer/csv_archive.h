﻿/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <optional>
#include <string>
#include <type_traits>
#include "bitserializer/export.h"
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/errors_handling.h"


namespace BitSerializer::Csv {
namespace Detail {

/// <summary>
/// The traits of CSV archive (internal implementation - no dependencies)
/// </summary>
struct CsvArchiveTraits  // NOLINT(cppcoreguidelines-special-member-functions)
{
	static constexpr ArchiveType archive_type = ArchiveType::Csv;
	using key_type = std::string;
	using supported_key_types = TSupportedKeyTypes<key_type, std::string_view>;
	using string_view_type = std::string_view;
	using preferred_output_format = std::basic_string<char, std::char_traits<char>>;
	using preferred_stream_char_type = char;
	static constexpr char path_separator = '/';
	static constexpr bool is_binary = false;

	static constexpr char allowed_separators[] = { ',', ';', '\t', ' ', '|' };

protected:
	~CsvArchiveTraits() = default;
};

class BITSERIALIZER_API ICsvWriter
{
public:
	virtual ~ICsvWriter() = default;

	virtual void SetEstimatedSize(size_t size) = 0;
	virtual void WriteValue(const std::string_view& key, std::string_view value) = 0;
	virtual void NextLine() = 0;
	[[nodiscard]] virtual size_t GetCurrentIndex() const noexcept = 0;
};

class BITSERIALIZER_API ICsvReader
{
public:
	virtual ~ICsvReader() = default;

	[[nodiscard]] virtual size_t GetCurrentIndex() const noexcept = 0;
	[[nodiscard]] virtual bool IsEnd() const = 0;
	virtual bool ReadValue(std::string_view key, std::string_view& out_value) = 0;
	virtual void ReadValue(std::string_view& out_value) = 0;
	virtual bool ParseNextRow() = 0;
	[[nodiscard]] virtual const std::vector<std::string>& GetHeaders() const noexcept = 0;
};


/// <summary>
/// CSV scope for writing objects (list of values with keys).
/// </summary>
class CCsvWriteObjectScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	explicit CCsvWriteObjectScope(ICsvWriter* csvWriter, SerializationContext& serializationContext) noexcept
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mCsvWriter(csvWriter)
	{ }

	~CCsvWriteObjectScope()
	{
		mCsvWriter->NextLine();
	}

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] std::string GetPath() const
	{
		return path_separator + Convert::ToString(mCsvWriter->GetCurrentIndex());
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		auto temp = Convert::ToString(value);
		mCsvWriter->WriteValue(std::forward<TKey>(key), std::string_view(temp));
		return true;
	}

	template <typename TKey>
	bool SerializeValue(TKey&& key, string_view_type& value)
	{
		mCsvWriter->WriteValue(std::forward<TKey>(key), value);
		return true;
	}

	template <typename TKey>
	bool SerializeValue(TKey&& key, std::nullptr_t&)
	{
		mCsvWriter->WriteValue(std::forward<TKey>(key), "");
		return true;
	}

private:
	ICsvWriter* mCsvWriter;
};

/// <summary>
/// CSV scope for serializing arrays (list of values without keys).
/// </summary>
class CsvWriteArrayScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	explicit CsvWriteArrayScope(ICsvWriter* csvWriter, SerializationContext& serializationContext) noexcept
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mCsvWriter(csvWriter)
	{ }

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] std::string GetPath() const
	{
		return path_separator + Convert::ToString(mCsvWriter->GetCurrentIndex());
	}

	[[nodiscard]] std::optional<CCsvWriteObjectScope> OpenObjectScope(size_t) const
	{
		return std::make_optional<CCsvWriteObjectScope>(mCsvWriter, GetContext());
	}

private:
	ICsvWriter* mCsvWriter;
};


/// <summary>
/// CSV root scope (can write only array)
/// </summary>
class BITSERIALIZER_API CsvWriteRootScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	CsvWriteRootScope(std::string& encodedOutputStr, SerializationContext& serializationContext);
	CsvWriteRootScope(std::ostream& outputStream, SerializationContext& serializationContext);
	~CsvWriteRootScope();

	CsvWriteRootScope(CsvWriteRootScope&&) = delete;
	CsvWriteRootScope& operator=(CsvWriteRootScope&&) = delete;
	CsvWriteRootScope(const CsvWriteRootScope&) = delete;
	CsvWriteRootScope& operator=(const CsvWriteRootScope&) = delete;

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] static constexpr std::string_view GetPath() noexcept
	{
		return {};
	}

	[[nodiscard]] std::optional<CsvWriteArrayScope> OpenArrayScope(size_t arraySize) const
	{
		mCsvWriter->SetEstimatedSize(arraySize);
		return std::make_optional<CsvWriteArrayScope>(mCsvWriter, GetContext());
	}

	void Finalize() const noexcept { /* Not required */ }

private:
	ICsvWriter* mCsvWriter = nullptr;
};


/// <summary>
/// CSV scope for reading objects (list of values with keys).
/// </summary>
class CCsvReadObjectScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Load>
{
public:
	CCsvReadObjectScope(ICsvReader* csvReader, SerializationContext& serializationContext) noexcept
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mCsvReader(csvReader)
	{ }

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] std::string GetPath() const
	{
		return path_separator + Convert::ToString(mCsvReader->GetCurrentIndex());
	}

	/// <summary>
	/// Returns the estimated number of items to load (for reserving the size of containers).
	/// </summary>
	[[nodiscard]] size_t GetEstimatedSize() const noexcept
	{
		return mCsvReader->GetHeaders().size();
	}

	/// <summary>
	/// Enumerates all keys by calling a passed function.
	/// </summary>
	template <typename TCallback>
	void VisitKeys(TCallback&& fn)
	{
		for (const auto& key : mCsvReader->GetHeaders()) {
			fn(key);
		}
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_fundamental_v<T>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		if (std::string_view strValue; mCsvReader->ReadValue(key, strValue))
		{
			if (strValue.empty())
			{
				// Empty string is treated as Null
				return std::is_null_pointer_v<T>;
			}

			try
			{
				value = Convert::To<T>(strValue);
				return true;
			}
			catch (const std::out_of_range&)
			{
				if (GetOptions().overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::Overflow,
						std::string("The size of target field '") + key + "' is not sufficient to deserialize number: " + std::string(strValue) +
						", line: " + Convert::ToString(mCsvReader->GetCurrentIndex()));
				}
			}
			catch (...)
			{
				if (GetOptions().mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::MismatchedTypes,
						std::string("The type of target field '") + key + "' does not match the value being loaded: " + std::string(strValue) +
						", line: " + Convert::ToString(mCsvReader->GetCurrentIndex()));
				}
			}
		}
		return false;
	}

	template <typename TKey>
	bool SerializeValue(TKey&& key, string_view_type& value)
	{
		return mCsvReader->ReadValue(key, value);
	}

	template <typename TKey>
	bool SerializeValue(TKey&& key, std::nullptr_t&)
	{
		if (std::string_view strValue; mCsvReader->ReadValue(key, strValue))
		{
			if (strValue.empty())
			{
				return true;
			}
			if (GetOptions().mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					std::string("The type of target field '") + key + "' does not match the value being loaded: " + std::string(strValue) +
					", line: " + Convert::ToString(mCsvReader->GetCurrentIndex()));
			}
			return false;
		}
		return false;
	}

private:
	ICsvReader* mCsvReader;
};


/// <summary>
/// CSV scope for serializing arrays (list of values with keys).
/// </summary>
class CsvReadArrayScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Load>
{
public:
	CsvReadArrayScope(ICsvReader* csvReader, SerializationContext& serializationContext) noexcept
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mCsvReader(csvReader)
	{ }

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] std::string GetPath() const
	{
		return path_separator + Convert::ToString(mCsvReader->GetCurrentIndex());
	}

	/// <summary>
	/// Returns the estimated number of items to load (for reserving the size of containers).
	/// </summary>
	[[nodiscard]] static constexpr size_t GetEstimatedSize() noexcept
	{
		return 0;
	}

	/// <summary>
	/// Returns `true` when all no more values to load.
	/// </summary>
	[[nodiscard]] bool IsEnd() const
	{
		return mCsvReader->IsEnd();
	}

	std::optional<CCsvReadObjectScope> OpenObjectScope(size_t)
	{
		if (mCsvReader->ParseNextRow())
		{
			return std::make_optional<CCsvReadObjectScope>(mCsvReader, GetContext());
		}
		return std::nullopt;
	}

private:
	ICsvReader* mCsvReader;
};


/// <summary>
/// CSV root scope (can read only array)
/// </summary>
class BITSERIALIZER_API CsvReadRootScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Load>
{
public:
	CsvReadRootScope(std::string_view encodedInputStr, SerializationContext& serializationContext);
	CsvReadRootScope(std::istream& encodedInputStream, SerializationContext& serializationContext);
	~CsvReadRootScope();

	CsvReadRootScope(CsvReadRootScope&&) = delete;
	CsvReadRootScope& operator=(CsvReadRootScope&&) = delete;
	CsvReadRootScope(const CsvReadRootScope&) = delete;
	CsvReadRootScope& operator=(const CsvReadRootScope&) = delete;

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] static constexpr std::string_view GetPath() noexcept
	{
		return {};
	}

	std::optional<CsvReadArrayScope> OpenArrayScope(size_t)
	{
		return std::make_optional<CsvReadArrayScope>(mCsvReader, GetContext());
	}

	void Finalize() const noexcept { /* Not required */ }

private:
	ICsvReader* mCsvReader = nullptr;
};

}


/// <summary>
/// CSV archive.
/// Supports load/save from:
/// - <c>std::string</c>: UTF-8
/// - <c>std::istream</c> and <c>std::ostream</c>: UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE
/// </summary>
using CsvArchive = TArchiveBase<
	Detail::CsvArchiveTraits,
	Detail::CsvReadRootScope,
	Detail::CsvWriteRootScope>;

}
