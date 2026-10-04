/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include "gtest/gtest.h"
#include "json/json_string_readers.h"
#include "json/json_stream_readers.h"


template <class TReader>
class JsonReaderTest : public ::testing::Test
{
public:
	void PrepareReader(std::string testJson,
		BitSerializer::OverflowNumberPolicy overflowNumberPolicy = BitSerializer::OverflowNumberPolicy::ThrowError,
		BitSerializer::MismatchedTypesPolicy mismatchedTypesPolicy = BitSerializer::MismatchedTypesPolicy::ThrowError)
	{
		mTestJson = std::move(testJson);
		mSerializationOptions.overflowNumberPolicy = overflowNumberPolicy;
		mSerializationOptions.mismatchedTypesPolicy = mismatchedTypesPolicy;
		if constexpr (TReader::is_stream_based)
		{
			mInputStream = std::make_optional<std::istringstream>(mTestJson);
			mJsonReader = std::make_shared<TReader>(mInputStream.value(), mSerializationOptions);
		}
		else
		{
			mJsonReader = std::make_shared<TReader>(mTestJson, mSerializationOptions);
		}
	}

	void PrepareStreamReader(std::string sourceData, BitSerializer::SerializationOptions options = {})
	{
		mSerializationOptions = options;
		mInputStream = std::make_optional<std::istringstream>(std::move(sourceData));
		mJsonReader = std::make_shared<TReader>(mInputStream.value(), mSerializationOptions);
	}

	template <BitSerializer::ArchiveType TFormat>
	void PrepareReaderWithFeaturesImpl(std::string testJson)
	{
		mTestJson = std::move(testJson);
		mSerializationOptions = {};
		if constexpr (TReader::is_stream_based)
		{
			mInputStream = std::make_optional<std::istringstream>(mTestJson);
			mJsonReader = std::make_shared<BitSerializer::Json::Detail::CJsonStreamReader<TFormat>>(mInputStream.value(), mSerializationOptions);
		}
		else
		{
			mJsonReader = std::make_shared<BitSerializer::Json::Detail::CJsonStringReader<TFormat>>(mTestJson, mSerializationOptions);
		}
	}

	void PrepareReaderWithFeatures(std::string testJson, BitSerializer::ArchiveType format)
	{
		switch (format)
		{
		case BitSerializer::ArchiveType::Json:
			PrepareReaderWithFeaturesImpl<BitSerializer::ArchiveType::Json>(std::move(testJson));
			break;
		case BitSerializer::ArchiveType::Jsonc:
			PrepareReaderWithFeaturesImpl<BitSerializer::ArchiveType::Jsonc>(std::move(testJson));
			break;
		default:
			GTEST_FAIL() << "Unexpected ArchiveType value";
			break;
		}
	}

	void AssertReadKeyValue(std::string_view expectedKey, std::string_view expectedValue)
	{
		ASSERT_TRUE(mJsonReader->OpenObject());
		std::string_view key, value;
		mJsonReader->ReadKey(key);
		EXPECT_EQ(expectedKey, key);
		mJsonReader->ReadValue(value);
		EXPECT_EQ(expectedValue, value);
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
	std::string mTestJson;
	BitSerializer::SerializationOptions mSerializationOptions;
	std::shared_ptr<BitSerializer::Json::Detail::IJsonReader> mJsonReader;
	std::optional<std::istringstream> mInputStream;
};
