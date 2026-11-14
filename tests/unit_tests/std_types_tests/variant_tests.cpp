/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "testing_tools/common_test_methods.h"
#include "testing_tools/string_like_test_types.h"
#include "testing_tools/archive_stub.h"

#include "bitserializer/types/std/variant.h"

// STD types (test fixtures)
#include "testing_tools/auto_fixture/std/variant.h"
#include "testing_tools/auto_fixture/std/vector.h"

//-----------------------------------------------------------------------------
// Serialization tests for STL types.
// Because for serialization of STL types used base common methods for serialization,
// there is no need to write special tests for other types of archives.
//-----------------------------------------------------------------------------

using namespace BitSerializer;

namespace
{
	// Type that throws on default construction - used to create valueless variant via emplace
	struct ThrowOnConstruct
	{
		ThrowOnConstruct() {
			throw std::runtime_error("forced");
		}
		ThrowOnConstruct(const ThrowOnConstruct&) = delete;

		template <typename TArchive>
		void Serialize(TArchive&) {}
	};

	// Object type that is deliberately NOT registered with BITSERIALIZER_REGISTER_TYPE.
	struct UnregisteredObject
	{
		int value = 0;

		template <typename TArchive>
		void Serialize(TArchive& archive)
		{
			archive << BitSerializer::KeyValue("Value", value);
		}
	};
}

// Register types for VariantAsNamed tests
BITSERIALIZER_REGISTER_TYPE(int, "Int")
BITSERIALIZER_REGISTER_TYPE(std::string, "String")
BITSERIALIZER_REGISTER_TYPE(float, "Float")
BITSERIALIZER_REGISTER_TYPE(std::vector<int>, "IntVector")
BITSERIALIZER_REGISTER_TYPE(TestClassWithSubType<int>, "TestClassWithSubTypeInt")
BITSERIALIZER_REGISTER_TYPE(std::monostate, "Monostate")

//-----------------------------------------------------------------------------
// Tests of serialization for std::variant
//-----------------------------------------------------------------------------
TEST(STD_Variant, SerializeVariantWithPrimitiveAlternative) {
	TestSerializeType<ArchiveStub>(std::variant<int, std::string, float>(123));
}

TEST(STD_Variant, SerializeVariantWithStringAlternative) {
	TestSerializeType<ArchiveStub>(std::variant<int, std::string, float>(std::string("test")));
}

TEST(STD_Variant, SerializeVariantWithCustomStringAlternative) {
	// Custom string-like types (declared via BITSERIALIZER_DECLARE_STRING_TYPE) are
	// detected by the variant alternative capability check via the string path.
	TestSerializeType<ArchiveStub>(std::variant<int, TestStringLikeType, float>(TestStringLikeType("custom string")));
}

TEST(STD_Variant, SerializeVariantWithObjectAlternative) {
	TestSerializeType<ArchiveStub>(std::variant<int, TestPointClass, std::vector<int>>(TestPointClass(10, 20)));
}

TEST(STD_Variant, SerializeVariantWithArrayAlternative) {
	TestSerializeType<ArchiveStub>(std::variant<int, TestPointClass, std::vector<int>>(std::vector<int>{ 1, 2, 3, 4 }));
}

