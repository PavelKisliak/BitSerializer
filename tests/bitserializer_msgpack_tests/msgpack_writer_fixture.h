/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include <variant>
#include "gtest/gtest.h"
#include "msgpack/msgpack_writers.h"

template <class TWriter>
class MsgPackWriterTest : public ::testing::Test
{
public:
	MsgPackWriterTest()
	{
		if constexpr (std::is_same_v<TWriter, BitSerializer::MsgPack::Detail::CMsgPackStringWriter>)
		{
			mResult = std::string();
			mMsgPackWriter = std::make_shared<TWriter>(std::get<std::string>(mResult));
		}
		else if constexpr (std::is_same_v<TWriter, BitSerializer::MsgPack::Detail::CMsgPackStreamWriter>)
		{
			mResult = std::ostringstream();
			mMsgPackWriter = std::make_shared<TWriter>(std::get<std::ostringstream>(mResult));
		}
	}

	std::string TakeResult()
	{
		return std::visit([this](auto&& arg)
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
	std::shared_ptr<TWriter> mMsgPackWriter;
	std::variant<std::string, std::ostringstream> mResult;
};
