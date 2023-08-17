/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cassert>
#include <string>
#include "base_test_models.h"
#include "bitserializer/bit_serializer.h"


/// <summary>
/// Base class for test serialization performance.
/// Override for implementing load/save via native library.
/// </summary>
template <typename TArchive, typename TModel, typename TKeyCharType = char>
class CArchiveBasePerfTest
{
public:
	using archive_t = TArchive;
	using model_t = TModel;
	using key_char_t = TKeyCharType;
	using key_string_t = std::basic_string<TKeyCharType, std::char_traits<TKeyCharType>>;
	using out_string_stream_t = std::basic_ostringstream<TKeyCharType, std::char_traits<TKeyCharType>, std::allocator<TKeyCharType>>;
	using in_string_stream_t = std::basic_ostringstream<TKeyCharType, std::char_traits<TKeyCharType>, std::allocator<TKeyCharType>>;
	using output_format_t = typename TArchive::preferred_output_format;

	virtual ~CArchiveBasePerfTest() = default;

	/// <summary>
	/// Returns name of testing archive.
	/// </summary>
	virtual std::string GetArchiveName() const
	{
		return BitSerializer::Convert::ToString(TArchive::archive_type);
	}

	/// <summary>
	/// Returns `true` when archive uses third party library for serialization.
	/// </summary>
	virtual bool IsUseNativeLib() const { return false; }

	/// <summary>
	/// Returns the number of fields in the model.
	/// </summary>
	constexpr size_t GetTotalFieldsCount() noexcept
	{
		if constexpr (BitSerializer::has_size_v<model_t>) {
			assert(!mSourceTestModel.empty());
			return mSourceTestModel.size() * model_t::value_type::GetTotalFieldsCount();
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
	/// Asserts loaded data (BitSerilizer and native library implementations).
	/// Make sense to invoke only on the first iteration of the performance test.
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
