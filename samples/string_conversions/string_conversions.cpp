#include <iostream>
#include <charconv>
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
REGISTER_ENUM_MAP(Number)
{
	{ Number::One, "One" },
	{ Number::Two, "Two" },
	{ Number::Three, "Three" },
	{ Number::Four, "Four" },
	{ Number::Five, "Five" }
} END_ENUM_MAP();


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

	void FromString(const std::string_view& str)
	{
		size_t offset = 0;
		for (int* value : { &x, &y, &z })
		{
			offset = str.find_first_not_of(' ', offset);
			if (offset == std::string_view::npos) {
				throw std::out_of_range("Bad argument");
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

int main()
{
	// Conversion fundamental types (this function may throw "std::out_of_range" exception)
	const auto u32Str = Convert::To<std::u32string>(3.14159f);
	const auto f1 = Convert::To<float>(u32Str);
	std::cout << "Conversion to float result: " << f1 << std::endl;

	// Convert from one UTF string to another (there is used "syntax sugar" function ToString() which is equivalent to Convert::To<std::string>)
	const auto u8Str = Convert::ToString(u"Привет мир!");
	assert(u8"Привет мир!" == u8Str);

	// Conversion with error handling (overflow, parse errors, etc)
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
	const auto strPoint = Convert::To<std::string>(point);
	std::cout << "Conversion CPoint3D to string result: " << strPoint << std::endl;

	return EXIT_SUCCESS;
}
