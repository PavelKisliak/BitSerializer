/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
// Benchmark headers
#include "benchmark_base.h"

// NlohmannJson library
#include "nlohmann/json.hpp"

/**
 * @brief Serializes the `CBasicTestModel` using NlohmannJson.
 */
template<>
struct nlohmann::adl_serializer<CBasicTestModel>
{
	static void to_json(json& j, const CBasicTestModel& model)
	{
		j = {
			{"BooleanValue", model.BooleanValue},
			{"SignedIntValue", model.SignedIntValue},
			{"UnsignedIntValue", model.UnsignedIntValue},
			{"FloatValue", model.FloatValue},
			{"DoubleValue", model.DoubleValue},
			{"ShortString", model.ShortString},
			{"StringWithLongKeyAndValue", model.StringWithLongKeyAndValue},
			{"UnicodeString", model.UnicodeString},
			{"StringWithEscapedChars", model.StringWithEscapedChars},
			{"MultiLineString", model.MultiLineString}
		};
	}

	static void from_json(const json& j, CBasicTestModel& model)
	{
		j.at("BooleanValue").get_to(model.BooleanValue);
		j.at("SignedIntValue").get_to(model.SignedIntValue);
		j.at("UnsignedIntValue").get_to(model.UnsignedIntValue);
		j.at("FloatValue").get_to(model.FloatValue);
		j.at("DoubleValue").get_to(model.DoubleValue);
		j.at("ShortString").get_to(model.ShortString);
		j.at("StringWithLongKeyAndValue").get_to(model.StringWithLongKeyAndValue);
		j.at("UnicodeString").get_to(model.UnicodeString);
		j.at("StringWithEscapedChars").get_to(model.StringWithEscapedChars);
		j.at("MultiLineString").get_to(model.MultiLineString);
	}
};

/**
 * @brief Benchmark implementation for the NlohmannJson library.
 */
class CNlohmannJsonBenchmark : public CBenchmarkBase
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
		return "NlohmannJson-Json";
	}

	[[nodiscard]] std::vector<TestStage> GetStagesList() const override
	{
		// Exclude serialization tests to streams
		return { TestStage::SaveToMemory, TestStage::LoadFromMemory };
	}

protected:
	void BenchmarkSaveToMemory(const CCommonTestModel& sourceTestModel, std::string& outputData) override
	{
		nlohmann::json json = sourceTestModel;
		outputData = json.dump();
	}

	void BenchmarkLoadFromMemory(CCommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		auto json = nlohmann::json::parse(sourceData);
		if (json.is_discarded() || !json.is_array()) {
			throw std::runtime_error("NlohmannJson parse error");
		}
		json.get_to(targetTestModel);
	}
};

/**
 * @brief Benchmark implementation for the NlohmannJson library (MsgPack).
 */
class CNlohmannMsgPackBenchmark : public CBenchmarkBase
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
		return "NlohmannJson-MsgPack";
	}

	[[nodiscard]] std::vector<TestStage> GetStagesList() const override
	{
		// Exclude serialization tests to streams
		return { TestStage::SaveToMemory, TestStage::LoadFromMemory };
	}

protected:
	void BenchmarkSaveToMemory(const CCommonTestModel& sourceTestModel, std::string& outputData) override
	{
		nlohmann::json json = sourceTestModel;
		auto msgPackData = nlohmann::json::to_msgpack(json);
		outputData.assign(reinterpret_cast<const char*>(msgPackData.data()), msgPackData.size());
	}

	void BenchmarkLoadFromMemory(CCommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		auto json = nlohmann::json::from_msgpack(sourceData);
		if (json.is_discarded() || !json.is_array()) {
			throw std::runtime_error("NlohmannJson parse error");
		}
		json.get_to(targetTestModel);
	}
};
