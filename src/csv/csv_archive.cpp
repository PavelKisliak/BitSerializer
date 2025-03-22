/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <algorithm>
#include <memory>
#include "csv_readers.h"
#include "csv_writers.h"


namespace
{
	void ValidateSeparator(const char separator)
	{
		using namespace BitSerializer;
		using namespace Csv::Detail;

		if (std::find(std::cbegin(CsvArchiveTraits::allowed_separators), std::cend(CsvArchiveTraits::allowed_separators), separator)
			== std::cend(CsvArchiveTraits::allowed_separators))
		{
			throw SerializationException(SerializationErrorCode::InvalidOptions,
				std::string("Unsupported value separator '") + separator + '\'');
		}
	}
}

namespace BitSerializer::Csv::Detail
{
	CsvWriteRootScope::CsvWriteRootScope(std::string& encodedOutputStr, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Save>(serializationContext)
	{
		ValidateSeparator(serializationContext.GetOptions().valuesSeparator);
		// Use `make_unique` to free memory gracefully when an exception occurs in the constructor
		mCsvWriter = std::make_unique<CCsvStringWriter>(encodedOutputStr, true, serializationContext.GetOptions().valuesSeparator).release();
	}

	CsvWriteRootScope::CsvWriteRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Save>(serializationContext)
	{
		ValidateSeparator(serializationContext.GetOptions().valuesSeparator);
		// Use `make_unique` to free memory gracefully when an exception occurs in the constructor
		mCsvWriter = std::make_unique<CCsvStreamWriter>(outputStream, true, serializationContext.GetOptions().valuesSeparator, serializationContext.GetOptions().utfEncodingErrorPolicy, serializationContext.GetOptions().streamOptions).release();
	}

	CsvWriteRootScope::~CsvWriteRootScope()
	{
		delete mCsvWriter;
	}

	CsvReadRootScope::CsvReadRootScope(std::string_view encodedInputStr, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
	{
		ValidateSeparator(serializationContext.GetOptions().valuesSeparator);
		// Use `make_unique` to free memory gracefully when an exception occurs in the constructor
		mCsvReader = std::make_unique<CCsvStringReader>(encodedInputStr, true, serializationContext.GetOptions().valuesSeparator).release();
	}

	CsvReadRootScope::CsvReadRootScope(std::istream& encodedInputStream, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
	{
		ValidateSeparator(serializationContext.GetOptions().valuesSeparator);
		// Use `make_unique` to free memory gracefully when an exception occurs in the constructor
		mCsvReader = std::make_unique<CCsvStreamReader>(encodedInputStream, true, serializationContext.GetOptions().valuesSeparator).release();
	}

	CsvReadRootScope::~CsvReadRootScope()
	{
		delete mCsvReader;
	}
}