TEST(STD_Variant, SerializeVariantAsClassMember) {
	using VariantType = std::variant<int, std::string, TestPointClass, std::vector<int>>;
	TestSerializeType<ArchiveStub>(TestClassWithSubType(VariantType(TestPointClass(7, 11))));
	TestSerializeType<ArchiveStub>(TestClassWithSubType(VariantType(std::vector<int>{ 3, 1, 4 })));
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::monostate (represented as `null`)
//-----------------------------------------------------------------------------
TEST(STD_Variant, SerializeVariantWithMonostate) {
	using VariantType = std::variant<std::monostate, int, std::string>;
	TestSerializeType<ArchiveStub>(VariantType(std::monostate{}));
	TestSerializeType<ArchiveStub>(VariantType(123));
	TestSerializeType<ArchiveStub>(VariantType(std::string("test")));
}

TEST(STD_Variant, SerializeVariantWithOnlyMonostate) {
	TestSerializeType<ArchiveStub>(std::variant<std::monostate>(std::monostate{}));
}

TEST(STD_Variant, SerializeVariantWithRandomMonostateAlternative) {
	using VariantType = std::variant<std::monostate, int, std::string, TestPointClass>;
	TestSerializeType<ArchiveStub, VariantType>();
}

TEST(STD_Variant, SerializeVariantWithMonostateAsClassMember) {
	using VariantType = std::variant<std::monostate, TestPointClass>;
	TestSerializeType<ArchiveStub>(TestClassWithSubType(VariantType(std::monostate{})));
	TestSerializeType<ArchiveStub>(TestClassWithSubType(VariantType(TestPointClass(7, 11))));
}

TEST(STD_Variant, SerializeVariantAsNamedWithMonostate) {
	using VariantType = std::variant<std::monostate, int, TestPointClass>;
	VariantType testValue(std::monostate{}), actual(123);

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsNamed(testValue), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsNamed(actual), outputArchive);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantAsDiscriminatedWithMonostate) {
	using VariantType = std::variant<std::monostate, TestPointClass, TestClassWithSubType<int>>;

	VariantType testValue(std::monostate{}), actual(TestPointClass(1, 2));
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsDiscriminated(testValue), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsDiscriminated(actual), outputArchive);
	GTestExpectEq(testValue, actual);

	// A non-empty alternative is still serialized as before.
	VariantType testValue2(TestClassWithSubType<int>(7)), actual2(std::monostate{});
	outputArchive = {};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsDiscriminated(testValue2), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsDiscriminated(actual2), outputArchive);
	GTestExpectEq(testValue2, actual2);
}

TEST(STD_Variant, SerializeVariantWithDefaultNamedModeAndMonostate) {
	using VariantType = std::variant<std::monostate, int, TestPointClass>;
	VariantType testValue(std::monostate{}), actual(123);

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Named;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive, options);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantWithDefaultDiscriminatedModeAndMonostate) {
	using VariantType = std::variant<std::monostate, TestPointClass, TestClassWithSubType<int>>;
	VariantType testValue(std::monostate{}), actual(TestPointClass(1, 2));

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Discriminated;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive, options);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SkipVariantWhenIndexOutOfRange) {
	using SourceVariant = std::variant<int, std::string, double>;
	using TargetVariant = std::variant<int, std::string>;
	TestClassWithSubType<SourceVariant> source(SourceVariant(3.14));
	TestClassWithSubType<TargetVariant> target(TargetVariant(0));

	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::Skip;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(source, outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(target, outputArchive, options);
	EXPECT_EQ(0, std::get<int>(target.GetValue()));
}

TEST(STD_Variant, ThrowVariantMismatchedTypesExceptionWhenIndexOutOfRange) {
	using SourceVariant = std::variant<int, std::string, double>;
	using TargetVariant = std::variant<int, std::string>;

	// Serialize SourceVariant with index=2 (double), then try to load into TargetVariant (only 2 alternatives)
	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::ThrowError;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(TestClassWithSubType(SourceVariant(3.14)), outputArchive);

	TestClassWithSubType<TargetVariant> target(TargetVariant(0));
	EXPECT_THROW(BitSerializer::LoadObject<ArchiveStub>(target, outputArchive, options), BitSerializer::SerializationException);
}

TEST(STD_Variant, ThrowWhenSavingValuelessVariant) {
	// Create valueless variant by throwing during emplace
	std::variant<int, ThrowOnConstruct> v(42);
	try {
		v.emplace<ThrowOnConstruct>(); // throws during construction
	} catch (...)  // NOLINT(bugprone-empty-catch)
	{ }

	EXPECT_TRUE(v.valueless_by_exception());
	ArchiveStub::preferred_output_type outputArchive{};
	EXPECT_THROW(BitSerializer::SaveObject<ArchiveStub>(v, outputArchive), BitSerializer::SerializationException);
}

//-----------------------------------------------------------------------------
// Tests of serialization for VariantAsNamed wrapper
//-----------------------------------------------------------------------------
TEST(STD_Variant, SerializeVariantAsNamedWithPrimitiveAlternative)
{
	using VariantType = std::variant<int, std::string, float>;
	VariantType testValue, actual;
	BuildFixture(testValue);

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsNamed(testValue), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsNamed(actual), outputArchive);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantAsNamedWithObjectAlternative)
{
	using VariantType = std::variant<int, TestPointClass, std::vector<int>>;
	VariantType testValue(TestPointClass(10, 20)), actual;

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsNamed(testValue), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsNamed(actual), outputArchive);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantAsNamedWithArrayAlternative)
{
	using VariantType = std::variant<int, TestPointClass, std::vector<int>>;
	VariantType testValue(std::vector<int>{ 1, 2, 3, 4 }), actual;
	ArchiveStub::preferred_output_type outputArchive{};

	BitSerializer::SaveObject<ArchiveStub>(VariantAsNamed(testValue), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsNamed(actual), outputArchive);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, ThrowWhenLoadingVariantAsNamedWithUnknownType)
{
	// Save a variant whose active alternative is not registered in the target variant
	using SourceVariant = std::variant<int, std::string, float>;
	using TargetVariant = std::variant<std::string, float>;
	SourceVariant testValue(123);
	TargetVariant actual;

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsNamed(testValue), outputArchive);

	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::ThrowError;
	EXPECT_THROW(
		BitSerializer::LoadObject<ArchiveStub>(BitSerializer::VariantAsNamed(actual), outputArchive, options),
		BitSerializer::SerializationException
	);
}

