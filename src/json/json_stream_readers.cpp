/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_stream_readers.h"
#include "json_reader_common.h"
#include <charconv>
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS == 0
#include "bitserializer/conversion_detail/convert_compatibility.h"
#endif

namespace
{
	char SkipWhitespaceAndPeek(BitSerializer::Convert::Utf::EncodedStreamReader<char>& encodedStreamReader, size_t& lineNumber)
	{
		std::string_view data;
		char ch;
		size_t index = 0;
		while (true)
		{
			if (index == data.size())
			{
				data = encodedStreamReader.PeekChars(index + 64);
				// No more data to read
				if (index == data.size())
				{
					ch = 0;
					break;
				}
			}

			ch = data[index];
			if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
				lineNumber += static_cast<size_t>(ch == '\n');
			}
			else {
				break;
			}
			++index;
		}

		if (index) {
			encodedStreamReader.SkipChars(index);
		}
		return ch;
	}
}

namespace BitSerializer::Json::Detail
{
	CJsonStreamReader::CJsonStreamReader(std::istream& inputStream, const SerializationOptions& serializationOptions)
		: mEncodedStreamReader(inputStream)
		, mSerializationOptions(serializationOptions)
	{ }

	void CJsonStreamReader::SetPosition(size_t pos)
	{
		try
		{
			mEncodedStreamReader.SetPosition(pos);
		}
		catch (const std::out_of_range&)
		{
			throw std::invalid_argument("Internal error: position is out of range of input data");
		}
	}

