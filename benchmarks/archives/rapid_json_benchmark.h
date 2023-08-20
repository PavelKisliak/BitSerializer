/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer/rapidjson_archive.h"
#include "benchmark_base.h"


using RapidJsonTestModel = CommonTestModel<>;
using RapidJsonBasePerfTest = CBenchmarkBase<BitSerializer::Json::RapidJson::JsonArchive, RapidJsonTestModel, char>;

class CRapidJsonBenchmark final : public RapidJsonBasePerfTest
{
public:
	using base_class_t = RapidJsonBasePerfTest;

	using RapidJsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>>;
	using RapidJsonNode = rapidjson::GenericValue<rapidjson::UTF8<>>;
	using StringBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF8<>>;

	[[nodiscard]] std::string GetArchiveName() const override { return "RapidJson"; }
	[[nodiscard]] bool IsUseNativeLib() const override { return true; }

	size_t SaveModelViaNativeLib() override
	{
		RapidJsonDocument jsonDoc;
		auto& allocator = jsonDoc.GetAllocator();
		jsonDoc.SetArray();

		// Save array of objects
		jsonDoc.Reserve(static_cast<rapidjson::SizeType>(mSourceTestModel.size()), allocator);
		for (const auto& item : mSourceTestModel)
		{
			RapidJsonNode jsonObject(rapidjson::kObjectType);
			jsonObject.AddMember("TestBoolValue", item.mTestBoolValue, allocator);
			jsonObject.AddMember("TestCharValue", item.mTestCharValue, allocator);
			jsonObject.AddMember("TestInt64Value", item.mTestInt64Value, allocator);
			jsonObject.AddMember("TestFloatValue", item.mTestFloatValue, allocator);
			jsonObject.AddMember("TestDoubleValue", item.mTestDoubleValue, allocator);
			jsonObject.AddMember("TestString1", RapidJsonNode::StringRefType(
				item.mTestString1.data(), static_cast<rapidjson::SizeType>(item.mTestString1.size())), allocator);
			jsonObject.AddMember("TestString2", RapidJsonNode::StringRefType(
				item.mTestString2.data(), static_cast<rapidjson::SizeType>(item.mTestString2.size())), allocator);
			jsonObject.AddMember("TestString3", RapidJsonNode::StringRefType(
				item.mTestString3.data(), static_cast<rapidjson::SizeType>(item.mTestString3.size())), allocator);
			jsonObject.AddMember("StringWithQuotes", RapidJsonNode::StringRefType(
				item.mStringWithQuotes.data(), static_cast<rapidjson::SizeType>(item.mStringWithQuotes.size())), allocator);
			jsonObject.AddMember("MultiLineString", RapidJsonNode::StringRefType(
				item.mMultiLineString.data(), static_cast<rapidjson::SizeType>(item.mMultiLineString.size())), allocator);

			jsonDoc.PushBack(std::move(jsonObject), allocator);
		}

		// Build
		StringBuffer buffer;
		rapidjson::Writer<StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>> writer(buffer);
		jsonDoc.Accept(writer);
		mNativeLibOutputData = buffer.GetString();
		return mNativeLibOutputData.size();
	}

	size_t LoadModelViaNativeLib() override
	{
		RapidJsonDocument jsonDoc;
		if (jsonDoc.Parse(mNativeLibOutputData.data(), mNativeLibOutputData.size()).HasParseError())
			throw std::runtime_error("RapidJson parse error");
		const auto& jArray = jsonDoc.GetArray();

		// Load array of objects
		int i = 0;
		for (auto jItem = jArray.Begin(); jItem != jArray.End(); ++jItem)
		{
			auto& obj = mNativeLibModel[i];
			const auto& jObj = jItem->GetObject();
			obj.mTestBoolValue = jObj.FindMember("TestBoolValue")->value.GetBool();
			obj.mTestCharValue = static_cast<char>(jObj.FindMember("TestCharValue")->value.GetInt());
			obj.mTestInt64Value = jObj.FindMember("TestInt64Value")->value.GetInt64();
			obj.mTestFloatValue = jObj.FindMember("TestFloatValue")->value.GetFloat();
			obj.mTestDoubleValue = jObj.FindMember("TestDoubleValue")->value.GetDouble();
			obj.mTestString1 = jObj.FindMember("TestString1")->value.GetString();
			obj.mTestString2 = jObj.FindMember("TestString2")->value.GetString();
			obj.mTestString3 = jObj.FindMember("TestString3")->value.GetString();
			obj.mStringWithQuotes = jObj.FindMember("StringWithQuotes")->value.GetString();
			obj.mMultiLineString = jObj.FindMember("MultiLineString")->value.GetString();
			++i;
		}

		return mNativeLibOutputData.size();
	}

	void Assert() const override
	{
		base_class_t::Assert();

		for (size_t i = 0; i < mSourceTestModel.size(); i++)
		{
			mSourceTestModel[i].Assert(mBitSerializerModel[i]);
		}
		// Output JSON from BitSerializer and base library should be equal
		assert(mNativeLibOutputData == mBitSerializerOutputData);
	}

	private:
		model_t mNativeLibModel;
		output_format_t mNativeLibOutputData;
};
