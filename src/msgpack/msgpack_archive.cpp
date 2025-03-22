/*******************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#include <memory>
#include "msgpack_readers.h"
#include "msgpack_writers.h"
#include "bitserializer/msgpack_archive.h"


namespace BitSerializer::MsgPack::Detail
{
	MsgPackWriteRootScope::MsgPackWriteRootScope(std::string& outputData, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mMsgPackWriter(std::make_unique<CMsgPackStringWriter>(outputData).release())
	{ }

	MsgPackWriteRootScope::MsgPackWriteRootScope(std::ostream& outputStream, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Save>(serializationContext)
		, mMsgPackWriter(std::make_unique<CMsgPackStreamWriter>(outputStream).release())
	{ }

	MsgPackWriteRootScope::~MsgPackWriteRootScope()
	{
		delete mMsgPackWriter;
	}

	MsgPackReadRootScope::MsgPackReadRootScope(std::string_view inputData, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mMsgPackReader(std::make_unique<CMsgPackStringReader>(inputData, serializationContext.GetOptions()).release())
	{ }

	MsgPackReadRootScope::MsgPackReadRootScope(std::istream& inputStream, SerializationContext& serializationContext)
		: TArchiveScope<SerializeMode::Load>(serializationContext)
		, mMsgPackReader(std::make_unique<CMsgPackStreamReader>(inputStream, serializationContext.GetOptions()).release())
	{ }

	MsgPackReadRootScope::~MsgPackReadRootScope()
	{
		delete mMsgPackReader;
	}
}
