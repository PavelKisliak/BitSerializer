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
		root |= ryml::SEQ;

		// Save array of objects
		for (const auto& obj : sourceTestModel)
		{
			auto yamlObj = root.append_child();
			yamlObj |= ryml::MAP;
			yamlObj.append_child() << ryml::key("active") << c4::fmt::boolalpha(obj.active);
			yamlObj.append_child() << ryml::key("id") << obj.id;
			yamlObj.append_child() << ryml::key("stock") << obj.stock;
			yamlObj.append_child() << ryml::key("price")
				<< c4::fmt::real(obj.price, std::numeric_limits<float>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
			yamlObj.append_child() << ryml::key("rating")
				<< c4::fmt::real(obj.rating, std::numeric_limits<double>::max_digits10, c4::RealFormat_e::FTOA_SCIENT);
			yamlObj.append_child() << ryml::key("name") << obj.name;
			yamlObj.append_child() << ryml::key("detailedProductDescription") << obj.detailedProductDescription;
			yamlObj.append_child() << ryml::key("localizedName") << obj.localizedName;
			yamlObj.append_child() << ryml::key("note") << obj.note;
			yamlObj.append_child() << ryml::key("specifications") << obj.specifications;
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
			yamlVal["active"] >> obj.active;
			yamlVal["id"] >> obj.id;
			yamlVal["stock"] >> obj.stock;
			yamlVal["price"] >> obj.price;
			yamlVal["rating"] >> obj.rating;
			yamlVal["name"] >> obj.name;
			yamlVal["detailedProductDescription"] >> obj.detailedProductDescription;
			yamlVal["localizedName"] >> obj.localizedName;
			yamlVal["note"] >> obj.note;
			yamlVal["specifications"] >> obj.specifications;
		}
	}
};
