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
			{"active", model.active},
			{"id", model.id},
			{"stock", model.stock},
			{"price", model.price},
			{"rating", model.rating},
			{"name", model.name},
			{"detailedProductDescription", model.detailedProductDescription},
			{"localizedName", model.localizedName},
			{"note", model.note},
			{"specifications", model.specifications}
		};
	}

	static void from_json(const json& j, BasicTestModel& model)
	{
		j.at("active").get_to(model.active);
		j.at("id").get_to(model.id);
		j.at("stock").get_to(model.stock);
		j.at("price").get_to(model.price);
		j.at("rating").get_to(model.rating);
		j.at("name").get_to(model.name);
		j.at("detailedProductDescription").get_to(model.detailedProductDescription);
		j.at("localizedName").get_to(model.localizedName);
		j.at("note").get_to(model.note);
		j.at("specifications").get_to(model.specifications);
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
