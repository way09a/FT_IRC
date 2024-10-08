#include "../includes/Channel.hpp"

Channel::Channel() {
}

Channel::Channel( std::string name, Client &creator ) {
    this->name = name;
    if (name.length() > 50)
        this->name = name.substr(0, 50);
    this->topic = "";
    this->limit = 0;
    this->inviteOnly = false;
    this->hasLimit = false;
    this->hasKey = false;
    this->needOpStat = true;
    this->defKickMsg = "Bye, miss you";
    this->currentCount = 1;
    this->modes = "+";
    this->pass = "";
    operators[creator.getNick()] = &creator;
    UserMessage("JOIN", name + " * :welcome\n", creator);
    whoIsChan(creator);
}

Channel::~Channel() {

}


void Channel::ChannelOpSet (Client &target)
{
    operators[target.getNick()] = &target;
}

void Channel::InvOnlyset( bool invite ) {
    this->inviteOnly = invite;
}

void Channel::Topicset(std::string topic)
{
    this->topic = topic;
    return ;
}

void Channel::Modeset(char mode) {
    size_t pos = modes.find(mode);
    if (pos == modes.npos)
        this->modes += mode;
}


std::string const Channel::Topicget(void) const
{
    return this->topic;
}

std::string const Channel::getDefKickMsg(void) const
{
    return this->defKickMsg;
}

std::string const Channel::getModes(void) const
{
    return this->modes;
}

unsigned int Channel::getCurrentCount(void) const
{
    return this->currentCount;
}


bool Channel::isInviteOnly( void ) {
    return inviteOnly;
}

bool Channel::hasTopic(void)
{
    if (!this->topic.empty())
        return true;
    return false;
}

bool Channel::isInChan( std::string Nick ) {
    iter member;
    iter oper;

    oper = operators.find(Nick);
    member = members.find(Nick);
    if (member == members.end() && oper == operators.end())
        return false;
    return true;
}

bool Channel::isInvited( std::string Nick ) {
    iter invite;

    invite = invited.find(Nick);
    if (invite == invited.end())
        return false;
    return true;
}

bool Channel::checkEntrance( std::string nick, Client &client, std::string key ) {
    if (hasLimit) {
        if (currentCount >= limit) {
            ServerMessage(ERR_CHANNELISFULL, name + " :Cannot join channel (+l) - channel is full, try again later\n", client);
            return false;
        }
    }
    if (inviteOnly) {
        if (!isInvited(nick)) {
            ServerMessage(ERR_INVITEONLYCHAN, name + " :Cannot join channel (+i) - you must be invited\n", client);
            return false;
        }
    }
    if (hasKey) {
        if (key != pass) {
            ServerMessage(ERR_BADCHANNELKEY, name + " :Cannot join channel (+k) - bad key\n", client);
            return false;
        }
    }
    return true;
}

bool Channel::isUserOp(std::string chanName, Client& user)
{
	if (operators.find(user.getNick()) == operators.end())
	{
		ServerMessage(ERR_CHANOPRIVSNEEDED, chanName + " :You're not a channel operator\n", user);
		return false;
	}
	return true;
}

bool Channel::needsOpStat(void)
{
    if (this->needOpStat)
        return true;
    return false;
}


std::string Channel::echoToAll(Client &client, std::string cmd, std::string trailing, bool chan, std::map<std::string, Client *> &sent) {
    std::string client_info = USER_FN(client.getNick(), client.getUserName(), client.getHostName());
    std::string message = ":" + client_info + " " + cmd;

    if (chan)
        message += " " + name;
    if (!trailing.empty())
        message += " " + trailing;
    message += "\n";
    for (iter it = members.begin(); it != members.end(); it++) {
        if (it->second->getNick() != client.getNick() && sent.find(it->second->getNick()) == sent.end()) {
            it->second->pushSendBuff(message);
            sent[it->second->getNick()] = it->second;
        }
    }
    for (iter it = operators.begin(); it != operators.end(); it++) {
        if (it->second->getNick() != client.getNick() && sent.find(it->second->getNick()) == sent.end()) {
            it->second->pushSendBuff(message);
            sent[it->second->getNick()] = it->second;
        }
    }
    return message;
}

void Channel::addMember( Client &add, std::string key ) {
    std::map<std::string, Client *> sent;

    if (!isInChan(add.getNick()) && checkEntrance(add.getNick(), add, key)) {
        members[add.getNick()] = &add;
        UserMessage("JOIN", name + " * :welcome\n", add);
        if (hasTopic())
            ServerMessage(RPL_TOPIC, name + " :" + topic + "\n", add);
        whoIsChan(add);
        echoToAll(add, "JOIN", "", true, sent);
        removeInvite(add.getNick());
        this->currentCount++;
    }
}

void Channel::makeChanOp( Client &src, Client &dst )
{
    iter member = members.find(dst.getNick());
    iter oper = operators.find(dst.getNick());

    if (isUserOp(this->name, src))
    {
        if (isInChan(dst.getNick()))
        {
            if (member != members.end())
                members.erase(member);
            if (oper == operators.end())
                operators[dst.getNick()] = &dst;
        }
        else
            ServerMessage(ERR_USERNOTINCHANNEL, dst.getNick() + " " + name + " :Not on channel\n", src);
    }
}

void Channel::unsetChanOp(Client &src, Client &dst)
{
    iter oper = operators.find(dst.getNick());

    if (isUserOp(this->name, src))
    {
        if (isInChan(dst.getNick()))
        {
            if (oper != operators.end())
                operators.erase(oper);
            members[dst.getNick()] = &dst;
        }
        else
            ServerMessage(ERR_USERNOTINCHANNEL, dst.getNick() + " " + name + " :Not on channel\n", src);
    }
}

