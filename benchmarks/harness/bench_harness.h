/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

/**
 * @file bench_harness.h
 * @brief Intra-binary A/B microbenchmark harness.
 *
 * Compares two implementations (base and variant) of the same operation inside a
 * single process. Because both are compiled into the same binary, any
 * code-layout effect of that build ("layout lottery") applies to both equally, so
 * the measured *difference* is immune to it — unlike comparing two separate
 * binaries. The two implementations are measured in alternating blocks to cancel
 * slow machine drift.
 *
 * The harness reports the minimum and the median per-call time of each
 * implementation and the block-to-block spread of the (min-based) delta as a
 * robust noise band. The min is the reported best-case number (matching the
 * archives benchmark), but the verdict is based on the median: the min is an
 * extreme-value statistic and can move by a few percent even for identical
 * implementations. If the two disagree in sign, the result is not trustworthy.
 *
 * Requirements for a callable:
 * - It must perform one representative unit of work and have observable side
 *   effects (otherwise the optimizer may remove it). Prefer reading/writing a
 *   buffer that outlives the call, or accumulating into a `volatile` sink.
 * - For very cheap operations it is measured in an adaptive batch internally, so
 *   the reported value is the per-call time including loop overhead.
 *
 * Command-line options (via `ParseConfig`): `--blocks N`, `--block-ms N`,
 * `--warmup-ms N`, `--help`/`-h`.
 */
namespace BenchHarness
{
	using Clock = std::chrono::steady_clock;

	/**
	 * @brief Configuration of a microbenchmark run.
	 */
	struct Config
	{
		/// Warm-up duration for each implementation (not measured).
		std::chrono::milliseconds warmup{200};

		/// Measurement duration of each implementation within one block.
		std::chrono::milliseconds measurePerBlock{100};

		/// Number of alternating blocks (base, variant, base, variant, ...).
		int blocks = 9;

		/// Minimum |delta| (percentage points) required to declare a result significant.
		double significanceThresholdPct = 1.0;
	};

	/**
	 * @brief Result of comparing a base and a variant implementation.
	 */
	struct Result
	{
		std::string name;				///< Benchmark name.
		double baseMinNs = 0.0;			///< Best per-call time of the base implementation (ns).
		double baseMedianNs = 0.0;		///< Median per-call time of the base implementation (ns).
		double variantMinNs = 0.0;		///< Best per-call time of the variant implementation (ns).
		double variantMedianNs = 0.0;	///< Median per-call time of the variant implementation (ns).
		double speedDeltaPct = 0.0;		///< Relative throughput change from the minimums: (baseMin / variantMin - 1) * 100; positive means the variant is faster.
		double medianSpeedDeltaPct = 0.0;	///< Relative throughput change from the medians; used for the verdict (robust to min noise).
		double noisePct = 0.0;			///< Interquartile spread of the per-block speed deltas (percentage points).
		bool significant = false;		///< True if the median delta exceeds the noise band and the threshold, and the min agrees in sign.

		/**
		 * @brief Speedup of the variant over the base (base / variant, > 1 means faster).
		 */
		[[nodiscard]] double SpeedupFactor() const noexcept
		{
			return variantMinNs > 0.0 ? baseMinNs / variantMinNs : 0.0;
		}
	};

	namespace Detail
	{
		/// Duration of one timed sample. Long enough (~5000 clock ticks at 100 ns)
		/// to make the clock quantization negligible, short enough to sample often.
		constexpr std::uint64_t TargetSampleNs = 500'000;

		/**
		 * @brief Estimates how many calls fit into one timed sample.
		 *
		 * Runs the callable until at least ~50 microseconds (500 clock ticks) have
		 * elapsed, so the per-call estimate is accurate to roughly one tick. The
		 * batch is then chosen so that a single timed sample lasts TargetSampleNs.
		 * Calibration is only used to size the batch; it does not need to be exact.
		 */
		template <typename TCallable>
		std::uint64_t CalibrateBatch(TCallable& fn)
		{
			constexpr auto CalibrationTarget = std::chrono::microseconds(50);
			constexpr std::uint64_t Chunk = 16;

			std::uint64_t calls = 0;
			const auto start = Clock::now();
			do
			{
				for (std::uint64_t i = 0; i < Chunk; ++i)
				{
					fn();
				}
				calls += Chunk;
			} while (Clock::now() - start < CalibrationTarget && calls < 10'000'000);

			const auto elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
			const auto perCallNs = std::max<std::uint64_t>(1, static_cast<std::uint64_t>(elapsedNs) / calls);
			return std::max<std::uint64_t>(1, TargetSampleNs / perCallNs);
		}

