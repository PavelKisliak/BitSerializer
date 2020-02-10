/*******************************************************************************
* Copyright (C) 2020 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer/string_conversion.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"
#include "base_test_models.h"


class RapidJsonPerformanceTestModel : public BasePerformanceTestModel<char>
{
public:
	using RapidJsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>>;
	using RapidJsonNode = rapidjson::GenericValue<rapidjson::UTF8<>>;
	using StringBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF8<>>;

	~RapidJsonPerformanceTestModel() = default;

	const char* GetName() override { return "RapidJson"; }

	std::string TestSave()
	{
		RapidJsonDocument jsonDoc;
		auto& allocator = jsonDoc.GetAllocator();
		jsonDoc.SetObject();

		// Save array of booleans
		auto booleansJsonArray = RapidJsonNode(rapidjson::kArrayType);
		booleansJsonArray.Reserve(static_cast<rapidjson::SizeType>(ARRAY_SIZE), allocator);
		for (auto item : mArrayOfBooleans) {
			booleansJsonArray.PushBack(RapidJsonNode(item), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>("ArrayOfBooleans"), booleansJsonArray.Move(), allocator);

		// Save array of integers
		auto intsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		intsJsonArray.Reserve(static_cast<rapidjson::SizeType>(ARRAY_SIZE), allocator);
		for (auto item : mArrayOfInts) {
			intsJsonArray.PushBack(RapidJsonNode(item), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>("ArrayOfInts"), intsJsonArray.Move(), allocator);

		// Save array of floats
		auto floatsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		floatsJsonArray.Reserve(static_cast<rapidjson::SizeType>(ARRAY_SIZE), allocator);
		for (auto item : mArrayOfFloats) {
			floatsJsonArray.PushBack(RapidJsonNode(item), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>("ArrayOfFloats"), floatsJsonArray.Move(), allocator);

		// Save array of strings
		auto stringsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		stringsJsonArray.Reserve(static_cast<rapidjson::SizeType>(ARRAY_SIZE), allocator);
		for (const auto& item : mArrayOfStrings) {
			stringsJsonArray.PushBack(RapidJsonNode::StringRefType(item.data(), static_cast<rapidjson::SizeType>(item.size())), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>("ArrayOfStrings"), stringsJsonArray.Move(), allocator);

		// Save array of objects
		auto objectsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		objectsJsonArray.Reserve(static_cast<rapidjson::SizeType>(ARRAY_SIZE), allocator);
		for (const auto& item : mArrayOfObjects)
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
		return buffer.GetString();
	}

	void TestLoad(const std::string& json)
	{
		RapidJsonDocument jsonDoc;
		if (jsonDoc.Parse(json.data(), json.size()).HasParseError())
			throw std::runtime_error("RapidJson parse error");
		const auto& jObject = jsonDoc.GetObject();

		// Load array of booleans
		const auto& booleansJsonArray = jObject.FindMember("ArrayOfBooleans")->value;
		int i = 0;
		for (auto jItem = booleansJsonArray.Begin(); jItem != booleansJsonArray.End(); ++jItem) {
			mArrayOfBooleans[i] = jItem->GetBool();
			++i;
		}

		// Load array of integers
		const auto& integersJsonArray = jObject.FindMember("ArrayOfInts")->value;
		i = 0;
		for (auto jItem = integersJsonArray.Begin(); jItem != integersJsonArray.End(); ++jItem) {
			mArrayOfInts[i] = jItem->GetInt64();
			++i;
		}

		// Load array of floats
		const auto& floatsJsonArray = jObject.FindMember("ArrayOfFloats")->value;
		i = 0;
		for (auto jItem = floatsJsonArray.Begin(); jItem != floatsJsonArray.End(); ++jItem) {
			mArrayOfFloats[i] = jItem->GetDouble();
			++i;
		}

		// Load array of strings
		const auto& stringsJsonArray = jObject.FindMember("ArrayOfStrings")->value;
		i = 0;
		for (auto jItem = stringsJsonArray.Begin(); jItem != stringsJsonArray.End(); ++jItem) {
			mArrayOfStrings[i] = jItem->GetString();
			++i;
		}

		// Load array of objects
		const auto& objectsJsonArray = jObject.FindMember("ArrayOfObjects")->value;
		i = 0;
		for (auto jItem = objectsJsonArray.Begin(); jItem != objectsJsonArray.End(); ++jItem) {
			auto& obj = mArrayOfObjects[i];
			const auto& jObj = jItem->GetObject();
			obj.mTestBoolValue = jObj.FindMember("TestBoolValue")->value.GetBool();
			obj.mTestCharValue = jObj.FindMember("TestCharValue")->value.GetInt();
			obj.mTestInt16Value = jObj.FindMember("TestInt16Value")->value.GetInt();
			obj.mTestInt32Value = jObj.FindMember("TestInt32Value")->value.GetInt();
			obj.mTestInt64Value = jObj.FindMember("TestInt64Value")->value.GetInt64();
			obj.mTestFloatValue = jObj.FindMember("TestFloatValue")->value.GetFloat();
			obj.mTestDoubleValue = jObj.FindMember("TestDoubleValue")->value.GetDouble();
			obj.mTestStringValue = jObj.FindMember("TestStringValue")->value.GetString();
			++i;
		}
	}
};