void Channel::removeMember( Client & remove) {
    iter member = members.find(remove.getNick());
    iter oper = operators.find(remove.getNick());

    if (member != members.end())
    {
        members.erase(member);
        this->currentCount--;
    }
    if (oper != operators.end())
    {
        operators.erase(oper);
        this->currentCount--;
    }
}

void Channel::updateMemberNick( Client &client, std::string old_nick, std::string new_nick ) {
    iter mems = members.find(old_nick);
    iter oper = operators.find(old_nick);
    iter invite = invited.find(old_nick);

    if (mems != members.end()) {
        members.erase(old_nick);
        members[new_nick] = &client;
    }
    if (oper != operators.end()) {
        operators.erase(old_nick);
        operators[new_nick] = &client;
    }
    if (invite != invited.end()) {
        invited.erase(old_nick);
        invited[new_nick] = &client;
    }
}

void Channel::whoIsChan( Client &client ) {
    std::string who = "@ " + name + " :";
    for (iter it = members.begin(); it != members.end(); it++) {
            who += it->second->getNick() + " ";
    }
    for (iter it = operators.begin(); it != operators.end(); it++) {
            who +=  "@" + it->second->getNick() + " ";
    }
    who += "\n";
    ServerMessage(RPL_NAMREPLY, who, client);
    ServerMessage(RPL_ENDOFNAMES, name + " :END of /NAMES list\n", client);
}

void Channel::addInvite( std::string nick, Client &client ) {
    invited[nick] = &client;
}

void Channel::removeInvite( std::string nick ) {
    iter invite;

    invite = invited.find(nick);
    if (invite != invited.end())
        invited.erase(invite);
}

void Channel::removeModes(char mode) {
    size_t pos = modes.find(mode);
    if (pos != modes.npos)
        modes.erase(pos, 1);
}

void Channel::chanModes(char mode, char sign, devector<std::string> &arguments, Client &current_client, std::string &modes, std::string &trailing)
{
    if (mode == 'i')
        modeI(sign, current_client, modes);
    if (mode == 'o')
        modeO(sign, arguments, current_client, modes, trailing);
    if (mode == 'k')
        modeK(sign, arguments, current_client, modes, trailing);
    if (mode == 'l')
        modeL(sign, arguments, current_client, modes, trailing);
    if (mode == 't')
        modeT(sign, current_client, modes);
}

void Channel::modeI(char sign, Client &current_client, std::string &modes)
{
    if (isUserOp(this->name, current_client))
    {
        if (sign == '+')
        {
            inviteOnly = true;
            Modeset('i');
        }
        else
        {
            inviteOnly = false;
            removeModes('i');
        }
        if (modes.find('i', modes.find(sign)) == modes.npos)
            modes += "i";
    }
}

void Channel::modeO(char sign, devector<std::string> &args, Client &current_client, std::string &modes, std::string &trailing)
{
    if (isUserOp(this->name, current_client))
    {
        if (!args.size())
            return ;
        iter member = members.find(args[0]);
        iter oper = operators.find(args[0]);
        std::string passed = args[0];
        args.pop_front();
        if (member == members.end() && oper == operators.end())
            ServerMessage(ERR_NOSUCHNICK, passed + " :No such nick/channel\n", current_client);
        else
        {
            if (sign == '+' && member != members.end())
                makeChanOp(current_client, *member->second);
            else if (sign == '-' && oper != operators.end())
                unsetChanOp(current_client, *oper->second);
            trailing += " " + passed;
            if (modes.find('o', modes.find(sign)) == modes.npos)
                modes += "o";
        }
    }
}

void Channel::modeK(char sign, devector<std::string> &args, Client &current_client, std::string &modes, std::string &trailing)
{
    std::string save;
    if (isUserOp(this->name, current_client))
    {
        if (args.size() && sign == '+')
        {
            this->pass = args[0];
            save = args[0];
            args.pop_front();
            this->hasKey = true;
            Modeset('k');
        }
        else
        {
            this->hasKey = false;
            removeModes('k');
        }
        if (modes.find('k', modes.find(sign)) == modes.npos) {
            trailing += " " + save;
            modes += "k";
        }
    }
}

void Channel::modeL(char sign, devector<std::string> &args, Client &current_client, std::string &modes, std::string &trailing)
{
    std::stringstream conv;
    int limit;
    std::string l_str;

    if (isUserOp(this->name, current_client))
    {
        if (args.size() && sign == '+')
        {
            conv << args[0];
            l_str = args[0];
            args.pop_front();
            if (!(conv >> limit))
                return ;
            if (limit <= (int)this->currentCount)
                return ;
            this->limit = limit;
            this->hasLimit = true;
            Modeset('l');
            trailing += " " + l_str;
            if (modes.find('l', modes.find(sign)) == modes.npos)
                modes += "l";
        }
        else
        {
            this->hasLimit = false;
            removeModes('l');
            if (modes.find('l', modes.find(sign)) == modes.npos)
                modes += "l";
        }
    }
}

void Channel::modeT(char sign, Client &current_client, std::string &modes)
{
    if (isUserOp(this->name, current_client))
    {
        if (sign == '+')
        {
            this->needOpStat = true;
            Modeset('t');
        }
        else
        {
            this->needOpStat = false;
            removeModes('t');
        }
        if (modes.find('t', modes.find(sign)) == modes.npos)
            modes += "t";
    }
}
