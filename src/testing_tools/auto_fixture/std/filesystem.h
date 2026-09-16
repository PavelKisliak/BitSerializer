/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <filesystem>

#include "testing_tools/auto_fixture.h"

namespace AutoFixture
{
	/**
	 * @brief Builds a test fixture for `std::filesystem::path`.
	 *
	 * @param fixture The fixture instance (provides the random engine).
	 * @param path Reference to the `std::filesystem::path`.
	 */
	inline void BuildFixture(Fixture& fixture, std::filesystem::path& path)
	{
		path = std::filesystem::temp_directory_path() / (fixture.Build<std::string>() + ".txt");
	}
} // namespace AutoFixture
