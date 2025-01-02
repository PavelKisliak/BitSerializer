/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "csv_readers.h"
#include "csv_writers.h"
#include <algorithm>


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
		, mCsvWriter(std::make_unique<CCsvStringWriter>(encodedOutputStr, true, serializationContext.GetOptions().valuesSeparator))
	{
		ValidateSeparator(serializationContext.GetOptions().valuesSeparator);
	}

	CsvWriteRootScope::CsvWriteRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mCsvWriter(std::make_unique<CCsvStreamWriter>(outputStream, true, serializationContext.GetOptions().valuesSeparator, serializationContext.GetOptions().utfEncodingErrorPolicy, serializationContext.GetOptions().streamOptions))
	{
		ValidateSeparator(serializationContext.GetOptions().valuesSeparator);
	}

	CsvReadRootScope::CsvReadRootScope(std::string_view encodedInputStr, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mCsvReader(std::make_unique<CCsvStringReader>(encodedInputStr, true, serializationContext.GetOptions().valuesSeparator))
	{
		ValidateSeparator(serializationContext.GetOptions().valuesSeparator);
	}

	CsvReadRootScope::CsvReadRootScope(std::istream& encodedInputStream, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mCsvReader(std::make_unique<CCsvStreamReader>(encodedInputStream, true, serializationContext.GetOptions().valuesSeparator))
	{
		ValidateSeparator(serializationContext.GetOptions().valuesSeparator);
	}
}