TEST(STD_Variant, SkipWhenLoadingVariantAsNamedWithUnknownType)
{
	using SourceVariant = std::variant<int, std::string, float>;
	using TargetVariant = std::variant<std::string, float>;
	SourceVariant testValue(123);
	TargetVariant actual;

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsNamed(testValue), outputArchive);

	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::Skip;
	EXPECT_NO_THROW(BitSerializer::LoadObject<ArchiveStub>(BitSerializer::VariantAsNamed(actual), outputArchive, options));
	EXPECT_EQ(0U, actual.index());
}

//-----------------------------------------------------------------------------
// Tests of serialization for VariantAsDiscriminated wrapper
//-----------------------------------------------------------------------------
TEST(STD_Variant, SerializeVariantAsDiscriminatedWithObjectAlternative)
{
	using VariantType = std::variant<TestPointClass, TestClassWithSubType<int>>;
	VariantType testValue(TestPointClass(10, 20)), actual;

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsDiscriminated(testValue), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsDiscriminated(actual), outputArchive);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantAsDiscriminatedWithCustomTypeKey)
{
	using VariantType = std::variant<TestPointClass, TestClassWithSubType<int>>;
	VariantType testValue(TestPointClass(10, 20)), actual;

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsDiscriminated(testValue, "kind"), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsDiscriminated(actual, "kind"), outputArchive);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, ThrowWhenLoadingVariantAsDiscriminatedWithUnknownType)
{
	// Both alternatives are object types (required by the discriminated representation).
	// The source variant holds `TestPointClass`, which is not present in the target's registry.
	using SourceVariant = std::variant<TestPointClass, TestClassWithSubType<int>>;
	using TargetVariant = std::variant<TestClassWithSubType<int>>;
	SourceVariant testValue(TestPointClass(10, 20));
	TargetVariant actual;

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsDiscriminated(testValue), outputArchive);

	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::ThrowError;
	EXPECT_THROW(
		BitSerializer::LoadObject<ArchiveStub>(BitSerializer::VariantAsDiscriminated(actual), outputArchive, options),
		BitSerializer::SerializationException
	);
}

TEST(STD_Variant, SkipWhenLoadingVariantAsDiscriminatedWithUnknownType)
{
	using SourceVariant = std::variant<TestPointClass, TestClassWithSubType<int>>;
	using TargetVariant = std::variant<TestClassWithSubType<int>>;
	SourceVariant testValue(TestPointClass(10, 20));
	TargetVariant actual;

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsDiscriminated(testValue), outputArchive);

	SerializationOptions options;
	options.mismatchedTypesPolicy = MismatchedTypesPolicy::Skip;
	EXPECT_NO_THROW(BitSerializer::LoadObject<ArchiveStub>(BitSerializer::VariantAsDiscriminated(actual), outputArchive, options));
	EXPECT_EQ(0U, actual.index());
}

