#include "../includes/commands.hpp"


void commands::PASS( void ) {
	if (!current_client->isPassGood()) {
		try {
			if (args.at(0) == pass)
				current_client->passGood();
			else {
				ServerMessage(ERR_PASSWDMISMATCH, "04 :password doesn't match\n", *current_client);
				multi_cmd.clear();
			}
		} catch (std::exception &e) {
			ServerMessage(ERR_NEEDMOREPARAMS, "PASS :need more params\n", *current_client);
		}
	} else
		ServerMessage(ERR_ALREADYREGISTERED, ":you're already registered\n", *current_client);
}

void commands::NICK( void ) {
	std:: string nick;
	if (args.size() < 1)
		ServerMessage(ERR_NONICKNAMEGIVEN, ":need to give a nick name\n", *current_client);
	else if (current_client->isPassGood()) {
		nick = args[0];
		if (checkNick(nick)) {
			if (nick.length() > 16)
				nick = nick.substr(0, 16);
			updateChannel(*current_client, current_client->getNick(), nick);
			UserMessage(cmd, " " + nick + " :" + nick + "\n", *current_client);
			nicks.erase(current_client->getNick());
			current_client->setNick(nick);
		}
	}
	else
		ServerMessage(ERR_PASSWDMISMATCH, " :need to give a password\n", *current_client);
}

void commands::USER( void ) {
	if (!current_client->isRegistered()) {
		if (current_client->isPassGood()) {
			if (RegisterUser()) {
				UserMessage("MODE", current_client->getNick() + " :+i\n", *current_client);
				current_client->setInvisibility(true);
			}
		}
		else
			ServerMessage(ERR_PASSWDMISMATCH, " :need to give password\n", *current_client);
	} else
		ServerMessage(ERR_ALREADYREGISTERED, " :you're already reistered\n", *current_client);
}

void commands::QUIT( void ) {
	client_it cli_nick;
	std::string nick = current_client->getNick();
	std::string reason;

	cli_nick = nicks.find(nick);
	if (args.size() > 0)
		reason = args[0];
	else
		reason = "";
	for (chan_it it = channels.begin(); it != channels.end(); it++) {
		if (it->second.isInChan(nick)) {
			it->second.echoToAll(*current_client, cmd, reason, true, sent);
			it->second.removeMember(*current_client);
		}
	}
	sent.clear();
	if (cli_nick != nicks.end())
		nicks.erase(cli_nick);
	current_client->set_removal(true);
	throw ErrExcIrc("Client has quit\n");
}



void commands::JOIN( void ) {
	std::vector<std::string> multi_Chan;
	std::string chanName;
	std::string pass;
	chan_it chan;

	if (isEnoughParams(1)) {
		if (args.size() > 1)
			pass = args[1];
		multi_Chan = split(args[0], ",");
		while (!multi_Chan.empty()) {
			chanName = multi_Chan.back();
			multi_Chan.pop_back();
			chan = channels.find(chanName);
			if (isChanName(chanName)) {
				if (chan == channels.end())
					channels[chanName] = Channel(chanName, *current_client);
				else
					chan->second.addMember(*current_client, pass);
			}
			else
				ServerMessage(ERR_BADCHANMASK, chanName + " :Bad Channel name\n", *current_client);
		}
	}
}

void commands::PART(void)
{
	std::string reason;
	std::vector<std::string> multi_Chan;
	std::string chanName;
	chan_it chan;

	if (isEnoughParams(1))
	{
		multi_Chan = split(args[0], ",");
		while (!multi_Chan.empty()) {
			chanName = multi_Chan.back();
			multi_Chan.pop_back();
			chan = channels.find(chanName);
			if (channelExist(chanName, chan) && userInChan(chanName, chan))
			{
				if (args.size() >= 2)
					reason = args[1];
				else
					reason = "";
				UserMessage("PART", chanName + "\n", *current_client);
				chan->second.removeMember(*current_client);
				chan->second.echoToAll(*current_client, cmd, reason, true, sent);
				sent.clear();
				if (!chan->second.getCurrentCount())
					channels.erase(chan);
			}
		}
	}
}

void commands::TOPIC(void)
{	
	if (isEnoughParams(1))
	{
		std::string chanName = args[0];
		chan_it chan = channels.find(chanName);
		if (channelExist(chanName, chan) && userInChan(chanName, chan))
		{
			if (args.size() == 2)
			{
				if (chan->second.needsOpStat() && !chan->second.isUserOp(chanName, *current_client))
					return ;
				UserMessage(cmd, chanName + " :" + args[1] + "\n", *current_client);
				chan->second.echoToAll(*current_client, cmd, ":" + args[1], true, sent);
				chan->second.Topicset(args[1]);
				sent.clear();
			}
			else
			{
				if (chan->second.hasTopic())
					ServerMessage(RPL_TOPIC, chanName + " :" + chan->second.Topicget() + "\n", *current_client);
				else
					ServerMessage(RPL_NOTOPIC, chanName + " :No topic set\n", *current_client);
			}
		}
	}
}

