/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <filesystem>
#include <map>
#include <optional>

#include "common_test_entities.h"
#include "bitserializer/types/std/array.h"
#include "bitserializer/types/std/vector.h"

// NOLINTBEGIN(bugprone-unchecked-optional-access)

/**
 * @brief Compares two floating-point values approximately using a given epsilon.
 *
 * @tparam T The floating-point type.
 * @param a First value to compare.
 * @param b Second value to compare.
 * @param epsilon The tolerance for comparison.
 * @return True if values are considered equal within the provided epsilon.
 */
template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
static bool ApproximatelyEqual(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
	return std::fabs(a - b) <= ((std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * epsilon);
}

/**
 * @brief Tests serialization and deserialization of a value directly at the root scope of an archive.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TValue The type of the value being tested.
 */
template <typename TArchive, typename TValue>
void TestSerializeType()
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive{};
	TValue expected{};
	::BuildFixture(expected);
	TValue actual{};
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(expected, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	GTestExpectEq(expected, actual);
}

/**
 * @brief Tests serialization and deserialization of a specific value instance (root scope of archive).
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam T The type of the value being tested.
 * @param value The test value to serialize and deserialize.
 */
template <typename TArchive, typename T>
void TestSerializeType(T&& value)
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive{};
	std::remove_reference_t<T> actual{};
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(value, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	GTestExpectEq(std::forward<T>(value), actual);
}

/**
 * @brief Tests serialization and deserialization of a key-value pair at the root scope.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TKey The type of the key.
 * @tparam TValue The type of the value.
 * @param key The key associated with the value.
 * @param value The value to be serialized/deserialized.
 */
template <class TArchive, class TKey, class TValue>
void TestSerializeType(const TKey& key, TValue&& value)
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive{};
	std::remove_reference_t<TValue> actual{};
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(BitSerializer::KeyValue(key, value), outputArchive);
	BitSerializer::LoadObject<TArchive>(BitSerializer::KeyValue(key, actual), outputArchive);

	// Assert
	GTestExpectEq(value, actual);
}

#if defined(__cpp_lib_memory_resource)
/**
 * @brief Tests serialization and deserialization of containers using polymorphic allocators.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TValue The container type with polymorphic allocator.
 */
