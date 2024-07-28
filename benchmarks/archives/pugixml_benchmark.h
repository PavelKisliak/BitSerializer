/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <stdexcept>
#include "bitserializer/pugixml_archive.h"
#include "benchmark_base.h"


using PugiXmlTestModel = CommonTestModel<>;
using PugiXmlBasePerfTest = CBenchmarkBase<BitSerializer::Xml::PugiXml::XmlArchive, PugiXmlTestModel, char>;

class CPugiXmlBenchmark final : public PugiXmlBasePerfTest
{
public:
	using base_class_t = PugiXmlBasePerfTest;

	[[nodiscard]] std::string GetArchiveName() const override { return "PugiXml"; }
	[[nodiscard]] bool IsUseNativeLib() const override { return true; }

	size_t SaveModelViaNativeLib() override
	{
		pugi::xml_document mDoc;
		auto rootNode = mDoc.append_child(PUGIXML_TEXT("array"));

		// Save array of objects
		for (const auto& item : mSourceTestModel) {
			auto objectNode = rootNode.append_child(PUGIXML_TEXT("object"));
			objectNode.append_child(PUGIXML_TEXT("TestBoolValue")).text().set(item.mTestBoolValue);
			objectNode.append_child(PUGIXML_TEXT("TestCharValue")).text().set(item.mTestCharValue);
			objectNode.append_child(PUGIXML_TEXT("TestInt64Value")).text().set(item.mTestInt64Value);
			objectNode.append_child(PUGIXML_TEXT("TestFloatValue")).text().set(item.mTestFloatValue);
			objectNode.append_child(PUGIXML_TEXT("TestDoubleValue")).text().set(item.mTestDoubleValue);
			objectNode.append_child(PUGIXML_TEXT("TestString1")).text().set(item.mTestString1.c_str());
			objectNode.append_child(PUGIXML_TEXT("TestString2")).text().set(item.mTestString2.c_str());
			objectNode.append_child(PUGIXML_TEXT("UnicodeString")).text().set(item.mTestUnicodeString.c_str());
			objectNode.append_child(PUGIXML_TEXT("StringWithQuotes")).text().set(item.mStringWithQuotes.c_str());
			objectNode.append_child(PUGIXML_TEXT("MultiLineString")).text().set(item.mMultiLineString.c_str());
		}

		// Build
		mNativeLibOutputData.clear();
		CXmlStringWriter xmlStringWriter(mNativeLibOutputData);
		mDoc.save(xmlStringWriter, PUGIXML_TEXT("\t"), pugi::format_raw, pugi::encoding_utf8);
		return mNativeLibOutputData.size();
	}

	size_t LoadModelViaNativeLib() override
	{
		pugi::xml_document mDoc;
		const auto result = mDoc.load_buffer(mNativeLibOutputData.data(), mNativeLibOutputData.size(), pugi::parse_default, pugi::encoding_utf8);
		if (!result)
			throw std::runtime_error("PugiXml parse error");

		const auto rootNode = mDoc.child(PUGIXML_TEXT("array"));

		// Load array of objects
		int i = 0;
		for (auto it = rootNode.begin(); it != rootNode.end(); ++it)
		{
			auto& obj = mNativeLibModel[i];
			obj.mTestBoolValue = it->child(PUGIXML_TEXT("TestBoolValue")).text().as_bool();
			obj.mTestCharValue = static_cast<char>(it->child(PUGIXML_TEXT("TestCharValue")).text().as_int());
			obj.mTestInt64Value = it->child(PUGIXML_TEXT("TestInt64Value")).text().as_llong();
			obj.mTestFloatValue = it->child(PUGIXML_TEXT("TestFloatValue")).text().as_float();
			obj.mTestDoubleValue = it->child(PUGIXML_TEXT("TestDoubleValue")).text().as_double();
			obj.mTestString1 = it->child(PUGIXML_TEXT("TestString1")).text().as_string();
			obj.mTestString2 = it->child(PUGIXML_TEXT("TestString2")).text().as_string();
			obj.mTestUnicodeString = it->child(PUGIXML_TEXT("UnicodeString")).text().as_string();
			obj.mStringWithQuotes = it->child(PUGIXML_TEXT("StringWithQuotes")).text().as_string();
			obj.mMultiLineString = it->child(PUGIXML_TEXT("MultiLineString")).text().as_string();
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
		// Output XML from BitSerializer and base library should be equal
		assert(mNativeLibOutputData == mBitSerializerOutputData);
	}

	private:
		class CXmlStringWriter : public pugi::xml_writer
		{
		public:
			CXmlStringWriter(output_format_t& outputStr)
				: mOutputString(outputStr)
			{ }

			void write(const void* data, size_t size) override
			{
				mOutputString.append(static_cast<const PUGIXML_CHAR*>(data), size);
			}

		private:
			output_format_t& mOutputString;
		};

		model_t mNativeLibModel;
		output_format_t mNativeLibOutputData;
};