	ValueType CJsonStreamReader::ReadValueTypeImpl()
	{
		switch (char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber))
		{
		case '{':
			return ValueType::Object;
		case '[':
			return ValueType::Array;
		case '"':
			return ValueType::String;
		case 'n':
			return ValueType::Null;
		case 't':
		case 'f':
			return ValueType::Boolean;
		case '\0':
			throw ParsingException("No more values to read", mLineNumber, GetPosition());
		default:
			// Detect number type
			const bool isSigned = ch == '-';
			std::string_view inputData = mEncodedStreamReader.PeekChars(MaxNumberLength);
			size_t pos = 0;
			if (isSigned)
			{
				++pos;
				if (inputData.size() == 1)
				{
					throw ParsingException("Unexpected end of input archive", mLineNumber, mEncodedStreamReader.GetPosition());
				}
				ch = inputData[pos];
			}
			if (ch >= '0' && ch <= '9')
			{
				// Find first non digit
				const size_t size = inputData.size();
				for (size_t i = pos + 1; i < size; ++i)
				{
					ch = inputData[i];
					if (ch < '0' || ch > '9')
					{
						if (ch == '.' || ch == 'e' || ch == 'E')
						{
							return ValueType::Float;
						}
						break;
					}
				}
				return isSigned ? ValueType::SignedInteger : ValueType::UnsignedInteger;
			}
			throw ParsingException("Invalid sequence", mLineNumber, mEncodedStreamReader.GetPosition());
		}
	}

	bool CJsonStreamReader::ReadStringImpl(std::string& outValue)
	{
		char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
		if (ch == 0) {
			throw ParsingException("No more values to read", mLineNumber, GetPosition());
		}
		if (ch != '"') {
			return false;
		}

		size_t pos = 1; // Skip opening quote
		const size_t start = pos;

		size_t workWindowSize = 64;
		std::string_view inputData = mEncodedStreamReader.PeekChars(workWindowSize);

		while (true)
		{
			const size_t firstEscape = inputData.find_first_of("\"\\", pos);
			if (firstEscape != std::string_view::npos)
			{
				// Fast path: string without escape sequences
				if (inputData[firstEscape] == '"')
				{
					outValue.assign(inputData.data() + start, firstEscape - start);
					mEncodedStreamReader.SkipChars(firstEscape + 1);
					return true;
				}
				pos = firstEscape;
				break;
			}

			// Read more data (forces the raw buffer to grow by at least one chunk if the stream has more data)
			std::string_view data = mEncodedStreamReader.PeekChars(inputData.size() + 1);
			if (data.size() == inputData.size())
			{
				// No more data
				throw ParsingException("Unterminated string literal", mLineNumber, inputData.size());
			}
			pos = inputData.size();
			inputData = data;
		}

		// Slow path: string contains escape sequences - copy prefix and process via buffer
		outValue.clear();
		outValue.assign(inputData.data() + start, pos - start);

		size_t size = inputData.size();
		const auto readMoreData = [&]()
		{
			// Read more data (forces the raw buffer to grow by at least one chunk if the stream has more data)
			const std::string_view data = mEncodedStreamReader.PeekChars(size + 1);
			if (data.size() == size) {
				return false; // No more data available
			}
			inputData = data;
			size = inputData.size();
			return true;
		};

		// Ensures that `count` characters are available starting at `pos`, reading more data from the stream if needed
		const auto ensureAvailable = [&](size_t count)
		{
			while (pos + count > size)
			{
				if (!readMoreData()) {
					return false;
				}
			}
			return true;
		};

		do
		{
			if (!ensureAvailable(1)) {
				throw ParsingException("Unterminated string literal", mLineNumber, pos);
			}
			ch = inputData[pos];
			++pos;
			if (ch == '"')
			{
				mEncodedStreamReader.SkipChars(pos);
				return true;
			}
			if (ch == '\\')
			{
				if (!ensureAvailable(1)) {
					throw ParsingException("Unexpected end of input in string escape sequence", mLineNumber, pos);
				}

				const char esc = inputData[pos];
				++pos;
				switch (esc)
				{
				case '"':  outValue += '"';  break;
				case '\\': outValue += '\\'; break;
				case '/':  outValue += '/';  break;
				case 'b':  outValue += '\b'; break;
				case 'f':  outValue += '\f'; break;
				case 'n':  outValue += '\n'; break;
				case 'r':  outValue += '\r'; break;
				case 't':  outValue += '\t'; break;
				case 'u':
				{
					if (!ensureAvailable(4)) {
						throw ParsingException("Unexpected end of input in string escape sequence", mLineNumber, pos - 1);
					}

					const uint16_t codepoint = ParseHex4(inputData, pos, mLineNumber);
					if (codepoint >= 0xD800 && codepoint <= 0xDBFF)
					{
						// High surrogate: expect \uXXXX low surrogate immediately after
						if (!ensureAvailable(10)) {
							throw ParsingException("Incomplete surrogate pair", mLineNumber, pos);
						}
						if (inputData[pos + 4] != '\\' || inputData[pos + 5] != 'u') {
							throw ParsingException("Incomplete surrogate pair", mLineNumber, pos);
						}
						const uint16_t lowSurrogate = ParseHex4(inputData, pos + 6, mLineNumber);
						if (lowSurrogate < 0xDC00 || lowSurrogate > 0xDFFF) {
							throw ParsingException("Invalid low surrogate", mLineNumber, pos + 6);
						}
						const uint32_t fullCp = 0x10000 + ((codepoint - 0xD800) << 10) + (lowSurrogate - 0xDC00);
						AppendUtf8Codepoint(outValue, fullCp);
						pos += 10; // Skip \uXXXX\uYYYY
					}
					else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF)
					{
						// Lone low surrogate without preceding high surrogate
						throw ParsingException("Invalid surrogate pair", mLineNumber, pos);
					}
					else
					{
						// BMP codepoint: 1-3 bytes UTF-8
						AppendUtf8Codepoint(outValue, codepoint);
						pos += 4;
					}
					break;
				}
				default:
					throw ParsingException("Invalid escape sequence", mLineNumber, pos - 1);
				}
			}
			else
			{
				outValue.push_back(ch);
			}
		} while (true);
	}

	template <typename T>
	bool CJsonStreamReader::ReadNumberImpl(T& outValue)
	{
		if (SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber))
		{
#if (BITSERIALIZER_HAS_FLOAT_FROM_CHARS == 0)
			if constexpr (std::is_floating_point_v<T>)
			{
				std::string_view data = mEncodedStreamReader.PeekChars(MaxNumberLength);

				// Copy to temporary buffer for prepare null-terminated c-string
				constexpr size_t maxBufSize = MaxNumberLength;
				const size_t copySize = (data.size() < maxBufSize - 1) ? data.size() : maxBufSize - 1;
				char buf[maxBufSize];
				std::memcpy(buf, data.data(), copySize);
				buf[copySize] = '\0';

				errno = 0;
				char* endPos = nullptr;
				T result = Convert::Detail::_stdWrappers::_fromStr<T>(buf, &endPos);
				if (errno == ERANGE)
				{
					if (mSerializationOptions.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
					{
						throw SerializationException(SerializationErrorCode::Overflow, "The target field range is insufficient for the value being loaded");
					}
					mEncodedStreamReader.SkipChars(endPos - buf);
					return false;
				}
				if (endPos == buf)
				{
					HandleMismatchedTypesPolicyStream();
					return false;
				}
				mEncodedStreamReader.SkipChars(endPos - buf);
				outValue = result;
				return true;
			}
			else
#endif
			{
				std::string_view data = mEncodedStreamReader.PeekChars(MaxNumberLength);

				const auto* const start = data.data();
				T parsedNumber;
				const std::from_chars_result rc = std::from_chars(start, start + data.size(), parsedNumber);
				if (rc.ec == std::errc())
				{
					if constexpr (std::is_integral_v<T>)
					{
						if (*rc.ptr == '.')
						{
							HandleMismatchedTypesPolicyStream();
							return false;
						}
					}
					outValue = parsedNumber;
					mEncodedStreamReader.SkipChars(rc.ptr - start);
					return true;
				}
				mEncodedStreamReader.SkipChars(rc.ptr - start);
				if (rc.ec == std::errc::result_out_of_range)
				{
					if (mSerializationOptions.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
					{
						throw SerializationException(SerializationErrorCode::Overflow, "The target field range is insufficient for the value being loaded");
					}
					return false;
				}
				HandleMismatchedTypesPolicyStream();
				return false;
			}
		}
		throw ParsingException("No more values to read", mLineNumber, GetPosition());
	}

	template bool CJsonStreamReader::ReadNumberImpl(uint8_t&);
	template bool CJsonStreamReader::ReadNumberImpl(uint16_t&);
	template bool CJsonStreamReader::ReadNumberImpl(uint32_t&);
	template bool CJsonStreamReader::ReadNumberImpl(uint64_t&);
	template bool CJsonStreamReader::ReadNumberImpl(char&);
	template bool CJsonStreamReader::ReadNumberImpl(int8_t&);
	template bool CJsonStreamReader::ReadNumberImpl(int16_t&);
	template bool CJsonStreamReader::ReadNumberImpl(int32_t&);
	template bool CJsonStreamReader::ReadNumberImpl(int64_t&);
	template bool CJsonStreamReader::ReadNumberImpl(float&);
	template bool CJsonStreamReader::ReadNumberImpl(double&);
	template bool CJsonStreamReader::ReadNumberImpl(long double&);

	void CJsonStreamReader::SkipValueImpl()
	{
		char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
		if (ch == 0) {
			throw ParsingException("No more values to read", mLineNumber, GetPosition());
		}

		size_t workWindowSize = 64;
		std::string_view inputData = mEncodedStreamReader.PeekChars(workWindowSize);

		// Skip string type
		if (ch == '"')
		{
			size_t pos = 1;
			while (true)
			{
				const size_t firstEscape = inputData.find_first_of("\"\\", pos);
				if (firstEscape == std::string_view::npos)
				{
					// No quote or escape in the buffered window: read more data
					std::string_view data = mEncodedStreamReader.PeekChars(inputData.size() + 1);
					if (data.size() == inputData.size()) {
						throw ParsingException("Unterminated string literal", mLineNumber, GetPosition() + data.size());
					}
					// Keep `pos` if it already points past the old window (escaped character in the new data)
					pos = (std::max)(pos, inputData.size());
					inputData = data;
					continue;
				}
				if (inputData[firstEscape] == '"')
				{
					mEncodedStreamReader.SkipChars(firstEscape + 1);
					return; // End of string
				}
				// Skip backslash and the escaped character (hex digits of \uXXXX are scanned as plain chars)
				pos = firstEscape + 2;
			}
		}

		// Skip object type
		if (ch == '{')
		{
			mEncodedStreamReader.SkipChars(1);
			{
				bool needComma = false;
				while (true)
				{
					ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
					if (ch == 0)
					{
						throw ParsingException("Expected ',' or '}' in object", mLineNumber, GetPosition());
					}
					if (ch == '}')
					{
						mEncodedStreamReader.SkipChars(1);
						return;
					}

					if (needComma)
					{
						if (ch != ',') {
							throw ParsingException("Expected ',' or '}' in object", mLineNumber, GetPosition());
						}
						mEncodedStreamReader.SkipChars(1);
					}

					SkipValueImpl();

					ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
					if (ch != ':') {
						throw ParsingException("Expected ':' in object", mLineNumber, GetPosition());
					}
					mEncodedStreamReader.SkipChars(1);

					SkipValueImpl();
					needComma = true;
				}
			}
		}

		// Skip array type
		if (ch == '[')
		{
			mEncodedStreamReader.SkipChars(1);
			{
				bool needComma = false;
				while (true)
				{
					ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
					if (ch == 0)
					{
						throw ParsingException("Expected ',' or ']' in array", mLineNumber, GetPosition());
					}
					if (ch == ']')
					{
						mEncodedStreamReader.SkipChars(1);
						return;
					}

					if (needComma)
					{
						if (ch != ',') {
							throw ParsingException("Expected ',' or ']' in array", mLineNumber, GetPosition());
						}
						mEncodedStreamReader.SkipChars(1);
					}

					SkipValueImpl();
					needComma = true;
				}
			}
		}

		// Skip boolean or null types
		if (std::isalpha(static_cast<unsigned char>(ch)))
		{
			if (inputData.compare(0, 4, "true") == 0)
			{
				mEncodedStreamReader.SkipChars(4);
				return;
			}
			if (inputData.compare(0, 5, "false") == 0)
			{
				mEncodedStreamReader.SkipChars(5);
				return;
			}
			if (inputData.compare(0, 4, "null") == 0)
			{
				mEncodedStreamReader.SkipChars(4);
				return;
			}
			throw ParsingException("Expected true, false, or null", mLineNumber, mEncodedStreamReader.GetPosition());
		}

		// Skip number (including scientific notation)
		if (std::isdigit(static_cast<unsigned char>(ch)) || ch == '-')
		{
			size_t pos = 1;
			while (true)
			{
				if (pos >= inputData.size())
				{
					// Read more data
					std::string_view data = mEncodedStreamReader.PeekChars(inputData.size() + 1);
					if (data.size() == inputData.size()) {
						break; // No more data: the number ends at the end of the stream
					}
					pos = inputData.size();
					inputData = data;
				}

				ch = inputData[pos];
				if (std::isdigit(static_cast<unsigned char>(ch)) ||
					ch == '.' || ch == 'e' || ch == 'E' || ch == '+' || ch == '-')
				{
					++pos;
					continue;
				}
				break;
			}
			mEncodedStreamReader.SkipChars(pos);
			return;
		}

		throw ParsingException("Unexpected character while skipping value", mLineNumber, GetPosition());
	}

	void CJsonStreamReader::HandleMismatchedTypesPolicyStream()
	{
		if (mSerializationOptions.mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
		{
			const ValueType actualValueType = ReadValueTypeImpl();
			if (actualValueType != ValueType::Null)
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					"The type of target field does not match the value being loaded");
			}
		}
		SkipValueImpl();
	}

	//------------------------------------------------------------------------------

	ValueType CJsonStreamReader::ReadValueType()
	{
		return ReadValueTypeImpl();
	}

	void CJsonStreamReader::ReadKey(std::string_view& key)
	{
		if (ReadStringImpl(mKeyBuffer))
		{
			key = mKeyBuffer;
			const char colon = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
			if (colon != ':') {
				throw ParsingException("Missing a colon between key and value", mLineNumber, GetPosition());
			}
			mEncodedStreamReader.SkipChars(1);
			return;
		}
		throw ParsingException("Expected string for JSON object key", mLineNumber, GetPosition());
	}

	bool CJsonStreamReader::ReadValue(std::nullptr_t&)
	{
		if (const char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber); ch)
		{
			std::string_view data = mEncodedStreamReader.PeekChars(4);
			if (ch == 'n' && data.compare(1, 3, "ull", 3) == 0)
			{
				mEncodedStreamReader.SkipChars(4);
				return true;
			}
			HandleMismatchedTypesPolicyStream();
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mEncodedStreamReader.GetPosition());
	}

	bool CJsonStreamReader::ReadValue(bool& value)
	{
		if (const char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber); ch)
		{
			switch (ch)
			{
			case 't':
			{
				std::string_view data = mEncodedStreamReader.PeekChars(4);
				if (data.compare(1, 3, "rue", 3) == 0)
				{
					value = true;
					mEncodedStreamReader.SkipChars(4);
					return true;
				}
				break;
			}
			case 'f':
			{
				std::string_view data = mEncodedStreamReader.PeekChars(5);
				if (data.compare(1, 4, "alse", 4) == 0)
				{
					value = false;
					mEncodedStreamReader.SkipChars(5);
					return true;
				}
				break;
			}

			// Allow to read unsigned to boolean
			case '1':
			{
				std::string_view data = mEncodedStreamReader.PeekChars(2);
				if (data.size() == 1 || (data[1] != '.' && !std::isdigit(data[1])))
				{
					value = true;
					mEncodedStreamReader.SkipChars(1);
					return true;
				}
				break;
			}
			case '0':
			{
				std::string_view data = mEncodedStreamReader.PeekChars(2);
				if (data.size() == 1 || (data[1] != '.' && !std::isdigit(data[1])))
				{
					value = false;
					mEncodedStreamReader.SkipChars(1);
					return true;
				}
				break;
			}
			default:
				break;
			}

			HandleMismatchedTypesPolicyStream();
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mEncodedStreamReader.GetPosition());
	}

	bool CJsonStreamReader::ReadValue(uint8_t& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(uint16_t& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(uint32_t& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(uint64_t& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(char& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(int8_t& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(int16_t& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(int32_t& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(int64_t& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(float& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(double& value) { return ReadNumberImpl(value); }
	bool CJsonStreamReader::ReadValue(long double& value) { return ReadNumberImpl(value); }

	bool CJsonStreamReader::ReadValue(std::string_view& value)
	{
		const size_t savedPos = GetPosition();
		if (ReadStringImpl(mValueBuffer))
		{
			value = std::string_view(mValueBuffer);
			return true;
		}

		SetPosition(savedPos);
		HandleMismatchedTypesPolicyStream();
		return false;
	}

	bool CJsonStreamReader::ReadValue(JsonArchiveTraits::raw_type& value)
	{
		SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
		const size_t startPos = GetPosition();

		SkipValueImpl();
		const size_t endPos = GetPosition();
		const size_t len = endPos - startPos;

		SetPosition(startPos);
		const std::string_view view = mEncodedStreamReader.PeekChars(len);
		value = JsonArchiveTraits::raw_type(JsonArchiveTraits::raw_type::value_type(view.substr(0, len)));
		mEncodedStreamReader.SkipChars(len);
		return !value.Get().empty();
	}

	void CJsonStreamReader::SkipValue()
	{
		SkipValueImpl();
	}

	void CJsonStreamReader::ReadValueSeparator()
	{
		const char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
		if (ch == ',')
		{
			mEncodedStreamReader.SkipChars(1);
		}
		else
		{
			throw ParsingException("Missing a comma between elements", mLineNumber, GetPosition());
		}
	}

	bool CJsonStreamReader::OpenArray()
	{
		const char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
		if (ch == '[')
		{
			mEncodedStreamReader.SkipChars(1);
			return true;
		}

		if (ch)
		{
			HandleMismatchedTypesPolicyStream();
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, GetPosition());
	}

	bool CJsonStreamReader::IsArrayEnd()
	{
		const char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
		if (ch == ']') {
			return true;
		}
		if (ch == '\0' && IsEnd()) {
			throw ParsingException("Missing closing bracket ']' at end of array JSON", mLineNumber, GetPosition());
		}
		return false;
	}

	void CJsonStreamReader::CloseArray(bool expectedComma)
	{
		while (true)
		{
			const char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
			if (ch == ']')
			{
				mEncodedStreamReader.SkipChars(1);
				mEncodedStreamReader.TrimConsumedData();
				return;
			}

			if (ch == '\0' && IsEnd()) {
				throw ParsingException("Missing closing bracket ']' at end of source JSON", mLineNumber, GetPosition());
			}

			if (expectedComma)
			{
				if (ch == ',')
				{
					mEncodedStreamReader.SkipChars(1);
				}
				else
				{
					throw ParsingException("Missing a comma between elements", mLineNumber, GetPosition());
				}
			}

			SkipValueImpl();
			expectedComma = true;
		}
	}

	bool CJsonStreamReader::OpenObject()
	{
		const char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
		if (ch == '{')
		{
			mEncodedStreamReader.SkipChars(1);
			return true;
		}

		if (ch)
		{
			HandleMismatchedTypesPolicyStream();
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, GetPosition());
	}

	bool CJsonStreamReader::IsObjectEnd()
	{
		const char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
		if (ch == '}') {
			return true;
		}
		if (ch == '\0' && IsEnd()) {
			throw ParsingException("Missing closing bracket '}' at end of source JSON", mLineNumber, GetPosition());
		}
		return false;
	}

	void CJsonStreamReader::CloseObject(bool expectedComma)
	{
		while (true)
		{
			const char ch = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
			if (ch == '}')
			{
				mEncodedStreamReader.SkipChars(1);
				mEncodedStreamReader.TrimConsumedData();
				return;
			}

			if (ch == '\0' && IsEnd()) {
				throw ParsingException("Missing closing bracket '}' at end of source JSON", mLineNumber, GetPosition());
			}

			if (expectedComma)
			{
				if (ch == ',')
				{
					mEncodedStreamReader.SkipChars(1);
				}
				else
				{
					throw ParsingException("Missing a comma between elements", mLineNumber, GetPosition());
				}
			}

			SkipValueImpl();
			const char colon = SkipWhitespaceAndPeek(mEncodedStreamReader, mLineNumber);
			if (colon != ':') {
				throw ParsingException("Missing a colon between key and value", mLineNumber, GetPosition());
			}
			mEncodedStreamReader.SkipChars(1);
			SkipValueImpl();
			expectedComma = true;
		}
	}
}
