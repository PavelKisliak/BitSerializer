/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "csv_readers.h"
#include "csv_writers.h"


namespace BitSerializer::Csv::Detail
{
	CsvWriteRootScope::CsvWriteRootScope(std::string& encodedOutputStr, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mCsvWriter(std::make_unique<CCsvStringWriter>(encodedOutputStr, true))
	{ }

	CsvWriteRootScope::CsvWriteRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mCsvWriter(std::make_unique<CCsvStreamWriter>(outputStream, true, ',', serializationContext.GetOptions().streamOptions))
	{ }

	CsvReadRootScope::CsvReadRootScope(const std::string& encodedInputStr, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mCsvReader(std::make_unique<CCsvStringReader>(encodedInputStr, true, ','))
	{ }

	CsvReadRootScope::CsvReadRootScope(std::istream& encodedInputStream, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mCsvReader(std::make_unique<CCsvStreamReader>(encodedInputStream, true, ','))
	{ }
}
