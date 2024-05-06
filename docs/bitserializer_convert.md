### [BitSerializer](../README.md) / Convert

Type conversion submodule of **BitSerializer** library. Basically, it is just convenient wrapper around STD, but with some interesting features:

- Supports modern STD types - `string_view`, `u16string` and `u32string`
- Supports conversion `chrono::time_point` and `chrono::duration` to/from ISO8601 strings
- Supports UTF transcoding between all STD string types
- Conversion any fundamental type to any STD string types and vice versa
- Conversion any fundamental type to any other fundamental type with overflow checking (not available in v0.65)
- Conversion enum types via declaring names map
- Allows to overload conversion functions for custom types
- Simple API - only two main functions `Convert::To<>()` and `Convert::TryTo<>()`

### Conversion fundamental types
The library provides several public functions for convert types:

- `TOut To<TOut>(TIn&& value)` may throw exceptions
- `std::optional<TOut> TryTo<TOut>(TIn&& value)` throws nothing
- `std::string ToString(TIn&& value)` just "syntax sugar" for `To<std::string>()`
- `std::wstring ToWString(TIn&& value)` just "syntax sugar" for `To<std::wstring>()`
- `bool IsConvertible<TIn, TOut>()` checks whether conversion from `TIn` to `TOut` is supported (not available in v0.65)

Under the hood, integer and floating types are converting via modern `std::from_chars()` and `std::to_chars()`, except for old versions of GCC and CLANG compilers, which do not support floating types. In this case, will be used older (and significantly slower) functions from C++11.
```cpp
#include "bitserializer/convert.h"

using namespace BitSerializer;

int main()
{
	// Conversion fundamental types (this function may throw exception)
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
In comparison with macro `REGISTER_ENUM_MAP` you have to take care of including the header file in which you declared this.

### Date and time conversion
*(Feature is not available in the previously released version 0.50)*<br>
Date, time and duration can be converted to string representation of ISO 8601 and vice versa. The following table contains all supported types with string examples:

| Type | Format | Examples | References |
| ------ | ------ | ------ | ------ |
| `std::time_t` | YYYY-MM-DDThh:mm:ssZ | 1677-09-21T00:12:44Z<br>2262-04-11T23:47:16Z | [ISO 8601/UTC](https://en.wikipedia.org/wiki/ISO_8601) |
| `chrono::time_point` | [±]YYYY-MM-DDThh:mm:ss[.SSS]Z | 1872-01-01T04:55:32.021Z<br>2262-04-11T23:47:16Z<br>9999-12-31T23:59:59.999Z<br>+12376-01-20T00:00:00Z<br>-1241-06-23T00:00:00Z | [ISO 8601/UTC](https://en.wikipedia.org/wiki/ISO_8601)  |
| `chrono::duration` | [±]PnWnDTnHnMnS | P125DT55M41S<br>PT10H20.346S<br>P10DT25M<br>P35W5D | [ISO 8601/Duration](https://en.wikipedia.org/wiki/ISO_8601#Durations)  |

**Time point notes:**
- Only UTC representation is supported, fractions of a second are optional ([±]YYYY-MM-DDThh:mm:ss[.SSS]Z).
- ISO-8601 doesn't specify precision for fractions of second, BitSerializer supports up to 9 digits, which is enough for values with nanosecond precision.
- Both decimal separators (dot and comma) are supported for fractions of a second.
- According to standard, to represent years before 0000 or after 9999 uses additional '-' or '+' sign.
- The date range depends on the `std::chrono::duration` type, for example implementation of `system_clock` on Linux has range **1678...2262 years**.
- Keep in mind that `std::chrono::system_clock` has time point with different duration on Windows and Linux, prefer to store time in custom `time_point` if you need predictable range (e.g. `time_point<system_clock, milliseconds>`).
- According to the C++20 standard, the EPOCH date for `system_clock` types is considered as *1970-01-01 00:00:00 UTC* excluding leap seconds.
- For avoid mistakes, time points with **steady_clock**  type are not allowed due to floating EPOCH.
- Allowed rounding only fractions of seconds, in all other cases `std::out_of_range` exception is thrown.

**Duration notes:**
- Supported a sign character at the start of the string (ISO 8601-2 extension).
- Durations which contains years, month, or with base UTC (2003-02-15T00:00:00Z/P2M) are not allowed.
- The decimal fraction supported only for seconds part, maximum 9 digits.
- Both decimal separators (dot and comma) are supported for fractions of a second.
- Allowed rounding only fractions of seconds, in all other cases `std::out_of_range` exception is thrown.

Since `std::time_t` is equal to `int64_t`, need to use special wrapper `CRawTime`, otherwise time will be converted as number.
```cpp
time_t time = Convert::To<CRawTime>("1969-12-31T23:59:59Z");
std::string utc = Convert::ToString(CRawTime(time));
```

### Conversion std::filesystem::path

The library allows to convert `std::filesystem::path` to `std::basic_string` family types and vice versa.
Like the other parts, it knows how to transcode the path between different UTF encodings.
Support of `std::filesystem` can be disabled via definition`BITSERIALIZER_HAS_FILESYSTEM 0`.

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
In addition to simple UTF conversion via `Convert::To()` function, there is also exists set of classes with more granular API for all kind of formats:

- `Convert::Utf8`
- `Convert::Utf16Le`
- `Convert::Utf16Be`
- `Convert::Utf32Le`
- `Convert::Utf32Be`

They all have the same API:

- `void Decode(beginIt, endIt, outStr, errSym)`
- `void Encode(beginIt, endIt, outStr, errSym)`

There are also exists two functions for detect the UTF encoding:

- `UtfType DetectEncoding(std::istream& inputStream, bool skipBomWhenFound = true)`
- `UtfType DetectEncoding(std::string_view inputString, size_t& out_dataOffset)`
