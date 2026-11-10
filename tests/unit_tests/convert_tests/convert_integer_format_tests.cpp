/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include "gtest/gtest.h"
#include "bitserializer/convert.h"
#include "bitserializer/conversion_detail/convert_integer.h"

namespace
{
	struct CountingAllocatorState
	{
		std::size_t allocations = 0;
	};

	template <typename T>
	struct CountingAllocator
	{
		using value_type = T;

		CountingAllocatorState* state = nullptr;

		CountingAllocator() = default;
		explicit CountingAllocator(CountingAllocatorState* inState) noexcept : state(inState) {}

		template <typename U>
		CountingAllocator(const CountingAllocator<U>& other) noexcept : state(other.state) {}

		[[nodiscard]] T* allocate(std::size_t count)
		{
			if (state != nullptr) {
				++state->allocations;
			}
			return std::allocator<T>{}.allocate(count);
		}

		void deallocate(T* ptr, std::size_t count) noexcept
		{
			std::allocator<T>{}.deallocate(ptr, count);
		}

		template <typename U>
		bool operator==(const CountingAllocator<U>& other) const noexcept { return state == other.state; }

		template <typename U>
		bool operator!=(const CountingAllocator<U>& other) const noexcept { return !(*this == other); }
	};

	template <typename T>
	std::string FormatWithHelper(T value)
	{
		char buffer[BitSerializer::Convert::Detail::MaxIntegerLength];
		const char* const end = BitSerializer::Convert::Detail::FormatInteger(buffer, value);
		return {buffer, static_cast<std::size_t>(end - buffer)};
	}

	template <typename T>
	std::string Expected(T value)
	{
		if constexpr (std::is_signed_v<T>) {
			return std::to_string(static_cast<long long>(value));
		}
		else {
			return std::to_string(static_cast<unsigned long long>(value));
		}
	}

	template <typename T>
	void ExpectAll(const std::initializer_list<T>& values)
	{
		for (const T value : values) {
			EXPECT_EQ(Expected(value), FormatWithHelper(value));
		}
	}

	template <typename T>
	std::string ExpectedConvertTo(T value)
	{
		if constexpr (std::is_signed_v<T>) {
			return std::to_string(static_cast<long long>(value));
		}
		else {
			return std::to_string(static_cast<unsigned long long>(value));
		}
	}

	template <typename T>
	void ExpectConvertAll(const std::initializer_list<T>& values)
	{
		for (const T value : values) {
			const std::string expected = ExpectedConvertTo(value);
			EXPECT_EQ(expected, BitSerializer::Convert::To<std::string>(value));
			EXPECT_EQ(std::u16string(expected.begin(), expected.end()), BitSerializer::Convert::To<std::u16string>(value));
			EXPECT_EQ(std::u32string(expected.begin(), expected.end()), BitSerializer::Convert::To<std::u32string>(value));
			if constexpr (std::is_signed_v<T>) {
				EXPECT_EQ(std::to_wstring(static_cast<long long>(value)), BitSerializer::Convert::To<std::wstring>(value));
			}
			else {
				EXPECT_EQ(std::to_wstring(static_cast<unsigned long long>(value)), BitSerializer::Convert::To<std::wstring>(value));
			}
		}
	}
}

TEST(ConvertIntegerFormat, SignedTypes)
{
	using Limits = std::numeric_limits<std::int64_t>;
	ExpectAll<std::int64_t>({
		0, 1, 2, 9, 10, 11, 99, 100, 101, 999, 1000, 9999, 10000, 99999, 100000,
		123456789, 1000000000, 9999999999LL, 123456789012345LL,
		Limits::max(), Limits::min(), -1, -9, -10, -99, -100, -999,
		-1000, -9999, -10000, -99999, -100000, -123456789
	});
}

TEST(ConvertIntegerFormat, UnsignedTypes)
{
	using Limits = std::numeric_limits<std::uint64_t>;
	ExpectAll<std::uint64_t>({
		0u, 1u, 2u, 9u, 10u, 11u, 99u, 100u, 101u, 999u, 1000u, 9999u, 10000u,
		99999u, 100000u, 123456789u, 1000000000u, 9999999999ULL,
		123456789012345ULL, 10000000000000000ULL, 1000000000000000000ULL,
		Limits::max()
	});
}

