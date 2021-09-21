### [BitSerializer](../README.md) / Convert

String conversion submodule of **BitSerializer** library. Basically, it is just convenient wrapper around STD, but with some interesting features:

- Supports modern STD types - `string_view`, `u16string` and `u32string`.
- Supports UTF transcoding between all STD string types.
- Converts any fundamental types to any STD string types and vice versa.
- Converts enum types via declaring names map.
- Allows to overload conversion functions for custom types.
- Simple API - only two main functions `Convert::To<>()` and `Convert::TryTo<>()`.

### Conversion fundamental types
The library provides several public functions for convert types:
- `TOut To<TOut>(TIn&& value)` may throw exceptions.
- `std::optional<TOut> TryTo<TOut>(TIn&& value)` throws nothing.
- `std::string ToString(TIn&& value)` just "syntax sugar" for To<std::string>().
- `std::wstring ToWString(TIn&& value)` just "syntax sugar" for To<std::wstring>().

Under the hood, integer types converts via modern `std::from_chars()`, but any other via functions from older C++ (there is a delay in their implentation by GCC and CLANG compilers).
```cpp
#include "bitserializer/convert.h"

using namespace BitSerializer;

int main()
{
	// Conversion fundamental types (this function may throw "std::out_of_range" exception)
	const auto u32Str = Convert::To<std::u32string>(3.14159f);
	const auto f1 = Convert::To<float>(u32Str);
	std::cout << "Conversion to float result: " << f1 << std::endl;

	// Conversion with error handling (overflow, parse errors, etc)
	if (auto result = Convert::TryTo<char>("500")) {
		std::cout << "Result: " << result.value() << std::endl;
	}
	else {
		std::cout << "Overflow error when Convert::TryTo<char>(\"500\")" << std::endl;
	}

	return EXIT_SUCCESS;
}
```

### Transcoding strings
The **BitSerializer::Convert** module provides an easy way to transcode strings from one UTF to another. There is used internal implementation because STD does not provide such functionality (`<codecvt>` was deprecated in C++ 17). By default, the library assumes that:
- `std::string` is UTF-8 (encoding with code pages is not supported)
- `std::wstring` is UTF-16 or UTF-32 (depending on the platform)
- `std::u16string` is UTF-16
- `std::u32string` is UTF-32
```cpp
#include "bitserializer/convert.h"

using namespace BitSerializer;

int main()
{
	// Convert from one UTF string to another (there is used "syntax sugar" function ToString() which is equivalent to Convert::To<std::string>)
	const auto u8Str = Convert::ToString(u"Привет мир!");
	assert(u8"Привет мир!" == u8Str);

	return EXIT_SUCCESS;
}
```

### Conversion enum types
For able to convert enum types, you need to define a name map, like as in the example below:
```cpp
#include <iostream>
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
} END_ENUM_MAP()


int main()
{
	// Convert enum types
	const auto u16Str = Convert::To<std::u16string>(Number::Five);
	assert(Number::Five == Convert::To<Number>(u16Str));

	return EXIT_SUCCESS;
}
```

Additionally, you can declare functions for support input/output streams using `DECLARE_ENUM_STREAM_OPS` macro, as shown below:
```cpp
DECLARE_ENUM_STREAM_OPS(EnumType)
```
In comparison with macro `REGISTER_ENUM_MAP` you should take care to include the header file in which you declared this.

### Conversion custom classes
There are several ways to convert custom classes from/to strings:
- Implementation pair of internal string conversion methods `ToString()` and `FromString()`.
- Implementation external function(s) `To(in, out)` (in any namespace).
- Implementation external function in STD style `to_string()`, but there is no backward conversion.
```cpp
#include <iostream>
#include <charconv>
#include "bitserializer/convert.h"

using namespace BitSerializer;

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
	// Convert custom class
	const auto point = Convert::To<CPoint3D>("640 480 120");
	const auto strPoint = Convert::To<std::string>(point);
	std::cout << "Conversion CPoint3D to string result: " << strPoint << std::endl;

	return EXIT_SUCCESS;
}
```
The important point is that you can implement just pair of methods for `std::string`, all other string types will be also automatically supported as well, but you can implement them to avoid the performance overhead when transcoding:
- `std::u16string ToU16String();`
- `void FromString(std::u16string_view);`
- `std::u32string ToU32String();`
- `void FromString(std::u32string_view);`

As an alternative to internal methods, you can achieve the same by implementing two global functions in any namespace:
- `void To(const CPoint3D& in, std::string& out);`
- `void To(std::string_view in, CPoint3D& out);`
Optionally, they can be overridden for conversions any other string types (when you are worried about performance).
As an examples, you also can see the conversion implementation for [filesystem::path](../include/bitserializer/conversion_detail/convert_std.h).

### UTF encoding
In addition to simple UTF conversion via Convert::To() function, there is also exists set of classes with more granular API for all kind of formats:
- `Convert::Utf8`
- `Convert::Utf16Le`
- `Convert::Utf16Be`
- `Convert::Utf32Le`
- `Convert::Utf32Be`

They all have the same API:
- `void Decode(beginIt, endIt, outStr, errSym)`
- `void Encode(beginIt, endIt, outStr, errSym)`

There is also exists a function which will help to detect UTF encoding by BOM:
`UtfType DetectEncoding(std::istream& inputStream, bool skipBomWhenFound = true)`
