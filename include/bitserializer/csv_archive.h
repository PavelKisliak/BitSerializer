/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <vector>
#include <optional>
#include <type_traits>
#include <variant>
#include "bitserializer/serialization_detail/archive_base.h"
#include "bitserializer/serialization_detail/errors_handling.h"


namespace BitSerializer::Csv {
namespace Detail {

/// <summary>
/// The traits of CSV archive (internal implementation - no dependencies)
/// </summary>
struct CsvArchiveTraits
{
	static constexpr ArchiveType archive_type = ArchiveType::Csv;
	using key_type = std::string;
	using supported_key_types = TSupportedKeyTypes<const char*, std::string_view, key_type>;
	using preferred_output_format = std::basic_string<char, std::char_traits<char>>;
	using preferred_stream_char_type = char;
	static constexpr char path_separator = '/';

protected:
	~CsvArchiveTraits() = default;
};

class ICsvWriter
{
public:
	virtual void SetEstimatedSize(size_t size) = 0;
	virtual void WriteValue(std::string_view key, const std::string& value) = 0;
	virtual void NextLine() = 0;
	[[nodiscard]] virtual size_t GetCurrentIndex() const = 0;

protected:
	~ICsvWriter() = default;
};

class ICsvReader
{
public:
	[[nodiscard]] virtual size_t GetCurrentIndex() const = 0;
	[[nodiscard]] virtual bool IsEnd() const = 0;
	virtual bool ReadValue(std::string_view key, std::string& value) = 0;
	virtual void ReadValue(std::string& value) = 0;
	virtual bool ParseNextRow() = 0;

protected:
	~ICsvReader() = default;
};

//------------------------------------------------------------------------------

class CCsvStringWriter final : public ICsvWriter
{
public:
	CCsvStringWriter(std::string& outputString, bool withHeader, char separator = ',');

	void SetEstimatedSize(size_t size) override;
	void WriteValue(std::string_view key, const std::string& value) override;
	void NextLine() override;
	[[nodiscard]] size_t GetCurrentIndex() const override { return mRowIndex; }

private:
	std::string& mOutputString;
	const bool mWithHeader;
	const char mSeparator;

	std::string mCsvHeader;
	std::string mCurrentRow;
	size_t mRowIndex = 0;
	size_t mEstimatedSize = 0;
	size_t mPrevValuesCount = 0;
};

class CCsvStreamWriter final : public ICsvWriter
{
public:
	CCsvStreamWriter(std::ostream& outputStream, bool withHeader, char separator = ',');

	void SetEstimatedSize(size_t size) override { /* Not required for stream */ }
	void WriteValue(std::string_view key, const std::string& value) override;
	void NextLine() override;
	[[nodiscard]] size_t GetCurrentIndex() const override { return mRowIndex; }

private:
	std::ostream& mOutputStream;
	const bool mWithHeader;
	const char mSeparator;

	std::string mCsvHeader;
	std::string mCurrentRow;
	size_t mRowIndex = 0;
	size_t mPrevValuesCount = 0;
};


/// <summary>
/// CSV scope for writing objects (list of values with keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
class CCsvWriteObjectScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	explicit CCsvWriteObjectScope(ICsvWriter* csvWriter)
		: mCsvWriter(csvWriter)
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
		mCsvWriter->WriteValue(std::forward<TKey>(key), Convert::ToString(value));
		return true;
	}

	template <typename TKey, typename TSym, typename TStrAllocator>
	bool SerializeValue(TKey&& key, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, char>)
		{
			mCsvWriter->WriteValue(std::forward<TKey>(key), value);
		}
		else
		{
			mCsvWriter->WriteValue(Convert::ToString(value));
		}
		return true;
	}

	template <typename TKey>
	bool SerializeValue(TKey&& key, std::nullptr_t&)
	{
		mCsvWriter->WriteValue(Convert::Detail::ToStringView(key), "");
		return true;
	}

