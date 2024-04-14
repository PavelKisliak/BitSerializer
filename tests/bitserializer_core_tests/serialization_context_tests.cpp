/*******************************************************************************
* Copyright (C) 2018-2024 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <gtest/gtest.h>
#include "bitserializer/serialization_detail/serialization_context.h"

using namespace BitSerializer;

//-----------------------------------------------------------------------------
// Tests of serialization for classes and unions
//-----------------------------------------------------------------------------
TEST(ValidatorRequired, ShouldThrowValidationExceptionWhenAnyErrors)
{
	// Arrange
	const SerializationOptions options;
	SerializationContext context(options);

	// Act
	context.AddValidationError("path", "error1");

	// Assert
	EXPECT_THROW(context.OnFinishSerialization(), BitSerializer::ValidationException);
}

TEST(ValidatorRequired, ShouldAddValidationErrorsToException)
{
	// Arrange
	const SerializationOptions options;
	SerializationContext context(options);
	ValidationMap errorsMap;

	// Act
	context.AddValidationError("path1", "error1");
	context.AddValidationError("path2", "error2");
	try
	{
		context.OnFinishSerialization();
	}
	catch (const ValidationException& ex)
	{
		errorsMap = ex.GetValidationErrors();
	}

	// Assert
	EXPECT_EQ(2, errorsMap.size());
	ASSERT_TRUE(errorsMap.find("path1") != errorsMap.cend());
	ASSERT_TRUE(errorsMap.find("path2") != errorsMap.cend());
	EXPECT_EQ("error1", errorsMap["path1"].front());
	EXPECT_EQ("error2", errorsMap["path2"].front());
}

TEST(ValidatorRequired, ShouldAllowToAddSeveralErrorsForOnePath)
{
	// Arrange
	const SerializationOptions options;
	SerializationContext context(options);
	ValidationMap errorsMap;

	// Act
	context.AddValidationError("path1", "error1");
	context.AddValidationError("path1", "error2");
	try
	{
		context.OnFinishSerialization();
	}
	catch (const ValidationException& ex)
	{
		errorsMap = ex.GetValidationErrors();
	}

	// Assert
	EXPECT_EQ(1, errorsMap.size());
	ASSERT_TRUE(errorsMap.find("path1") != errorsMap.cend());
	ASSERT_EQ(2, errorsMap["path1"].size());
	EXPECT_EQ("error1", errorsMap["path1"][0]);
	EXPECT_EQ("error2", errorsMap["path1"][1]);
}

TEST(ValidatorRequired, ShouldThrowErrorWhenExceededLimit)
{
	// Arrange
	SerializationOptions options;
	options.maxValidationErrors = 3;
	SerializationContext context(options);

	// Act / Assert
	context.AddValidationError("path1", "error1");
	context.AddValidationError("path2", "error2");
	EXPECT_THROW(context.AddValidationError("path3", "error3"), BitSerializer::ValidationException);
}
