/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

#include "json_string_readers.h"
#include "json_reader_common.h"
#include "bitserializer/common/text.h"
#include <charconv>
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS == 0
#include "bitserializer/conversion_detail/convert_compatibility.h"
#endif

namespace BitSerializer::Json::Detail
{
	/**
	 * @brief Skips a comment starting at `data[pos] == '/'`.
	 *
	 * Returns the position after the comment, or the original position when the
	 * slash does not start a comment (a lone `/` at the end of the input). Kept
	 * out of the hot scanning loop via `BITSERIALIZER_NOINLINE`.
	 */
	[[maybe_unused]] static BITSERIALIZER_NOINLINE size_t SkipComment(std::string_view data, size_t pos, size_t& line)
	{
		if (pos + 1 >= data.size())
		{
			return pos; // A lone '/' at the end of input is not a comment
		}
		if (data[pos + 1] == '/')
		{
			// Line comment: skip up to (but not including) the new-line character
			pos += 2;
			while (pos < data.size() && data[pos] != '\n') {
				++pos;
			}
			return pos;
		}
		if (data[pos + 1] == '*')
		{
			// Block comment: skip up to and including the closing "*/"
			pos += 2;
			bool closed = false;
			while (pos < data.size())
			{
				if (data[pos] == '*' && pos + 1 < data.size() && data[pos + 1] == '/')
				{
					pos += 2;
					closed = true;
					break;
				}
				line += static_cast<size_t>(data[pos] == '\n');
				++pos;
			}
			if (!closed) {
				throw ParsingException("Unterminated block comment", line, pos);
			}
			return pos;
		}
		return pos; // Not a comment
	}

	template <ArchiveType TFormat>
	BITSERIALIZER_FORCEINLINE char SkipWhitespaceAndPeek(std::string_view data, size_t& pos, size_t& line) noexcept(!HasFeature(GetFormatFeatures(TFormat), JsonFeatures::Comments))
	{
		constexpr bool AllowComments = HasFeature(GetFormatFeatures(TFormat), JsonFeatures::Comments);
		const size_t len = data.size();
		char ch = 0;
		while (pos < len)
		{
			ch = data[pos];
			if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
			{
				line += static_cast<size_t>(ch == '\n');
				++pos;
				continue;
			}
			if constexpr (AllowComments)
			{
				if (ch == '/')
				{
					const size_t newPos = SkipComment(data, pos, line);
					if (newPos == pos) {
						break; // A lone '/' is not a comment
					}
					pos = newPos;
					ch = 0;
					continue;
				}
			}
			break;
		}
		return ch;
	}