private:
	ICsvWriter* mCsvWriter;
};

/// <summary>
/// CSV scope for serializing arrays (list of values without keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
class CsvWriteArrayScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	explicit CsvWriteArrayScope(ICsvWriter* csvWriter)
		: mCsvWriter(csvWriter)
	{ }

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] std::string GetPath() const
	{
		return path_separator + Convert::ToString(mCsvWriter->GetCurrentIndex());
	}

	std::optional<CCsvWriteObjectScope> OpenObjectScope()
	{
		return std::make_optional<CCsvWriteObjectScope>(mCsvWriter);
	}

private:
	ICsvWriter* mCsvWriter;
};


/// <summary>
/// CSV root scope (can write only array)
/// </summary>
class CsvWriteRootScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Save>
{
public:
	explicit CsvWriteRootScope(std::string& encodedOutputStr, const SerializationOptions& serializationOptions = {})
		: mCsvWriter(CCsvStringWriter(encodedOutputStr, true))
		, mSerializationOptions(serializationOptions)
	{ }

	CsvWriteRootScope(std::ostream& outputStream, const SerializationOptions& serializationOptions = {})
		: mCsvWriter(CCsvStreamWriter(outputStream, true))
		, mSerializationOptions(serializationOptions)
	{ }

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] std::string GetPath() const
	{
		return "";
	}

	std::optional<CsvWriteArrayScope> OpenArrayScope(size_t arraySize)
	{
		auto csvWriter = GetWriter();
		csvWriter->SetEstimatedSize(arraySize);
		return std::make_optional<CsvWriteArrayScope>(csvWriter);
	}

	void Finalize() const { /* Not required */ }

private:
	ICsvWriter* GetWriter()
	{
		return std::visit([](auto&& arg)
		{
			return static_cast<ICsvWriter*>(&arg);
		}, mCsvWriter);
	}

	std::variant<CCsvStringWriter, CCsvStreamWriter> mCsvWriter;
	SerializationOptions mSerializationOptions;
};

//------------------------------------------------------------------------------

class CCsvStringReader final : public ICsvReader
{
public:
	CCsvStringReader(std::string_view inputString, bool withHeader, char separator = ',');

	[[nodiscard]] size_t GetCurrentIndex() const override { return mRowIndex; }
	[[nodiscard]] bool IsEnd() const override;
	bool ReadValue(std::string_view key, std::string& value) override;
	void ReadValue(std::string& value) override;
	bool ParseNextRow() override;

private:
	bool ParseLine(std::vector<std::string>& out_values);

	std::string_view mSourceString;
	const bool mWithHeader;
	const char mSeparator;

	std::vector<std::string> mHeader;
	std::vector<std::string> mRowValues;
	size_t mCurrentPos = 0;
	size_t mLineNumber = 0;
	size_t mRowIndex = 0;
	size_t mValueIndex = 0;
	size_t mPrevValuesCount = 0;
};

class CCsvStreamReader final : public ICsvReader
{
public:
	CCsvStreamReader(std::istream& inputStream, bool withHeader, char separator = ',', size_t chunkSize = 128);

	[[nodiscard]] size_t GetCurrentIndex() const override { return mRowIndex; }
	[[nodiscard]] bool IsEnd() const override;
	bool ReadValue(std::string_view key, std::string& value) override;
	void ReadValue(std::string& value) override;
	bool ParseNextRow() override;

private:
	bool ParseLine(std::vector<std::string>& out_values);
	bool ReadNextChunk();
	void RemoveParsedStringPart();

	std::istream& mInput;
	std::string mBuffer;
	const bool mWithHeader;
	const char mSeparator;
	const size_t mChunkSize;

	std::vector<std::string> mHeader;
	std::vector<std::string> mRowValues;
	size_t mCurrentPos = 0;
	size_t mLineNumber = 0;
	size_t mRowIndex = 0;
	size_t mValueIndex = 0;
	size_t mPrevValuesCount = 0;
};


