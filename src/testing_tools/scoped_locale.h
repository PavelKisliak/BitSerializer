/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <algorithm>
#include <clocale>
#include <iterator>
#include <string>

/**
 * @brief Changes the C numeric locale for the lifetime of the object and restores the previous one.
 */
class ScopedLocale
{
public:
	ScopedLocale()
	{
		if (const char* current = std::setlocale(LC_NUMERIC, nullptr)) {
			mPrevious = current;
		}
	}

	~ScopedLocale()
	{
		std::setlocale(LC_NUMERIC, mPrevious.empty() ? "C" : mPrevious.c_str());
	}

	ScopedLocale(const ScopedLocale&) = delete;
	ScopedLocale& operator=(const ScopedLocale&) = delete;

	static bool Set(const char* localeName)
	{
		return std::setlocale(LC_NUMERIC, localeName) != nullptr;
	}

	/**
	 * @brief Tries to activate a locale whose decimal separator is a comma.
	 * @return `true` when such locale was activated, `false` when none is available.
	 */
	static bool TrySetComma()
	{
		static const char* const localeNames[] = {
#if defined(_WIN32)
			"German_Germany.1252", "German_Germany.utf8", "de-DE", "de-DE.UTF-8",
			"French_France.1252", "fr-FR", "fr-FR.UTF-8",
			"Russian_Russia.1251", "ru-RU", "ru-RU.UTF-8"
#else
			"de_DE.UTF-8", "de_DE.utf8", "de_DE",
			"fr_FR.UTF-8", "fr_FR.utf8", "fr_FR",
			"ru_RU.UTF-8", "ru_RU.utf8", "ru_RU"
#endif
		};

		return std::any_of(std::begin(localeNames), std::end(localeNames), [](const char* localeName)
		{
			if (!Set(localeName)) {
				return false;
			}
			const char* decimalPoint = std::localeconv()->decimal_point;
			return decimalPoint != nullptr && decimalPoint[0] == ',' && decimalPoint[1] == '\0';
		});
	}

private:
	std::string mPrevious;
};
