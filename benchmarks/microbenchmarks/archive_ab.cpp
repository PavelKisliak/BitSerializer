/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "bench_harness.h"
#include "perf_utils.h"

#if defined(JSON_BENCHMARK)

#include "bitserializer/bit_serializer.h"
#include "bitserializer/json_archive.h"

#include "benchmark_models.h"

namespace
{
	// Volatile sink so the compiler cannot remove the measured work.
	volatile std::uint64_t gSink = 0;

	// Two instantiations of the same archive that differ only in one component
	// (here: the format trait, strict JSON vs JSONC). Both coexist in this binary,
	// so the intra-binary harness makes the delta layout-robust.
	//
	// To A/B your own change, replace `VariantArchive` with a second instantiation
	// that differs only in the component you changed, e.g.:
	//
	//   using VariantArchive = BitSerializer::ArchiveBase<
	//       BitSerializer::Detail::JsonArchiveTraits<BitSerializer::ArchiveType::Json>,
	//       BitSerializer::Detail::JsonReadRootScope<BitSerializer::ArchiveType::Json>,
	//       MyModifiedWriteScope<BitSerializer::ArchiveType::Json>>;
	//
	// Do not fork the whole archive: keep the shared components and swap only the
	// changed one (traits, scope, or a policy/template parameter).
	using BaseArchive = BitSerializer::Json::JsonArchive;
	using VariantArchive = BitSerializer::Json::JsoncArchive;

	/// Per-model state that must outlive the single `Runner::RunAll()` call.
	struct IModelState
	{
		virtual ~IModelState() = default;
	};

	template <typename TModel>
	struct ModelState final : IModelState
	{
		TModel model;
		std::string baseData;
		std::string variantData;
		TModel loadedBase;
		TModel loadedVariant;
	};

	/**
	 * @brief Prepares one model and registers its Save/Load A/B pairs.
	 *
	 * @tparam TModel Model type to benchmark.
	 */
	template <typename TModel>
	void AddModel(BenchHarness::Runner& runner, std::vector<std::unique_ptr<IModelState>>& states, const std::string& modelName)
	{
		auto state = std::make_unique<ModelState<TModel>>();
		state->model = TModel::Make();
		BitSerializer::SaveObject<BaseArchive>(state->model, state->baseData);
		BitSerializer::SaveObject<VariantArchive>(state->model, state->variantData);

		// One-time correctness check before measuring (both archives).
		{
			TModel check;
			BitSerializer::LoadObject<BaseArchive>(check, state->baseData);
			if (!check.Equals(state->model))
			{
				std::cerr << modelName << ": base round-trip check failed" << std::endl;
				return;
			}
			BitSerializer::LoadObject<VariantArchive>(check, state->variantData);
			if (!check.Equals(state->model))
			{
				std::cerr << modelName << ": variant round-trip check failed" << std::endl;
				return;
			}
		}

		ModelState<TModel>* raw = state.get();
		runner.Add(modelName + " | save: base vs variant",
			[raw] {
				raw->baseData.clear();
				BitSerializer::SaveObject<BaseArchive>(raw->model, raw->baseData);
				gSink += raw->baseData.size();
			},
			[raw] {
				raw->variantData.clear();
				BitSerializer::SaveObject<VariantArchive>(raw->model, raw->variantData);
				gSink += raw->variantData.size();
			});
		runner.Add(modelName + " | load: base vs variant",
			[raw] {
				raw->loadedBase = TModel{};
				BitSerializer::LoadObject<BaseArchive>(raw->loadedBase, raw->baseData);
				gSink += raw->loadedBase.Checksum();
			},
			[raw] {
				raw->loadedVariant = TModel{};
				BitSerializer::LoadObject<VariantArchive>(raw->loadedVariant, raw->variantData);
				gSink += raw->loadedVariant.Checksum();
			});
		states.push_back(std::move(state));
	}

	struct ModelEntry
	{
		const char* name;
		const char* description;
		void (*add)(BenchHarness::Runner&, std::vector<std::unique_ptr<IModelState>>&, const std::string&);
	};

	constexpr ModelEntry Models[] = {
		{ "mixed",        "a bit of everything",                   &AddModel<MixedBenchmarkModel> },
		{ "numeric",      "all fundamental number types",          &AddModel<NumericBenchmarkModel> },
		{ "strings",      "ASCII strings of different lengths",    &AddModel<StringBenchmarkModel> },
		{ "escaped",      "quotes/backslashes/control characters", &AddModel<EscapedBenchmarkModel> },
		{ "utf8",         "1/2/3/4-byte UTF-8 sequences",          &AddModel<Utf8BenchmarkModel> },
		{ "escaped-keys", "escaped characters in keys",            &AddModel<EscapedKeyBenchmarkModel> },
	};
}

int main(int argc, char* argv[])	// NOLINT(bugprone-exception-escape)
{
	BenchHarness::Config config;
	if (!BenchHarness::ParseConfig(argc, argv, config))
	{
		std::cout << "Models: --models <name[,name...]|all> (default: mixed), --list" << std::endl;
		return 0;
	}

	std::string modelsArg = "mixed";
	bool listOnly = false;
	for (int i = 1; i < argc; ++i)
	{
		const std::string arg = argv[i];
		if (arg == "--models" && i + 1 < argc)
		{
			modelsArg = argv[++i];
		}
		else if (arg == "--list")
		{
			listOnly = true;
		}
	}

	if (listOnly)
	{
		std::cout << "Available models:" << std::endl;
		for (const auto& model : Models)
		{
			std::cout << "  " << model.name << " - " << model.description << std::endl;
		}
		return 0;
	}

	std::vector<std::string> selected;
	if (modelsArg == "all")
	{
		for (const auto& model : Models)
		{
			selected.emplace_back(model.name);
		}
	}
	else
	{
		std::string current;
		for (const char ch : modelsArg)
		{
			if (ch == ',')
			{
				if (!current.empty())
				{
					selected.push_back(current);
					current.clear();
				}
			}
			else if (ch != ' ')
			{
				current += ch;
			}
		}
		if (!current.empty())
		{
			selected.push_back(current);
		}
	}

	if (selected.empty())
	{
		std::cerr << "No models selected. Use --list to see the available names." << std::endl;
		return 1;
	}

	BenchHarness::PrepareEnvironment();

	BenchHarness::Runner runner(config);
	std::vector<std::unique_ptr<IModelState>> states;
	for (const auto& name : selected)
	{
		const ModelEntry* found = nullptr;
		for (const auto& model : Models)
		{
			if (name == model.name)
			{
				found = &model;
				break;
			}
		}
		if (found == nullptr)
		{
			std::cerr << "Unknown model: " << name << ". Use --list to see the available names." << std::endl;
			return 1;
		}
		found->add(runner, states, name);
	}

	// One table for all selected models (a single title and column header).
	runner.RunAll(std::cout, false);
	runner.Print();

	return 0;
}

#else

int main()
{
	std::cout << "archive_ab_benchmarks: enable BUILD_JSON_ARCHIVE to build this example." << std::endl;
	return 0;
}

#endif
