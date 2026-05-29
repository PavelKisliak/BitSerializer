/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <memory>
#include <gtest/gtest.h>
#include "bitserializer/convert.h"
#include "common/encoded_stream_reader.h"

template <class TTargetCharType>
class EncodedStreamReaderTest : public ::testing::Test
{
public:
	using reader_type = BitSerializer::Convert::Utf::EncodedStreamReader<TTargetCharType>;
	using target_string_type = std::basic_string<TTargetCharType, std::char_traits<TTargetCharType>>;

	template <typename TSourceUtfType>
	void PrepareEncodedStreamReader(std::u32string_view testStr, bool addBom = false,
		BitSerializer::Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy = BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip,
		const TTargetCharType* errorMark = BitSerializer::Convert::Utf::Detail::GetDefaultErrorMark<TTargetCharType>(),
		size_t chunkSize = 32)
	{
		using source_char_type = typename TSourceUtfType::char_type;
		using source_string_type = std::basic_string<source_char_type, std::char_traits<source_char_type>>;

		mExpectedString = BitSerializer::Convert::To<target_string_type>(testStr);

		mInputString.clear();
		if (addBom) {
			mInputString.append(TSourceUtfType::bom, sizeof TSourceUtfType::bom);
		}
		source_string_type sourceEncodedString;
		TSourceUtfType::Encode(testStr.cbegin(), testStr.cend(), sourceEncodedString);
		mInputString.append(reinterpret_cast<const char*>(sourceEncodedString.data()), sourceEncodedString.size() * sizeof(source_char_type));

		mInputStream.str(mInputString);
		mInputStream.clear();
		mEncodedStreamReader = std::make_shared<reader_type>(mInputStream, encodingErrorPolicy, errorMark, chunkSize);
	}

	void PrepareRawStream(const std::string& rawData,
		BitSerializer::Convert::Utf::UtfEncodingErrorPolicy encodingErrorPolicy = BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip,
		size_t chunkSize = 32)
	{
		mInputString = rawData;
		mInputStream.str(mInputString);
		mInputStream.clear();
		mEncodedStreamReader = std::make_shared<reader_type>(mInputStream, encodingErrorPolicy,
			BitSerializer::Convert::Utf::Detail::GetDefaultErrorMark<TTargetCharType>(), chunkSize);
	}

	target_string_type ReadAll()
	{
		target_string_type result;
		while (true)
		{
			const auto view = mEncodedStreamReader->PeekData(1);
			if (view.empty()) { break; }
			result.append(view.data(), view.size());
			mEncodedStreamReader->SkipChars(view.size());
		}
		return result;
	}

	target_string_type ReadAllCharByChar()
	{
		target_string_type result;
		while (auto ch = mEncodedStreamReader->ReadChar())
		{
			result.push_back(*ch);
		}
		return result;
	}

protected:
	std::string mInputString;
	std::stringstream mInputStream;
	std::shared_ptr<reader_type> mEncodedStreamReader;
	target_string_type mExpectedString;
};
