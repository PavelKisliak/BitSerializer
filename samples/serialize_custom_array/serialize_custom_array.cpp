#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/types/std/vector.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

namespace MyApp
{
	// Some custom array type
	template <typename T>
	class CMyArray
	{
	public:
		CMyArray() = default;
		CMyArray(std::initializer_list<T> initList)
			: mArray(initList)
		{ }

		[[nodiscard]] size_t GetSize() const noexcept { return mArray.size(); }
		void Resize(size_t newSize) { mArray.resize(newSize); }

		[[nodiscard]] const T& At(size_t index) const { return mArray.at(index); }
		[[nodiscard]] T& At(size_t index) { return mArray.at(index); }

		T& PushBack(T&& value) { return mArray.emplace_back(std::forward<T>(value)); }

	private:
		std::vector<T> mArray;
	};

	// Returns the size of the CMyArray.
	template <class T>
	size_t size(const CMyArray<T>& cont) noexcept { return cont.GetSize(); }

	// Serializes CMyArray.
	template <class TArchive, class TValue>
	void SerializeArray(TArchive& arrayScope, CMyArray<TValue>& cont)
	{
		if constexpr (TArchive::IsLoading())
		{
			// Resize container when approximate size is known
			if (const auto estimatedSize = arrayScope.GetEstimatedSize(); estimatedSize != 0 && cont.GetSize() < estimatedSize) {
				cont.Resize(estimatedSize);
			}

			// Load
			size_t loadedItems = 0;
			for (; !arrayScope.IsEnd(); ++loadedItems)
			{
				TValue& value = (loadedItems < cont.GetSize()) ? cont.At(loadedItems) : cont.PushBack({});
				Serialize(arrayScope, value);
			}
			// Resize container for case when loaded items less than there are or were estimated
			cont.Resize(loadedItems);
		}
		else
		{
			for (size_t i = 0; i < cont.GetSize(); ++i)
			{
				Serialize(arrayScope, cont.At(i));
			}
		}
	}
}

int main()	// NOLINT(bugprone-exception-escape)
{
	// Save custom array to JSON
	MyApp::CMyArray<int> myArray = { 1, 2, 3, 4, 5 };
	std::string jsonResult;
	BitSerializer::SaveObject<JsonArchive>(myArray, jsonResult);
	std::cout << "Saved JSON: " << jsonResult << std::endl;

	// Load from JSON-array
	MyApp::CMyArray<std::string> arrayOfString;
	const std::string srcJson = R"([ "Red", "Green", "Blue" ])";
	BitSerializer::LoadObject<JsonArchive>(arrayOfString, srcJson);
	std::cout << std::endl << "Loaded array: ";
	for (size_t i = 0; i < arrayOfString.GetSize(); ++i)
	{
		std::cout << arrayOfString.At(i);
		if (i + 1 != arrayOfString.GetSize()) {
			std::cout << ", ";
		}
	}
	std::cout << std::endl;

	return 0;
}