		/**
		 * @brief Measures one block, collecting per-sample per-call times.
		 *
		 * Each timed sample runs `batch` calls and divides the measured interval by
		 * the batch size, which amortizes the clock overhead (and its 100 ns
		 * quantization on Windows) by ~4 orders of magnitude.
		 *
		 * @return The best per-call time within the block.
		 */
		template <typename TCallable>
		double MeasureBlock(TCallable& fn, std::chrono::milliseconds duration, std::uint64_t batch, std::vector<double>& samples)
		{
			const auto end = Clock::now() + duration;
			double blockMinNs = std::numeric_limits<double>::max();
			do
			{
				const auto start = Clock::now();
				for (std::uint64_t i = 0; i < batch; ++i)
				{
					fn();
				}
				const auto totalNs = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
				const double perCallNs = static_cast<double>(totalNs) / static_cast<double>(batch);
				samples.push_back(perCallNs);
				blockMinNs = std::min(blockMinNs, perCallNs);
			} while (Clock::now() < end);
			return blockMinNs;
		}

		/**
		 * @brief Runs a callable for the given duration (warm-up).
		 */
		template <typename TCallable>
		void Warmup(TCallable& fn, std::chrono::milliseconds duration)
		{
			const auto end = Clock::now() + duration;
			do
			{
				fn();
			} while (Clock::now() < end);
		}

		/**
		 * @brief Returns the given percentile of the values (linear interpolation).
		 */
		inline double Percentile(std::vector<double> values, double percentile)
		{
			if (values.empty())
			{
				return 0.0;
			}
			std::sort(values.begin(), values.end());
			const auto last = values.size() - 1;
			const double position = percentile * static_cast<double>(last);
			const auto lower = static_cast<std::size_t>(position);
			const auto upper = std::min(lower + 1, last);
			const double fraction = position - static_cast<double>(lower);
			return values[lower] + (values[upper] - values[lower]) * fraction;
		}
	} // namespace Detail

	/**
	 * @brief Parses common benchmark options from the command line.
	 *
	 * Supported: `--blocks N`, `--block-ms N`, `--warmup-ms N`, `--help`/`-h`.
	 * @return False when `--help` was requested (usage printed), otherwise true.
	 */
	inline bool ParseConfig(int argc, char* argv[], Config& config)
	{
		for (int i = 1; i < argc; ++i)
		{
			const std::string arg = argv[i];
			auto readValue = [&](int& index) -> const char*
			{
				return index + 1 < argc ? argv[++index] : nullptr;
			};

			if (arg == "--blocks")
			{
				if (const char* value = readValue(i))
				{
					config.blocks = std::atoi(value);
				}
			}
			else if (arg == "--block-ms")
			{
				if (const char* value = readValue(i))
				{
					config.measurePerBlock = std::chrono::milliseconds(std::atoi(value));
				}
			}
			else if (arg == "--warmup-ms")
			{
				if (const char* value = readValue(i))
				{
					config.warmup = std::chrono::milliseconds(std::atoi(value));
				}
			}
			else if (arg == "--help" || arg == "-h")
			{
				std::cout << "Options: --blocks <n> --block-ms <n> --warmup-ms <n>" << std::endl;
				return false;
			}
		}
		return true;
	}

