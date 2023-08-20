/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
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
		::BuildFixture(fixture.mTestString3);
		if constexpr (std::is_same_v<TKeyCharType, char>)
		{
			fixture.mStringWithQuotes = "Test \"<quoted>\" string";
			fixture.mMultiLineString = "Test\nmulti\nline\nstring";
		}
		else
		{
			fixture.mStringWithQuotes = L"Test \"<quoted>\" string";
			fixture.mMultiLineString = L"Test\nmulti\nline\nstring";
		}
	}

	static constexpr size_t GetTotalFieldsCount() noexcept
	{
		return 10;
	}

	void Assert(const TestModelWithBasicTypes& rhs) const
	{
		assert(mTestBoolValue == rhs.mTestBoolValue);
		assert(mTestCharValue == rhs.mTestCharValue);
		assert(mTestInt64Value == rhs.mTestInt64Value);
		assert(mTestFloatValue == rhs.mTestFloatValue);
		assert(mTestDoubleValue == rhs.mTestDoubleValue);
		assert(mTestString1 == rhs.mTestString1);
		assert(mTestString2 == rhs.mTestString2);
		assert(mTestString3 == rhs.mTestString3);
		assert(mStringWithQuotes == rhs.mStringWithQuotes);
		assert(mMultiLineString == rhs.mMultiLineString);
	}

	template <class TArchive>
	void Serialize(TArchive& archive)
	{
		if constexpr (std::is_same_v<TKeyCharType, char>)
		{
			archive << BitSerializer::MakeKeyValue("TestBoolValue", mTestBoolValue);
			archive << BitSerializer::MakeKeyValue("TestCharValue", mTestCharValue);
			archive << BitSerializer::MakeKeyValue("TestInt64Value", mTestInt64Value);
			archive << BitSerializer::MakeKeyValue("TestFloatValue", mTestFloatValue);
			archive << BitSerializer::MakeKeyValue("TestDoubleValue", mTestDoubleValue);
			archive << BitSerializer::MakeKeyValue("TestString1", mTestString1);
			archive << BitSerializer::MakeKeyValue("TestString2", mTestString2);
			archive << BitSerializer::MakeKeyValue("TestString3", mTestString3);
			archive << BitSerializer::MakeKeyValue("StringWithQuotes", mStringWithQuotes);
			archive << BitSerializer::MakeKeyValue("MultiLineString", mMultiLineString);
		}
		else
		{
			archive << BitSerializer::MakeKeyValue(L"TestBoolValue", mTestBoolValue);
			archive << BitSerializer::MakeKeyValue(L"TestCharValue", mTestCharValue);
			archive << BitSerializer::MakeKeyValue(L"TestInt64Value", mTestInt64Value);
			archive << BitSerializer::MakeKeyValue(L"TestFloatValue", mTestFloatValue);
			archive << BitSerializer::MakeKeyValue(L"TestDoubleValue", mTestDoubleValue);
			archive << BitSerializer::MakeKeyValue(L"TestString1", mTestString1);
			archive << BitSerializer::MakeKeyValue(L"TestString2", mTestString2);
			archive << BitSerializer::MakeKeyValue(L"TestString3", mTestString3);
			archive << BitSerializer::MakeKeyValue(L"StringWithQuotes", mStringWithQuotes);
			archive << BitSerializer::MakeKeyValue(L"MultiLineString", mMultiLineString);
		}
	}

	bool mTestBoolValue = false;
	char mTestCharValue = 0;
	int64_t mTestInt64Value = 0;
	float mTestFloatValue = 0.f;
	double mTestDoubleValue = 0.0;
	string_t mTestString1;
	string_t mTestString2;
	string_t mTestString3;
	string_t mStringWithQuotes;
	string_t mMultiLineString;
};

// Common test model for all archives (represents an array of objects)
template <typename TKeyCharType = char, size_t ArraySize=30>
using CommonTestModel = std::array<TestModelWithBasicTypes<TKeyCharType>, ArraySize>;
