/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "json_string_readers.h"
#include "bitserializer/common/text.h"
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS == 0
#include "bitserializer/conversion_detail/convert_compatibility.h"
#endif

namespace
{
	constexpr uint8_t kHexLUT[256] =
	{
		// 0x00-0x2F: all invalid (0xFF)
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		// 0x30-0x39: '0'-'9' -> 0-9
		0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,  0x8,  0x9,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		// 0x40: '@' invalid, 0x41-0x46: 'A'-'F' -> 10-15
		0xFF,
		0xA,  0xB,  0xC,  0xD,  0xE,  0xF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		// 0x60: '`' invalid, 0x61-0x66: 'a'-'f' -> 10-15
		0xFF,
		0xA,  0xB,  0xC,  0xD,  0xE,  0xF,
		// 0x67-0xFF: all invalid
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

	[[nodiscard]] uint16_t ParseHex4(std::string_view input, size_t pos, size_t lineNumber)
	{
		if (pos + 4 > input.size()) {
			throw BitSerializer::ParsingException("Unexpected end of input in string escape sequence", lineNumber, pos);
		}

		uint16_t codepoint = 0;
		for (int i = 0; i < 4; ++i)
		{
			const uint8_t hexVal = kHexLUT[static_cast<uint8_t>(input[pos + i])];
			if (hexVal == 0xFF) {
				throw BitSerializer::ParsingException("Invalid hex digit in \\u escape", lineNumber, pos + i);
			}
			codepoint = (codepoint << 4) | hexVal;
		}
		return codepoint;
	}

	void AppendUtf8Codepoint(std::string& buffer, uint32_t codepoint) noexcept
	{
		if (codepoint < 0x80)
		{
			buffer += static_cast<char>(codepoint);
		}
		else if (codepoint < 0x800)
		{
			buffer += static_cast<char>(0xC0 | (codepoint >> 6));
			buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else if (codepoint < 0x10000)
		{
			// BMP: 3 bytes
			buffer += static_cast<char>(0xE0 | (codepoint >> 12));
			buffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else
		{
			// Supplementary plane (surrogate pairs decoded): 4 bytes
			buffer += static_cast<char>(0xF0 | (codepoint >> 18));
			buffer += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
			buffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
	}

	char SkipWhitespaceAndPeek(std::string_view data, size_t& pos, size_t& line) noexcept
	{
		const size_t len = data.size();
		char ch = 0;
		while (pos < len)
		{
			ch = data[pos];
			if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
			{
				line += static_cast<size_t>(ch == '\n');
				++pos;
			}
			else {
				break;
			}
		}
		return ch;
	}
}


//-----------------------------------------------------------------------------
// CJsonStringReader
//-----------------------------------------------------------------------------
namespace
{
	using namespace BitSerializer;
	using ValueType = Json::Detail::ValueType;

	ValueType ReadValueTypeImpl(std::string_view inputData, size_t pos, size_t line)
	{
		const size_t startPos = pos;
		const size_t size = inputData.size();
		if (pos < size)
		{
			char ch = inputData[pos];
			// Whitespace should be skipped before
			assert(!Text::IsWhitespace(ch));

			if (ch == '"')
			{
				return ValueType::String;
			}
			if (ch == '{')
			{
				return ValueType::Object;
			}
			if (ch == '[')
			{
				return ValueType::Array;
			}
			if (pos + 3 < size &&
				inputData[pos] == 't' && inputData[pos + 1] == 'r' && inputData[pos + 2] == 'u' && inputData[pos + 3] == 'e')
			{
				return ValueType::Boolean;
			}
			if (pos + 4 < size &&
				inputData[pos] == 'f' && inputData[pos + 1] == 'a' && inputData[pos + 2] == 'l' && inputData[pos + 3] == 's' && inputData[pos + 4] == 'e')
			{
				return ValueType::Boolean;
			}
			if (pos + 3 < size &&
				inputData[pos] == 'n' && inputData[pos + 1] == 'u' && inputData[pos + 2] == 'l' && inputData[pos + 3] == 'l')
			{
				return ValueType::Null;
			}

			// Detect number type
			const bool isSigned = ch == '-';
			if (isSigned)
			{
				if (pos + 1 == size)
				{
					throw ParsingException("Unexpected end of input archive", line, pos);
				}
				ch = inputData[++pos];
			}
			if (ch >= '0' && ch <= '9')
			{
				// Find first non digit
				for (size_t i = pos + 1; i < size; ++i)
				{
					ch = inputData[i];
					if (ch <= '0' || ch > '9')
					{
						if (ch == '.' || ch == 'e' || ch == 'E' || ch == '+' || ch == '-')
						{
							return ValueType::Float;
						}
					}
				}
				return isSigned ? ValueType::SignedInteger : ValueType::UnsignedInteger;
			}
			throw ParsingException("Invalid sequence", line, startPos);
		}
		throw ParsingException("No more values to read", line, startPos);
	}

	void SkipValueImpl(std::string_view inputData, size_t& pos, size_t& line)
	{
		char ch = SkipWhitespaceAndPeek(inputData, pos, line);
		if (ch == 0) {
			throw ParsingException("No more values to read", line, pos);
		}

		// Skip string type
		const size_t size = inputData.size();
		if (ch == '"')
		{
			++pos;
			while (pos < size)
			{
				ch = inputData[pos];
				++pos;
				if (ch == '"') {
					return; // End of string
				}
				if (ch == '\\')
				{
					if (pos >= size) {
						throw ParsingException("Unexpected end of input in string escape", line, pos);
					}
					++pos; // Skip escaped character (including \uXXXX, but we don't validate)
				}
			}
			throw ParsingException("Unterminated string literal", line, pos);
		}

		// Skip object type
		if (ch == '{')
		{
			++pos;

			// Empty object {}
			if (SkipWhitespaceAndPeek(inputData, pos, line) == '}')
			{
				++pos;
				return;
			}

			// Skip object's key/value pairs
			while (true)
			{
				// Skip key (must be string)
				SkipValueImpl(inputData, pos, line); // This will skip the string key

				// Expect colon
				if (SkipWhitespaceAndPeek(inputData, pos, line) != ':') {
					throw ParsingException("Expected ':' in object", line, pos);
				}
				++pos;

				// Skip value
				SkipValueImpl(inputData, pos, line);

				ch = SkipWhitespaceAndPeek(inputData, pos, line);
				if (ch == '}')
				{
					++pos;
					return;
				}
				if (ch != ',') {
					throw ParsingException("Expected ',' or '}' in object", line, pos);
				}
				if (ch == 0) {
					throw ParsingException("Unexpected end of input in object", line, pos);
				}
				++pos;
			}
		}

		// Skip array type
		if (ch == '[')
		{
			++pos;

			// Empty array []
			if (SkipWhitespaceAndPeek(inputData, pos, line) == ']')
			{
				++pos;
				return;
			}

			// Skip array elements
			while (true)
			{
				SkipValueImpl(inputData, pos, line);

				ch = SkipWhitespaceAndPeek(inputData, pos, line);
				if (ch == ']')
				{
					++pos;
					return;
				}
				if (ch != ',') {
					throw ParsingException("Expected ',' or ']' in array", line, pos);
				}
				if (ch == 0) {
					throw ParsingException("Unexpected end of input in array", line, pos);
				}
				++pos;
			}
		}

		// Skip boolean or null types
		if (std::isalpha(static_cast<unsigned char>(ch)))
		{
			if (inputData.substr(pos, 4) == "true")
			{
				pos += 4;
				return;
			}
			if (inputData.substr(pos, 5) == "false")
			{
				pos += 5;
				return;
			}
			if (inputData.substr(pos, 4) == "null")
			{
				pos += 4;
				return;
			}
			throw ParsingException("Expected true, false, or null", line, pos);
		}

		// Skip number (including scientific notation)
		if (std::isdigit(static_cast<unsigned char>(ch)) || ch == '-' || ch == '+')
		{
			while (pos < size)
			{
				ch = inputData[pos];
				if (std::isdigit(static_cast<unsigned char>(ch)) ||
					ch == '.' || ch == 'e' || ch == 'E' || ch == '+' || ch == '-')
				{
					++pos;
				}
				else
				{
					break;
				}
			}
			return;
		}

		throw ParsingException("Unexpected character while skipping value", line, pos);
	}

	void HandleMismatchedTypesPolicy(std::string_view inputData, size_t& pos, size_t& line, MismatchedTypesPolicy mismatchedTypesPolicy)
	{
		if (mismatchedTypesPolicy == MismatchedTypesPolicy::ThrowError)
		{
			const ValueType actualValueType = ReadValueTypeImpl(inputData, pos, line);
			// Null value is excluded from MismatchedTypesPolicy processing
			if (actualValueType != ValueType::Null)
			{
				throw SerializationException(SerializationErrorCode::MismatchedTypes,
					"The type of target field does not match the value being loaded");
			}
		}
		SkipValueImpl(inputData, pos, line);
	}

	template <typename T, std::enable_if_t<(std::is_integral_v<T>
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS
	|| std::is_floating_point_v<T>
#endif
		), int> = 0>
	bool ReadNumber(std::string_view inputData, size_t& pos, T& outValue, size_t& line, const SerializationOptions& serializationOptions)
	{
		if (SkipWhitespaceAndPeek(inputData, pos, line))
		{
			const auto* const start = inputData.data() + pos;
			T parsedNumber;
			const std::from_chars_result rc = std::from_chars(start, inputData.data() + inputData.size(), parsedNumber);
			if (rc.ec == std::errc())
			{
				if constexpr (std::is_integral_v<T>)
				{
					if (*rc.ptr == '.')
					{
						HandleMismatchedTypesPolicy(inputData, pos, line, serializationOptions.mismatchedTypesPolicy);
						return false;
					}
				}
				outValue = parsedNumber;
				pos += rc.ptr - start;
				return true;
			}
			pos += rc.ptr - start;
			if (rc.ec == std::errc::result_out_of_range)
			{
				if (serializationOptions.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::Overflow, "The target field range is insufficient for the value being loaded");
				}
				return false;
			}
			HandleMismatchedTypesPolicy(inputData, pos, line, serializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", line, pos);
	}

#if (BITSERIALIZER_HAS_FLOAT_FROM_CHARS == 0)
	template <typename T, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
	bool ReadNumber(std::string_view inputData, size_t& pos, T& outValue, size_t& line, const SerializationOptions& serializationOptions)
	{
		if (SkipWhitespaceAndPeek(inputData, pos, line))
		{
			// Copy to temporary buffer for prepare null-terminated c-string
			constexpr size_t maxBufSize = 64;
			const size_t remainingSize = inputData.size() - pos;
			char buf[maxBufSize];

			const size_t copySize = (remainingSize < maxBufSize - 1) ? remainingSize : maxBufSize - 1;
			std::memcpy(buf, inputData.data() + pos, copySize);
			buf[copySize] = '\0';

			errno = 0;
			char* endPos = nullptr;
			T result = Convert::Detail::_stdWrappers::_fromStr<T>(buf, &endPos);
			if (errno == ERANGE)
			{
				if (serializationOptions.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::Overflow, "The target field range is insufficient for the value being loaded");
				}
				pos += endPos - buf;
				return false;
			}
			if (endPos == buf)
			{
				HandleMismatchedTypesPolicy(inputData, pos, line, serializationOptions.mismatchedTypesPolicy);
				return false;
			}
			pos += endPos - buf;
			outValue = result;
			return true;
		}
		throw ParsingException("No more values to read", line, pos);
	}
#endif

	bool ReadString(std::string_view inputData, size_t& pos, std::string_view& outValue, size_t& line, std::string& buffer)
	{
		char ch = SkipWhitespaceAndPeek(inputData, pos, line);
		if (ch == 0) {
			throw ParsingException("No more values to read", line, inputData.size());
		}
		if (ch != '"') {
			return false;
		}

		++pos; // Skip opening quote
		const size_t start = pos;
		const size_t firstEscape = inputData.find_first_of("\"\\", pos);
		if (firstEscape == std::string_view::npos) {
			throw ParsingException("Unterminated string literal", line, inputData.size());
		}

		// Fast path: string without escape sequences
		if (inputData[firstEscape] == '"')
		{
			outValue = std::string_view(inputData.data() + start, firstEscape - start);
			pos = firstEscape + 1;
			return true;
		}

		// Slow path: string contains escape sequences — copy prefix and process via buffer
		pos = firstEscape;
		buffer.clear();
		buffer.assign(inputData.data() + start, pos - start);

		const size_t size = inputData.size();
		do
		{
			ch = inputData[pos];
			++pos;
			if (ch == '"')
			{
				outValue = buffer;
				return true;
			}
			if (ch == '\\')
			{
				if (pos >= size) {
					throw ParsingException("Unexpected end of input in string escape sequence", line, pos);
				}

				const char esc = inputData[pos];
				++pos;
				switch (esc)
				{
				case '"':  buffer += '"';  break;
				case '\\': buffer += '\\'; break;
				case '/':  buffer += '/';  break;
				case 'b':  buffer += '\b'; break;
				case 'f':  buffer += '\f'; break;
				case 'n':  buffer += '\n'; break;
				case 'r':  buffer += '\r'; break;
				case 't':  buffer += '\t'; break;
				case 'u':
				{
					if (pos + 4 > size) {
						throw ParsingException("Unexpected end of input in string escape sequence", line, pos - 1);
					}

					const uint16_t codepoint = ParseHex4(inputData, pos, line);
					if (codepoint >= 0xD800 && codepoint <= 0xDBFF)
					{
						// High surrogate: expect \uXXXX low surrogate immediately after
						if (pos + 10 > size || inputData[pos + 4] != '\\' || inputData[pos + 5] != 'u') {
							throw ParsingException("Incomplete surrogate pair", line, pos);
						}
						const uint16_t lowSurrogate = ParseHex4(inputData, pos + 6, line);
						if (lowSurrogate < 0xDC00 || lowSurrogate > 0xDFFF) {
							throw ParsingException("Invalid low surrogate", line, pos + 6);
						}
						const uint32_t fullCp = 0x10000 + ((codepoint - 0xD800) << 10) + (lowSurrogate - 0xDC00);
						AppendUtf8Codepoint(buffer, fullCp);
						pos += 10; // Skip \uXXXX\uYYYY
					}
					else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF)
					{
						// Lone low surrogate without preceding high surrogate
						throw ParsingException("Invalid surrogate pair", line, pos);
					}
					else
					{
						// BMP codepoint: 1-3 bytes UTF-8
						AppendUtf8Codepoint(buffer, codepoint);
						pos += 4;
					}
					break;
				}
				default:
					throw ParsingException("Invalid escape sequence", line, pos - 1);
				}
			}
			else
			{
				buffer.push_back(ch);
			}
		} while (pos < size);

		throw ParsingException("Unterminated string literal", line, pos);
	}

	bool TryConsumeColon(std::string_view inputData, size_t& pos, size_t& line) noexcept
	{
		if (SkipWhitespaceAndPeek(inputData, pos, line) == ':')
		{
			++pos;
			return true;
		}
		return false;
	}
}

namespace BitSerializer::Json::Detail
{
	CJsonStringReader::CJsonStringReader(std::string_view inputData, const SerializationOptions& serializationOptions) noexcept
		: mInputData(inputData)
		, mSerializationOptions(serializationOptions)
	{
	}

	void CJsonStringReader::SetPosition(size_t pos)
	{
		if (pos <= mInputData.size()) {
			mPos = pos;
		}
		else {
			throw std::invalid_argument("Internal error: position is out of range of input data");
		}
	}

	ValueType CJsonStringReader::ReadValueType()
	{
		return ReadValueTypeImpl(mInputData, mPos, mLineNumber);
	}

	void CJsonStringReader::ReadKey(std::string_view& key)
	{
		if (ReadString(mInputData, mPos, key, mLineNumber, mBuffer))
		{
			if (!TryConsumeColon(mInputData, mPos, mLineNumber)) {
				throw ParsingException("Missing a colon between key and value", mLineNumber, mPos);
			}
			return;
		}
		throw ParsingException("Expected string for JSON object key", mLineNumber, mPos);
	}

	bool CJsonStringReader::ReadValue(std::nullptr_t&)
	{
		if (const char ch = SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber); ch)
		{
			if (ch == 'n' && mInputData.compare(mPos + 1, 3, "ull", 3) == 0)
			{
				mPos += 4;
				return true;
			}
			HandleMismatchedTypesPolicy(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mPos);
	}

	bool CJsonStringReader::ReadValue(bool& value)
	{
		if (const char ch = SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber); ch)
		{
			switch (ch)
			{
			case 't':
				if (mInputData.compare(mPos + 1, 3, "rue", 3) == 0)
				{
					value = true;
					mPos += 4;
					return true;
				}
				break;
			case 'f':
				if (mInputData.compare(mPos + 1, 4, "alse", 4) == 0)
				{
					value = false;
					mPos += 5;
					return true;
				}
				break;

			// Allow to read unsigned to boolean
			case '1':
				if (mPos + 1 == mInputData.size() || (mInputData[mPos + 1] != '.' && !std::isdigit(mInputData[mPos + 1])))
				{
					value = true;
					++mPos;
					return true;
				}
				break;
			case '0':
				if (mPos + 1 == mInputData.size() || (mInputData[mPos + 1] != '.' && !std::isdigit(mInputData[mPos + 1])))
				{
					value = false;
					++mPos;
					return true;
				}
				break;
			default:
				break;
			}
			HandleMismatchedTypesPolicy(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mPos);
	}

	bool CJsonStringReader::ReadValue(uint8_t& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(uint16_t& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(uint32_t& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(uint64_t& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(char& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(int8_t& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(int16_t& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(int32_t& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(int64_t& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(float& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(double& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(long double& value)
	{
		return ReadNumber(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	bool CJsonStringReader::ReadValue(std::string_view& value)
	{
		if (ReadString(mInputData, mPos, value, mLineNumber, mBuffer)) {
			return true;
		}

		HandleMismatchedTypesPolicy(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
		return false;
	}

	bool CJsonStringReader::ReadValue(JsonArchiveTraits::raw_type& value)
	{
		SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber);
		const size_t startPos = mPos;

		SkipValueImpl(mInputData, mPos, mLineNumber);
		value = JsonArchiveTraits::raw_type(JsonArchiveTraits::raw_type::value_type(mInputData.substr(startPos, mPos - startPos)));

		return !value.Get().empty();
	}

	void CJsonStringReader::SkipValue()
	{
		SkipValueImpl(mInputData, mPos, mLineNumber);
	}

	void CJsonStringReader::ReadValueSeparator()
	{
		if (SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber) == ',')
		{
			++mPos;
		}
		else
		{
			throw ParsingException("Missing a comma between elements", mLineNumber, mPos);
		}
	}

	bool CJsonStringReader::OpenArray()
	{
		const char ch = SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber);
		if (ch == '[')
		{
			++mPos;
			return true;
		}

		if (ch)
		{
			HandleMismatchedTypesPolicy(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mPos);
	}

	bool CJsonStringReader::IsArrayEnd()
	{
		if (const char ch = SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber))
		{
			return ch == ']';
		}
		throw ParsingException("Missing closing bracket ']' at end of array JSON", mLineNumber, mPos);
	}

	void CJsonStringReader::CloseArray(bool expectedComma)
	{
		while (const char ch = SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber))
		{
			if (ch == ']')
			{
				++mPos;
				return;
			}

			if (expectedComma)
			{
				if (ch == ',')
				{
					++mPos;
				}
				else
				{
					throw ParsingException("Missing a comma between elements", mLineNumber, mPos);
				}
			}

			// Skip all remaining elements in the array
			SkipValueImpl(mInputData, mPos, mLineNumber);
			expectedComma = true;
		}
		throw ParsingException("Missing closing bracket ']' at end of source JSON", mLineNumber, mPos);
	}

	bool CJsonStringReader::OpenObject()
	{
		const char ch = SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber);
		if (ch == '{')
		{
			++mPos;
			return true;
		}

		if (ch)
		{
			HandleMismatchedTypesPolicy(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mPos);
	}

	bool CJsonStringReader::IsObjectEnd()
	{
		if (const char ch = SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber))
		{
			return ch == '}';
		}
		throw ParsingException("Missing closing bracket '}' at end of source JSON", mLineNumber, mPos);
	}

	void CJsonStringReader::CloseObject(bool expectedComma)
	{
		while (const char ch = SkipWhitespaceAndPeek(mInputData, mPos, mLineNumber))
		{
			if (ch == '}')
			{
				++mPos;
				return;
			}

			if (expectedComma)
			{
				if (ch == ',')
				{
					++mPos;
				}
				else
				{
					throw ParsingException("Missing a comma between elements", mLineNumber, mPos);
				}
			}

			// Skip all remaining elements in the object
			SkipValueImpl(mInputData, mPos, mLineNumber);
			if (!TryConsumeColon(mInputData, mPos, mLineNumber)) {
				throw ParsingException("Missing a colon between key and value", mLineNumber, mPos);
			}
			SkipValueImpl(mInputData, mPos, mLineNumber);
			expectedComma = true;
		}
		throw ParsingException("Missing closing bracket '}' at end of source JSON", mLineNumber, mPos);
	}
}
