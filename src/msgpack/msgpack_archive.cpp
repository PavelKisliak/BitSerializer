/*******************************************************************************
* Copyright (C) 2018-2023 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include "msgpack_readers.h"
#include "msgpack_writers.h"
#include "bitserializer/msgpack_archive.h"


namespace BitSerializer::MsgPack::Detail
{
	MsgPackWriteRootScope::MsgPackWriteRootScope(std::string& inputData, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mMsgPackWriter(std::make_unique<CMsgPackStringWriter>(inputData))
	{ }

	//MsgPackWriteRootScope::MsgPackWriteRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
	//	: TArchiveScope<SerializeMode::Save>(serializationContext)
	//	, mMsgPackWriter(std::make_unique<CMsgPackStreamWriter>(outputStream, true, serializationContext.GetOptions().valuesSeparator, serializationContext.GetOptions().streamOptions))
	//{ }

	MsgPackReadRootScope::MsgPackReadRootScope(std::string_view inputData, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mMsgPackReader(std::make_unique<CMsgPackStringReader>(inputData, serializationContext.GetOptions().overflowNumberPolicy, serializationContext.GetOptions().mismatchedTypesPolicy))
	{ }

	//MsgPackReadRootScope::MsgPackReadRootScope(std::istream& encodedInputStream, SerializationContext& serializationContext)
	//	: TArchiveScope<SerializeMode::Load>(serializationContext)
	//	, mMsgPackReader(std::make_unique<CMsgPackStreamReader>(encodedInputStream, true, serializationContext.GetOptions().valuesSeparator))
	//{ }
}
