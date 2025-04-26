/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer/rapidjson_archive.h"
#include "benchmark_base.h"


template <class TModel = CommonTestModel>
class CRapidJsonBenchmark : public CBenchmarkBase<TModel>
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

	void BenchmarkSaveToMemory(const TModel& sourceTestModel, std::string& outputData) override
	{
		RapidJsonDocument jsonDoc;
		auto& allocator = jsonDoc.GetAllocator();
		jsonDoc.SetArray();

		// Save array of objects
		jsonDoc.Reserve(static_cast<rapidjson::SizeType>(sourceTestModel.size()), allocator);
		for (const auto& item : sourceTestModel)
		{
			RapidJsonNode jsonObject(rapidjson::kObjectType);
			jsonObject.AddMember("BooleanValue", item.BooleanValue, allocator);
			jsonObject.AddMember("SignedIntValue", item.SignedIntValue, allocator);
			jsonObject.AddMember("UnsignedIntValue", item.UnsignedIntValue, allocator);
			jsonObject.AddMember("FloatValue", item.FloatValue, allocator);
			jsonObject.AddMember("DoubleValue", item.DoubleValue, allocator);
			jsonObject.AddMember("ShortString", RapidJsonNode::StringRefType(
				item.ShortString.data(), static_cast<rapidjson::SizeType>(item.ShortString.size())), allocator);
			jsonObject.AddMember("StringWithLongKeyAndValue", RapidJsonNode::StringRefType(
				item.StringWithLongKeyAndValue.data(), static_cast<rapidjson::SizeType>(item.StringWithLongKeyAndValue.size())), allocator);
			jsonObject.AddMember("UnicodeString", RapidJsonNode::StringRefType(
				item.UnicodeString.data(), static_cast<rapidjson::SizeType>(item.UnicodeString.size())), allocator);
			jsonObject.AddMember("StringWithEscapedChars", RapidJsonNode::StringRefType(
				item.StringWithEscapedChars.data(), static_cast<rapidjson::SizeType>(item.StringWithEscapedChars.size())), allocator);
			jsonObject.AddMember("MultiLineString", RapidJsonNode::StringRefType(
				item.MultiLineString.data(), static_cast<rapidjson::SizeType>(item.MultiLineString.size())), allocator);

			jsonDoc.PushBack(std::move(jsonObject), allocator);
		}

		// Build
		StringBuffer buffer;
		rapidjson::Writer writer(buffer);
		jsonDoc.Accept(writer);
		outputData = buffer.GetString();
	}

	void BenchmarkLoadFromMemory(const std::string& sourceData, TModel& targetTestModel) override
	{
		RapidJsonDocument jsonDoc;
		if (jsonDoc.Parse(sourceData.data(), sourceData.size()).HasParseError()) {
			throw std::runtime_error("RapidJson parse error");
		}
		const auto& jArray = jsonDoc.GetArray();

		// Load array of objects
		int i = 0;
		for (auto* jItem = jArray.Begin(); jItem != jArray.End(); ++jItem)
		{
			auto& obj = targetTestModel[i];
			const auto& jObj = jItem->GetObject();
			obj.BooleanValue = jObj.FindMember("BooleanValue")->value.GetBool();
			obj.SignedIntValue = static_cast<char>(jObj.FindMember("SignedIntValue")->value.GetInt());
			obj.UnsignedIntValue = jObj.FindMember("UnsignedIntValue")->value.GetInt64();
			obj.FloatValue = jObj.FindMember("FloatValue")->value.GetFloat();
			obj.DoubleValue = jObj.FindMember("DoubleValue")->value.GetDouble();
			obj.ShortString = jObj.FindMember("ShortString")->value.GetString();
			obj.StringWithLongKeyAndValue = jObj.FindMember("StringWithLongKeyAndValue")->value.GetString();
			obj.UnicodeString = jObj.FindMember("UnicodeString")->value.GetString();
			obj.StringWithEscapedChars = jObj.FindMember("StringWithEscapedChars")->value.GetString();
			obj.MultiLineString = jObj.FindMember("MultiLineString")->value.GetString();
			++i;
		}
	}
};
