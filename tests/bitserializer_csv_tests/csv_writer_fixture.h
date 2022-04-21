/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include "gtest/gtest.h"
#include "bitserializer/csv_archive.h"

template <class TWriter>
class CsvWriterTest : public ::testing::Test
{
public:
	void PrepareCsvReader(bool withHeader, char separator = ',')
	{
		if constexpr (std::is_same_v<TWriter, BitSerializer::Csv::Detail::CCsvStringWriter>)
		{
			mResult = std::string();
			mCsvWriter = std::make_shared<TWriter>(std::get<std::string>(mResult), withHeader, separator);
		}
		else if constexpr (std::is_same_v<TWriter, BitSerializer::Csv::Detail::CCsvStreamWriter>)
		{
			mResult = std::ostringstream();
			mCsvWriter = std::make_shared<TWriter>(std::get<std::ostringstream>(mResult), withHeader, separator);
		}
	}

	std::string GetResult()
	{
		return std::visit([this](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::string>)
			{
				return arg;
			}
			else if constexpr (std::is_same_v<T, std::ostringstream>)
			{
				return arg.str();
			}
		}, mResult);
	}

protected:
	std::shared_ptr<TWriter> mCsvWriter;
	std::variant<std::string, std::ostringstream> mResult;
};
