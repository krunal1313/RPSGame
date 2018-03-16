#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

//we're using a cpp wraper redox for the redis client to manage our database
#include <redox.hpp>

using namespace std;
using namespace redox;

#define PORT 1313
#define BSIZE 1024
#define PLAYER_COUNT 2		   //number of players 


//struct for receiving endpoints of peers
struct PlayerData
{
	unsigned int host;
	unsigned int port;
};

class Player
{
public:
	Player(string name, string address)			//constructor
	{
		m_name = name;
		m_PlayerChoice = 0;
	 	m_OppChoice = 0;
	 	m_PlayerScore = 0;
	 	m_OppSCore = 0;
		m_cnt = 0;
	 	m_serverAdd = address;

		Update();			//we'll call this in constrcutor as main thread is not allowed to call it. 
	}


	//Database related functions
	void          InitDb();
	void          SaveDb();
	void	      LoadDb();
	string        getValueFromDb(const char*);
	void          setValueInDb(const char*,const char*);
	const char*   getValueByIndex(int);
	int   	      getIndexByValue(const char*);

	//game related functions
	float         GetRatio(int);
	int	      ComputeChoice(int);
	void          SendReady();
	bool  	      CheckReady();
	void          SendChoice(int);
	bool          ReceiveChoice();
	void          DeclareWinner();
	void          DisplayScore();
	void          DeclareFinalWinner();

	//core connection related functions
	bool          Connect();
	void          Update();
	void          Rectify();
	
private:
	string 	      m_name;	
	int           m_PlayerChoice;
	int           m_OppChoice;
	float         m_PlayerScore;
	float         m_OppSCore;
	map<int,int>  m_wins;
	map<int,int>  m_count;
	int           m_sockFd;
	int           m_socket;
	int           m_cnt;
	string	      m_serverAdd;
	struct sockaddr_in m_ep1,m_ep2,m_dest;	
	struct PlayerData m_peers[5], m_buf, m_server;	
	vector<struct PlayerData> m_connectedPlayers;
	char m_buffer[BSIZE];
	socklen_t m_eplen;

	Redox m_rdx;
};



#endif
