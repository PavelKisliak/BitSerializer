/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include <variant>
#include "gtest/gtest.h"
#include "csv/csv_writers.h"

template <class TWriter>
class CsvWriterTest : public ::testing::Test
{
public:
	void PrepareCsvReader(bool withHeader, char separator = ',',
		bool writeBom = false, BitSerializer::Convert::UtfType utfType = BitSerializer::Convert::UtfType::Utf8)
	{
		if constexpr (std::is_same_v<TWriter, BitSerializer::Csv::Detail::CCsvStringWriter>)
		{
			mResult = std::string();
			mCsvWriter = std::make_shared<TWriter>(std::get<std::string>(mResult), withHeader, separator);
		}
		else if constexpr (std::is_same_v<TWriter, BitSerializer::Csv::Detail::CCsvStreamWriter>)
		{
			BitSerializer::StreamOptions streamOptions;
			streamOptions.writeBom = writeBom;
			streamOptions.encoding = utfType;
			mResult = std::ostringstream();
			mCsvWriter = std::make_shared<TWriter>(std::get<std::ostringstream>(mResult), withHeader, separator, streamOptions);
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

	[[nodiscard]] bool IsStreamWriter() const
	{
		return std::holds_alternative<std::ostringstream>(mResult);
	}

protected:
	std::shared_ptr<TWriter> mCsvWriter;
	std::variant<std::string, std::ostringstream> mResult;
};
