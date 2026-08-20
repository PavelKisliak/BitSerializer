/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include <variant>
#include "gtest/gtest.h"
#include "json/json_string_writers.h"
#include "json/json_stream_writers.h"

template <class TWriter>
class JsonWriterTest : public ::testing::Test
{
public:
	JsonWriterTest()
	{
		if constexpr (std::is_same_v<TWriter, BitSerializer::Json::Detail::CJsonStringWriter>)
		{
			mResult.emplace<std::string>();
			mJsonWriter = std::make_shared<TWriter>(std::get<std::string>(mResult));
		}
		if constexpr (std::is_same_v<TWriter, BitSerializer::Json::Detail::CJsonStringPrettyWriter>)
		{
			mResult.emplace<std::string>();
			mJsonWriter = std::make_shared<TWriter>(std::get<std::string>(mResult), mPaddingChar, mPaddingCharNum);
		}
		if constexpr (std::is_same_v<TWriter, BitSerializer::Json::Detail::CJsonStreamWriter>)
		{
			mResult.emplace<std::ostringstream>();
			BitSerializer::StreamOptions streamOptions;
			streamOptions.writeBom = false;
			mJsonWriter = std::make_shared<TWriter>(std::get<std::ostringstream>(mResult), streamOptions);
		}
		if constexpr (std::is_same_v<TWriter, BitSerializer::Json::Detail::CJsonStreamPrettyWriter>)
		{
			mResult.emplace<std::ostringstream>();
			BitSerializer::StreamOptions streamOptions;
			streamOptions.writeBom = false;
			mJsonWriter = std::make_shared<TWriter>(std::get<std::ostringstream>(mResult), streamOptions, mPaddingChar, mPaddingCharNum);
		}
	}

	std::string TakeResult()
	{
		return std::visit([](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::string>)
			{
				std::string s;
				std::swap(s, arg);
				return s;
			}
			else if constexpr (std::is_same_v<T, std::ostringstream>)
			{
				std::string s = arg.str();
				arg.str("");
				return s;
			}
		}, mResult);
	}

	[[nodiscard]] bool IsStreamWriter() const
	{
		return std::holds_alternative<std::ostringstream>(mResult);
	}

	static std::string GenTestString(size_t size)
	{
		std::string testStr(size, '_');
		for (size_t i = 0; i < size; ++i) {
			testStr[i] = static_cast<char>('A' + i % 26);
		}
		return testStr;
	}

protected:
	char mPaddingChar = '\t';
	uint16_t mPaddingCharNum = 1;

	std::shared_ptr<TWriter> mJsonWriter;
	std::variant<std::string, std::ostringstream> mResult;
};
