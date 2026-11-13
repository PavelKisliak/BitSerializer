### [BitSerializer](../../README.md) / Code style

When in doubt, follow the patterns you see in the surrounding code.

### Language and tooling

- **C++17** minimum standard
- **Clang-tidy** is configured (`.clang-tidy`) with all warnings treated as errors
- **Tabs** for indentation
- `#pragma once` for include guards

### Naming conventions

| Element | Convention | Examples |
|---------|-----------|----------|
| Classes / Structs (incl. class templates) | CamelCase, no `T` prefix | `ArchiveScope`, `SerializationOptions`, `JsonWriteObjectScope` |
| Interfaces | `I` + CamelCase | `IJsonReader`, `ICsvReader` |
| Public/protected methods | CamelCase | `ReadValue()`, `OpenArray()`, `GetPosition()` |
| Private methods | CamelCase | `ParseNextLine()`, `UnescapeValue()` |
| Private member variables | `m` + CamelCase | `mPos`, `mLineNumber`, `mInputData` |
| Public struct members | camelCase | `name`, `object`, `index` |
| Constructor parameters | camelCase with `in` prefix to avoid shadowing | `inValue`, `inName` |
| Local variables / Constants | camelCase | `codepoint`, `hexVal`, `targetValue` |
| Template parameters | `T` + CamelCase | `TSource`, `TTarget`, `TArchive` |
| Type aliases (public, in traits/classes) | `snake_case_type` | `value_type`, `key_type`, `input_archive_type` |
| Type aliases (internal) | `snake_case` | `archive_string_view`, `msgpack_variable_key` |
| Macros | `BITSERIALIZER_` + `SCREAMING_SNAKE_CASE` | `BITSERIALIZER_HAS_FILESYSTEM`, `BITSERIALIZER_REGISTER_ENUM` |
| Namespaces | CamelCase | `BitSerializer::Json::Detail` |

> [!NOTE]
> Legacy `C`-prefixed aliases (`CRawTime`, `CTimeRef`) are deprecated in favor of `UnixTime` and `TimeRef`; new code must use the non-prefixed names.
>
> The `T` prefix marks a template **parameter** (a placeholder), not a template **class**. Concrete types keep plain CamelCase even when they are class templates (e.g. `ArchiveScope<TMode>`); non-template types never take the `T` prefix either (e.g. `ScopeUnopened`).

### Braces and formatting

- **Always use braces** for `if`/`else`/`for`/`while` blocks
- Opening brace on a separate line:
  ```cpp
  if (ch == 'n' && mInputData.compare(mPos + 1, 3, "ull", 3) == 0)
  {
      mPos += 4;
      return true;
  }
  ```
- For single-statement blocks, it is acceptable to put the opening brace on the same line:
  ```cpp
  if (hexVal == 0xFF) {
      throw ParsingException("Invalid hex digit", lineNumber, pos);
  }
  ```
- `else` on its own line:
  ```cpp
  if (codepoint < 0x80)
  {
      buffer += static_cast<char>(codepoint);
  }
  else if (codepoint < 0x800)
  {
      buffer += static_cast<char>(0xC0 | (codepoint >> 6));
  }
  ```

### Documentation and headers

- **Doxygen-style** comments in English for public API: `/** @brief ... */`
- **MIT license header** required on all source files:
  ```cpp
  /*******************************************************************************
  * Copyright (C) 2018-2026 by Pavel Kisliak                                     *
  * This file is part of BitSerializer library, licensed under the MIT license.  *
  *******************************************************************************/
  ```
- Include order: license header, `#pragma once`, standard library, third-party, project headers
- Use `[[nodiscard]]` on const getters, `noexcept` where appropriate
