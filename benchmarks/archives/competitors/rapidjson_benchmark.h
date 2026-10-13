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
			obj.booleanValue = jObj.FindMember("BooleanValue")->value.GetBool();
			obj.signedIntValue = static_cast<char>(jObj.FindMember("SignedIntValue")->value.GetInt());
			obj.unsignedIntValue = jObj.FindMember("UnsignedIntValue")->value.GetInt64();
			obj.floatValue = jObj.FindMember("FloatValue")->value.GetFloat();
			obj.doubleValue = jObj.FindMember("DoubleValue")->value.GetDouble();
			obj.shortString = jObj.FindMember("ShortString")->value.GetString();
			obj.stringWithLongKeyAndValue = jObj.FindMember("StringWithLongKeyAndValue")->value.GetString();
			obj.unicodeString = jObj.FindMember("UnicodeString")->value.GetString();
			obj.stringWithEscapedChars = jObj.FindMember("StringWithEscapedChars")->value.GetString();
			obj.multiLineString = jObj.FindMember("MultiLineString")->value.GetString();
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
			jsonObject.AddMember("BooleanValue", item.booleanValue, allocator);
			jsonObject.AddMember("SignedIntValue", item.signedIntValue, allocator);
			jsonObject.AddMember("UnsignedIntValue", item.unsignedIntValue, allocator);
			jsonObject.AddMember("FloatValue", item.floatValue, allocator);
			jsonObject.AddMember("DoubleValue", item.doubleValue, allocator);
			jsonObject.AddMember("ShortString", RapidJsonNode::StringRefType(
				item.shortString.data(), static_cast<rapidjson::SizeType>(item.shortString.size())), allocator);
			jsonObject.AddMember("StringWithLongKeyAndValue", RapidJsonNode::StringRefType(
				item.stringWithLongKeyAndValue.data(), static_cast<rapidjson::SizeType>(item.stringWithLongKeyAndValue.size())), allocator);
			jsonObject.AddMember("UnicodeString", RapidJsonNode::StringRefType(
				item.unicodeString.data(), static_cast<rapidjson::SizeType>(item.unicodeString.size())), allocator);
			jsonObject.AddMember("StringWithEscapedChars", RapidJsonNode::StringRefType(
				item.stringWithEscapedChars.data(), static_cast<rapidjson::SizeType>(item.stringWithEscapedChars.size())), allocator);
			jsonObject.AddMember("MultiLineString", RapidJsonNode::StringRefType(
				item.multiLineString.data(), static_cast<rapidjson::SizeType>(item.multiLineString.size())), allocator);

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