TEST(ConvertIntegerFormat, NarrowTypes)
{
	ExpectAll<std::int8_t>({
		std::numeric_limits<std::int8_t>::min(), std::numeric_limits<std::int8_t>::max(), -1, 0, 1, 9, 10, 99, 100, 127
	});
	ExpectAll<std::uint8_t>({
		std::numeric_limits<std::uint8_t>::min(), std::numeric_limits<std::uint8_t>::max(), 0, 1, 9, 10, 99, 100, 255
	});
	ExpectAll<std::int16_t>({
		std::numeric_limits<std::int16_t>::min(), std::numeric_limits<std::int16_t>::max(), -1, 0, 1, 999, 1000, 32767
	});
	ExpectAll<std::uint16_t>({
		std::numeric_limits<std::uint16_t>::min(), std::numeric_limits<std::uint16_t>::max(), 0, 1, 999, 1000, 65535
	});
	ExpectAll<std::int32_t>({
		std::numeric_limits<std::int32_t>::min(), std::numeric_limits<std::int32_t>::max(), -1, 0, 1, 999999999, 1000000000
	});
	ExpectAll<std::uint32_t>({
		std::numeric_limits<std::uint32_t>::min(), std::numeric_limits<std::uint32_t>::max(), 0, 1, 999999999, 1000000000
	});
}

TEST(ConvertIntegerFormat, CharacterTypes)
{
	ExpectAll<char>({ std::numeric_limits<char>::min(), std::numeric_limits<char>::max(), 0, 1, 42 });
	ExpectAll<signed char>({ -128, -1, 0, 1, 127 });
	ExpectAll<unsigned char>({ 0, 1, 128, 255 });
}

TEST(ConvertIntegerFormat, Exhaustive8Bit)
{
	for (int value = -128; value <= 127; ++value) {
		const auto typed = static_cast<std::int8_t>(value);
		EXPECT_EQ(Expected(typed), FormatWithHelper(typed));
	}
	for (int value = 0; value <= 255; ++value) {
		const auto typed = static_cast<std::uint8_t>(value);
		EXPECT_EQ(Expected(typed), FormatWithHelper(typed));
	}
}

TEST(ConvertIntegerFormat, Exhaustive16Bit)
{
	for (int value = -32768; value <= 32767; ++value) {
		const auto typed = static_cast<std::int16_t>(value);
		EXPECT_EQ(Expected(typed), FormatWithHelper(typed));
	}
	for (int value = 0; value <= 65535; ++value) {
		const auto typed = static_cast<std::uint16_t>(value);
		EXPECT_EQ(Expected(typed), FormatWithHelper(typed));
	}
}

TEST(ConvertIntegerFormat, Random64Bit)
{
	std::mt19937_64 random(123456789);
	for (int i = 0; i < 100000; ++i)
	{
		const std::uint64_t unsignedValue = random();
		EXPECT_EQ(Expected(unsignedValue), FormatWithHelper(unsignedValue));

		const auto signedValue = static_cast<std::int64_t>(unsignedValue);
		EXPECT_EQ(Expected(signedValue), FormatWithHelper(signedValue));
	}
}

TEST(ConvertIntegerFormat, ConvertToMatchesStandardConversion)
{
	using Limits = std::numeric_limits<std::int64_t>;
	ExpectConvertAll<std::int64_t>({
		0, 1, -1, 9, 10, -10, 99, 100, 123456789, 123456789012345,
		Limits::min(), Limits::max()
	});
	ExpectConvertAll<std::uint64_t>({
		0, 1, 9, 10, 99, 100, 123456789, 123456789012345,
		std::numeric_limits<std::uint64_t>::max()
	});
	ExpectConvertAll<std::int8_t>({ -128, -1, 0, 1, 127 });
	ExpectConvertAll<std::uint8_t>({ 0, 1, 255 });
	ExpectConvertAll<std::int16_t>({ -32768, -1, 0, 32767 });
	ExpectConvertAll<std::uint16_t>({ 0, 1, 65535 });
	ExpectConvertAll<std::int32_t>({ std::numeric_limits<std::int32_t>::min(), -1, 0, std::numeric_limits<std::int32_t>::max() });
	ExpectConvertAll<std::uint32_t>({ 0, 1, std::numeric_limits<std::uint32_t>::max() });
}

