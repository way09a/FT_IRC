#pragma once

#include "OvAllFun.hpp"

class commands : public Functions {
    public:
		
		void CAP( void );
		void PASS( void );
        void NICK( void );
		void USER( void );
		void QUIT( void );

		void JOIN( void );
		void PART( void );
		void TOPIC(void);
		void INVITE(void);
		void KICK(void);

		
		void MODE( void );

		
		void PRIVMSG( void );
};