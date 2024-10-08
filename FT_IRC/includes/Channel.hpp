#pragma once

#include <map>
#include <string>
#include <sstream>
#include "Client.hpp"
#include "Messages.hpp"
#include "ErrorCodes.hpp"

class Channel
{
	private:
		typedef std::map<std::string, Client *>::iterator iter;
		std::map<std::string, Client *> members;
		std::map<std::string, Client *> invited;
		std::map<std::string, Client *> operators;
		std::string		name;
		std::string 	pass;
		std::string		topic;
		size_t			limit;
		bool			inviteOnly;
		bool			hasLimit;
		bool			hasKey;
		bool			needOpStat;
		std::string		defKickMsg;
		unsigned int	currentCount;
		std::string		modes;

	public:
		Channel();
		Channel( std::string, Client & );
		~Channel();

		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Setters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		void Topicset(std::string topic);
		void ChannelOpSet (Client& target);
		void InvOnlyset( bool invite );
		void Modeset(char mode);

		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Getters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		std::string const Topicget(void) const;
		std::string const getDefKickMsg(void) const;
		std::string const getModes(void) const;
		unsigned int getCurrentCount(void) const;

		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Booleans ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		bool isInviteOnly( void );
		bool hasTopic(void);
		bool isInChan( std::string );
		bool isInvited( std::string );
		bool checkEntrance( std::string nick, Client &client, std::string key );
		bool isUserOp(std::string chanName, Client &user);
		bool needsOpStat(void);

		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		std::string echoToAll(Client &client, std::string cmd, std::string trailng, bool chan, std::map<std::string, Client *>&sent );
		void addMember( Client &add , std::string key);
		void makeChanOp( Client &, Client & );
		void unsetChanOp(Client &src, Client &dst);
		void removeMember( Client & );
		void updateMemberNick( Client &, std::string, std::string );
		void whoIsChan( Client & );
		void addInvite( std::string nick, Client &client );
		void removeInvite( std::string nick );
		void removeModes(char mode);
		void chanModes(char mode, char sign, devector<std::string> &arguments, Client &current, std::string &modes, std::string &trailing);
		void modeI(char sign, Client &current, std::string &modes);
		void modeO(char sign, devector<std::string> &args, Client &current, std::string &modes, std::string &trailing);
		void modeK(char sign, devector<std::string> &args, Client &current, std::string &modes, std::string &trailing);
		void modeL(char sign, devector<std::string> &args, Client &current, std::string &modes, std::string &trailing);
		void modeT(char sign, Client &current_client, std::string &modes);

};
