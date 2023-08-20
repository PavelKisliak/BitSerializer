/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include "gtest/gtest.h"
#include "csv/csv_readers.h"

template <class TReader>
class CsvReaderTest : public ::testing::Test
{
public:
	void PrepareCsvReader(std::string testCsv, bool withHeader, char separator = ',')
	{
		mTestCsv = std::move(testCsv);
		if constexpr (std::is_same_v<TReader, BitSerializer::Csv::Detail::CCsvStringReader>)
		{
			mCsvReader = std::make_shared<TReader>(mTestCsv, withHeader, separator);
		}
		else if constexpr (std::is_same_v<TReader, BitSerializer::Csv::Detail::CCsvStreamReader>)
		{
			mInputStream = std::make_optional<std::istringstream>(mTestCsv);
			mCsvReader = std::make_shared<TReader>(mInputStream.value(), withHeader, separator);
		}
	}

protected:
	std::string mTestCsv;
	std::shared_ptr<TReader> mCsvReader;
	std::optional<std::istringstream> mInputStream;
};
