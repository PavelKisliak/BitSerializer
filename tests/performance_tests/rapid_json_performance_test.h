/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include "bitserializer/string_conversion.h"
#include "bitserializer_rapidjson/rapidjson_archive.h"
#include "base_test_models.h"


class RapidJsonPerformanceTestModel : public BasePerformanceTestModel
{
public:
	using RapidJsonDocument = rapidjson::GenericDocument<rapidjson::UTF16<>>;
	using RapidJsonNode = rapidjson::GenericValue<rapidjson::UTF16<>>;
	using StringBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF16<>>;

	~RapidJsonPerformanceTestModel() = default;

	const char* GetName() override { return "RapidJson (std::wstring)"; }

	std::wstring TestSave()
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
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>(L"ArrayOfBooleans"), booleansJsonArray.Move(), allocator);

		// Save array of integers
		auto intsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		intsJsonArray.Reserve(static_cast<rapidjson::SizeType>(ARRAY_SIZE), allocator);
		for (auto item : mArrayOfInts) {
			intsJsonArray.PushBack(RapidJsonNode(item), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>(L"ArrayOfInts"), intsJsonArray.Move(), allocator);

		// Save array of floats
		auto floatsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		floatsJsonArray.Reserve(static_cast<rapidjson::SizeType>(ARRAY_SIZE), allocator);
		for (auto item : mArrayOfFloats) {
			floatsJsonArray.PushBack(RapidJsonNode(item), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>(L"ArrayOfFloats"), floatsJsonArray.Move(), allocator);

		// Save array of strings
		auto stringsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		stringsJsonArray.Reserve(static_cast<rapidjson::SizeType>(ARRAY_SIZE), allocator);
		for (const auto& item : mArrayOfStrings) {
			stringsJsonArray.PushBack(MakeRapidJsonNodeFromString(item, allocator), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>(L"ArrayOfStrings"), stringsJsonArray.Move(), allocator);

		// Save array of objects
		auto objectsJsonArray = RapidJsonNode(rapidjson::kArrayType);
		objectsJsonArray.Reserve(static_cast<rapidjson::SizeType>(ARRAY_SIZE), allocator);
		for (const auto& item : mArrayOfObjects)
		{
			RapidJsonNode jsonObject(rapidjson::kObjectType);
			jsonObject.AddMember(L"TestBoolValue", item.mTestBoolValue, allocator);
			jsonObject.AddMember(L"TestCharValue", item.mTestCharValue, allocator);
			jsonObject.AddMember(L"TestInt16Value", item.mTestInt16Value, allocator);
			jsonObject.AddMember(L"TestInt32Value", item.mTestInt32Value, allocator);
			jsonObject.AddMember(L"TestInt64Value", item.mTestInt64Value, allocator);
			jsonObject.AddMember(L"TestFloatValue", item.mTestFloatValue, allocator);
			jsonObject.AddMember(L"TestDoubleValue", item.mTestDoubleValue, allocator);
			jsonObject.AddMember(L"TestStringValue", MakeRapidJsonNodeFromString(item.mTestStringValue, allocator), allocator);
			jsonObject.AddMember(L"TestWStringValue", MakeRapidJsonNodeFromString(item.mTestWStringValue, allocator), allocator);

			objectsJsonArray.PushBack(jsonObject.Move(), allocator);
		}
		jsonDoc.AddMember(rapidjson::GenericStringRef<RapidJsonNode::Ch>(L"ArrayOfObjects"), objectsJsonArray.Move(), allocator);

		// Build
		StringBuffer buffer;
		rapidjson::Writer<StringBuffer, rapidjson::UTF16<>, rapidjson::UTF16<>> writer(buffer);
		jsonDoc.Accept(writer);
		return buffer.GetString();
	}

	void TestLoad(const std::wstring& json)
	{
		RapidJsonDocument jsonDoc;
		if (jsonDoc.Parse(json.c_str()).HasParseError())
			throw std::exception("RapidJson parse error");
		const auto& jObject = jsonDoc.GetObject();

		// Load array of booleans
		const auto& booleansJsonArray = jObject.FindMember(L"ArrayOfBooleans")->value;
		int i = 0;
		for (auto jItem = booleansJsonArray.Begin(); jItem != booleansJsonArray.End(); ++jItem) {
			mArrayOfBooleans[i] = jItem->GetBool();
			++i;
		}

		// Load array of integers
		const auto& integersJsonArray = jObject.FindMember(L"ArrayOfInts")->value;
		i = 0;
		for (auto jItem = integersJsonArray.Begin(); jItem != integersJsonArray.End(); ++jItem) {
			mArrayOfInts[i] = jItem->GetInt64();
			++i;
		}

		// Load array of floats
		const auto& floatsJsonArray = jObject.FindMember(L"ArrayOfFloats")->value;
		i = 0;
		for (auto jItem = floatsJsonArray.Begin(); jItem != floatsJsonArray.End(); ++jItem) {
			mArrayOfFloats[i] = jItem->GetDouble();
			++i;
		}

		// Load array of strings
		const auto& stringsJsonArray = jObject.FindMember(L"ArrayOfStrings")->value;
		i = 0;
		for (auto jItem = stringsJsonArray.Begin(); jItem != stringsJsonArray.End(); ++jItem) {
			mArrayOfStrings[i] = BitSerializer::Convert::ToWString(jItem->GetString());
			++i;
		}

		// Load array of objects
		const auto& objectsJsonArray = jObject.FindMember(L"ArrayOfObjects")->value;
		i = 0;
		for (auto jItem = objectsJsonArray.Begin(); jItem != objectsJsonArray.End(); ++jItem) {
			auto& obj = mArrayOfObjects[i];
			const auto& jObj = jItem->GetObject();
			obj.mTestBoolValue = jObj.FindMember(L"TestBoolValue")->value.GetBool();
			obj.mTestCharValue = jObj.FindMember(L"TestCharValue")->value.GetInt();
			obj.mTestInt16Value = jObj.FindMember(L"TestInt16Value")->value.GetInt();
			obj.mTestInt32Value = jObj.FindMember(L"TestInt32Value")->value.GetInt();
			obj.mTestInt64Value = jObj.FindMember(L"TestInt64Value")->value.GetInt64();
			obj.mTestFloatValue = jObj.FindMember(L"TestFloatValue")->value.GetFloat();
			obj.mTestDoubleValue = jObj.FindMember(L"TestDoubleValue")->value.GetDouble();
			AssignStringFromJsonValue(jObj.FindMember(L"TestStringValue")->value, obj.mTestStringValue);
			AssignStringFromJsonValue(jObj.FindMember(L"TestWStringValue")->value, obj.mTestWStringValue);
			++i;
		}
	}

	template <typename TSym, typename TAllocator, typename TRapidAllocator>
	RapidJsonNode MakeRapidJsonNodeFromString(const std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value, TRapidAllocator& allocator)
	{
		using TargetSymType = RapidJsonNode::EncodingType::Ch;
		if constexpr (std::is_same_v<TSym, RapidJsonNode::EncodingType::Ch>)
			return RapidJsonNode(value.data(), static_cast<rapidjson::SizeType>(value.size()), allocator);
		else {
			const auto str = BitSerializer::Convert::To<std::basic_string<TargetSymType, std::char_traits<TargetSymType>>>(value);
			return RapidJsonNode(str.data(), static_cast<rapidjson::SizeType>(str.size()), allocator);
		}
	}

	template <typename TSym, typename TAllocator>
	bool AssignStringFromJsonValue(const RapidJsonNode& jsonValue, std::basic_string<TSym, std::char_traits<TSym>, TAllocator>& value)
	{
		if (!jsonValue.IsString())
			return false;

		if constexpr (std::is_same_v<TSym, RapidJsonNode::EncodingType::Ch>)
			value = jsonValue.GetString();
		else
			value = BitSerializer::Convert::To<std::basic_string<TSym, std::char_traits<TSym>, TAllocator>>(jsonValue.GetString());
		return true;
	}
};