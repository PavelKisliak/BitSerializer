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
 * @brief Serializes the `BasicTestModel` using NlohmannJson.
 */
template<>
struct nlohmann::adl_serializer<BasicTestModel>
{
	static void to_json(json& j, const BasicTestModel& model)
	{
		j = {
			{"BooleanValue", model.booleanValue},
			{"SignedIntValue", model.signedIntValue},
			{"UnsignedIntValue", model.unsignedIntValue},
			{"FloatValue", model.floatValue},
			{"DoubleValue", model.doubleValue},
			{"ShortString", model.shortString},
			{"StringWithLongKeyAndValue", model.stringWithLongKeyAndValue},
			{"UnicodeString", model.unicodeString},
			{"StringWithEscapedChars", model.stringWithEscapedChars},
			{"MultiLineString", model.multiLineString}
		};
	}

	static void from_json(const json& j, BasicTestModel& model)
	{
		j.at("BooleanValue").get_to(model.booleanValue);
		j.at("SignedIntValue").get_to(model.signedIntValue);
		j.at("UnsignedIntValue").get_to(model.unsignedIntValue);
		j.at("FloatValue").get_to(model.floatValue);
		j.at("DoubleValue").get_to(model.doubleValue);
		j.at("ShortString").get_to(model.shortString);
		j.at("StringWithLongKeyAndValue").get_to(model.stringWithLongKeyAndValue);
		j.at("UnicodeString").get_to(model.unicodeString);
		j.at("StringWithEscapedChars").get_to(model.stringWithEscapedChars);
		j.at("MultiLineString").get_to(model.multiLineString);
	}
};

/**
 * @brief Benchmark implementation for the NlohmannJson library.
 */
class NlohmannJsonBenchmark : public BenchmarkBase
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
	void BenchmarkSaveToMemory(const CommonTestModel& sourceTestModel, std::string& outputData) override
	{
		nlohmann::json json = sourceTestModel;
		outputData = json.dump();
	}

	void BenchmarkLoadFromMemory(CommonTestModel& targetTestModel, const std::string& sourceData) override
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
class NlohmannMsgPackBenchmark : public BenchmarkBase
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
	void BenchmarkSaveToMemory(const CommonTestModel& sourceTestModel, std::string& outputData) override
	{
		nlohmann::json json = sourceTestModel;
		auto msgPackData = nlohmann::json::to_msgpack(json);
		outputData.assign(reinterpret_cast<const char*>(msgPackData.data()), msgPackData.size());
	}

	void BenchmarkLoadFromMemory(CommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		auto json = nlohmann::json::from_msgpack(sourceData);
		if (json.is_discarded() || !json.is_array()) {
			throw std::runtime_error("NlohmannJson parse error");
		}
		json.get_to(targetTestModel);
	}
};
