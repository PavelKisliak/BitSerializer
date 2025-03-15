/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include <gtest/gtest.h>
#include "bitserializer/convert.h"

template <class TTargetCharType>
class EncodedStreamReaderTest : public ::testing::Test
{
public:
	// Use minimal chunk size for simplify testing all streaming cases
	using reader_type = BitSerializer::Convert::Utf::CEncodedStreamReader<TTargetCharType, 32>;
	using target_string_type = std::basic_string<TTargetCharType, std::char_traits<TTargetCharType>>;

	template <typename TSourceUtfType>
	void PrepareEncodedStreamReader(std::u32string_view testStr, bool addBom = false,
		BitSerializer::Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy = BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip,
		const TTargetCharType* errorMark = BitSerializer::Convert::Utf::Detail::GetDefaultErrorMark<TTargetCharType>())
	{
		using source_char_type = typename TSourceUtfType::char_type;
		using source_string_type = std::basic_string<source_char_type, std::char_traits<source_char_type>>;

		// Prepare expected string in the native UTF encoding
		mExpectedString = BitSerializer::Convert::To<target_string_type>(testStr);

		// Encode test string to specified UTF which will be used as source of stream
		if (addBom) {
			mInputString.append(TSourceUtfType::bom, sizeof TSourceUtfType::bom);
		}
		source_string_type sourceEncodedString;
		TSourceUtfType::Encode(testStr.cbegin(), testStr.cend(), sourceEncodedString);
		mInputString.append(reinterpret_cast<const char*>(sourceEncodedString.data()), sourceEncodedString.size() * sizeof(source_char_type));

		// Prepare stream reader
		mInputStream = std::stringstream(mInputString);
		mEncodedStreamReader = std::make_shared<reader_type>(mInputStream, encodingErrorPolicy, errorMark);
	}

	void ReadFromStream()
	{
		static constexpr int MaxIterations = 100;

		for (int i = 0; i < MaxIterations; ++i)
		{
			const auto result = mEncodedStreamReader->ReadChunk(mActualString);
			if (result == BitSerializer::Convert::Utf::EncodedStreamReadResult::EndFile)
			{
				break;
			}
			ASSERT_TRUE(result != BitSerializer::Convert::Utf::EncodedStreamReadResult::DecodeError);
			// For prevent infinite loop when something went wrong
			ASSERT_TRUE(i < 100);
		}
	}

protected:
	std::string mInputString;
	std::stringstream mInputStream;
	std::shared_ptr<reader_type> mEncodedStreamReader;
	target_string_type mExpectedString;
	target_string_type mActualString;
};