	//-----------------------------------------------------------------------------
	// JsonStringReader
	//-----------------------------------------------------------------------------
	inline ValueType ReadValueTypeImpl(std::string_view inputData, size_t pos, size_t line)
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
			throw ParsingException("Invalid sequence", line, startPos);
		}
		throw ParsingException("No more values to read", line, startPos);
	}

	template <ArchiveType TFormat>
	void SkipValueImpl(std::string_view inputData, size_t& pos, size_t& line)
	{
		constexpr bool AllowTrailingCommas = HasFeature(GetFormatFeatures(TFormat), JsonFeatures::TrailingCommas);
		char ch = SkipWhitespaceAndPeek<TFormat>(inputData, pos, line);
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
			if (SkipWhitespaceAndPeek<TFormat>(inputData, pos, line) == '}')
			{
				++pos;
				return;
			}

			// Skip object's key/value pairs
			while (true)
			{
				// Skip key (must be string)
				SkipValueImpl<TFormat>(inputData, pos, line); // This will skip the string key

				// Expect colon
				if (SkipWhitespaceAndPeek<TFormat>(inputData, pos, line) != ':') {
					throw ParsingException("Expected ':' in object", line, pos);
				}
				++pos;

				// Skip value
				SkipValueImpl<TFormat>(inputData, pos, line);

				ch = SkipWhitespaceAndPeek<TFormat>(inputData, pos, line);
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

				// Trailing comma before the closing brace
				if constexpr (AllowTrailingCommas)
				{
					if (SkipWhitespaceAndPeek<TFormat>(inputData, pos, line) == '}')
					{
						++pos;
						return;
					}
				}
			}
		}

		// Skip array type
		if (ch == '[')
		{
			++pos;

			// Empty array []
			if (SkipWhitespaceAndPeek<TFormat>(inputData, pos, line) == ']')
			{
				++pos;
				return;
			}

			// Skip array elements
			while (true)
			{
				SkipValueImpl<TFormat>(inputData, pos, line);

				ch = SkipWhitespaceAndPeek<TFormat>(inputData, pos, line);
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

				// Trailing comma before the closing bracket
				if constexpr (AllowTrailingCommas)
				{
					if (SkipWhitespaceAndPeek<TFormat>(inputData, pos, line) == ']')
					{
						++pos;
						return;
					}
				}
			}
		}

		// Skip boolean or null types
		if (std::isalpha(static_cast<unsigned char>(ch)))
		{
			if (inputData.compare(pos, 4, "true") == 0)
			{
				pos += 4;
				return;
			}
			if (inputData.compare(pos, 5, "false") == 0)
			{
				pos += 5;
				return;
			}
			if (inputData.compare(pos, 4, "null") == 0)
			{
				pos += 4;
				return;
			}
			throw ParsingException("Expected true, false, or null", line, pos);
		}

		// Skip number (including scientific notation)
		if (std::isdigit(static_cast<unsigned char>(ch)) || ch == '-' )
		{
			++pos;
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

	template <ArchiveType TFormat>
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
		SkipValueImpl<TFormat>(inputData, pos, line);
	}

	template <ArchiveType TFormat, typename T, std::enable_if_t<(std::is_integral_v<T>
#if BITSERIALIZER_HAS_FLOAT_FROM_CHARS
	|| std::is_floating_point_v<T>
#endif
		), int> = 0>
	bool ReadNumber(std::string_view inputData, size_t& pos, T& outValue, size_t& line, const SerializationOptions& serializationOptions)
	{
		const char firstChar = SkipWhitespaceAndPeek<TFormat>(inputData, pos, line);
		if (!firstChar)
		{
			throw ParsingException("No more values to read", line, pos);
		}

		const char* const tokenStart = inputData.data() + pos;
		const size_t available = inputData.size() - pos;
		T parsedNumber;
		const std::from_chars_result rc = std::from_chars(tokenStart, tokenStart + available, parsedNumber);

		if (rc.ec == std::errc())
		{
			const size_t length = static_cast<size_t>(rc.ptr - tokenStart);

			if constexpr (std::is_integral_v<T>)
			{
				// Strict JSON integer grammar: no leading zeros. `from_chars` would not consume
				// a non-digit, so a multi-character integer starting with '0' is always a leading zero.
				const size_t digitStart = tokenStart[0] == '-' ? 1 : 0;
				if (length > digitStart + 1 && tokenStart[digitStart] == '0')
				{
					throw ParsingException("Invalid JSON number", line, pos + digitStart + 1);
				}

				// The char after the token must be a terminator; `.`/`e`/`E` mean a float token,
				// which is a type mismatch for an integral target.
				if (length < available)
				{
					const char next = tokenStart[length];
					if (!IsJsonNumberTerminator(next))
					{
						if (!IsJsonFloatStart(next))
						{
							throw ParsingException("Invalid JSON number", line, pos + length);
						}

						JsonNumberInfo numberInfo;
						if (ScanJsonNumber(inputData.substr(pos), true, numberInfo) != JsonNumberScanResult::Valid)
						{
							throw ParsingException("Invalid JSON number", line, pos + numberInfo.errorOffset);
						}
						HandleMismatchedTypesPolicy<TFormat>(inputData, pos, line, serializationOptions.mismatchedTypesPolicy);
						return false;
					}
				}
			}
			else
			{
				size_t errorOffset = 0;
				if (!IsValidParsedNumber<true>(tokenStart, available, length, errorOffset))
				{
					// Use the scanner for the error position, it is consistent with the `strtod` fallback path.
					JsonNumberInfo numberInfo;
					if (ScanJsonNumber(std::string_view(tokenStart, available), true, numberInfo) == JsonNumberScanResult::Invalid)
					{
						errorOffset = numberInfo.errorOffset;
					}
					throw ParsingException("Invalid JSON number", line, pos + errorOffset);
				}
			}

			outValue = parsedNumber;
			pos += length;
			return true;
		}

		// Any non-numeric value is a type mismatch, not a malformed number.
		// A leading '+' is still treated as a malformed number (JSON does not allow it).
		if (!IsJsonDigit(firstChar) && firstChar != '-' && firstChar != '+')
		{
			HandleMismatchedTypesPolicy<TFormat>(inputData, pos, line, serializationOptions.mismatchedTypesPolicy);
			return false;
		}

		// Overflow, or a value that cannot be converted (e.g. a negative value into an unsigned type).
		// Re-scan the token to tell these cases apart and to know how much to skip.
		JsonNumberInfo numberInfo;
		if (ScanJsonNumber(inputData.substr(pos), true, numberInfo) != JsonNumberScanResult::Valid)
		{
			throw ParsingException("Invalid JSON number", line, pos + numberInfo.errorOffset);
		}

		if (rc.ec == std::errc::result_out_of_range)
		{
			if (serializationOptions.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
			{
				throw SerializationException(SerializationErrorCode::Overflow, "The target field range is insufficient for the value being loaded");
			}
			pos += numberInfo.length;
			return false;
		}

		// A valid JSON number that cannot be represented by the target type is a type mismatch.
		HandleMismatchedTypesPolicy<TFormat>(inputData, pos, line, serializationOptions.mismatchedTypesPolicy);
		return false;
	}

#if (BITSERIALIZER_HAS_FLOAT_FROM_CHARS == 0)
	template <ArchiveType TFormat, typename T, std::enable_if_t<(std::is_floating_point_v<T>), int> = 0>
	bool ReadNumber(std::string_view inputData, size_t& pos, T& outValue, size_t& line, const SerializationOptions& serializationOptions)
	{
		const char firstChar = SkipWhitespaceAndPeek<TFormat>(inputData, pos, line);
		if (firstChar)
		{
			// Any non-numeric value is a type mismatch, not a malformed number.
			// A leading '+' is still treated as a malformed number (JSON does not allow it).
			if (!IsJsonDigit(firstChar) && firstChar != '-' && firstChar != '+')
			{
				HandleMismatchedTypesPolicy<TFormat>(inputData, pos, line, serializationOptions.mismatchedTypesPolicy);
				return false;
			}

			JsonNumberInfo numberInfo;
			if (ScanJsonNumber(inputData.substr(pos), true, numberInfo) != JsonNumberScanResult::Valid)
			{
				throw ParsingException("Invalid JSON number", line, pos + numberInfo.errorOffset);
			}

			constexpr size_t maxBufSize = MaxNumberLength;
			if (numberInfo.length >= maxBufSize)
			{
				if (serializationOptions.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::Overflow, "The target field range is insufficient for the value being loaded");
				}
				pos += numberInfo.length;
				return false;
			}

			// Copy to temporary buffer for prepare null-terminated c-string
			char buf[maxBufSize];
			std::memcpy(buf, inputData.data() + pos, numberInfo.length);
			buf[numberInfo.length] = '\0';

			errno = 0;
			char* endPos = nullptr;
			const T result = Convert::Detail::_stdWrappers::_fromStr<T>(buf, &endPos);
			if (errno == ERANGE)
			{
				if (serializationOptions.overflowNumberPolicy == OverflowNumberPolicy::ThrowError)
				{
					throw SerializationException(SerializationErrorCode::Overflow, "The target field range is insufficient for the value being loaded");
				}
				pos += numberInfo.length;
				return false;
			}
			if (endPos == buf)
			{
				throw ParsingException("Invalid JSON number", line, pos);
			}
			pos += numberInfo.length;
			outValue = result;
			return true;
		}
		throw ParsingException("No more values to read", line, pos);
	}
