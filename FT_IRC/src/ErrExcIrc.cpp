#include "../includes/ErrExcIrc.hpp"

ErrExcIrc::ErrExcIrc(const char *what_arg) : what_arg(what_arg) {}

const char * ErrExcIrc::what ( ) const throw () {
	return what_arg;
}