//-----------------------------------------------------------------------------
// Tests of serialization for VariantAsNamed with custom field names
//-----------------------------------------------------------------------------
TEST(STD_Variant, SerializeVariantAsNamedWithCustomFieldNames)
{
	using VariantType = std::variant<int, std::string, float>;
	VariantType testValue(std::string("test")), actual;

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsNamed(testValue, "kind", "payload"), outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsNamed(actual, "kind", "payload"), outputArchive);
	GTestExpectEq(testValue, actual);
}

//-----------------------------------------------------------------------------
// Tests of serialization for std::variant with default representation (SerializationOptions)
//-----------------------------------------------------------------------------
TEST(STD_Variant, SerializeVariantWithDefaultIndexedMode)
{
	using VariantType = std::variant<int, std::string, float>;
	VariantType testValue(123), actual;
	BuildFixture(testValue);

	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantWithDefaultNamedMode)
{
	using VariantType = std::variant<int, std::string, float>;
	VariantType testValue(std::string("test")), actual;

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Named;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive, options);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantWithDefaultNamedModeAndCustomKeys)
{
	using VariantType = std::variant<int, std::string, float>;
	VariantType testValue(std::string("test")), actual;

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Named;
	options.variantOptions.typeKey = "kind";
	options.variantOptions.valueKey = "payload";
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive, options);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantInVectorWithDefaultNamedMode)
{
	using VariantType = std::variant<int, std::string, float>;
	std::vector<VariantType> testValue = { VariantType(1), VariantType(std::string("test")), VariantType(2.5f) };
	std::vector<VariantType> actual(testValue.size());

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Named;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive, options);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, ExplicitVariantAsIndexedIsNotAffectedByDefaultMode)
{
	using VariantType = std::variant<int, std::string, float>;
	VariantType testValue(std::string("test")), actual;

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Named;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(VariantAsIndexed(testValue), outputArchive, options);
	BitSerializer::LoadObject<ArchiveStub>(VariantAsIndexed(actual), outputArchive, options);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantWithDefaultDiscriminatedMode)
{
	using VariantType = std::variant<TestPointClass, TestClassWithSubType<int>>;
	VariantType testValue(TestPointClass(10, 20)), actual;

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Discriminated;
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive, options);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, SerializeVariantWithDefaultDiscriminatedModeAndCustomTypeKey)
{
	using VariantType = std::variant<TestPointClass, TestClassWithSubType<int>>;
	VariantType testValue(TestClassWithSubType<int>(7)), actual;

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Discriminated;
	options.variantOptions.typeKey = "kind";
	ArchiveStub::preferred_output_type outputArchive{};
	BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive, options);
	BitSerializer::LoadObject<ArchiveStub>(actual, outputArchive, options);
	GTestExpectEq(testValue, actual);
}

TEST(STD_Variant, ThrowWhenDiscriminatedModeIsNotSupportedByAlternatives)
{
	using VariantType = std::variant<int, std::string, float>;
	VariantType testValue(123);

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Discriminated;
	ArchiveStub::preferred_output_type outputArchive{};
	const auto exception = GTestExpectException<BitSerializer::SerializationException>([&] {
		BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive, options);
	});
	EXPECT_NE(std::string(exception.what()).find("object type"), std::string::npos);
}

TEST(STD_Variant, ThrowWhenDiscriminatedModeIsNotSupportedByUnregisteredAlternatives)
{
	// Both alternatives are object types but not registered with BITSERIALIZER_REGISTER_TYPE.
	using VariantType = std::variant<TestPointClass, UnregisteredObject>;
	VariantType testValue(TestPointClass(10, 20));

	SerializationOptions options;
	options.variantOptions.mode = VariantSerializationMode::Discriminated;
	ArchiveStub::preferred_output_type outputArchive{};
	const auto exception = GTestExpectException<BitSerializer::SerializationException>([&] {
		BitSerializer::SaveObject<ArchiveStub>(testValue, outputArchive, options);
	});
	EXPECT_NE(std::string(exception.what()).find("registered"), std::string::npos);
}
