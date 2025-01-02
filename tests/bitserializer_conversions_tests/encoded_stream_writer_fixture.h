/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

template <class TUtfEncoder>
class EncodedStreamWriterTest : public ::testing::Test
{
public:
	using utf_encoder = TUtfEncoder;
	using encoded_char_type = typename TUtfEncoder::char_type;

	void SetUp() override
	{
		mEncodedStreamWriter = std::make_shared<BitSerializer::Convert::Utf::CEncodedStreamWriter>(mOutputStream, TUtfEncoder::utfType, false);
	}

	EncodedStreamWriterTest<TUtfEncoder>& WithBom(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy = BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip)
	{
		mWithBom = true;
		mUtfEncodingErrorPolicy = encodingErrorPolicy;
		mEncodedStreamWriter = std::make_shared<BitSerializer::Convert::Utf::CEncodedStreamWriter>(mOutputStream, TUtfEncoder::utfType, true, encodingErrorPolicy);
		return *this;
	}

	template <typename TCharType>
	BitSerializer::Convert::Utf::UtfEncodingErrorCode TestWrite(const std::basic_string_view<TCharType>& str)
	{
		// Encoding string for test assert
		if constexpr (sizeof(encoded_char_type) == 1 && sizeof(TCharType) == 1)
		{
			mExpectedString.assign(str);
		}
		else
		{
			mEncoder.Encode(str.data(), str.data() + str.size(), mExpectedString, mUtfEncodingErrorPolicy);
		}
		// Write to stream
		return mEncodedStreamWriter->Write(str);
	}

	template <typename TCharType>
	BitSerializer::Convert::Utf::UtfEncodingErrorCode TestWrite(const std::basic_string<TCharType>& str)
	{
		return TestWrite(std::basic_string_view<TCharType>(str));
	}

	template <typename TCharType>
	BitSerializer::Convert::Utf::UtfEncodingErrorCode TestWrite(const TCharType* str)
	{
		return TestWrite(std::basic_string_view<TCharType>(str));
	}

	void Assert()
	{
		constexpr auto encodedCharSize = sizeof(encoded_char_type);

		const std::string streamData = mOutputStream.str();
		size_t pos = 0;
		if (mWithBom)
		{
			constexpr auto bomSize = sizeof(TUtfEncoder::bom);
			const std::string_view expectedBom(TUtfEncoder::bom, bomSize);
			ASSERT_TRUE(bomSize <= streamData.size()) << "The number of encoded bytes is less than BOM size";
			
			const std::string_view actualBom(streamData.data(), bomSize);
			EXPECT_EQ(expectedBom, actualBom);
			pos += bomSize;
		}
		const size_t encodedChars = streamData.size() - pos;
		ASSERT_TRUE(encodedChars % encodedCharSize == 0) << "The number of encoded bytes is not a multiple of the character size";
		std::basic_string_view<encoded_char_type> encodedStr(reinterpret_cast<const encoded_char_type*>(&streamData[pos]), (streamData.size() - pos) / encodedCharSize);
		EXPECT_EQ(mExpectedString, encodedStr);
	}

protected:
	std::shared_ptr<BitSerializer::Convert::Utf::CEncodedStreamWriter> mEncodedStreamWriter;
	std::ostringstream mOutputStream;
	BitSerializer::Convert::Utf::UtfEncodingErrorPolicy mUtfEncodingErrorPolicy = BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip;
	TUtfEncoder mEncoder;
	std::basic_string<encoded_char_type> mExpectedString;
	bool mWithBom = false;
};
