/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <gtest/gtest.h>
#include "bitserializer/conversion_detail/convert_utf.h"


template <typename TUtfType>
class DetectEncodingTest : public ::testing::Test
{
public:
	using utf_type = TUtfType;
	using target_char_type = typename TUtfType::char_type;

	void PrepareEncodedData(std::u32string_view testStr,
		BitSerializer::Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy = BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip,
		const target_char_type* errorMark = BitSerializer::Convert::Utf::Detail::GetDefaultErrorMark<target_char_type>())
	{
		// Encode string
		using target_string_type = std::basic_string<target_char_type, std::char_traits<target_char_type>>;
		target_string_type outputString;
		TUtfType::Encode(testStr.cbegin(), testStr.cend(), outputString, encodingErrorPolicy, errorMark);

		// Prepare byte streams
		mEncodedBuffer.append(reinterpret_cast<const char*>(outputString.data()), outputString.size() * sizeof(target_char_type));
	}

	template <size_t BomSize = 0>
	void AppendBom(const char(&bom)[BomSize])
	{
		mBomSize = BomSize;
		if (mBomSize) {
			mEncodedBuffer.append(bom, BomSize);
		}
	}

	void TestDetectInString()
	{
		size_t dataOffset = 0;
		const BitSerializer::Convert::Utf::UtfType expectedUtf = TUtfType::utfType;
		EXPECT_EQ(expectedUtf, BitSerializer::Convert::Utf::DetectEncoding(mEncodedBuffer, dataOffset));
		// Validate data offset
		EXPECT_EQ(mBomSize, dataOffset);
	}

	void TestDetectInStream(bool skipBom = true)
	{
		std::stringstream encodedStream(mEncodedBuffer);
		const BitSerializer::Convert::Utf::UtfType expectedUtf = TUtfType::utfType;
		const auto actualUtf = BitSerializer::Convert::Utf::DetectEncoding(encodedStream, skipBom);
		EXPECT_EQ(expectedUtf, actualUtf);
		// Validate stream position
		if (skipBom)
		{
			EXPECT_EQ(static_cast<std::stringstream::pos_type>(mBomSize), encodedStream.tellg());
		}
		else
		{
			EXPECT_EQ(0, encodedStream.tellg());
		}
	}

protected:
	size_t mBomSize = 0;
	std::string mEncodedBuffer;
};
