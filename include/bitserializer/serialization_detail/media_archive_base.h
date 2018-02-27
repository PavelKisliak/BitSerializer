/*******************************************************************************
* Copyright (C) 2018 by Pavel Kisliak                                          *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once

namespace BitSerializer {

enum class SerializeState
{
	Idle,
	Save,
	Load
};

class MediaArchiveBase
{
public:
	MediaArchiveBase() = default;
	MediaArchiveBase(SerializeState state) : mSerializeState(state) {}

	inline SerializeState GetState() const noexcept	{ return mSerializeState; }
	inline bool IsSaving() const noexcept			{ return mSerializeState == SerializeState::Save; }
	inline bool IsLoading() const noexcept			{ return mSerializeState == SerializeState::Load; }

protected:
	SerializeState mSerializeState;
};

}	// namespace BitSerializer
