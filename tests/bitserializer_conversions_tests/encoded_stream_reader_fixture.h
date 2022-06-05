/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <memory>
#include <gtest/gtest.h>
#include "bitserializer/conversion_detail/convert_utf.h"

template <class TTargetUtfType>
class EncodedStreamReaderTest : public ::testing::Test
{
public:
	// Use minimal chunk size for simplify testing all streaming cases
	using reader_type = BitSerializer::Convert::CEncodedStreamReader<TTargetUtfType, 4>;
	using target_char_type = typename TTargetUtfType::char_type;
	using target_string_type = std::basic_string<target_char_type, std::char_traits<target_char_type>>;

	template <typename TSourceUtfType>
	void PrepareEncodedStreamReader(std::u32string_view testStr, bool addBom = false,
		BitSerializer::Convert::EncodeErrorPolicy encodeErrorPolicy = BitSerializer::Convert::EncodeErrorPolicy::WriteErrorMark,
		const target_char_type* errorMark = BitSerializer::Convert::Detail::GetDefaultErrorMark<target_char_type>())
	{
		using source_char_type = typename TSourceUtfType::char_type;
		using source_string_type = std::basic_string<source_char_type, std::char_traits<source_char_type>>;

		// Prepare expected string in target UTF encoding
		auto it = TTargetUtfType::Encode(testStr.cbegin(), testStr.cend(), mExpectedString, encodeErrorPolicy, errorMark);

		// Encode test string to specified UTF which will be used as source of stream
		if (addBom) {
			mInputString.append(TSourceUtfType::bom, sizeof TSourceUtfType::bom);
		}
		source_string_type sourceEncodedString;
		TSourceUtfType::Encode(testStr.cbegin(), testStr.cend(), sourceEncodedString);
		mInputString.append(reinterpret_cast<const char*>(sourceEncodedString.data()), sourceEncodedString.size() * sizeof(source_char_type));

		// Prepare stream reader
		mInputStream = std::stringstream(mInputString);
		mEncodedStreamReader = std::make_shared<reader_type>(mInputStream, encodeErrorPolicy, errorMark);
	}

	void ReadFromStream()
	{
		static constexpr int MaxIterartions = 100;

		for (int i = 0; i < MaxIterartions; ++i)
		{
			if (!mEncodedStreamReader->ReadChunk(mActualString))
			{
				break;
			}
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
