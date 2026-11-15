/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Benchmark headers
#include "benchmark_base.h"

// msgpack-cxx library
#include "msgpack.hpp"

namespace msgpack
{
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
{
	namespace adaptor
	{
		template <>
		struct pack<BasicTestModel>
		{
			template <typename Stream>
			msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const BasicTestModel& value) const
			{
				o.pack_map(10);
				o.pack("active");
				o.pack(value.active);
				o.pack("id");
				o.pack(value.id);
				o.pack("stock");
				o.pack(value.stock);
				o.pack("price");
				o.pack(value.price);
				o.pack("rating");
				o.pack(value.rating);
				o.pack("name");
				o.pack(value.name);
				o.pack("detailedProductDescription");
				o.pack(value.detailedProductDescription);
				o.pack("localizedName");
				o.pack(value.localizedName);
				o.pack("note");
				o.pack(value.note);
				o.pack("specifications");
				o.pack(value.specifications);
				return o;
			}
		};

		template <>
		struct convert<BasicTestModel>
		{
			const msgpack::object& operator()(const msgpack::object& o, BasicTestModel& value) const
			{
				if (o.type != msgpack::type::MAP) {
					throw msgpack::type_error();
				}

				for (uint32_t i = 0; i < o.via.map.size; ++i)
				{
					const msgpack::object_kv& kv = o.via.map.ptr[i];
					if (kv.key.type != msgpack::type::STR) {
						continue;
					}

					const std::string_view key(kv.key.via.str.ptr, kv.key.via.str.size);
					if (key == "active") {
						kv.val.convert(value.active);
					}
					else if (key == "id") {
						kv.val.convert(value.id);
					}
					else if (key == "stock") {
						kv.val.convert(value.stock);
					}
					else if (key == "price") {
						kv.val.convert(value.price);
					}
					else if (key == "rating") {
						kv.val.convert(value.rating);
					}
					else if (key == "name") {
						kv.val.convert(value.name);
					}
					else if (key == "detailedProductDescription") {
						kv.val.convert(value.detailedProductDescription);
					}
					else if (key == "localizedName") {
						kv.val.convert(value.localizedName);
					}
					else if (key == "note") {
						kv.val.convert(value.note);
					}
					else if (key == "specifications") {
						kv.val.convert(value.specifications);
					}
				}
				return o;
			}
		};
	} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

/**
 * @brief Benchmark implementation for the official msgpack-cxx (C++) library.
 */
class MsgpackCxxBenchmark final : public BenchmarkBase
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
		return "msgpack-cxx";
	}

	[[nodiscard]] std::string GetLibraryVersion() const override
	{
		return MSGPACK_VERSION;
	}

	[[nodiscard]] std::vector<TestStage> GetStagesList() const override
	{
		// Exclude serialization tests to streams
		return { TestStage::SaveToMemory, TestStage::LoadFromMemory };
	}

protected:
	void BenchmarkSaveToMemory(const CommonTestModel& sourceTestModel, std::string& outputData) override
	{
		msgpack::sbuffer buffer;
		msgpack::pack(buffer, sourceTestModel);
		outputData.assign(buffer.data(), buffer.size());
	}

	void BenchmarkLoadFromMemory(CommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		msgpack::object_handle objectHandle = msgpack::unpack(sourceData.data(), sourceData.size());
		objectHandle.get().convert(targetTestModel);
	}
};
