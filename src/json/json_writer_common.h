/*******************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
*******************************************************************************/
#pragma once
#include <string>
#include <string_view>

namespace BitSerializer::Json::Detail
{
	inline void WriteString(std::string_view source, std::string& target)
	{
		size_t extra = 0;
		bool has_escapes = false;

		for (char c : source)
		{
			unsigned char uc = static_cast<unsigned char>(c);
			if (uc == '"' || uc == '\\')
			{
				extra += 1;
				has_escapes = true;
			}
			else if (uc <= 0x1F)
			{
				has_escapes = true;
				switch (uc) {
				case '\b': case '\f': case '\n': case '\r': case '\t':
					extra += 1;
					break;
				default:
					extra += 5;
					break;
				}
			}
		}

		const size_t orig_size = target.size();
		target.reserve(orig_size + 2 + source.size() + extra);

		target.push_back('"');

		if (!has_escapes) {
			target.append(source);
			target.push_back('"');
			return;
		}

		static constexpr char hex[] = "0123456789ABCDEF";
		for (char c : source)
		{
			unsigned char uc = static_cast<unsigned char>(c);
			switch (uc) {
			case '"':  target.append("\\\""); break;
			case '\\': target.append("\\\\"); break;
			case '\b': target.append("\\b"); break;
			case '\f': target.append("\\f"); break;
			case '\n': target.append("\\n"); break;
			case '\r': target.append("\\r"); break;
			case '\t': target.append("\\t"); break;
			default:
				if (uc <= 0x1F) {
					char buf[7] = { '\\', 'u', '0', '0', hex[uc >> 4], hex[uc & 0x0F], '\0' };
					target.append(buf, 6);
				}
				else {
					target.push_back(c);
				}
				break;
			}
		}

		target.push_back('"');
	}
}
