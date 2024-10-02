#pragma once

#include <exception>

class ErrExcIrc : public std::exception {
	private:
		const char *what_arg;
	public:
		ErrExcIrc(const char *what_arg);
		const char * what ( ) const throw ();
};