TEST(ConvertIntegerFormat, FitsInCapacity)
{
	namespace Detail = BitSerializer::Convert::Detail;
	EXPECT_FALSE(Detail::FitsInCapacity<std::int8_t>(0, 0));
	EXPECT_TRUE(Detail::FitsInCapacity<std::int8_t>(0, 2));
	EXPECT_TRUE(Detail::FitsInCapacity<std::int8_t>(9, 2));
	EXPECT_FALSE(Detail::FitsInCapacity<std::int8_t>(10, 2));
	EXPECT_TRUE(Detail::FitsInCapacity<std::int8_t>(-9, 3));
	EXPECT_FALSE(Detail::FitsInCapacity<std::int8_t>(-9, 2));
	EXPECT_TRUE(Detail::FitsInCapacity<std::int64_t>(12345678901234LL, 15));
	EXPECT_FALSE(Detail::FitsInCapacity<std::int64_t>(123456789012345LL, 15));
	EXPECT_TRUE(Detail::FitsInCapacity<std::int64_t>(-1234567890123LL, 15));
	EXPECT_FALSE(Detail::FitsInCapacity<std::int64_t>(-12345678901234LL, 15));
	EXPECT_TRUE(Detail::FitsInCapacity<std::int64_t>((std::numeric_limits<std::int64_t>::min)(), 21));
	EXPECT_TRUE(Detail::FitsInCapacity<std::uint64_t>((std::numeric_limits<std::uint64_t>::max)(), 21));
	EXPECT_FALSE(Detail::FitsInCapacity<std::uint64_t>((std::numeric_limits<std::uint64_t>::max)(), 20));
}

TEST(ConvertIntegerFormat, MaxIntegerLengthFor)
{
	namespace Detail = BitSerializer::Convert::Detail;
	EXPECT_EQ(5u, Detail::MaxIntegerLengthFor<std::int8_t>());
	EXPECT_EQ(4u, Detail::MaxIntegerLengthFor<std::uint8_t>());
	EXPECT_EQ(7u, Detail::MaxIntegerLengthFor<std::int16_t>());
	EXPECT_EQ(6u, Detail::MaxIntegerLengthFor<std::uint16_t>());
	EXPECT_EQ(12u, Detail::MaxIntegerLengthFor<std::int32_t>());
	EXPECT_EQ(11u, Detail::MaxIntegerLengthFor<std::uint32_t>());
	EXPECT_EQ(21u, Detail::MaxIntegerLengthFor<std::int64_t>());
	EXPECT_EQ(21u, Detail::MaxIntegerLengthFor<std::uint64_t>());
}

TEST(ConvertIntegerFormat, ConvertToStringDoesNotAllocateForSmallTypes)
{
	using CountingString = std::basic_string<char, std::char_traits<char>, CountingAllocator<char>>;
	CountingAllocatorState state;
	CountingString text{CountingAllocator<char>{&state}};

	// The string itself may allocate (e.g. a debug container proxy); measure only the conversion.
	state.allocations = 0;

	BitSerializer::Convert::Detail::To(std::int8_t{ -42 }, text);
	EXPECT_EQ("-42", std::string_view(text.data(), text.size()));
	EXPECT_EQ(0u, state.allocations);

	state.allocations = 0;
	text.clear();
	BitSerializer::Convert::Detail::To(std::int32_t{ -2147483647 }, text);
	EXPECT_EQ("-2147483647", std::string_view(text.data(), text.size()));
	EXPECT_EQ(0u, state.allocations);

	// A short int64 value still fits into the SSO capacity of a fresh string.
	state.allocations = 0;
	text.clear();
	BitSerializer::Convert::Detail::To(std::int64_t{ -42 }, text);
	EXPECT_EQ("-42", std::string_view(text.data(), text.size()));
	EXPECT_EQ(0u, state.allocations);
}

TEST(ConvertIntegerFormat, ConvertToStringWritesDirectlyIntoAvailableCapacity)
{
	using CountingString = std::basic_string<char, std::char_traits<char>, CountingAllocator<char>>;
	CountingAllocatorState state;
	CountingString text{CountingAllocator<char>{&state}};
	text.reserve(32);
	state.allocations = 0;

	const auto value = (std::numeric_limits<std::int64_t>::min)();
	BitSerializer::Convert::Detail::To(value, text);
	EXPECT_EQ(std::to_string(value), std::string_view(text.data(), text.size()));
	EXPECT_EQ(0u, state.allocations);
}