template <typename TArchive, typename TValue>
void TestSerializePmrType()
{
	static_assert(std::is_same_v<typename TValue::allocator_type, std::pmr::polymorphic_allocator<typename TValue::value_type>>,
		"TestSerializePmrType: Invalid container type. Expected a `std::pmr` container.");

	// Arrange
	typename TArchive::preferred_output_format outputArchive{};
	char buffer[512]{};
	std::pmr::monotonic_buffer_resource pool{ std::data(buffer), std::size(buffer) };
	TValue expected(&pool);
	::BuildFixture(expected);
	TValue actual(&pool);
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(expected, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	GTestExpectEq(expected, actual);
}
#endif

/**
 * @brief Tests loading data into a different target type than the source type.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TSource The source value type.
 * @tparam TExpected The expected target value type.
 * @param value Source value to serialize.
 * @param expected Expected value after deserialization.
 */
template <typename TArchive, typename TSource, typename TExpected>
void TestLoadingToDifferentType(TSource&& value, const TExpected& expected)
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive{};
	TExpected actual{};
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(value, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	GTestExpectEq(expected, actual);
}

/**
 * @brief Tests serialization and deserialization of a C-style array.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TValue Type of elements in the array.
 * @tparam SourceArraySize Size of the source array.
 * @tparam TargetArraySize Size of the target array.
 */
template<typename TArchive, typename TValue, size_t SourceArraySize = 7, size_t TargetArraySize = 7>
void TestSerializeArray()
{
	// Arrange
	TValue testArray[SourceArraySize]{};
	BuildFixture(testArray);
	typename TArchive::preferred_output_format outputArchive{};
	TValue actual[TargetArraySize]{};
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	for (size_t i = 0; i < (std::min)(SourceArraySize, TargetArraySize); i++)
	{
		GTestExpectEq(testArray[i], actual[i]);
	}
}

/**
 * @brief Tests serialization and deserialization of a named C-style array.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TValue Type of elements in the array.
 * @tparam SourceArraySize Size of the source array.
 * @tparam TargetArraySize Size of the target array.
 */
template<typename TArchive, typename TValue, size_t SourceArraySize = 7, size_t TargetArraySize = 7>
void TestSerializeArrayWithKey()
{
	// Arrange
	TValue testArray[SourceArraySize]{};
	BuildFixture(testArray);
	typename TArchive::preferred_output_format outputArchive{};
	TValue actual[TargetArraySize]{};
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(BitSerializer::KeyValue(L"Root", testArray), outputArchive);
	BitSerializer::LoadObject<TArchive>(BitSerializer::KeyValue(L"Root", actual), outputArchive);

	// Assert
	for (size_t i = 0; i < (std::min)(SourceArraySize, TargetArraySize); i++) {
		GTestExpectEq(testArray[i], actual[i]);
	}
}

/**
 * @brief Tests serialization and deserialization of a two-dimensional C-style array.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TValue Type of elements in the array.
 * @tparam ArraySize1 Number of rows.
 * @tparam ArraySize2 Number of columns.
 */
template<typename TArchive, typename TValue, size_t ArraySize1 = 3, size_t ArraySize2 = 5>
void TestSerializeTwoDimensionalArray()
{
	// Arrange
	TValue testArray[ArraySize1][ArraySize2]{};
	BuildFixture(testArray);
	typename TArchive::preferred_output_format outputArchive{};
	TValue actual[ArraySize1][ArraySize2]{};
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	for (size_t i = 0; i < ArraySize1; i++) {
		for (size_t c = 0; c < ArraySize2; c++) {
			GTestExpectEq(testArray[i][c], actual[i][c]);
		}
	}
}

/**
 * @brief Tests serialization and deserialization of a class through stream I/O.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam T The class type to test.
 * @param value Instance of the class to serialize/deserialize.
 */
template <typename TArchive, typename T>
void TestSerializeClassToStream(T&& value)
{
	// Arrange
	std::stringstream outputStream;
	std::decay_t<T> actual{};
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(value, outputStream);
	outputStream.seekg(0, std::ios::beg);
	BitSerializer::LoadObject<TArchive>(actual, outputStream);

	// Assert
	value.Assert(actual);
}

/**
 * @brief Tests serialization and deserialization of an array through stream I/O.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam T The element type of the array.
 * @tparam ArraySize Number of elements in the array.
 * @param testArray The array to serialize/deserialize.
 */
template <typename TArchive, typename T, size_t ArraySize = 3>
void TestSerializeArrayToStream(T(&testArray)[ArraySize])
{
	// Arrange
	std::stringstream outputStream;
	T actual[ArraySize]{};
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(testArray, outputStream);
	outputStream.seekg(0, std::ios::beg);
	BitSerializer::LoadObject<TArchive>(actual, outputStream);

	// Assert
	for (size_t i = 0; i < ArraySize; i++)
	{
		testArray[i].Assert(actual[i]);
	}
}

/**
 * @brief Tests serialization and deserialization of an array to/from a file.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam ArraySize Number of elements in the array.
 * @param testOverwrite Whether to overwrite existing files.
 */
template <typename TArchive, size_t ArraySize = 3>
void TestSerializeArrayToFile(bool testOverwrite = false)
{
	// Arrange
	auto testFilePath = std::filesystem::temp_directory_path() / ("TestArchive." + BitSerializer::Convert::ToString(TArchive::archive_type));
	if (!testOverwrite)
	{
		std::filesystem::remove(testFilePath);
	}
	TestPointClass testArray[ArraySize], actual[ArraySize];
	BuildFixture(testArray);
	::BuildFixture(actual);

	// Act
	BitSerializer::SaveObjectToFile<TArchive>(testArray, testFilePath, {}, testOverwrite);
	BitSerializer::LoadObjectFromFile<TArchive>(actual, testFilePath);

	// Assert
	for (size_t i = 0; i < ArraySize; i++)
	{
		testArray[i].Assert(actual[i]);
	}
}

/**
 * @brief Tests that an exception is thrown when trying to save to an existing file.
 *
 * @tparam TArchive The archive type used for serialization.
 */
template <typename TArchive>
void TestThrowExceptionWhenFileAlreadyExists()
{
	// Arrange
	const auto testFilePath = std::filesystem::temp_directory_path() / "TestArchive.data";
	std::ofstream stream;
	stream.open(testFilePath, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	ASSERT_TRUE(stream.is_open());
	stream << std::string("Test");
	stream.close();

	// Act
	try
	{
		TestPointClass testArray[1];
		BitSerializer::SaveObjectToFile<TArchive>(testArray, testFilePath);
		ASSERT_FALSE(true);
	}
	catch (BitSerializer::SerializationException& ex)
	{
		// Assert
		EXPECT_EQ(BitSerializer::SerializationErrorCode::InputOutputError, ex.GetErrorCode());
	}
}

/**
 * @brief Tests that deserializing into a non-empty container replaces its contents correctly.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TContainer The container type.
 * @param targetContainerSize Initial size of the target container before deserialization.
 */
template <typename TArchive, typename TContainer>
void TestLoadToNotEmptyContainer(size_t targetContainerSize)
{
	// Arrange
	typename TArchive::preferred_output_format outputArchive{};
	TContainer expected{};
	::BuildFixture(expected);
	TContainer actual(targetContainerSize);
	::BuildFixture(actual);

	// Act
	auto archiveData = BitSerializer::SaveObject<TArchive>(expected);
	BitSerializer::LoadObject<TArchive>(actual, archiveData);

	// Assert
	EXPECT_EQ(expected, actual);
}

/**
 * @brief Tests that a non-empty container is properly cleared when deserializing an empty array.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TContainer The container type.
 */
template<typename TArchive, typename TContainer>
void TestLoadingEmptyContainer()
{
	// Arrange
	TContainer emptyContainer;
	typename TArchive::preferred_output_format outputArchive{};
	TContainer actual;
	BuildFixture(actual);

	// Act
	BitSerializer::SaveObject<TArchive>(emptyContainer, outputArchive);
	BitSerializer::LoadObject<TArchive>(actual, outputArchive);

	// Assert
	ASSERT_TRUE(actual.empty());
}

/**
 * @brief Test that verifies that appropriate exception is thrown when validation fails.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam T The object type containing named fields.
 */
template <typename TArchive, class T>
void TestValidationForNamedValues()
{
	// Arrange
	T testObj[1]{};
	BuildFixture(testObj);
	typename TArchive::preferred_output_format outputArchive{};

	// Act
	bool result = false;
	BitSerializer::SaveObject<TArchive>(testObj, outputArchive);
	try
	{
		BitSerializer::LoadObject<TArchive>(testObj, outputArchive);
		result = true;
	}
	catch (BitSerializer::ValidationException& ex)
	{
		// Assert
		EXPECT_EQ(BitSerializer::SerializationErrorCode::FailedValidation, ex.GetErrorCode());
		EXPECT_EQ(1U, ex.GetValidationErrors().size());
	}

	EXPECT_FALSE(result);
}

/**
 * @brief Tests overflow handling policy during deserialzation.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TSourceType The original numeric type.
 * @tparam TTargetType The target numeric type.
 * @param overflowNumberPolicy Policy for handling overflows.
 */
template <typename TArchive, class TSourceType, class TTargetType>
void TestOverflowNumberPolicy(BitSerializer::OverflowNumberPolicy overflowNumberPolicy)
{
	// Arrange
	static_assert(std::is_arithmetic_v<TSourceType> && std::is_arithmetic_v<TTargetType>);
	static_assert(sizeof (TSourceType) >= sizeof (TTargetType));

	TSourceType testValue;
	if constexpr (std::is_floating_point_v<TTargetType>)
	{
		if constexpr (std::is_floating_point_v<TTargetType>) {
			testValue = TSourceType((std::numeric_limits<TTargetType>::max)()) * 1.00001;
		}
		else {
			testValue = (std::numeric_limits<TSourceType>::min)();
		}
	}
	else
	{
		if constexpr (std::is_floating_point_v<TSourceType>)
		{
			// Test cast from floating point number to integer
			testValue = TSourceType(3.141592654f);
		}
		else
		{
			if constexpr (std::is_signed_v<TTargetType>) {
				testValue = TSourceType((std::numeric_limits<TTargetType>::min)()) - 1;
			}
			else {
				testValue = TSourceType((std::numeric_limits<TTargetType>::max)()) + 1;
			}
		}
	}
	TestClassWithSubTypes<TSourceType, TSourceType> sourceObj[1]{ { testValue, BuildFixture<TSourceType>() } };
	TestClassWithSubTypes<TTargetType, TSourceType> targetObj[1];
	targetObj->WithRequired();

	BitSerializer::SerializationOptions options;
	options.overflowNumberPolicy = overflowNumberPolicy;
	typename TArchive::preferred_output_format outputArchive{};
	BitSerializer::SaveObject<TArchive>(sourceObj, outputArchive);

	// Act / Assert
	switch (overflowNumberPolicy)
	{
	case BitSerializer::OverflowNumberPolicy::ThrowError:
		try
		{
			BitSerializer::LoadObject<TArchive>(targetObj, outputArchive, options);
			EXPECT_TRUE(false);
		}
		catch (const BitSerializer::SerializationException& ex)
		{
			EXPECT_EQ(BitSerializer::SerializationErrorCode::Overflow, ex.GetErrorCode());
		}
		break;

	case BitSerializer::OverflowNumberPolicy::Skip:
		try
		{
			BitSerializer::LoadObject<TArchive>(targetObj, outputArchive, options);
			EXPECT_TRUE(false);
		}
		catch (const BitSerializer::ValidationException& ex)
		{
			EXPECT_EQ(BitSerializer::SerializationErrorCode::FailedValidation, ex.GetErrorCode());
			EXPECT_EQ(1U, ex.GetValidationErrors().size());
		}
		// Second value should be loaded
		GTestExpectEq(std::get<1>(sourceObj[0]), std::get<1>(targetObj[0]));
		break;
	}
}

/**
 * @brief Tests mismatched types policy behavior during deserialization.
 *
 * @tparam TArchive The archive type used for serialization.
 * @tparam TSourceType Original data type.
 * @tparam TTargetType Expected data type.
 * @param mismatchedTypesPolicy Policy for handling type mismatches.
 */
template <typename TArchive, class TSourceType, class TTargetType>
void TestMismatchedTypesPolicy(BitSerializer::MismatchedTypesPolicy mismatchedTypesPolicy)
{
	// Arrange
	static_assert(!std::is_same_v<TSourceType, TTargetType>);

	// Use array with one object for compatibility with formats like CSV
	TestClassWithSubTypes<TSourceType, TTargetType> sourceObj[1];
	BuildFixture(sourceObj);
	TestClassWithSubTypes<TTargetType, TTargetType> targetObj[1];
	// Loading with "Required" validator for force throw ValidationException
	targetObj->WithRequired();

	BitSerializer::SerializationOptions options;
	options.mismatchedTypesPolicy = mismatchedTypesPolicy;
	typename TArchive::preferred_output_format outputArchive{};
	BitSerializer::SaveObject<TArchive>(sourceObj, outputArchive);

	// Act / Assert
	try
	{
		BitSerializer::LoadObject<TArchive>(targetObj, outputArchive, options);
		EXPECT_TRUE(false);
	}
	catch (const BitSerializer::ValidationException& ex)
	{
		// Null values are handled separately and not affected by MismatchedTypesPolicy
		if (mismatchedTypesPolicy == BitSerializer::MismatchedTypesPolicy::ThrowError && !std::is_same_v<TSourceType, std::nullptr_t>)
		{
			EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		}
		else
		{
			EXPECT_EQ(BitSerializer::SerializationErrorCode::FailedValidation, ex.GetErrorCode());
			EXPECT_EQ(1U, ex.GetValidationErrors().size());
		}
		// Second value should still be loaded correctly
		GTestExpectEq(std::get<1>(sourceObj[0]), std::get<1>(targetObj[0]));
	}
	catch (const BitSerializer::SerializationException& ex)
	{
		// Null values are handled separately and not affected by MismatchedTypesPolicy
		if (mismatchedTypesPolicy == BitSerializer::MismatchedTypesPolicy::ThrowError && !std::is_same_v<TSourceType, std::nullptr_t>) {
			EXPECT_EQ(BitSerializer::SerializationErrorCode::MismatchedTypes, ex.GetErrorCode());
		}
		else
		{
			EXPECT_TRUE(false);
		}
	}
}

/**
 * @brief Tests UTF encoding error handling policy during serialization/deserialization.
 *
 * @tparam TArchive The archive type used for serialization.
 * @param utfEncodingErrorPolicy Policy for handling UTF encoding errors.
 */
template <typename TArchive>
void TestEncodingPolicy(BitSerializer::Convert::Utf::UtfEncodingErrorPolicy utfEncodingErrorPolicy)
{
	// Arrange: Create strings with invalid UTF sequences
	const std::string wrongUtf8(MakeStringFromSequence(0b11111110, 0b11111111));
	const std::string testUtf8Value = wrongUtf8 + "test_value" + wrongUtf8;

	const std::u16string wrongUtf16({ BitSerializer::Convert::Utf::UnicodeTraits::LowSurrogatesEnd, BitSerializer::Convert::Utf::UnicodeTraits::LowSurrogatesStart });
	const std::u16string testUtf16Value = wrongUtf16 + u"test_value" + wrongUtf16;

	TestClassWithSubTypes<std::string, std::u16string> sourceObj[1]{ { testUtf8Value, testUtf16Value } };
	TestClassWithSubTypes<std::u16string, std::string> targetObj[1];

	BitSerializer::SerializationOptions options;
	options.utfEncodingErrorPolicy = utfEncodingErrorPolicy;
	typename TArchive::preferred_output_format outputArchive{};

	// Act / Assert
	switch (utfEncodingErrorPolicy)
	{
	case BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::ThrowError:
		try
		{
			BitSerializer::SaveObject<TArchive>(sourceObj, outputArchive, options);
			BitSerializer::LoadObject<TArchive>(targetObj, outputArchive, options);
			EXPECT_TRUE(false) << "Should throw exception when encoding wrong UTF sequence";
		}
		catch (const BitSerializer::SerializationException& ex)
		{
			EXPECT_EQ(BitSerializer::SerializationErrorCode::UtfEncodingError, ex.GetErrorCode());
		}
		catch (...)
		{
			EXPECT_TRUE(false) << "Should throw BitSerializer::SerializationException type when encoding wrong UTF sequence";
		}
		break;

	case BitSerializer::Convert::Utf::UtfEncodingErrorPolicy::Skip:
		try
		{
			BitSerializer::SaveObject<TArchive>(sourceObj, outputArchive, options);
			BitSerializer::LoadObject<TArchive>(targetObj, outputArchive, options);

#pragma warning(push)
#pragma warning(disable: 4566)
			EXPECT_EQ(u"☐☐test_value☐☐", std::get<0>(targetObj[0]));
			EXPECT_EQ(UTF8("☐☐test_value☐☐"), std::get<1>(targetObj[0]));
#pragma warning(pop)
		}
		catch (...)
		{
			EXPECT_TRUE(false) << "Should not throw exception when policy is `Skip`";
		}
		break;
	}
}

/**
 * @brief Tests visiting keys in an object scope during deserialization.
 *
 * @tparam TArchive The archive type used for serialization.
 * @param skipValues If true, skips value validation after visiting keys.
 */
template <typename TArchive>
void TestVisitKeysInObjectScope(bool skipValues = false)
{
	// Arrange
	TestPointClass testObj[1];
	::BuildFixture(testObj);
	const std::map<typename TArchive::key_type, decltype(testObj->x), std::less<>> expectedValues {
		{ BitSerializer::Convert::To<typename TArchive::key_type>("x"), testObj->x },
		{ BitSerializer::Convert::To<typename TArchive::key_type>("y"), testObj->y }
	};

	using OutputFormat = typename TArchive::preferred_output_format;
	OutputFormat outputData{};
	BitSerializer::SaveObject<TArchive>(testObj, outputData);
	const BitSerializer::SerializationOptions options;
	BitSerializer::SerializationContext context(options);
	typename TArchive::input_archive_type inputArchive(static_cast<const OutputFormat&>(outputData), context);

	// Act / Assert
	auto arrScope = inputArchive.OpenArrayScope(std::size(testObj));
	ASSERT_TRUE(arrScope.has_value());
	auto objScope = arrScope->OpenObjectScope(0);
	ASSERT_TRUE(objScope.has_value());

	size_t index = 0;
	objScope->VisitKeys([&objScope, &expectedValues, &index, &skipValues](auto&& key)
	{
		using T = std::decay_t<decltype(key)>;
		ASSERT_TRUE(index < expectedValues.size());
		if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
		{
			auto it = expectedValues.find(key);
			ASSERT_TRUE(it != expectedValues.cend());

			if (!skipValues)
			{
				decltype(testObj->x) actualValue{};
				objScope->SerializeValue(key, actualValue);
				EXPECT_EQ(it->second, actualValue);
			}
		}
		++index;
	});
	EXPECT_EQ(expectedValues.size(), index);
}

// NOLINTEND(bugprone-unchecked-optional-access)
