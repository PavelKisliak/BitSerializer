/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include <variant>
#include "gtest/gtest.h"
#include "msgpack/msgpack_readers.h"


template <class TReader>
class MsgPackReaderTest : public ::testing::Test
{
public:
	void PrepareReader(std::string testMsgPack,
		BitSerializer::OverflowNumberPolicy overflowNumberPolicy = BitSerializer::OverflowNumberPolicy::ThrowError,
		BitSerializer::MismatchedTypesPolicy mismatchedTypesPolicy = BitSerializer::MismatchedTypesPolicy::ThrowError)
	{
		mTestMsgPack = std::move(testMsgPack);
		if constexpr (std::is_same_v<TReader, BitSerializer::MsgPack::Detail::CMsgPackStringReader>)
		{
			mMsgPackReader = std::make_shared<TReader>(mTestMsgPack, overflowNumberPolicy, mismatchedTypesPolicy);
		}
		//else if constexpr (std::is_same_v<TReader, BitSerializer::MsgPack::Detail::CMsgPackStreamReader>)
		//{
		//	mInputStream = std::make_optional<std::istringstream>(mTestMsgPack, overflowNumberPolicy);
		//	mCsvReader = std::make_shared<TReader>(mInputStream.value());
		//}
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
	std::string mTestMsgPack;
	std::shared_ptr<TReader> mMsgPackReader;
	std::optional<std::istringstream> mInputStream;
};