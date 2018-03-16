/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "gtest/gtest.h"
#include "auto_fixture.h"

/// <summary>
/// Test template of serialization for fundamental type.
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename T>
void TestSerializeType(T&& value)
{
	typename TArchive::output_format outputArchive;
	::BitSerializer::SaveObject<TArchive>(value, outputArchive);
	ASSERT_FALSE(outputArchive.empty());

	std::decay_t<T> actual;
	::BitSerializer::LoadObject<TArchive>(actual, outputArchive);
	EXPECT_EQ(value, actual);
}

/// <summary>
/// Test template of serialization for c-array.
/// </summary>
/// <param name="value">The value.</param>
template<typename TArchive, typename TValue, size_t ArraySize = 7>
void TestSerializeArray()
{
	TValue testArray[ArraySize];
	BuildFixture(testArray);

	typename TArchive::output_format outputArchive;
	::BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	ASSERT_FALSE(outputArchive.empty());

	TValue actual[ArraySize];
	::BitSerializer::LoadObject<TArchive>(actual, outputArchive);
	for (size_t i = 0; i < ArraySize; i++) {
		ASSERT_EQ(testArray[i], actual[i]);
	}
}

/// <summary>
/// Test template of serialization for two-dimensional c-array.
/// </summary>
/// <param name="value">The value.</param>
template<typename TArchive, typename TValue, size_t ArraySize1 = 3, size_t ArraySize2 = 5>
void TestSerializeTwoDimensionalArray()
{
	TValue testArray[ArraySize1][ArraySize2];
	BuildFixture(testArray);

	typename TArchive::output_format outputArchive;
	::BitSerializer::SaveObject<TArchive>(testArray, outputArchive);
	ASSERT_FALSE(outputArchive.empty());

	TValue actual[ArraySize1][ArraySize2];
	::BitSerializer::LoadObject<TArchive>(actual, outputArchive);
	for (size_t i = 0; i < ArraySize1; i++) {
		for (size_t c = 0; c < ArraySize2; c++) {
			ASSERT_EQ(testArray[i][c], actual[i][c]);
		}
	}
}

/// <summary>
/// Test template of serialization for class (must have constant method Assert()).
/// </summary>
/// <param name="value">The value.</param>
template <typename TArchive, typename T>
void TestSerializeClass(T&& value)
{
	typename TArchive::output_format outputArchive;
	::BitSerializer::SaveObject<TArchive>(value, outputArchive);
	ASSERT_FALSE(outputArchive.empty());

	std::decay_t<T> actual;
	::BitSerializer::LoadObject<TArchive>(actual, outputArchive);
	actual.Assert(value);
}
