#include "server.h"


Server::Server()
{
	int sc;
	socklen_t eplen = sizeof(m_ep2);
	int cnt=0;

	//creating a UDP based socket for server
	if((sc=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		cout<<"Server: socket creation failed.."<<endl<<endl;
		exit(EXIT_FAILURE);	
	}	
	
	//initializing socket structs for our own infos
	memset((char*)&m_ep1, 0, sizeof(m_ep1));
	m_ep1.sin_family = AF_INET;
	m_ep1.sin_port = htons(PORT);
	m_ep1.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind the created socket with socket options we initialized above
	if(bind(sc,(struct sockaddr*)(&m_ep1), sizeof(m_ep1))==-1)
	{
		cout<<"Server : binding failed.."<<endl<<endl;
		exit(EXIT_FAILURE);	
	}

	//we run the server unless we dont't have desired number of players connected.
	// in this scenario NUM_PLAYERS. change the value accordingly as per required players
	while(cnt<NUM_PLAYERS)
	{
		//here server checks for any packets from players
		if(recvfrom(sc, buf, BSIZE, 0, (struct sockaddr*)(&m_ep2), &eplen)==-1)
		{
			cout<<"Server : problem in receiving datagram.."<<endl<<endl;
			exit(EXIT_FAILURE);		
		}
		
		cout<<"Received packet from "<<inet_ntoa(m_ep2.sin_addr)<<" : "<<ntohs(m_ep2.sin_port)<<endl<<endl;
		
		//strong the player endpoint data 
		m_players[cnt].host = m_ep2.sin_addr.s_addr;
		m_players[cnt].port = m_ep2.sin_port;
		cnt++;
		
		//broadcasting the endpoint data to all the players in the network
		for( int i=0; i<cnt; i++)
		{
			m_ep2.sin_addr.s_addr = m_players[i].host;
			m_ep2.sin_port = m_players[i].port;
			
			for(int j=0; j<cnt; j++)
			{
				cout<<"Sending to "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<endl<<endl;
				if(sendto(sc, &m_players[j], sizeof(m_players[j]), 0, (struct sockaddr*)(&m_ep2), eplen)==-1)
				{
					cout<<"Server : broadcasting public endpoints failed.."<<endl<<endl;				
				}			
			}		
		}
		
		cout<<"Server : we have now "<<cnt<<" Players online"<<endl<<endl;		
	}
	
	close(sc);
}

