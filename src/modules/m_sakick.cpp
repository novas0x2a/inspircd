/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009 John Brooks <john.brooks@dereferenced.net>
 *   Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
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

/** Handle /SAKICK
 */
class CommandSakick : public Command
{
 public:
	CommandSakick(Module* Creator) : Command(Creator,"SAKICK", 2, 3)
	{
		flags_needed = 'o'; Penalty = 0; syntax = "<channel> <nick> [reason]";
		TRANSLATE3(TR_TEXT, TR_NICK, TR_TEXT);
	}

	CmdResult Handle (const std::vector<std::string>& parameters, User *user)
	{
		User* dest = ServerInstance->FindNick(parameters[1]);
		Channel* channel = ServerInstance->FindChan(parameters[0]);
		const char* reason = "";

		if ((dest) && (dest->registered == REG_ALL) && (channel))
		{
			if (parameters.size() > 2)
			{
				reason = parameters[2].c_str();
			}
			else
			{
				reason = dest->nick.c_str();
			}

			if (ServerInstance->ULine(dest->server))
			{
				user->WriteNumeric(ERR_NOPRIVILEGES, "%s :Cannot use an SA command on a u-lined client", user->nick.c_str());
				return CMD_FAILURE;
			}

			/* For local clients, directly kick them. For remote clients,
			 * just return CMD_SUCCESS knowing the protocol module will route the SAKICK to the user's
			 * local server and that will kick them instead.
			 */
			if (IS_LOCAL(dest))
			{
				channel->KickUser(ServerInstance->FakeClient, dest, reason);

				Channel *n = ServerInstance->FindChan(parameters[1]);
				if (n && n->HasUser(dest))
				{
					/* Sort-of-bug: If the command was issued remotely, this message won't be sent */
					user->WriteNotice("*** Unable to kick " + dest->nick + " from " + parameters[0]);
					return CMD_FAILURE;
				}
			}

			if (IS_LOCAL(user))
			{
				/* Locally issued command; send the snomasks */
				ServerInstance->SNO->WriteGlobalSno('a', user->nick + " SAKICKed " + dest->nick + " on " + parameters[0]);
			}

			return CMD_SUCCESS;
		}
		else
		{
			user->WriteNotice("*** Invalid nickname or channel");
		}

		return CMD_FAILURE;
	}

	RouteDescriptor GetRouting(User* user, const std::vector<std::string>& parameters)
	{
		User* dest = ServerInstance->FindNick(parameters[1]);
		if (dest)
			return ROUTE_OPT_UCAST(dest->server);
		return ROUTE_LOCALONLY;
	}
};

class ModuleSakick : public Module
{
	CommandSakick cmd;
 public:
	ModuleSakick()
		: cmd(this)
	{
	}

	void init() CXX11_OVERRIDE
	{
		ServerInstance->Modules->AddService(cmd);
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Provides a SAKICK command", VF_OPTCOMMON|VF_VENDOR);
	}
};

MODULE_INIT(ModuleSakick)
