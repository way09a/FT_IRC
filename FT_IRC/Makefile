NAME = ircserv

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = src/main.cpp src/Client.cpp src/Parser.cpp src/OvAllFun.cpp src/Server.cpp src/ErrExcIrc.cpp src/Channel.cpp \
		src/Messages.cpp src/commands.cpp

OBJS = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all