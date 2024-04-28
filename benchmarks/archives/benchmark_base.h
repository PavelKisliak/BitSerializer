/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <string>
#include <cstdint>
#include "testing_tools/common_test_methods.h"
#include "bitserializer/bit_serializer.h"
#include "bitserializer/types/std/array.h"


/// <summary>
/// Base class of archive benchmark.
/// </summary>
template <typename TArchive, typename TModel, typename TKeyCharType = char>
class CBenchmarkBase
{
public:
	using archive_t = TArchive;
	using model_t = TModel;
	using key_char_t = TKeyCharType;
	using key_string_t = std::basic_string<TKeyCharType, std::char_traits<TKeyCharType>>;
	using out_string_stream_t = std::basic_ostringstream<TKeyCharType, std::char_traits<TKeyCharType>, std::allocator<TKeyCharType>>;
	using in_string_stream_t = std::basic_ostringstream<TKeyCharType, std::char_traits<TKeyCharType>, std::allocator<TKeyCharType>>;
	using output_format_t = typename TArchive::preferred_output_format;

	virtual ~CBenchmarkBase() = default;

	/// <summary>
	/// Returns name of testing archive.
	/// </summary>
	[[nodiscard]] virtual std::string GetArchiveName() const
	{
		return BitSerializer::Convert::ToString(TArchive::archive_type);
	}

	/// <summary>
	/// Returns `true` when archive uses third party library for serialization.
	/// </summary>
	[[nodiscard]] virtual bool IsUseNativeLib() const { return false; }

	/// <summary>
	/// Returns the number of fields in the model.
	/// </summary>
	constexpr size_t GetTotalFieldsCount() noexcept
	{
		if constexpr (BitSerializer::has_size_v<model_t>)
		{
			assert(!mSourceTestModel.empty());
			// Total size of all fields (each element of array also counting as field)
			return mSourceTestModel.size() * model_t::value_type::GetTotalFieldsCount() + mSourceTestModel.size();
		}
		else {
			return model_t::GetTotalFieldsCount();
		}
	}

	/// <summary>
	/// Prepares model for test.
	/// </summary>
	virtual void Prepare()
	{
		BuildFixture(mSourceTestModel);
	}

	/// <summary>
	/// Saves model via BitSerializer.
	/// </summary>
	virtual size_t SaveModelViaBitSerializer()
	{
		mBitSerializerOutputData = BitSerializer::SaveObject<archive_t>(mSourceTestModel);
		return std::size(mBitSerializerOutputData);
	}

	/// <summary>
	/// Saves model via library which uses as base for BitSerializer's archive.
	/// </summary>
	virtual size_t SaveModelViaNativeLib() { return 0; }

	/// <summary>
	/// Loads model via BitSerializer.
	/// </summary>
	virtual size_t LoadModelViaBitSerializer()
	{
		BitSerializer::LoadObject<TArchive>(mBitSerializerModel, mBitSerializerOutputData);
		return std::size(mBitSerializerOutputData);
	}

	/// <summary>
	/// Loads model via library which uses as base for BitSerializer's archive.
	/// </summary>
	virtual size_t LoadModelViaNativeLib() { return 0; }

	/// <summary>
	/// Asserts loaded data (BitSerializer and native library implementations).
	/// Must be not counted, make sense to invoke only on the first iteration of the benchmark.
	/// </summary>
	virtual void Assert() const
	{
		if constexpr (has_assert_method_v<TModel>)
		{
			mSourceTestModel.Assert(mBitSerializerModel);
		}
	}

protected:
	TModel mSourceTestModel;
	output_format_t mBitSerializerOutputData;
	TModel mBitSerializerModel;
};


/// <summary>
/// Test model with set of basic types.
/// </summary>
template <typename TKeyCharType>
class TestModelWithBasicTypes
{
public:
	using key_char_t = TKeyCharType;
	using string_t = std::basic_string<TKeyCharType, std::char_traits<TKeyCharType>>;

	static void BuildFixture(TestModelWithBasicTypes& fixture)
	{
		::BuildFixture(fixture.mTestBoolValue);
		::BuildFixture(fixture.mTestCharValue);
		::BuildFixture(fixture.mTestInt64Value);
		::BuildFixture(fixture.mTestFloatValue);
		::BuildFixture(fixture.mTestDoubleValue);
		::BuildFixture(fixture.mTestString1);
		::BuildFixture(fixture.mTestString2);
		if constexpr (std::is_same_v<TKeyCharType, char>)
		{
			fixture.mTestUnicodeString = UTF8("Съешь ещё этих мягких французских булок, да выпей чаю");
			fixture.mStringWithQuotes = "Test \"<quoted>\" string";
			fixture.mMultiLineString = "Test\nmulti\nline\nstring";
		}
		else
		{
			fixture.mTestUnicodeString = L"Съешь ещё этих мягких французских булок, да выпей чаю";
			fixture.mStringWithQuotes = L"Test \"<quoted>\" string";
			fixture.mMultiLineString = L"Test\nmulti\nline\nstring";
		}
	}

