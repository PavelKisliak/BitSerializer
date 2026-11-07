/*******************************************************************************
* Copyright (C) 2018-2026 by Artsiom Marozau, Pavel Kisliak                    *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
// Benchmark headers
#include "benchmark_base.h"

// RapidYaml library
#include "c4/format.hpp"
#include "ryml/ryml.hpp"


/**
 * @brief Benchmark implementation for the RapidYaml library.
 */
class RapidYamlBenchmark final : public BenchmarkBase
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
		return "RapidYaml";
	}

	[[nodiscard]] std::vector<TestStage> GetStagesList() const override
	{
		// Exclude serialization tests to streams (not supported by RapidYaml)
		return { TestStage::SaveToMemory, TestStage::LoadFromMemory };
	}

protected:
	void BenchmarkSaveToMemory(const CommonTestModel& sourceTestModel, std::string& outputData) override
	{
		ryml::Tree tree;
		auto root = tree.rootref();
		root.set_seq();

		// Save array of objects
		for (const auto& obj : sourceTestModel)
		{
			auto yamlObj = root.append_child();
			yamlObj.set_map();
			yamlObj["active"].save(c4::fmt::boolalpha(obj.active));
			yamlObj["id"].save(obj.id);
			yamlObj["stock"].save(obj.stock);
			yamlObj["price"].save(
				c4::fmt::real(obj.price, std::numeric_limits<float>::max_digits10, c4::RealFormat_e::FTOA_SCIENT));
			yamlObj["rating"].save(
				c4::fmt::real(obj.rating, std::numeric_limits<double>::max_digits10, c4::RealFormat_e::FTOA_SCIENT));
			yamlObj["name"].save(obj.name);
			yamlObj["detailedProductDescription"].save(obj.detailedProductDescription);
			yamlObj["localizedName"].save(obj.localizedName);
			yamlObj["note"].save(obj.note);
			yamlObj["specifications"].save(obj.specifications);
		}

		outputData = ryml::emitrs_yaml<std::string>(tree);
	}

	void BenchmarkLoadFromMemory(CommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		ryml::Tree tree = ryml::parse_in_arena(c4::to_csubstr(sourceData));
		auto root = tree.rootref();

		// Load array of objects
		for (size_t i = 0; i < targetTestModel.size(); ++i)
		{
			auto& obj = targetTestModel[i];
			auto yamlVal = root[i];
			yamlVal["active"].load(&obj.active);
			yamlVal["id"].load(&obj.id);
			yamlVal["stock"].load(&obj.stock);
			yamlVal["price"].load(&obj.price);
			yamlVal["rating"].load(&obj.rating);
			yamlVal["name"].load(&obj.name);
			yamlVal["detailedProductDescription"].load(&obj.detailedProductDescription);
			yamlVal["localizedName"].load(&obj.localizedName);
			yamlVal["note"].load(&obj.note);
			yamlVal["specifications"].load(&obj.specifications);
		}
	}
};
