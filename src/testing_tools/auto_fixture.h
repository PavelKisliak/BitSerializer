/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#if defined __has_include && __has_include(<version>)
#include <version>
#endif
#if defined(__cpp_lib_memory_resource)
#include <memory_resource>
#endif
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <type_traits>

#include "string_utils.h"
#include "bitserializer/convert.h"
#include "bitserializer/serialization_detail/bin_timestamp.h"

namespace AutoFixture
{
	/**
	 * @brief Generates random test fixtures with configurable generation parameters.
	 *
	 * Provides a seeded random engine, the default size for generated containers, and
	 * the `Build()` entry point that dispatches to the free `BuildFixture(Fixture&, T&)`
	 * overloads (found via ADL on `Fixture&`).
	 *
	 * The configuration is immutable once constructed; use `WithContainerSize()` or
	 * `WithSeed()` to obtain a copy with a different configuration (fluent style).
	 */
	class Fixture
	{
	public:
		/**
		 * @brief Creates a fixture with the given seed.
		 *
		 * @param seed The seed for the random engine.
		 */
		explicit Fixture(unsigned seed = 5489u)
			: mRandomEngine(seed)
		{ }

		/**
		 * @brief Returns a copy of the fixture with a different container size.
		 *
		 * @param size The number of elements to generate in containers.
		 * @return A new fixture with the updated container size.
		 */
		[[nodiscard]] Fixture WithContainerSize(size_t size) const
		{
			Fixture result(*this);
			result.mContainerSize = size;
			return result;
		}

		/**
		 * @brief Returns a copy of the fixture with a different random engine seed.
		 *
		 * @param seed The new seed for the random engine.
		 * @return A new fixture with the updated seed.
		 */
		[[nodiscard]] Fixture WithSeed(unsigned seed) const
		{
			Fixture result(*this);
			result.mRandomEngine.seed(seed);
			return result;
		}

		/** @brief Returns the number of elements generated for containers. */
		[[nodiscard]] size_t GetContainerSize() const noexcept { return mContainerSize; }

		/** @brief Returns a random unsigned value. */
		unsigned Rand() { return mRandomEngine(); }

		/** @brief Returns a random value in the full range of an integral type. */
		template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
		T Rand()
		{
			std::uniform_int_distribution<T> distr((std::numeric_limits<T>::lowest)(), (std::numeric_limits<T>::max)());
			return distr(mRandomEngine);
		}

		/** @brief Returns a random value in the full range of a floating-point type. */
		template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
		T Rand()
		{
			std::uniform_real_distribution<T> distr((std::numeric_limits<T>::lowest)(), (std::numeric_limits<T>::max)());
			return distr(mRandomEngine);
		}

		/** @brief Returns a random value in the given [min, max] range. */
		template <typename T>
		T Rand(T min, T max)
		{
			if constexpr (std::is_integral_v<T>)
			{
				std::uniform_int_distribution<T> distr(min, max);
				return distr(mRandomEngine);
			}
			else
			{
				std::uniform_real_distribution<T> distr(min, max);
				return distr(mRandomEngine);
			}
		}

		/**
		 * @brief Builds a test fixture for the given value (dispatches to `BuildFixture(Fixture&, T&)`).
		 */
		template <typename T>
		void Build(T& value)
		{
			BuildFixture(*this, value);
		}

		/**
		 * @brief Builds a test fixture and returns it by value (can't be applied to C-array types).
		 *
		 * @return The generated fixture.
		 */
		template <typename T, std::enable_if_t<!std::is_array_v<T>, int> = 0>
		T Build()
		{
			T fixture{};
			Build(fixture);
			return fixture;
		}

		/** @brief Returns the default shared fixture instance (used by the global `BuildFixture()` forwarders). */
		static Fixture& GetDefault()
		{
			static Fixture instance;
			return instance;
		}

	private:
		size_t mContainerSize = 7;
		std::mt19937 mRandomEngine;
	};

	//-----------------------------------------------------------------------------
	// Type traits
	//-----------------------------------------------------------------------------

	/**
	 * @brief Checks whether a type has a static `BuildFixture()` method.
	 */
	template <typename T>
	struct has_build_fixture_method
	{
	private:
		template <typename U>
		static decltype(U::BuildFixture(std::declval<U&>()), std::true_type()) test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<T>(0)) type;
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool has_build_fixture_method_v = has_build_fixture_method<T>::value;

	/**
	 * @brief Checks whether a type has an `Assert()` method.
	 */
	template <typename T>
	struct has_assert_method
	{
	private:
		template <typename U>
		static decltype(std::declval<U>().Assert(std::declval<const U&>()), std::true_type()) test(int);

		template <typename>
		static std::false_type test(...);

	public:
		typedef decltype(test<T>(0)) type;
		enum { value = type::value };
	};

	template <typename T>
	constexpr bool has_assert_method_v = has_assert_method<T>::value;

	//-----------------------------------------------------------------------------

	/**
	 * @brief Builds a test fixture for class/union types that define a static `BuildFixture()` method.
	 *
	 * @tparam T Class or union type with a static `BuildFixture()` method.
	 * @param value Reference to the object to initialize.
	 */
	template <typename T, std::enable_if_t<(std::is_class_v<T> || std::is_union_v<T>) && has_build_fixture_method_v<T>, int> = 0>
	void BuildFixture(Fixture&, T& value)
	{
		T::BuildFixture(value);
	}

