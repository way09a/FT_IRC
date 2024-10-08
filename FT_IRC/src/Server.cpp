#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/ErrExcIrc.hpp"

Server::Server()
{
	this->sockfd = -1;
	this->new_client = -1;
	this->fd_size = 5;
	this->fd_count = 1;
	this->nbytes = 0;
	this->sender_fd = 0;
	this->port = "6667";
	this->pfds = new struct pollfd[this->fd_size];
}

void	Server::setPort(char *port)
{
	std::stringstream conv;
	int iport;

	conv << port;
	conv >> iport;
	if (iport >= 0 && iport <= 1023)
		throw ErrExcIrc("ports 0 - 1023 are reserved ports! Please use a different port.");
	this->port = port;
	return ;
}

void	Server::getAddrInfo(void)
{
	struct addrinfo	hints;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;
	if ((getaddrinfo(NULL, &this->port[0], &hints, &this->res)) != 0)
		throw ErrExcIrc("Error with getaddrinfo");
	return ;
}

void	Server::ftSocket(void)
{
	this->sockfd = socket(this->res->ai_family, this->res->ai_socktype,
		this->res->ai_protocol);
	if (this->sockfd < 0)
		throw ErrExcIrc("Error with socket");
	return ;
}

void	Server::ftFcntl(void)
{
	if ((fcntl(this->sockfd, F_SETFL, O_NONBLOCK)) < 0)
		throw ErrExcIrc("Error with fcntl");
	return ;
}

void	Server::ftSetSockOpt(void)
{
	int	yes = 1;

	if ((setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
		< 0)
		throw ErrExcIrc("Error with setsockopt");
	return ;
}

void	Server::ftBind(void)
{
	if ((bind(this->sockfd, this->res->ai_addr, this->res->ai_addrlen)) < 0)
		throw ErrExcIrc("Error with bind");
	return ;
}

void	Server::ftListen(void)
{
	if ((listen(this->sockfd, 10)) < 0)
		throw ErrExcIrc("Error with listen");
	return ;
}

void	Server::ftPoll(void)
{
	this->poll_count = poll(this->pfds, this->fd_count, -1);
	if (this->poll_count < 0)
	{
		std::cout << "Error with poll" << std::endl;
		return ;
	}
	return ;
}

void	Server::addNewClient(void)
{
	struct sockaddr_in	new_client_addr;
	socklen_t			new_client_addr_size;

	new_client_addr_size = sizeof(new_client_addr);
	this->new_client = accept(this->sockfd, (struct sockaddr *)&new_client_addr,
		&new_client_addr_size);
	if (this->new_client < 0)
	{
		std::cout << "Error with accept" << std::endl;
		return ;
	}
	std::cout << "Client IP: " << inet_ntoa(new_client_addr.sin_addr) << std::endl;
	std::cout << "Client fd: " << this->new_client << std::endl;
	this->clients[this->new_client] = Client(this->new_client, inet_ntoa(new_client_addr.sin_addr));
	this->resizePfds();
	return ;
}

void	Server::resizePfds(void)
{
	struct pollfd	*temp;
	if (this->fd_count == this->fd_size)
	{
		this->fd_size++;
		temp = new struct pollfd[this->fd_size];
		for (int i = 0; i < this->fd_size - 1; i++)
		{
			temp[i].fd = this->pfds[i].fd;
			temp[i].events = this->pfds[i].events;
		}
		delete [] this->pfds;
		this->pfds = temp;
	}
	this->pfds[this->fd_count].fd = this->new_client;
	this->pfds[this->fd_count].events = POLLIN | POLLOUT;
	this->fd_count += 1;
	return ;
}

void	Server::ftRecv(int index)
{

	this->nbytes = recv(this->pfds[index].fd, this->buf, 255, 0);
	this->sender_fd = this->pfds[index].fd;
	return ;
}

void	Server::checkNBytes(int index)
{
	if (this->nbytes < 0)
		return ;
	if (this->nbytes == 0)
	{
		std::cout << "Client " << this->sender_fd << " disconnected" << std::endl;
		this->removeClient(index);
	}
	if (this->nbytes > 0 && index != 0)
		this->ftSend();
	return ;
}

void	Server::removeClient(int index)
{
	std::cout << "Client " << this->pfds[index].fd << " deleted" << std::endl;
	parser.removeClient(this->clients[this->pfds[index].fd].getNick());
	this->clients[this->pfds[index].fd].getBuff().clear();
	clients.erase(this->pfds[index].fd);
	close(this->pfds[index].fd);
	this->pfds[index].fd = -1;
	return ;
}

void	Server::ftSend()
{
	std::string input;

	this->clients[this->sender_fd].getBuff() += buf;
	input = this->clients[this->sender_fd].getBuff();
	if (input.find("\n") != std::string::npos) {
		this->parser.takeInput(input, this->sender_fd, this->clients[this->sender_fd]);
		this->clients[this->sender_fd].getBuff().clear();
	}
	std::memset(this->buf, 0, 256);
	return ;
}

void	Server::sendToClient(Client &client, int index) {
	devector<std::string> &messages = client.getSendBuff();
	std::string sending;
	
	while (!messages.empty()) {
		sending = messages.front();
		messages.pop_front();
		if (send(client.getFD(), sending.data(), sending.length(), 0) < 0)
			std::cout << "send problems" << std::endl;	
	}
	if (client.remove_me())
		this->removeClient(index);
}

void	Server::ftIRC(void)
{
	this->getAddrInfo();
	this->ftSocket();
	this->ftSetSockOpt();
	this->ftFcntl();
	this->ftBind();
	this->ftListen();
	this->pfds[0].fd = this->sockfd;
	this->pfds[0].events = POLLIN;
	std::memset(this->buf, 0, 256);
	while(true)
	{
		this->ftPoll();
		if (this->pfds[0].revents & POLLIN)
			this->addNewClient();
		for (int i = 1; i < this->fd_count; i++)
		{
			if (this->pfds[i].revents & POLLIN)
			{
				this->ftRecv(i);
				this->checkNBytes(i);
			}
			if (this->pfds[i].revents & POLLOUT)
				this->sendToClient(this->clients[this->pfds[i].fd], i);
			if (this->pfds[i].revents == POLLHUP)
				this->removeClient(i);
		}
	}
}

Server::~Server()
{
	delete [] this->pfds;
}