#include <stack>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include <SDL_net.h>

#include "Core.h"

#define EXIT { SDLNet_TCP_DelSocket(set, client); SDLNet_TCP_Close(client); std::cout << "Wrong request\n"; return; }

std::vector<std::string> split(std::string line, char delim, const std::vector<char>& exclude = {})
{
	std::vector<std::string> words(1);
	for (const char& c : line)
		if (c == delim) words.push_back("");
		else if (std::find(exclude.begin(), exclude.end(), c) == exclude.end())
			words.back().push_back(c);
	return words;
}

void printIP(IPaddress* ip)
{
	unsigned int ipData{ SDL_Swap32(ip->host) };
	std::cout << ((ipData >> 24) & 0xff) << '.';
	std::cout << ((ipData >> 16) & 0xff) << '.';
	std::cout << ((ipData >> 8) & 0xff) << '.';
	std::cout << (ipData & 0xff) << '\n';
}

std::vector<char> rdFile(const std::string& path)
{
	std::ifstream in{ "./src" + (path == "/" ? "/index.html" : path), std::ios::binary };
	std::vector<char> content{};
	do content.push_back(0);
	while (in.read(&content.back(), 1));
	if (!content.empty()) content.pop_back();
	return content;
}

void Core::init()
{
	SDLNet_Init();

	SDLNet_ResolveHost(&ip, NULL, 80);
	server = SDLNet_TCP_Open(&ip);
	set = SDLNet_AllocSocketSet(1);

	std::cout << "Server started\n";
}

void Core::loop()
{
	TCPsocket client{ SDLNet_TCP_Accept(server) };
	if (!client) return;

	SDLNet_TCP_AddSocket(set, client);
	IPaddress* clientIP{ SDLNet_TCP_GetPeerAddress(client) };
	std::cout << "Client connected with IP "; printIP(clientIP);

	std::vector<char> req{};
	int bytes{};
	do
	{
		req.push_back(0);
		if (SDLNet_CheckSockets(set, 0) == 1)
			bytes = SDLNet_TCP_Recv(client, &req.back(), 1);
		else bytes = 0;
	} while (bytes);
	if (req.empty()) EXIT;

	std::vector<std::string> lines{ split(std::string{ req.data() }, '\n', { '\r' }) };
	std::vector<std::string> statusLine{ split(lines[0], ' ') };
	if (statusLine.size() != 3) EXIT;
	if (statusLine[1].find("..") != std::string::npos) EXIT;

	std::stack<char> extension{};
	for (std::string::const_reverse_iterator c{ statusLine[1].crbegin() }; c != statusLine[1].crend(); c++)
		if (*c == '.') break;
		else extension.push(*c);
	std::string fileType{};
	while (!extension.empty())
		fileType.push_back(extension.top()), extension.pop();

	fileType = "*/*";

	if (statusLine[0] == "GET")
	{
		std::vector<char> fileContent{ rdFile(statusLine[1]) };
		std::string response{ "HTTP/2.0 200 Ok\nConnection: close\nContent-type: " + fileType + "\n\n" };
		for (const char& c : fileContent) response.push_back(c);
		SDLNet_TCP_Send(client, response.data(), static_cast<int>(response.size()));
	}

	SDLNet_TCP_DelSocket(set, client);
	SDLNet_TCP_Close(client);
}

void Core::quit()
{
	SDLNet_FreeSocketSet(set);
	SDLNet_Quit();
}