	static constexpr size_t GetTotalFieldsCount() noexcept
	{
		return 10;
	}

	template <typename T>
	static void Assert(const char* fieldName, const T& val1, const T& val2)
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			// Approximately compare floating point numbers due to possible loss precision when store in the text formats
			if (ApproximatelyEqual(val1, val2, std::numeric_limits<T>::epsilon() * 5))
			{
				return;
			}
		}
		else
		{
			if (val1 == val2)
			{
				return;
			}
		}
		throw std::runtime_error(std::string(" - Failed verification on field '") + fieldName + "': " + BitSerializer::Convert::ToString(val1) + " != " + BitSerializer::Convert::ToString(val2));
	}

	void Assert(const TestModelWithBasicTypes& rhs) const
	{
		Assert("TestBoolValue", mTestBoolValue, rhs.mTestBoolValue);
		Assert("TestCharValue", mTestCharValue, rhs.mTestCharValue);
		Assert("TestInt64Value", mTestInt64Value, rhs.mTestInt64Value);
		Assert("TestFloatValue", mTestFloatValue, rhs.mTestFloatValue);
		Assert("TestDoubleValue", mTestDoubleValue, rhs.mTestDoubleValue);
		Assert("TestString1", mTestString1, rhs.mTestString1);
		Assert("TestString2", mTestString2, rhs.mTestString2);
		Assert("TestString3", mTestUnicodeString, rhs.mTestUnicodeString);
		Assert("StringWithQuotes", mStringWithQuotes, rhs.mStringWithQuotes);
		Assert("MultiLineString", mMultiLineString, rhs.mMultiLineString);
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		if constexpr (std::is_same_v<TKeyCharType, char>)
		{
			archive << BitSerializer::KeyValue("TestBoolValue", mTestBoolValue);
			archive << BitSerializer::KeyValue("TestCharValue", mTestCharValue);
			archive << BitSerializer::KeyValue("TestInt64Value", mTestInt64Value);
			archive << BitSerializer::KeyValue("TestFloatValue", mTestFloatValue);
			archive << BitSerializer::KeyValue("TestDoubleValue", mTestDoubleValue);
			archive << BitSerializer::KeyValue("TestString1", mTestString1);
			archive << BitSerializer::KeyValue("TestString2", mTestString2);
			archive << BitSerializer::KeyValue("TestString3", mTestUnicodeString);
			archive << BitSerializer::KeyValue("StringWithQuotes", mStringWithQuotes);
			archive << BitSerializer::KeyValue("MultiLineString", mMultiLineString);
		}
		else
		{
			archive << BitSerializer::KeyValue(L"TestBoolValue", mTestBoolValue);
			archive << BitSerializer::KeyValue(L"TestCharValue", mTestCharValue);
			archive << BitSerializer::KeyValue(L"TestInt64Value", mTestInt64Value);
			archive << BitSerializer::KeyValue(L"TestFloatValue", mTestFloatValue);
			archive << BitSerializer::KeyValue(L"TestDoubleValue", mTestDoubleValue);
			archive << BitSerializer::KeyValue(L"TestString1", mTestString1);
			archive << BitSerializer::KeyValue(L"TestString2", mTestString2);
			archive << BitSerializer::KeyValue(L"TestString3", mTestUnicodeString);
			archive << BitSerializer::KeyValue(L"StringWithQuotes", mStringWithQuotes);
			archive << BitSerializer::KeyValue(L"MultiLineString", mMultiLineString);
		}
	}

	bool mTestBoolValue = false;
	char mTestCharValue = 0;
	int64_t mTestInt64Value = 0;
	float mTestFloatValue = 0.f;
	double mTestDoubleValue = 0.0;
	string_t mTestString1;
	string_t mTestString2;
	string_t mTestUnicodeString;
	string_t mStringWithQuotes;
	string_t mMultiLineString;
};

// Common test model for all archives (represents an array of objects)
template <typename TKeyCharType = char, size_t ArraySize=30>
using CommonTestModel = std::array<TestModelWithBasicTypes<TKeyCharType>, ArraySize>;
