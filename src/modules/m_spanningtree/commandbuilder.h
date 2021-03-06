/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2013 Attila Molnar <attilamolnar@hush.com>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#include "utils.h"

class TreeServer;

class CmdBuilder
{
	std::string content;

 public:
	CmdBuilder(const char* cmd)
		: content(1, ':')
	{
		content.append(ServerInstance->Config->GetSID());
		push(cmd);
	}

	CmdBuilder(const std::string& src, const char* cmd)
		: content(1, ':')
	{
		content.append(src);
		push(cmd);
	}

	CmdBuilder(User* src, const char* cmd)
		: content(1, ':')
	{
		content.append(src->uuid);
		push(cmd);
	}

	CmdBuilder& push_raw(const std::string& str)
	{
		content.append(str);
		return *this;
	}

	CmdBuilder& push_raw(const char* str)
	{
		content.append(str);
		return *this;
	}

	CmdBuilder& push_raw(char c)
	{
		content.push_back(c);
		return *this;
	}

	CmdBuilder& push(const std::string& str)
	{
		content.push_back(' ');
		content.append(str);
		return *this;
	}

	CmdBuilder& push(const char* str)
	{
		content.push_back(' ');
		content.append(str);
		return *this;
	}

	CmdBuilder& push(char c)
	{
		content.push_back(' ');
		content.push_back(c);
		return *this;
	}

	template <typename T>
	CmdBuilder& push_int(T i)
	{
		content.push_back(' ');
		content.append(ConvToStr(i));
		return *this;
	}

	CmdBuilder& push_last(const std::string& str)
	{
		content.push_back(' ');
		content.push_back(':');
		content.append(str);
		return *this;
	}

	template<typename T>
	CmdBuilder& insert(const T& cont)
	{
		for (typename T::const_iterator i = cont.begin(); i != cont.end(); ++i)
			push(*i);
		return *this;
	}

	void push_back(const std::string& str) { push(str); }

	const std::string& str() const { return content; }
	operator const std::string&() const { return str(); }

	void Broadcast() const
	{
		Utils->DoOneToMany(*this);
	}

	void Forward(TreeServer* omit) const
	{
		Utils->DoOneToAllButSender(*this, omit);
	}

	bool Unicast(const std::string& target) const
	{
		return Utils->DoOneToOne(*this, target);
	}

	bool Unicast(User* target) const
	{
		return Unicast(target->server);
	}
};
