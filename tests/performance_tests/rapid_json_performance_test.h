/*******************************************************************************
* Copyright (C) 2018-2022 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer/rapidjson_archive.h"
#include "archive_base_perf_test.h"
#include "base_test_models.h"


using RapidJsonTestModel = TestModelWithSubArrays<char>;
using RapidJsonBasePerfTest = CArchiveBasePerfTest<BitSerializer::Json::RapidJson::JsonArchive, RapidJsonTestModel, char>;

class CRapidJsonPerformanceTest final : public RapidJsonBasePerfTest
{
public:
	using model_t = RapidJsonTestModel;
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
		jsonDoc.SetObject();

		// Save array of booleans
		auto booleansJsonArray = RapidJsonNode(rapidjson::kArrayType);
		booleansJsonArray.Reserve(static_cast<rapidjson::SizeType>(model_t::ARRAY_SIZE), allocator);
		for (auto item : mSourceTestModel.mArrayOfBooleans) {
			booleansJsonArray.PushBack(RapidJsonNode(item), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>("ArrayOfBooleans"), booleansJsonArray.Move(), allocator);

		// Save array of integers
		auto intsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		intsJsonArray.Reserve(static_cast<rapidjson::SizeType>(model_t::ARRAY_SIZE), allocator);
		for (auto item : mSourceTestModel.mArrayOfInts) {
			intsJsonArray.PushBack(RapidJsonNode(item), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>("ArrayOfInts"), intsJsonArray.Move(), allocator);

		// Save array of floats
		auto floatsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		floatsJsonArray.Reserve(static_cast<rapidjson::SizeType>(model_t::ARRAY_SIZE), allocator);
		for (auto item : mSourceTestModel.mArrayOfDoubles) {
			floatsJsonArray.PushBack(RapidJsonNode(item), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>("ArrayOfDoubles"), floatsJsonArray.Move(), allocator);

		// Save array of strings
		auto stringsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		stringsJsonArray.Reserve(static_cast<rapidjson::SizeType>(model_t::ARRAY_SIZE), allocator);
		for (const auto& item : mSourceTestModel.mArrayOfStrings) {
			stringsJsonArray.PushBack(RapidJsonNode::StringRefType(item.data(), static_cast<rapidjson::SizeType>(item.size())), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>("ArrayOfStrings"), stringsJsonArray.Move(), allocator);

		// Save array of objects
		auto objectsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		objectsJsonArray.Reserve(static_cast<rapidjson::SizeType>(model_t::ARRAY_SIZE), allocator);
		for (const auto& item : mSourceTestModel.mArrayOfObjects)
		{
			RapidJsonNode jsonObject(rapidjson::kObjectType);
			jsonObject.AddMember("TestBoolValue", item.mTestBoolValue, allocator);
			jsonObject.AddMember("TestCharValue", item.mTestCharValue, allocator);
			jsonObject.AddMember("TestInt16Value", item.mTestInt16Value, allocator);
			jsonObject.AddMember("TestInt32Value", item.mTestInt32Value, allocator);
			jsonObject.AddMember("TestInt64Value", item.mTestInt64Value, allocator);
			jsonObject.AddMember("TestFloatValue", item.mTestFloatValue, allocator);
			jsonObject.AddMember("TestDoubleValue", item.mTestDoubleValue, allocator);
			jsonObject.AddMember("TestStringValue", RapidJsonNode::StringRefType(
				item.mTestStringValue.data(), static_cast<rapidjson::SizeType>(item.mTestStringValue.size())), allocator);

			objectsJsonArray.PushBack(jsonObject.Move(), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>("ArrayOfObjects"), objectsJsonArray.Move(), allocator);

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
		const auto& jObject = jsonDoc.GetObject();

		// Load array of booleans
		const auto& booleansJsonArray = jObject.FindMember("ArrayOfBooleans")->value;
		int i = 0;
		for (auto jItem = booleansJsonArray.Begin(); jItem != booleansJsonArray.End(); ++jItem) {
			mNativeLibModel.mArrayOfBooleans[i] = jItem->GetBool();
			++i;
		}

		// Load array of integers
		const auto& integersJsonArray = jObject.FindMember("ArrayOfInts")->value;
		i = 0;
		for (auto jItem = integersJsonArray.Begin(); jItem != integersJsonArray.End(); ++jItem) {
			mNativeLibModel.mArrayOfInts[i] = jItem->GetInt64();
			++i;
		}

		// Load array of floats
		const auto& floatsJsonArray = jObject.FindMember("ArrayOfDoubles")->value;
		i = 0;
		for (auto jItem = floatsJsonArray.Begin(); jItem != floatsJsonArray.End(); ++jItem) {
			mNativeLibModel.mArrayOfDoubles[i] = jItem->GetDouble();
			++i;
		}

		// Load array of strings
		const auto& stringsJsonArray = jObject.FindMember("ArrayOfStrings")->value;
		i = 0;
		for (auto jItem = stringsJsonArray.Begin(); jItem != stringsJsonArray.End(); ++jItem) {
			mNativeLibModel.mArrayOfStrings[i] = jItem->GetString();
			++i;
		}

		// Load array of objects
		const auto& objectsJsonArray = jObject.FindMember("ArrayOfObjects")->value;
		i = 0;
		for (auto jItem = objectsJsonArray.Begin(); jItem != objectsJsonArray.End(); ++jItem)
		{
			auto& obj = mNativeLibModel.mArrayOfObjects[i];
			const auto& jObj = jItem->GetObject();
			obj.mTestBoolValue = jObj.FindMember("TestBoolValue")->value.GetBool();
			obj.mTestCharValue = static_cast<char>(jObj.FindMember("TestCharValue")->value.GetInt());
			obj.mTestInt16Value = static_cast<int16_t>(jObj.FindMember("TestInt16Value")->value.GetInt());
			obj.mTestInt32Value = jObj.FindMember("TestInt32Value")->value.GetInt();
			obj.mTestInt64Value = jObj.FindMember("TestInt64Value")->value.GetInt64();
			obj.mTestFloatValue = jObj.FindMember("TestFloatValue")->value.GetFloat();
			obj.mTestDoubleValue = jObj.FindMember("TestDoubleValue")->value.GetDouble();
			obj.mTestStringValue = jObj.FindMember("TestStringValue")->value.GetString();
			++i;
		}

		return mNativeLibOutputData.size();
	}

	void Assert() const override
	{
		base_class_t::Assert();

		mSourceTestModel.Assert(mNativeLibModel);
		// Output JSON from BitSerializer and base library should be equal
		assert(mNativeLibOutputData == mBitSerializerOutputData);
	}

	private:
		model_t mNativeLibModel;
		output_format_t mNativeLibOutputData;
};