/// <summary>
/// CSV scope for reading objects (list of values with keys).
/// </summary>
/// <seealso cref="RapidJsonScopeBase" />
class CCsvReadObjectScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Load>
{
public:
	CCsvReadObjectScope(ICsvReader* csvReader)
		: mCsvReader(csvReader)
	{ }

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] std::string GetPath() const
	{
		return path_separator + Convert::ToString(mCsvReader->GetCurrentIndex());
	}

	template <typename TKey, typename TSym, typename TStrAllocator>
	bool SerializeValue(TKey&& key, std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>& value)
	{
		if constexpr (std::is_same_v<TSym, char>)
		{
			return mCsvReader->ReadValue(key, value);
		}
		else
		{
			if (std::string strValue; mCsvReader->ReadValue(key, strValue))
			{
				if (auto result = Convert::TryTo<std::basic_string<TSym, std::char_traits<TSym>, TStrAllocator>>(strValue); result.has_value())
				{
					value = std::move(result.value());
				}
			}
		}
		return false;
	}

	template <typename TKey, typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
	bool SerializeValue(TKey&& key, T& value)
	{
		std::string strValue;
		if (mCsvReader->ReadValue(key, strValue))
		{
			if (auto result = Convert::TryTo<T>(strValue); result.has_value())
			{
				value = result.value();
				return true;
			}
		}
		return false;
	}

	template <typename TKey>
	bool SerializeValue(TKey&& key, std::nullptr_t&)
	{
		std::string strValue;
		if (mCsvReader->ReadValue(key, strValue))
		{
			return strValue.empty();
		}
		return false;
	}

private:
	ICsvReader* mCsvReader;
};


/// <summary>
/// CSV scope for serializing arrays (list of values with keys).
/// </summary>
/// <seealso cref="TArchiveScope" />
class CsvReadArrayScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Load>
{
public:
	CsvReadArrayScope(ICsvReader* csvReader)
		: mCsvReader(csvReader)
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
	[[nodiscard]] size_t GetEstimatedSize() const {
		return 0;
	}

	/// <summary>
	/// Returns `true` when all no more values to load.
	/// </summary>
	[[nodiscard]] bool IsEnd() const
	{
		return mCsvReader->IsEnd();
	}

	std::optional<CCsvReadObjectScope> OpenObjectScope()
	{
		if (mCsvReader->ParseNextRow())
		{
			return std::make_optional<CCsvReadObjectScope>(mCsvReader);
		}
		return std::nullopt;
	}

private:
	ICsvReader* mCsvReader;
};


/// <summary>
/// CSV root scope (can read only array)
/// </summary>
class CsvReadRootScope final : public CsvArchiveTraits, public TArchiveScope<SerializeMode::Load>
{
public:
	explicit CsvReadRootScope(const std::string& encodedInputStr, const SerializationOptions& serializationOptions = {})
		: mCsvReader(CCsvStringReader(encodedInputStr, true, ','))
		, mSerializationOptions(serializationOptions)
	{ }

	explicit CsvReadRootScope(std::istream& encodedInputStream, const SerializationOptions& serializationOptions = {})
		: mCsvReader(CCsvStreamReader(encodedInputStream, true, ','))
		, mSerializationOptions(serializationOptions)
	{ }

	/// <summary>
	/// Gets the current path in CSV.
	/// </summary>
	[[nodiscard]] std::string GetPath() const
	{
		return "";
	}

	std::optional<CsvReadArrayScope> OpenArrayScope(size_t arraySize)
	{
		return std::make_optional<CsvReadArrayScope>(GetReader());
	}

	void Finalize() const { /* Not required */ }

private:
	ICsvReader* GetReader()
	{
		return std::visit([](auto&& arg)
		{
			return static_cast<ICsvReader*>(&arg);
		}, mCsvReader);
	}

	std::variant<CCsvStringReader, CCsvStreamReader> mCsvReader;
	SerializationOptions mSerializationOptions;
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
