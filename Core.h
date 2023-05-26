#ifndef CORE_H
#define CORE_H

#include <SDL_net.h>

namespace Core
{
	inline TCPsocket server{};
	inline SDLNet_SocketSet set{};
	inline IPaddress ip{};
	inline bool running{};

	void init();
	void loop();
	void quit();
}

#endif