#endif

	template <ArchiveType TFormat>
	bool ReadString(std::string_view inputData, size_t& pos, std::string_view& outValue, size_t& line, std::string& buffer)
	{
		char ch = SkipWhitespaceAndPeek<TFormat>(inputData, pos, line);
		if (ch == 0) {
			throw ParsingException("No more values to read", line, inputData.size());
		}
		if (ch != '"') {
			return false;
		}

		++pos; // Skip opening quote
		const size_t start = pos;
		const size_t firstEscape = BitSerializer::Simd::FindFirstOf<'"', '\\'>(inputData, pos);
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

		// Slow path: string contains escape sequences - copy prefix and process via buffer
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

	template <ArchiveType TFormat>
	bool TryConsumeColon(std::string_view inputData, size_t& pos, size_t& line) noexcept(!HasFeature(GetFormatFeatures(TFormat), JsonFeatures::Comments))
	{
		if (SkipWhitespaceAndPeek<TFormat>(inputData, pos, line) == ':')
		{
			++pos;
			return true;
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	// JsonStringReader implementation
	//-----------------------------------------------------------------------------
	template <ArchiveType TFormat>
	JsonStringReader<TFormat>::JsonStringReader(std::string_view inputData, const SerializationOptions& serializationOptions) noexcept
		: mInputData(inputData)
		, mSerializationOptions(serializationOptions)
	{
	}

	template <ArchiveType TFormat>
	void JsonStringReader<TFormat>::SetPosition(size_t pos)
	{
		if (pos <= mInputData.size()) {
			mPos = pos;
		}
		else {
			throw std::invalid_argument("Internal error: position is out of range of input data");
		}
	}

	template <ArchiveType TFormat>
	ValueType JsonStringReader<TFormat>::ReadValueType()
	{
		SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber);
		return ReadValueTypeImpl(mInputData, mPos, mLineNumber);
	}

	template <ArchiveType TFormat>
	void JsonStringReader<TFormat>::ReadKey(std::string_view& key)
	{
		if (ReadString<TFormat>(mInputData, mPos, key, mLineNumber, mBuffer))
		{
			if (!TryConsumeColon<TFormat>(mInputData, mPos, mLineNumber)) {
				throw ParsingException("Missing a colon between key and value", mLineNumber, mPos);
			}
			return;
		}
		throw ParsingException("Expected string for JSON object key", mLineNumber, mPos);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(std::nullptr_t&)
	{
		if (const char ch = SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber); ch)
		{
			if (ch == 'n' && mInputData.compare(mPos + 1, 3, "ull", 3) == 0)
			{
				mPos += 4;
				return true;
			}
			HandleMismatchedTypesPolicy<TFormat>(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mPos);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(bool& value)
	{
		if (const char ch = SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber); ch)
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

			// Allow to read 0/1 to boolean (as RapidJSON does), but not larger numbers or floats
			case '1':
				if (mPos + 1 == mInputData.size() || IsJsonNumberTerminator(mInputData[mPos + 1]))
				{
					value = true;
					++mPos;
					return true;
				}
				break;
			case '0':
				if (mPos + 1 == mInputData.size() || IsJsonNumberTerminator(mInputData[mPos + 1]))
				{
					value = false;
					++mPos;
					return true;
				}
				break;
			default:
				break;
			}
			HandleMismatchedTypesPolicy<TFormat>(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mPos);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(uint8_t& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(uint16_t& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(uint32_t& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(uint64_t& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(char& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(int8_t& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(int16_t& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(int32_t& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(int64_t& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(float& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(double& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(long double& value)
	{
		return ReadNumber<TFormat>(mInputData, mPos, value, mLineNumber, mSerializationOptions);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadValue(std::string_view& value)
	{
		if (ReadString<TFormat>(mInputData, mPos, value, mLineNumber, mBuffer)) {
			return true;
		}

		HandleMismatchedTypesPolicy<TFormat>(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
		return false;
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::ReadRawValue(std::string& value)
	{
		SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber);
		const size_t startPos = mPos;

		SkipValueImpl<TFormat>(mInputData, mPos, mLineNumber);
		value.assign(mInputData.substr(startPos, mPos - startPos));

		return !value.empty();
	}

	template <ArchiveType TFormat>
	void JsonStringReader<TFormat>::SkipValue()
	{
		SkipValueImpl<TFormat>(mInputData, mPos, mLineNumber);
	}

	template <ArchiveType TFormat>
	void JsonStringReader<TFormat>::ReadValueSeparator()
	{
		if (SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber) == ',')
		{
			++mPos;
		}
		else
		{
			throw ParsingException("Missing a comma between elements", mLineNumber, mPos);
		}
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::OpenArray()
	{
		const char ch = SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber);
		if (ch == '[')
		{
			++mPos;
			return true;
		}

		if (ch)
		{
			HandleMismatchedTypesPolicy<TFormat>(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mPos);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::IsArrayEnd()
	{
		constexpr bool AllowTrailingCommas = HasFeature(GetFormatFeatures(TFormat), JsonFeatures::TrailingCommas);
		const char ch = SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber);
		if (ch == ']')
		{
			return true;
		}

		// A trailing comma before the closing bracket is also treated as the end
		if constexpr (AllowTrailingCommas)
		{
			if (ch == ',')
			{
				// Non-mutating lookahead: scan a local copy so a normal separator
				// leaves the reader state untouched for ReadValueSeparator().
				size_t lookPos = mPos + 1;
				size_t lookLine = mLineNumber;
				if (SkipWhitespaceAndPeek<TFormat>(mInputData, lookPos, lookLine) == ']')
				{
					mPos = lookPos;
					mLineNumber = lookLine;
					return true;
				}
				return false;
			}
		}

		if (ch == 0) {
			throw ParsingException("Missing closing bracket ']' at end of array JSON", mLineNumber, mPos);
		}
		return false;
	}

	template <ArchiveType TFormat>
	void JsonStringReader<TFormat>::CloseArray(bool expectedComma)
	{
		constexpr bool AllowTrailingCommas = HasFeature(GetFormatFeatures(TFormat), JsonFeatures::TrailingCommas);
		while (const char ch = SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber))
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

					// A trailing comma before the closing bracket
					if constexpr (AllowTrailingCommas)
					{
						if (SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber) == ']')
						{
							++mPos;
							return;
						}
					}
				}
				else
				{
					throw ParsingException("Missing a comma between elements", mLineNumber, mPos);
				}
			}

			// Skip all remaining elements in the array
			SkipValueImpl<TFormat>(mInputData, mPos, mLineNumber);
			expectedComma = true;
		}
		throw ParsingException("Missing closing bracket ']' at end of source JSON", mLineNumber, mPos);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::OpenObject()
	{
		const char ch = SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber);
		if (ch == '{')
		{
			++mPos;
			return true;
		}

		if (ch)
		{
			HandleMismatchedTypesPolicy<TFormat>(mInputData, mPos, mLineNumber, mSerializationOptions.mismatchedTypesPolicy);
			return false;
		}
		throw ParsingException("No more values to read", mLineNumber, mPos);
	}

	template <ArchiveType TFormat>
	bool JsonStringReader<TFormat>::IsObjectEnd()
	{
		constexpr bool AllowTrailingCommas = HasFeature(GetFormatFeatures(TFormat), JsonFeatures::TrailingCommas);
		const char ch = SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber);
		if (ch == '}')
		{
			return true;
		}

		// A trailing comma before the closing brace is also treated as the end
		if constexpr (AllowTrailingCommas)
		{
			if (ch == ',')
			{
				// Non-mutating lookahead: scan a local copy so a normal separator
				// leaves the reader state untouched for ReadValueSeparator().
				size_t lookPos = mPos + 1;
				size_t lookLine = mLineNumber;
				if (SkipWhitespaceAndPeek<TFormat>(mInputData, lookPos, lookLine) == '}')
				{
					mPos = lookPos;
					mLineNumber = lookLine;
					return true;
				}
				return false;
			}
		}

		if (ch == 0) {
			throw ParsingException("Missing closing bracket '}' at end of source JSON", mLineNumber, mPos);
		}
		return false;
	}

	template <ArchiveType TFormat>
	void JsonStringReader<TFormat>::CloseObject(bool expectedComma)
	{
		constexpr bool AllowTrailingCommas = HasFeature(GetFormatFeatures(TFormat), JsonFeatures::TrailingCommas);
		while (const char ch = SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber))
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

					// A trailing comma before the closing brace
					if constexpr (AllowTrailingCommas)
					{
						if (SkipWhitespaceAndPeek<TFormat>(mInputData, mPos, mLineNumber) == '}')
						{
							++mPos;
							return;
						}
					}
				}
				else
				{
					throw ParsingException("Missing a comma between elements", mLineNumber, mPos);
				}
			}

			// Skip all remaining elements in the object
			SkipValueImpl<TFormat>(mInputData, mPos, mLineNumber);
			if (!TryConsumeColon<TFormat>(mInputData, mPos, mLineNumber)) {
				throw ParsingException("Missing a colon between key and value", mLineNumber, mPos);
			}
			SkipValueImpl<TFormat>(mInputData, mPos, mLineNumber);
			expectedComma = true;
		}
		throw ParsingException("Missing closing bracket '}' at end of source JSON", mLineNumber, mPos);
	}

}
