#include "player.h"
#include "server.h"

int  main(int argc, char *argv[]) 
{
// we are going to fork two processes for two players and another one for server.
// server process is just for initial purpose to find the peers in network
	
	//initialize server's address
	string address;
	if(argv[1])
	{
		address = argv[1];
	}
	else//intialize with localhost if not given
	{
		address = "127.0.0.1";	
	}
	
	int pid = fork();
	if(pid == 0)
	{
		Server s;	
	}
	else
	{
	    pid = fork();
	    if(pid == 0)
	    {
		string name = "PlayerC"; // initializing constructor with player name for identification and debugging purposes
		Player playerC(name, address);		
	    }
	    else
	    {
		string name = "PlayerD";
		Player playerD(name, address);
	    }
	}	
	return 0;
}

