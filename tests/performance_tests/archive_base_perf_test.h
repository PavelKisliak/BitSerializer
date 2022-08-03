/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>
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
	/// Prepares model for test.
	/// </summary>
	virtual void Prepare()
	{
		BuildFixture<TModel>(mSourceTestModel);
	}

	/// <summary>
	/// Saves model via BitSerializer.
	/// </summary>
	virtual void SaveModelViaBitSerializer()
	{
		mBitSerializerOutputData = BitSerializer::SaveObject<archive_t>(mSourceTestModel);
	}

	/// <summary>
	/// Saves model via library which uses as base for BitSerializer's archive.
	/// </summary>
	virtual void SaveModelViaNativeLib() {}

	/// <summary>
	/// Loads model via BitSerializer.
	/// </summary>
	virtual void LoadModelViaBitSerializer()
	{
		BitSerializer::LoadObject<TArchive>(mBitSerializerModel, mBitSerializerOutputData);
	}

	/// <summary>
	/// Loads model via library which uses as base for BitSerializer's archive.
	/// </summary>
	virtual void LoadModelViaNativeLib() {}

	/// <summary>
	/// Asserts loaded data (BitSerilizer and native library implementations).
	/// Make sense to invoke only on the first iteration of the performance test.
	/// </summary>
	virtual void Assert() const
	{
		mSourceTestModel.Assert(mBitSerializerModel);
	}

protected:
	TModel mSourceTestModel;
	output_format_t mBitSerializerOutputData;
	TModel mBitSerializerModel;
};