void commands::INVITE(void)
{
	std::string chanName;
	client_it dest;
	chan_it chan;
	if (isEnoughParams(2))
	{
		chanName = args[1];
		dest = nicks.find(args[0]);
		chan = channels.find(chanName);
		if (dest == nicks.end())
			ServerMessage(ERR_NOSUCHNICK, args[0] + " :no such nick\n", *current_client);
		else if (channelExist(chanName, chan) && userInChan(chanName, chan))
		{
			if (chan->second.isInviteOnly() && !chan->second.isUserOp(chanName, *current_client))
				return ;
			if (chan->second.isInChan(dest->second->getNick()))
				ServerMessage(ERR_USERONCHANNEL, args[0] + " " + chanName + " :User already on channel\n", *current_client);
			else
			{
				ServerMessage(RPL_INVITING, args[0] + " " + chanName + "\n", *current_client);
				killMsg(*current_client, *dest->second);
				chan->second.addInvite(args[0], *dest->second);
			}
		}
	}
}

void commands::KICK(void)
{
	std::vector<std::string> multi_nick;
	std::string chanName;
	client_it dest;
	chan_it chan;
	std::string reason;
	
	if (isEnoughParams(2))
	{
		chanName = args[0];
		multi_nick = split(args[1], ",");
		while (!multi_nick.empty()) {
			dest = nicks.find(multi_nick.back());
			multi_nick.pop_back();
			if (dest == nicks.end())
				return ;
			chan = channels.find(chanName);
			reason = chan->second.getDefKickMsg();
			if (args.size() == 3)
				reason = args[2];
			if (channelExist(chanName, chan) && userInChan(chanName, chan))
			{
				if (!chan->second.isInChan(dest->second->getNick()))
					ServerMessage(ERR_USERNOTINCHANNEL, dest->second->getNick() + " " + chanName + " :User is not on channel\n", *current_client);
				else if (chan->second.isUserOp(chanName, *current_client))
				{
					UserMessage(cmd, chanName + " " + dest->second->getNick() + " " + reason + "\n", *current_client);
					chan->second.echoToAll(*current_client, cmd, dest->second->getNick() + " " + reason, true, sent);
					chan->second.removeMember(*dest->second);
					sent.clear();
					if (!chan->second.getCurrentCount())
						channels.erase(chan);
				}
			}
		}
	}
}



void commands::MODE( void ) {
	std::string name;
	std::string modes;
	chan_it chan;

	if (isEnoughParams(1))
	{
		name = args[0];
		if (args.size() > 1)
			modes = args[1];
		if (!isChanName(args[0])) {
			userMode(modes);
		} else {
			chan = channels.find(name);
			if (channelExist(name, chan) && userInChan(name, chan))
			{
				if (args.size() == 1)
					ServerMessage(RPL_CHANNELMODEIS, name + " " + chan->second.getModes() + "\n", *current_client);
				else if (chan->second.isUserOp(name, *current_client))
					channelMode(modes, chan);
			}
		}
	}

}


void commands::PRIVMSG( void ) {
	std::vector<std::string> multi_dest;
	std::string dest;
	chan_it chan;
	client_it client;
	
	if (isEnoughParams(2)) {
		multi_dest = split(args[0], ",");
		while (!multi_dest.empty()) {
			dest = multi_dest.back();
			multi_dest.pop_back();
			if (isChanName(dest)) {
				chan = channels.find(dest);
				if (chan != channels.end())
					chan->second.echoToAll(*current_client, cmd, args[1], true, sent);
				else
					ServerMessage(ERR_NOSUCHNICK, args[0] + " :no such nick/channel\n", *current_client);
				sent.clear();
			} else {
				client = nicks.find(dest);
				if (client != nicks.end())
					UsertoUser(*current_client, *client->second, cmd, args[1] + "\n");
				else
					ServerMessage(ERR_NOSUCHNICK, ":" + dest + "\n", *current_client);
			}
		}
	} 
}

void commands::CAP( void ) {
	std::string mes = "CAP * LS :multi-prefix userhost-in-names\n";
	if (isEnoughParams(1)) {
		if (args[0] == "LS")
			current_client->pushSendBuff(mes);
		if (args[0] == "REQ") {
			mes = "CAP * ACK " + args[1] + "\n";
			current_client->pushSendBuff(mes);
		}
	}
}
