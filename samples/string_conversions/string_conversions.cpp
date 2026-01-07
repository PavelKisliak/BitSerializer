#include <iostream>
#include <charconv>
#include <vector>
#if defined __has_include && __has_include(<version>)
#include <version>
#endif
#if defined(__cpp_lib_memory_resource)
#include <memory_resource>
#endif

#include "bitserializer/convert.h"

using namespace BitSerializer;

// Test enum
enum class Number {
	One = 1,
	Two = 2,
	Three = 3,
	Four = 4,
	Five = 5
};

// Registration names map
BITSERIALIZER_REGISTER_ENUM(Number, {
	{ Number::One, "One" },
	{ Number::Two, "Two" },
	{ Number::Three, "Three" },
	{ Number::Four, "Four" },
	{ Number::Five, "Five" }
})


class CPoint3D
{
public:
	CPoint3D() = default;

	CPoint3D(int x, int y) noexcept
		: x(x), y(y)
	{ }

	[[nodiscard]] std::string ToString() const {
		return std::to_string(x) + ' ' + std::to_string(y) + ' ' + std::to_string(z);
	}

	void FromString(std::string_view str)
	{
		size_t offset = 0;
		for (int* value : { &x, &y, &z })
		{
			offset = str.find_first_not_of(' ', offset);
			if (offset == std::string_view::npos) {
				throw std::invalid_argument("Bad argument");
			}

			const auto r = std::from_chars(str.data() + offset, str.data() + str.size(), *value);
			if (r.ec != std::errc()) {
				throw std::out_of_range("Argument out of range");
			}
			offset = r.ptr - str.data();
		}
	}

	int x = 0, y = 0, z = 0;
};

int main()	// NOLINT(bugprone-exception-escape)
{
	// Conversion fundamental types (this function may throw "std::out_of_range" exception)
	const auto u32Str = Convert::To<std::u32string>(3.14159f);
	const auto f1 = Convert::To<float>(u32Str);
	std::cout << "Conversion to float result: " << f1 << std::endl;

	// Convert from one UTF string to another (there is used "syntax sugar" function ToString() which is equivalent to Convert::To<std::string>)
	const auto u8Str = Convert::ToString(u"Привет мир!");
	assert(reinterpret_cast<const char*>(u8"Привет мир!") == u8Str);

	// Convert with error handling (overflow, parse errors, etc)
	if (auto result = Convert::TryTo<char>("500")) {
		std::cout << "Result: " << result.value() << std::endl;
	}
	else {
		std::cout << "Overflow error when Convert::TryTo<char>(\"500\")" << std::endl;
	}

	// Convert enum types
	const auto u16Str = Convert::To<std::u16string>(Number::Five);
	assert(Number::Five == Convert::To<Number>(u16Str));

	// Convert custom class
	const auto point = Convert::To<CPoint3D>("640 480 120");
	const auto pointStr = Convert::To<std::string>(point);
	std::cout << "Conversion CPoint3D to string result: " << pointStr << std::endl;

	// Convert using an additional argument to construct the target type
	std::cout << Convert::To<std::string>(point, "Coordinates: ") << std::endl;

	// Convert to existing string
	std::string str = "FPS: ";
	[[maybe_unused]] const char* data = str.data();
	str = Convert::ToString(100, std::move(str));
	std::cout << str << std::endl;
	assert(data == str.data());

	// Convert to target type using its allocator
#if defined(__cpp_lib_memory_resource)
	char buffer[512]{};
	std::pmr::monotonic_buffer_resource pool{ std::data(buffer), std::size(buffer) };
	std::pmr::vector<std::pmr::string> vec(&pool);
	vec.emplace_back(Convert::To<std::pmr::string>(L"The quick brown fox jumps over the lazy dog.", vec.get_allocator()));
#endif

	return EXIT_SUCCESS;
}