	/**
	 * @brief Compares two implementations of the same operation in one process.
	 *
	 * @param name Human-readable name of the comparison.
	 * @param base Base implementation.
	 * @param variant Variant implementation.
	 * @param config Measurement configuration.
	 * @return Collected result.
	 */
	template <typename TBase, typename TVariant>
	Result RunAB(std::string name, TBase&& base, TVariant&& variant, const Config& config = Config{})
	{
		Detail::Warmup(base, config.warmup);
		Detail::Warmup(variant, config.warmup);

		// Calibrate the per-sample batch once per implementation.
		const std::uint64_t baseBatch = Detail::CalibrateBatch(base);
		const std::uint64_t variantBatch = Detail::CalibrateBatch(variant);

		const int blocks = std::max(1, config.blocks);
		std::vector<double> blockDeltas(static_cast<std::size_t>(blocks));
		std::vector<double> baseSamples;
		std::vector<double> variantSamples;
		double baseMinNs = std::numeric_limits<double>::max();
		double variantMinNs = std::numeric_limits<double>::max();

		for (int block = 0; block < blocks; ++block)
		{
			double baseBlockNs = 0.0;
			double variantBlockNs = 0.0;
			// Alternate the order to cancel slow machine drift.
			if (block % 2 == 0)
			{
				baseBlockNs = Detail::MeasureBlock(base, config.measurePerBlock, baseBatch, baseSamples);
				variantBlockNs = Detail::MeasureBlock(variant, config.measurePerBlock, variantBatch, variantSamples);
			}
			else
			{
				variantBlockNs = Detail::MeasureBlock(variant, config.measurePerBlock, variantBatch, variantSamples);
				baseBlockNs = Detail::MeasureBlock(base, config.measurePerBlock, baseBatch, baseSamples);
			}
			baseMinNs = std::min(baseMinNs, baseBlockNs);
			variantMinNs = std::min(variantMinNs, variantBlockNs);
			blockDeltas[static_cast<std::size_t>(block)] =
				(baseBlockNs / variantBlockNs - 1.0) * 100.0;
		}

		Result result;
		result.name = std::move(name);
		result.baseMinNs = baseMinNs;
		result.baseMedianNs = Detail::Percentile(baseSamples, 0.5);
		result.variantMinNs = variantMinNs;
		result.variantMedianNs = Detail::Percentile(variantSamples, 0.5);
		result.speedDeltaPct = (baseMinNs / variantMinNs - 1.0) * 100.0;
		result.medianSpeedDeltaPct = result.variantMedianNs > 0.0
			? (result.baseMedianNs / result.variantMedianNs - 1.0) * 100.0
			: 0.0;
		// Robust noise band: the interquartile spread of the per-block deltas
		// (resistant to a single slow block).
		result.noisePct = Detail::Percentile(blockDeltas, 0.75) - Detail::Percentile(blockDeltas, 0.25);
		const double band = std::max(result.noisePct, config.significanceThresholdPct);
		// The min is an extreme-value statistic: on identical implementations its
		// ratio can move by a few percent, which the block-to-block IQR does not
		// capture. Base the verdict on the median and require the min to agree in
		// sign; otherwise the result is not trustworthy.
		result.significant = std::fabs(result.medianSpeedDeltaPct) > band &&
			((result.speedDeltaPct > 0.0) == (result.medianSpeedDeltaPct > 0.0));
		return result;
	}

	/**
	 * @brief Runs a set of A/B microbenchmarks and prints a summary table.
	 */
	class Runner
	{
	public:
		explicit Runner(Config config = {})
			: mConfig(config)
		{
		}

		/**
		 * @brief Registers a base/variant pair.
		 */
		template <typename TBase, typename TVariant>
		void Add(std::string name, TBase&& base, TVariant&& variant)
		{
			mEntries.push_back(Entry{
				std::move(name),
				std::function<void()>(std::forward<TBase>(base)),
				std::function<void()>(std::forward<TVariant>(variant)) });
		}

		/**
		 * @brief Runs all registered pairs and optionally prints the summary.
		 *
		 * @param out Output stream for the table.
		 * @param print When false, only collects results; call Print() later.
		 */
		void RunAll(std::ostream& out = std::cout, bool print = true)
		{
			mResults.clear();
			mResults.reserve(mEntries.size());
			for (auto& entry : mEntries)
			{
				mResults.push_back(RunAB(entry.name, entry.base, entry.variant, mConfig));
			}

			if (print)
			{
				printTable(out);
			}
		}

		/**
		 * @brief Prints the summary table for the results collected by RunAll().
		 */
		void Print(std::ostream& out = std::cout) const
		{
			printTable(out);
		}

