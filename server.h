#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define PORT 1313
#define BSIZE 1024
#define NUM_PLAYERS 2

//struct for storing player endpoints
struct PlayerInfo
{
   unsigned int host;
   unsigned int port;
};

class Server
{
public:
	Server();
       
private:

	struct sockaddr_in m_ep1,m_ep2;
	char buf[BSIZE];
	struct PlayerInfo m_players[NUM_PLAYERS];
};
