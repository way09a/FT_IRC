#include "../includes/Messages.hpp"
#include "../includes/ErrorCodes.hpp"

void ConnectionMessage( Client &client) {
	ServerMessage(RPL_WELCOME, " :Welcome You are now known as " + USER_FN(client.getNick(), client.getUserName(), client.getHostName()) + "\n", client);
}

void ServerMessage(std::string error, std::string message, Client &client) {
	std::string mes = ":" + client.getServerName() + error + client.getNick() + " " + message;
	client.pushSendBuff(mes);
}

void UserMessage(std::string cmd, std::string message, Client &client) {
	std::string mes = ":" + USER_FN(client.getNick(), client.getUserName(), client.getHostName()) + " " + cmd + " " + message;
	client.pushSendBuff(mes);
}

void UsertoUser(Client &origin, Client &dest, std::string cmd, std::string mess) {
	std::string message = ":" + USER_FN(origin.getNick(), origin.getUserName(), origin.getHostName());
	message += " " + cmd + " " + origin.getNick() + " ";
	message += ":";
	message += mess + "\n";
	dest.pushSendBuff(message);
}