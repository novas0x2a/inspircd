/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2008 Craig Edwards <craigedwards@brainbox.cc>
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


#include "inspircd.h"
#include "utils.h"
#include "treeserver.h"
#include "protocolinterface.h"
#include "commands.h"

/*
 * For documentation on this class, see include/protocol.h.
 */

void SpanningTreeProtocolInterface::GetServerList(ServerList& sl)
{
	for (server_hash::iterator i = Utils->serverlist.begin(); i != Utils->serverlist.end(); i++)
	{
		ServerInfo ps;
		ps.servername = i->second->GetName();
		TreeServer* s = i->second->GetParent();
		ps.parentname = s ? s->GetName() : "";
		ps.usercount = i->second->UserCount;
		ps.opercount = i->second->OperCount;
		ps.gecos = i->second->GetDesc();
		ps.latencyms = i->second->rtt;
		sl.push_back(ps);
	}
}

bool SpanningTreeProtocolInterface::SendEncapsulatedData(const parameterlist &encap)
{
	CmdBuilder params("ENCAP");
	params.insert(encap);
	if (encap[0].find_first_of("*?") != std::string::npos)
	{
		params.Broadcast();
		return true;
	}
	return params.Unicast(encap[0]);
}

void SpanningTreeProtocolInterface::SendMetaData(Extensible* target, const std::string &key, const std::string &data)
{
	User* u = dynamic_cast<User*>(target);
	Channel* c = dynamic_cast<Channel*>(target);
	if (u)
		CommandMetadata::Builder(u, key, data).Broadcast();
	else if (c)
		CommandMetadata::Builder(c, key, data).Broadcast();
	else
		CommandMetadata::Builder(key, data).Broadcast();
}

void SpanningTreeProtocolInterface::SendTopic(Channel* channel, std::string &topic)
{
	CommandFTopic::Builder(ServerInstance->FakeClient, channel).Broadcast();
}

void SpanningTreeProtocolInterface::SendMode(User* source, User* u, Channel* c, const std::vector<std::string>& modedata, const std::vector<TranslateType>& translate)
{
	if (u)
	{
		if (u->registered != REG_ALL)
			return;

		CmdBuilder params(source, "MODE");
		params.push_back(u->uuid);
		params.insert(modedata);
		params.Broadcast();
	}
	else
	{
		CmdBuilder params(source, "FMODE");
		params.push_back(c->name);
		params.push_back(ConvToStr(c->age));
		params.push_back(CommandParser::TranslateUIDs(translate, modedata));
		params.Broadcast();
	}
}

void SpanningTreeProtocolInterface::SendSNONotice(const std::string &snomask, const std::string &text)
{
	CmdBuilder("SNONOTICE").push(snomask).push_last(text).Broadcast();
}

void SpanningTreeProtocolInterface::PushToClient(User* target, const std::string &rawline)
{
	CmdBuilder("PUSH").push(target->uuid).push_last(rawline).Unicast(target);
}

void SpanningTreeProtocolInterface::SendMessage(Channel* target, char status, const std::string& text, MessageType msgtype)
{
	const char* cmd = (msgtype == MSG_PRIVMSG ? "PRIVMSG" : "NOTICE");
	CUList exempt_list;
	Utils->SendChannelMessage(ServerInstance->Config->GetSID(), target, text, status, exempt_list, cmd);
}

void SpanningTreeProtocolInterface::SendMessage(User* target, const std::string& text, MessageType msgtype)
{
	CmdBuilder p(msgtype == MSG_PRIVMSG ? "PRIVMSG" : "NOTICE");
	p.push_back(target->uuid);
	p.push_last(text);
	p.Unicast(target);
}