	/**
	 * @brief Compile-time fallback for class/union types without a known fixture generator.
	 *
	 * Fails with a helpful message, telling the user to either include the corresponding
	 * header from `auto_fixture/std/` or define a static `BuildFixture()` method.
	 *
	 * @tparam T Class or union type without a static `BuildFixture()` method.
	 * @param value Reference to the object that can't be initialized.
	 */
	template <typename T, std::enable_if_t<(std::is_class_v<T> || std::is_union_v<T>) && !has_build_fixture_method_v<T>, int> = 0>
	void BuildFixture(Fixture&, T& value)
	{
		static_assert(has_build_fixture_method_v<T>,
			"BuildFixture: the type has no `static BuildFixture()` method and is not a known STD type. "
			"Include the corresponding header from \"testing_tools/auto_fixture/std/\" or define a static BuildFixture() method.");
		(void)value;
	}

	/**
	 * @brief Builds a test fixture for integral types (any type convertible from a random `int`).
	 *
	 * @tparam T Integral type.
	 * @param value Reference to the value to populate.
	 */
	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	void BuildFixture(Fixture& fixture, T& value)
	{
		value = static_cast<T>(fixture.Rand());
	}

	inline void BuildFixture(Fixture& fixture, int64_t& value)		{ value = fixture.Rand<int64_t>(); }
	inline void BuildFixture(Fixture& fixture, uint64_t& value)		{ value = fixture.Rand<uint64_t>(); }
	inline void BuildFixture(Fixture& fixture, bool& value)			{ value = static_cast<bool>(fixture.Rand() % 2); }
	inline void BuildFixture(Fixture& fixture, float& value)		{ value = static_cast<float>((fixture.Rand() % 1000) + 1) * 1.141592f; }
	inline void BuildFixture(Fixture& fixture, double& value)		{ value = static_cast<double>((fixture.Rand() % 100000) + 1) * 1.141592; }
	inline void BuildFixture(Fixture&, std::nullptr_t& value)		{ value = nullptr; }

	inline void BuildFixture(Fixture& fixture, std::string& value)		{ value = UTF8("UTF-8 Тест_") + std::to_string(fixture.Rand()); }
#if defined(__cpp_lib_memory_resource)
	inline void BuildFixture(Fixture& fixture, std::pmr::string& value)	{ value = UTF8("UTF-8 Тест_") + std::to_string(fixture.Rand()); }
#endif
#if defined(__cpp_lib_char8_t)
	inline void BuildFixture(Fixture&, std::u8string& value)		{ value = u8"U8-string Тест_"; }
#endif
	inline void BuildFixture(Fixture& fixture, std::wstring& value)		{ value = L"WString Тест_" + std::to_wstring(fixture.Rand()); }
	inline void BuildFixture(Fixture& fixture, std::u16string& value)	{ value = u"UTF-16 Тест_" + BitSerializer::Convert::To<std::u16string>(fixture.Rand()); }
	inline void BuildFixture(Fixture& fixture, std::u32string& value)	{ value = U"UTF-32 Тест_" + BitSerializer::Convert::To<std::u32string>(fixture.Rand()); }

	inline void BuildFixture(Fixture& fixture, std::byte& value)
	{
		value = static_cast<std::byte>(fixture.Rand() % (std::numeric_limits<unsigned char>::max)());
	}

	/**
	 * @brief Builds a test fixture for enum types using a randomly selected registered value.
	 *
	 * @tparam T Enum type.
	 * @param value Reference to the enum value to populate.
	 */
	template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
	void BuildFixture(Fixture& fixture, T& value)
	{
		const auto randIndex = fixture.Rand() % BitSerializer::Convert::Detail::EnumRegistry<T>::size();
		value = (BitSerializer::Convert::Detail::EnumRegistry<T>::cbegin() + randIndex)->value;
	}

	/**
	 * @brief Builds a test fixture for C-style arrays.
	 *
	 * For arithmetic element types the array is populated with the type's min/max
	 * boundary values; otherwise each element is initialized recursively.
	 *
	 * @tparam TValue Element type.
	 * @tparam ArraySize Number of elements in the array.
	 * @param fixture The fixture instance (propagated to nested elements).
	 * @param arr Reference to the array.
	 */
	template <typename TValue, size_t ArraySize>
	void BuildFixture(Fixture& fixture, TValue(&arr)[ArraySize])
	{
		static_assert(ArraySize != 0);

		if constexpr (std::is_arithmetic_v<TValue>)
		{
			// Using min/max values as generated elements in the array
			arr[0] = (std::numeric_limits<TValue>::lowest)();
			if constexpr (ArraySize > 1)
			{
				for (size_t i = 1; i < (ArraySize - 1); i++) {
					BuildFixture(fixture, arr[i]);
				}
				arr[ArraySize - 1] = (std::numeric_limits<TValue>::max)();
			}
		}
		else
		{
			for (size_t i = 0; i < ArraySize; i++) {
				BuildFixture(fixture, arr[i]);
			}
		}
	}
} // namespace AutoFixture

/**
 * @brief Builds a test fixture for any type (delegates to the default `AutoFixture::Fixture`).
 *
 * @tparam T Type to initialize.
 * @param value Reference to the object to populate.
 */
template <typename T>
void BuildFixture(T& value)
{
	AutoFixture::Fixture::GetDefault().Build(value);
}

/**
 * @brief Builds a test fixture for a C-style array (delegates to the default `AutoFixture::Fixture`).
 *
 * @tparam TValue Element type.
 * @tparam ArraySize Number of elements in the array.
 * @param arr Reference to the array.
 */
template <typename TValue, size_t ArraySize>
void BuildFixture(TValue(&arr)[ArraySize])
{
	AutoFixture::Fixture::GetDefault().Build(arr);
}

/**
 * @brief Builds a test fixture and returns it by value (can't be applied to C-array types).
 *
 * @tparam T Type to generate.
 * @return The generated fixture.
 */
template <typename T, std::enable_if_t<!std::is_array_v<T>, int> = 0>
T BuildFixture()
{
	return AutoFixture::Fixture::GetDefault().Build<T>();
}
