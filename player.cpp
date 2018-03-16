#include "player.h"

//initializing constance string values which we're going to use for mainly database purposes
const char* strRock = "Rock";
const char* strPaper = "Paper";
const char* strScissor = "Scissor";
const char* strRockCnt = "RockCnt";
const char* strPaperCnt = "PaperCnt";
const char* strScissorCnt = "ScissorCnt";
const char* strRockWin = "RockWin";
const char* strPaperWin = "PaperWin";
const char* strScissorWin = "ScissorWin";
const char* strPlayerScore = "PlayerScore";
const char* strOppScore = "OppScore";



//method to find peers.. based on UDP hole punching
//here we send the 'hi' packet to server address thus server gets our public endpoint info
//this happens with other players also and as server broadcasts all public endpoints we recevie them and store them
 
bool Player::Connect()
{
	m_eplen = sizeof(m_ep2);
	
	//socket init
	if((m_sockFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		cout<<m_name<<" : Socket creation Failed.."<<endl<<endl;
		exit(EXIT_FAILURE);
	}
	
	//cout<<m_name<<": socket file descriptor "<<m_sockFd<<endl<<endl;

	//our public endpoint data
	memset((char*)&m_ep1, 0, sizeof(m_ep1));
	m_ep1.sin_family = AF_INET;
	m_ep1.sin_port = htons(PORT);
	m_ep1.sin_addr.s_addr = htonl(INADDR_ANY);


	//server's info
	memset((char*)&m_ep2, 0, sizeof(m_ep2));
	m_ep2.sin_family = AF_INET;
	m_ep2.sin_port = htons(PORT);

	if(inet_aton(m_serverAdd.c_str(), &m_ep2.sin_addr)==0)
	{
		cout<<m_name<<" : server's endpoint aton failed.."<<endl<<endl;
		exit(EXIT_FAILURE);	
	}
	
	m_server.host = m_ep2.sin_addr.s_addr;
	m_server.port = m_ep2.sin_port;


	//sending hello packet to server to initiate peering process
	if(sendto(m_sockFd, "hi", 2, 0, (struct sockaddr*)(&m_ep2), m_eplen)==-1)
	{
		cout<<m_name<<" : sending packet to server failed.."<<endl<<endl;
		exit(EXIT_FAILURE);	
	}
	
	while(1)
	{
		//we constantly check for incoming packets of peers
		if(recvfrom(m_sockFd, &m_buf, sizeof(m_buf), 0, (struct sockaddr*)(&m_ep2), &m_eplen)==-1)
		{
			cout<<m_name<<" : error in receiving packet.."<<endl<<endl;
			exit(EXIT_FAILURE);		
		}
		
		//check whether packet is from server, if it's from server it has the info of other peers
		if(m_server.host == m_ep2.sin_addr.s_addr && m_server.port == (unsigned int)(m_ep2.sin_port))
		{
			//cout<<m_name<<" : server packet received.."<<endl<<endl;	
			//cout<<m_name<<" : Received packet from "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<endl<<endl;		
			int found = 0;
			
			for(int i=0; i<m_cnt && found == 0; i++)
			{
				if(m_peers[i].host == m_buf.host && m_peers[i].port == m_buf.port)
				{
					found = 1;
				}
			}

			//storing other peer's info
			if(found == 0)
			{
				m_peers[m_cnt].host = m_buf.host;
				m_peers[m_cnt].port = m_buf.port;
				m_cnt++;
			}

			m_ep2.sin_addr.s_addr = m_buf.host;
			m_ep2.sin_port = m_buf.port;
			//cout<<m_name<<" : Added peer "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<endl<<endl;
			//cout<<m_name<<" : Now we have "<<cnt<<" peers"<<endl<<endl;

			//now we'll send packets to peers to check if connections are possible
			for(int i=0; i<5; i++)
			{
				for(int j=0; j<m_cnt; j++)
				{
					m_ep2.sin_addr.s_addr = m_peers[j].host;
					m_ep2.sin_port = m_peers[j].port;

					if(sendto(m_sockFd, m_name.c_str(), m_name.length(), 0, (struct sockaddr*)(&m_ep2), m_eplen)==-1)
					{
						cout<<m_name<<" : sending packets to peer  "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<" failed.."<<endl<<endl;
					}
				}
			}
		}
		else // that means packet is sent by peers
		{
			//cout<<m_name<<": Peers packet received.."<<endl<<endl;		
			//cout<<m_name<<" : Received packet from "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<endl<<endl;		
			for(int i=0; i<m_cnt; i++)
			{
				if(m_peers[i].host == m_ep2.sin_addr.s_addr && m_peers[i].port == (unsigned int)(m_ep2.sin_port))
				{
					//cout<<m_name<<" : Received packer from peer "<<i<<endl<<endl;
					m_connectedPlayers.push_back(m_peers[i]);					
					break;
				}
			}
		}
		//we'll stop looking for peers as we got enough peers to start the session
		if(m_cnt == PLAYER_COUNT)
			return true;		
	}
	return false;
}


//utility function helpers to convert string to int and vice versa for our game's logic
const char* Player::getValueByIndex(int ind)
{	
	const char* val = "nil";
	switch(ind)
	{
		case 0:
			val = strRock;
			break;
		case 1:
			val = strPaper;
			break;
		case 2:
			val = strScissor;
			break;
		default:
			cout<<m_name<<" : Invalid index passed.."<<endl;			
			break;
	}
	return val;
}

int Player::getIndexByValue(const char* val)
{
	if(val == strRock)
		return 0;
	else if(val == strPaper)
		return 1;
	else if(val == strScissor)
		return 2;
	else
		cout<<m_name<<" : Invalid value passed.."<<endl;
}


//Helper function to compute ratio of the current choice
float Player::GetRatio(int choice)
{
	float ratio;
	
	//avoid devide zero
	if(m_count[choice] != 0)
	{
		ratio = m_wins[choice]/m_count[choice];
	}
	else
	{
		ratio = 0;
	}
	
	return ratio;
}

//to compute our choice
int Player::ComputeChoice(int round)
{
	int choice;	
	vector<float> choicesRatio(3);
	for(int i=0; i<=2; ++i)
	{
		choicesRatio[i] = GetRatio(i);
	}

	//here initial rounds, we dont have enough data to have biased choice so we just generate random choice
	// random choice is also generated when all the ratios are equal
	if(round < 10 || adjacent_find(choicesRatio.begin(), choicesRatio.end(), not_equal_to<float>() ) == choicesRatio.end())
	{
		srand(time(NULL) + getpid()); // unique seed for random number as both child process get same instance of time
		choice = rand() % 3;	
		//cout<<m_name<<" : pid "<<getpid()<<endl;	
	}
	else
	{
		choice = max_element(choicesRatio.begin(),choicesRatio.end()) - choicesRatio.begin(); //otherwisw we just check the ratio of all choices and pick the one with highest value
												      //i.e with high successrate in past (wins/chances)
	}

	m_PlayerChoice = choice;
	m_count[choice]++;
	return m_PlayerChoice;
}


//socket function to send the peer signal of ready
void Player::SendReady()
{
	m_eplen = sizeof(m_dest);

	if(sendto(m_sockFd, "Ready", sizeof("Ready"), 0, (struct sockaddr*)(&m_dest), m_eplen)==-1)
	{
		cout<<m_name<<" : send ready to "<<inet_ntoa(m_dest.sin_addr)<<" "<<ntohs(m_dest.sin_port)<<" failed.."<<endl<<endl;
	}
	//cout<<m_name<<" : send ready to "<<inet_ntoa(m_dest.sin_addr)<<" "<<ntohs(m_dest.sin_port)<<endl<<endl;
}

//socket fuction to receive ready from peer so that we can move ahead
bool Player::CheckReady()
{
	m_eplen = sizeof(m_dest);

	if(recvfrom(m_sockFd, m_buffer, BSIZE, 0, (struct sockaddr*)(&m_ep2), &m_eplen)==-1)
	{
		cout<<m_name<<" : Received ready from "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<"failed"<<endl<<endl;
		return false;	
	}
	
	if(strstr(m_buffer,"Ready"))
	{
		//cout<<m_name<<" : Received ready from "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<endl<<endl;
		memset(m_buffer, 0 , sizeof(m_buffer));
		return true;	
	}	
}

//socket based function to send our current choice to peer
void Player::SendChoice(int choice)
{
	m_eplen = sizeof(m_dest);
	string s = to_string(choice);

	if(sendto(m_sockFd, s.c_str() , s.length(), 0, (struct sockaddr*)(&m_dest), m_eplen)==-1)
	{
		cout<<m_name<<" : Send choice to "<<inet_ntoa(m_dest.sin_addr)<<" "<<ntohs(m_dest.sin_port)<<"failed.."<<endl<<endl;
	}
	cout<<m_name<<" : Send choice : "<<choice<<" to "<<inet_ntoa(m_dest.sin_addr)<<" "<<ntohs(m_dest.sin_port)<<endl<<endl;
}

//socket based call to receive choice of the peer
bool Player::ReceiveChoice()
{
	m_eplen = sizeof(m_dest);

	if(recvfrom(m_sockFd, m_buffer, BSIZE, 0, (struct sockaddr*)(&m_ep2), &m_eplen)==-1)
	{
		cout<<m_name<<" : Received choice from "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<"failed.."<<endl<<endl;
		return false;	
	}
	m_OppChoice = atoi(m_buffer);
	cout<<m_name<<" : Received choice from "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<" : "<<m_buffer<<endl<<endl;
	memset(m_buffer, 0 , sizeof(m_buffer));
	return true;
}

//helper function to decide the winner after completion of each round
void Player::DeclareWinner()
{
	//if both player picks the same choice, it's a tie and both get half point
	if(m_PlayerChoice == m_OppChoice)
	{
		m_PlayerScore +=0.5;
		m_OppSCore += 0.5;
		cout<<m_name<<" : IT'S A TIE.."<<endl<<endl;
		m_wins[m_PlayerChoice]++;
	}
	else if(m_PlayerChoice == 0) //if player picks Rock and opponent picks paper then opponent wins 
	{
		if(m_OppChoice == 1)
		{		
			m_OppSCore += 1;
		}
		else //otherwisw if opponent picks scissor then player wins
		{
			cout<<m_name<<" : I Win"<<endl<<endl;			
			m_PlayerScore += 1;
			m_wins[m_PlayerChoice]++;
		}
	}
	else if(m_PlayerChoice == 1) //if player choses paper and opponent picks scissor then opponent wins
	{
		if(m_OppChoice == 2)
		{	
			m_OppSCore += 1;
		}
		else //otherwise if opponent picks rock then player wins
		{
			cout<<m_name<<" : I Win"<<endl<<endl;			
			m_PlayerScore += 1;
			m_wins[m_PlayerChoice]++;
		}
	}
	else if(m_PlayerChoice == 2) //if player picks scissor and opponent picks rock then opponent wins
	{
		if(m_OppChoice ==  0)
		{
			m_OppSCore += 1;
		}
		else //otherwise if opponent picks paper then player wins
		{
			cout<<m_name<<" : I Win"<<endl<<endl;			
			m_PlayerScore += 1;
			m_wins[m_PlayerChoice]++;
		}
	}
}

//utility function to display live score of each round
void Player::DisplayScore()
{
	cout<<m_name<<" has score : "<< m_PlayerScore<<endl<<endl<<endl<<endl;
}

//this is the most essensial part of peer discovery..
//since server sends all peers all publice end point it also contains our own endpoint
// so we need to rectify the desired peer among our data
void Player::Rectify()
{
	//cout<<m_name<<" : rectification started..with player count "<<m_cnt<<endl<<endl;


	//one way is we'll send the packet of our name to all the players in our list
	// if we receive packet with our own name, then it's our endpoint data and we need to neglect it	
	for(int i=0;i<m_cnt;i++)
	{
		m_ep2.sin_addr.s_addr = m_peers[i].host;
		m_ep2.sin_port = m_peers[i].port;		
		if(sendto(m_sockFd, m_name.c_str() , m_name.length(), 0, (struct sockaddr*)(&m_ep2), m_eplen)==-1)
		{	
			cout<<m_name<<" : Send name to "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<"failed.."<<endl<<endl;
		}	
		//cout<<m_name<<" : Send name to "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<endl<<endl;
	}

	while(1)
	{
		if(recvfrom(m_sockFd, m_buffer, BSIZE, 0, (struct sockaddr*)(&m_ep2), &m_eplen)==-1)
		{
			cout<<m_name<<" : Received name from "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<"failed.."<<endl<<endl;	
		}
		//cout<<m_name<<" : Received "<< m_buffer<<" from "<<inet_ntoa(m_ep2.sin_addr)<<" "<<ntohs(m_ep2.sin_port)<<endl<<endl;
		if(strstr(m_buffer,m_name.c_str())) // it's send by us
		{
			//cout<<m_name<<" : rectified own port : "<<ntohs(m_ep2.sin_port)<<endl<<endl;			
			m_ep1.sin_port = m_ep2.sin_port;	
		}
		else if(ntohs(m_ep2.sin_port) != PORT) //it's not send by us and also not by server so it's our desired destination
		{
			cout<<m_name<<" : found dest port "<<ntohs(m_ep2.sin_port)<<endl<<endl;			
			m_dest = m_ep2;		
			break;
		}		
	}

}

//utility function to determine final winner as one player has scored more than 100
void Player::DeclareFinalWinner()
{
	if(m_PlayerScore >=100) //if player scored more than 100 then player wins
	{
		cout<<m_name<<" : I WON !!!!"<<endl<<endl;	
	}
	if(m_PlayerScore>=100 && m_OppSCore>=100)
	{
		//if both players score more than 100 at same time then we need to determine by super round
		//here both players generate the random number and send to each other. who generates the larger number wins and breaks the tie
		srand(time(NULL) + getpid());		
		int ownChoice = rand();
		
		SendChoice(ownChoice);
		
		//blocking call for receving choice of opponent in order to synchronize each step parallely
		while(!ReceiveChoice()) {}
		if(ownChoice > m_OppChoice)
		{
			cout<<m_name<<" : I WON AGAIN !!!!!!!!"<<endl<<endl;		
		}	
	}
}

void Player::InitDb()
{
	//initiating redis client on local host
	// can be initialized on server if it's being run on different machine
	if(!m_rdx.connect("localhost", 6379)) 
 	{
		cout<<m_name<<" : error initializing db.."<<endl;
		exit(EXIT_FAILURE);
 	}

	//intializing default values for stats
	m_rdx.set("PlayerScore","0");
	m_rdx.set("OppScore","0");
	m_rdx.set("RockCnt","0");
	m_rdx.set("PaperCnt","0");
	m_rdx.set("ScissorCnt","0");
	m_rdx.set("RockWin","0");
	m_rdx.set("PaperWin","0");
	m_rdx.set("ScissorWin","0");
	
}


// database helper function to load the data 
void Player::LoadDb()
{
	string str;

	//load Rock chances and Win stats
	str = getValueFromDb(strRockCnt);
	m_count[0] = atoi(str.c_str());
	str = getValueFromDb(strRockWin);
	m_wins[0] = atoi(str.c_str());

	//load paper chances and win stats
	str = getValueFromDb(strPaperCnt);
	m_count[1] = atoi(str.c_str());
	str = getValueFromDb(strPaperWin);
	m_wins[1] = atoi(str.c_str());

	//load scissor chances and win stats
	str = getValueFromDb(strScissorCnt);
	m_count[2] = atoi(str.c_str());
	str = getValueFromDb(strScissorWin);
	m_wins[2] = atoi(str.c_str());

	//load player score
	str = getValueFromDb(strPlayerScore);
	m_PlayerScore = atof(str.c_str());

	//load opponent score
	str = getValueFromDb(strOppScore);
	m_OppSCore = atof(str.c_str());
}

//database helper function to save data
void Player::SaveDb()
{
	string str;
	
	//save Rock chanecs and wins
	str=to_string(m_count[0]);
	setValueInDb(strRockCnt, str.c_str());
	str=to_string(m_wins[0]);
	setValueInDb(strRockWin, str.c_str());

	//save paper chances and wins
	str=to_string(m_count[1]);
	setValueInDb(strPaperCnt, str.c_str());
	str=to_string(m_wins[1]);
	setValueInDb(strPaperWin, str.c_str());

	//save scissor chances and wins
	str=to_string(m_count[2]);
	setValueInDb(strScissorCnt, str.c_str());
	str=to_string(m_wins[2]);
	setValueInDb(strScissorWin, str.c_str());

	//save player score
	str = to_string(m_PlayerScore);
	setValueInDb(strPlayerScore, str.c_str());

	//save opponent score
	str = to_string(m_OppSCore);
	setValueInDb(strOppScore, str.c_str());	
}

//database helper function to retrieve value from database by key
string Player::getValueFromDb(const char* key)
{
	return m_rdx.get(key);
}


//database helper function to set the value by key
void Player::setValueInDb(const char*key, const char* val)
{
	m_rdx.set(key, val);
}


//main method of execution flow
void Player::Update()
{
	//initialize our database connection with redis
	InitDb();	
	int round = 0;

	//blocking call to check if we're connected or not
	while(!Connect()){}
	// we need to rectify destination host among list of peers
	Rectify();
	
	cout<<m_name<<" : connection eshtablished.."<<endl<<endl;
	while(1)
	{
		round++;
		//at the begining of each round load the stats from Db 
		LoadDb();

		//send the ready signal to peer
		SendReady();
		//blocking call to receive ready from peer in order to synchronize flow
		while(!CheckReady()) {}

		//after player gets affirmation that peers is ready, it sends the choice to peer
		SendChoice(ComputeChoice(round));
		//player waits for the choice of peer
		while(!ReceiveChoice()) {}

		//after receiving choice of opponent, both players will decide who the winner is
		DeclareWinner();
		//display score at end of each round
		DisplayScore();

		//to check if any player has score more than 100
		if(m_PlayerScore >100 || m_OppSCore>100)
		{
			cout<<endl<<endl<<endl<<endl;		
			//if so choose the final winner	
			DeclareFinalWinner();
			break;
		}
		//we write stats to Db at end of each round
		SaveDb();

		//random delay so game dont get ended in 1s ;-)
		sleep(2);		
	}
}