		/**
		 * @brief Returns true if any variant is significantly slower than its base.
		 */
		[[nodiscard]] bool HasRegression() const noexcept
		{
			return std::any_of(mResults.begin(), mResults.end(), [](const Result& result)
			{
				return result.significant && result.medianSpeedDeltaPct < 0.0;
			});
		}

		[[nodiscard]] const std::vector<Result>& Results() const noexcept
		{
			return mResults;
		}

	private:
		struct Entry
		{
			std::string name;
			std::function<void()> base;
			std::function<void()> variant;
		};

		void printTable(std::ostream& out) const
		{
			struct Row
			{
				std::string name;
				std::string base;		///< "min/median" per-call time of the base implementation.
				std::string variant;	///< "min/median" per-call time of the variant implementation.
				std::string speed;		///< "min/median" relative throughput change.
				std::string noise;
				std::string verdict;
			};

			auto format2 = [](const char* format, double first, double second)
			{
				char buffer[64];
				std::snprintf(buffer, sizeof(buffer), format, first, second);
				return std::string(buffer);
			};
			auto format1 = [](const char* format, double value)
			{
				char buffer[64];
				std::snprintf(buffer, sizeof(buffer), format, value);
				return std::string(buffer);
			};

			static const std::string baseHeader = "base ns(min/med)";
			static const std::string variantHeader = "var ns(min/med)";
			static const std::string speedHeader = "speed%(min/med)";
			static const std::string noiseHeader = "noise%";

			std::size_t nameWidth = 24;
			std::size_t baseWidth = baseHeader.size();
			std::size_t variantWidth = variantHeader.size();
			std::size_t speedWidth = speedHeader.size();
			std::size_t noiseWidth = noiseHeader.size();

			std::vector<Row> rows;
			rows.reserve(mResults.size());
			for (const auto& result : mResults)
			{
				Row row;
				row.name = result.name;
				row.base = format2("%.2f/%.2f", result.baseMinNs, result.baseMedianNs);
				row.variant = format2("%.2f/%.2f", result.variantMinNs, result.variantMedianNs);
				row.speed = format2("%+.2f/%+.2f", result.speedDeltaPct, result.medianSpeedDeltaPct);
				row.noise = format1("%.2f", result.noisePct);
				if (result.significant)
				{
					row.verdict = result.medianSpeedDeltaPct > 0.0 ? "FASTER" : "SLOWER";
				}
				else
				{
					row.verdict = "~noise";
				}

				nameWidth = std::max(nameWidth, row.name.size());
				baseWidth = std::max(baseWidth, row.base.size());
				variantWidth = std::max(variantWidth, row.variant.size());
				speedWidth = std::max(speedWidth, row.speed.size());
				noiseWidth = std::max(noiseWidth, row.noise.size());
				rows.push_back(std::move(row));
			}

			auto padRight = [](const std::string& value, std::size_t width)
			{
				return value + std::string(width > value.size() ? width - value.size() : 0, ' ');
			};
			auto padLeft = [](const std::string& value, std::size_t width)
			{
				return std::string(width > value.size() ? width - value.size() : 0, ' ') + value;
			};

			out << "\nIntra-binary A/B microbenchmark (" << mConfig.blocks
				<< " blocks x " << mConfig.measurePerBlock.count() << " ms, warmup "
				<< mConfig.warmup.count() << " ms)\n\n";
			out << "  " << padRight(std::string(), nameWidth) << "  "
				<< padLeft(baseHeader, baseWidth) << "  "
				<< padLeft(variantHeader, variantWidth) << "  "
				<< padLeft(speedHeader, speedWidth) << "  "
				<< padLeft(noiseHeader, noiseWidth) << "  verdict\n";
			for (const auto& row : rows)
			{
				out << "  " << padRight(row.name, nameWidth) << "  "
					<< padLeft(row.base, baseWidth) << "  "
					<< padLeft(row.variant, variantWidth) << "  "
					<< padLeft(row.speed, speedWidth) << "  "
					<< padLeft(row.noise, noiseWidth) << "  " << row.verdict << "\n";
			}
			out << std::endl;
		}

		Config mConfig;
		std::vector<Entry> mEntries;
		std::vector<Result> mResults;
	};
} // namespace BenchHarness
