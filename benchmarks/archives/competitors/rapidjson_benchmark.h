/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
// Benchmark headers
#include "benchmark_base.h"

// RapidJson library
#include "rapidjson/document.h"
#include "rapidjson/encodings.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/stream.h"
#include "rapidjson/writer.h"

#ifdef GetObject
// see https://github.com/Tencent/rapidjson/issues/1448
// a former included windows.h might have defined a macro called GetObject, which affects
// GetObject defined here. This ensures the macro does not get applied
#pragma push_macro("GetObject")
#define RAPIDJSON_WINDOWS_GETOBJECT_WORKAROUND_APPLIED
#undef GetObject
#endif

/**
 * @brief Benchmark implementation for the RapidJson library.
 */
class RapidJsonBenchmark final : public BenchmarkBase
{
public:
	[[nodiscard]] std::string GetLibraryName() const override
	{
		return "RapidJson";
	}

protected:
	using RapidJsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>>;
	using RapidJsonNode = rapidjson::GenericValue<rapidjson::UTF8<>>;
	using StringBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF8<>>;

	void LoadFromJsonDocument(CommonTestModel& targetTestModel, RapidJsonDocument& jsonDoc)	// NOLINT(readability-convert-member-functions-to-static)
	{
		const auto& jArray = jsonDoc.GetArray();

		// Load array of objects
		int i = 0;
		for (auto* jItem = jArray.Begin(); jItem != jArray.End(); ++jItem)
		{
			auto& obj = targetTestModel[i];
			const auto& jObj = jItem->GetObject();
			obj.active = jObj.FindMember("active")->value.GetBool();
			obj.id = static_cast<int32_t>(jObj.FindMember("id")->value.GetInt());
			obj.stock = jObj.FindMember("stock")->value.GetInt64();
			obj.price = jObj.FindMember("price")->value.GetFloat();
			obj.rating = jObj.FindMember("rating")->value.GetDouble();
			obj.name = jObj.FindMember("name")->value.GetString();
			obj.detailedProductDescription = jObj.FindMember("detailedProductDescription")->value.GetString();
			obj.localizedName = jObj.FindMember("localizedName")->value.GetString();
			obj.note = jObj.FindMember("note")->value.GetString();
			obj.specifications = jObj.FindMember("specifications")->value.GetString();
			++i;
		}
	}

	void SaveToJsonDocument(const CommonTestModel& sourceTestModel, RapidJsonDocument& jsonDoc)	// NOLINT(readability-convert-member-functions-to-static)
	{
		auto& allocator = jsonDoc.GetAllocator();
		jsonDoc.SetArray();

		// Save array of objects
		jsonDoc.Reserve(static_cast<rapidjson::SizeType>(sourceTestModel.size()), allocator);
		for (const auto& item : sourceTestModel)
		{
			RapidJsonNode jsonObject(rapidjson::kObjectType);
			jsonObject.AddMember("active", item.active, allocator);
			jsonObject.AddMember("id", item.id, allocator);
			jsonObject.AddMember("stock", item.stock, allocator);
			jsonObject.AddMember("price", item.price, allocator);
			jsonObject.AddMember("rating", item.rating, allocator);
			jsonObject.AddMember("name", RapidJsonNode::StringRefType(
				item.name.data(), static_cast<rapidjson::SizeType>(item.name.size())), allocator);
			jsonObject.AddMember("detailedProductDescription", RapidJsonNode::StringRefType(
				item.detailedProductDescription.data(), static_cast<rapidjson::SizeType>(item.detailedProductDescription.size())), allocator);
			jsonObject.AddMember("localizedName", RapidJsonNode::StringRefType(
				item.localizedName.data(), static_cast<rapidjson::SizeType>(item.localizedName.size())), allocator);
			jsonObject.AddMember("note", RapidJsonNode::StringRefType(
				item.note.data(), static_cast<rapidjson::SizeType>(item.note.size())), allocator);
			jsonObject.AddMember("specifications", RapidJsonNode::StringRefType(
				item.specifications.data(), static_cast<rapidjson::SizeType>(item.specifications.size())), allocator);

			jsonDoc.PushBack(std::move(jsonObject), allocator);
		}
	}

	void BenchmarkSaveToMemory(const CommonTestModel& sourceTestModel, std::string& outputData) override
	{
		RapidJsonDocument jsonDoc;
		SaveToJsonDocument(sourceTestModel, jsonDoc);

		// Build
		StringBuffer buffer;
		rapidjson::Writer writer(buffer);
		jsonDoc.Accept(writer);
		outputData.assign(buffer.GetString(), buffer.GetSize());
	}

	void BenchmarkLoadFromMemory(CommonTestModel& targetTestModel, const std::string& sourceData) override
	{
		RapidJsonDocument jsonDoc;
		if (jsonDoc.Parse(sourceData.data(), sourceData.size()).HasParseError()) {
			throw std::runtime_error("RapidJson parse error");
		}
		LoadFromJsonDocument(targetTestModel, jsonDoc);
	}

	void BenchmarkSaveToStream(const CommonTestModel& sourceTestModel, std::ostream& outputStream) override
	{
		RapidJsonDocument jsonDoc;
		SaveToJsonDocument(sourceTestModel, jsonDoc);

		// Build JSON
		rapidjson::OStreamWrapper osw(outputStream);
		using AutoOutputStream = rapidjson::AutoUTFOutputStream<uint32_t, rapidjson::OStreamWrapper>;
		AutoOutputStream eos(osw, rapidjson::UTFType::kUTF8, true);
		rapidjson::Writer<AutoOutputStream, rapidjson::UTF8<>, rapidjson::AutoUTF<uint32_t>> writer(eos);
		jsonDoc.Accept(writer);
	}

	void BenchmarkLoadFromStream(CommonTestModel& targetTestModel, std::istream& inputStream) override
	{
		rapidjson::IStreamWrapper isw(inputStream);
		rapidjson::AutoUTFInputStream<uint32_t, rapidjson::IStreamWrapper> eis(isw);

		RapidJsonDocument jsonDoc;
		if (jsonDoc.ParseStream(eis).HasParseError()) {
			throw std::runtime_error("RapidJson parse error");
		}

		LoadFromJsonDocument(targetTestModel, jsonDoc);
	}
};

#ifdef RAPIDJSON_WINDOWS_GETOBJECT_WORKAROUND_APPLIED
#pragma pop_macro("GetObject")
#undef RAPIDJSON_WINDOWS_GETOBJECT_WORKAROUND_APPLIED
#endif
