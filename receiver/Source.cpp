/*
Author: Pindrought
Date: 11/13/2015
This is the solution for the client that you should have at the end of tutorial 1.
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <iostream>

#define MAX_SEQ 7
#define TIME_OUT_TIME 4
#include <iostream>
typedef enum {frame_arrival, cksum_err, timeout, network_layer_ready} event_type;
#include "protocol.h"
using namespace std;
event_type event;
SOCKET Connection;
char MOTD[256];
static int no_of_msgs_in_buffer=0;
char physical_layer_buffer_char[8];
int hold_ack_flag = 0;
int hold_ack_timer = 0;
int current_index = 0;
int go_back_flag = 0;

static bool between(seq_nr a, seq_nr b, seq_nr c)
{
	/* Return true if a <= b < c circularly; false otherwise. */
	if(((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
		return(true);
	else
		return(false);
}
static void send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
	/* Construct and send a data frame. */
	frame s;                                                                           /* scratch variable */
	int msg = send(Connection, MOTD, sizeof(MOTD), NULL); //Send MOTD buffer
}
void enable_network_layer(void)
{
	//write code here
}

void from_network_layer(packet * p)
{
	static unsigned int i =0;
	*p = network_layer_buffer[i];
	i = (i+1) %8;
  }
void from_physical_layer(frame * r)
{
	static unsigned int i =0;
	if (hold_ack_flag != 1)
	{
		*r = physical_layer_buffer[i];
		physical_layer_buffer_char[i] = MOTD[0];
		i = (i + 1) % 8;
		no_of_msgs_in_buffer = (no_of_msgs_in_buffer + 1);
	}
	else
	{
		while (physical_layer_buffer_char[i] != network_layer_buffer_char[current_index-1])
		{
			physical_layer_buffer_char[i] = NULL;
			i--;
		}
	}
}
void to_network_layer(packet * p)
{
	static unsigned int i =0;
	if(no_of_msgs_in_buffer>2)
	{
		if(physical_layer_buffer_char[i]!=' ')
		{
			if (hold_ack_flag != 1)
			{
				current_index = (current_index + 1) % 8;
				received_buffer[i] = physical_layer_buffer_char[i];
				cout <<"received: "<< received_buffer[i] << '\n';
				MOTD[0] = received_buffer[i]; //x no acknoledgement
				int msg = send(Connection, MOTD, sizeof(MOTD), NULL); //Send MOTD buffer
				i = (i + 1) % 8;
			}
			else{
				if (hold_ack_timer != 0)hold_ack_timer++;
				MOTD[0] = 'x'; //x no acknoledgement
				int msg = send(Connection, MOTD, sizeof(MOTD), NULL); //Send MOTD buffer
			}
			event=network_layer_ready;
		}
		else
		{
			current_index = (current_index + 1) % 8;
			received_buffer[i]= physical_layer_buffer_char[i];
			physical_layer_buffer_char[i] = network_layer_buffer_char[i];
			cout<<'\n';
			hold_ack_flag = 1;
			hold_ack_timer++;
			MOTD[0]='x'; //x no acknoledgement
			int msg = send(Connection, MOTD, sizeof(MOTD), NULL); //Send MOTD buffer
			event = network_layer_ready;
			i = (i + 1) % 8;
		}
	}
	else 
	{
		if (hold_ack_timer != 0)hold_ack_timer++;
		MOTD[0] = 'x'; //x no acknoledgement
		int msg = send(Connection, MOTD, sizeof(MOTD), NULL); //Send MOTD buffer
		event = network_layer_ready;
	}
}


void stop_timer(seq_nr k)
{
	//write code here
}

void to_physical_layer(frame * s)
{
	static unsigned int i =0;
	physical_layer_buffer[i]=*s;
	cout<<network_layer_buffer_char[i];
	i = (i+1) %8;
}
void start_timer(seq_nr k)
{
	static int i=0;
	physical_layer_buffer_timer[k]=i;
	physical_layer_buffer_flag[k]=1;
	i++;
	for(int j=0;j<8;j++)
	{
		if (physical_layer_buffer_flag[j]==1)
			if((i-physical_layer_buffer_timer[j])>TIME_OUT_TIME) event=timeout;
	}

}
void disable_network_layer(void)
{
	//write code here
}
void protocol5(int x)
{
	seq_nr next_frame_to_send;                                                         /* MAX_SEQ > 1; used for outbound stream */
	seq_nr ack_expected;                                                               /* oldest frame as yet unacknowledged */
	seq_nr frame_expected;                                                             /* next frame_expected on inbound stream */
	frame r;                                                                           /* scratch variable */
	packet buffer[MAX_SEQ + 1];                                                        /* buffers for the outbound stream */
	seq_nr nbuffered;                                                                  /* number of output buffers currently in use */
	seq_nr i;       
	enable_network_layer();                                                            /* allow network_layer_ready events */
	ack_expected = 0;                                                                  /* next ack_expected inbound */
	next_frame_to_send = 0;                                                            /* next frame going out */
	frame_expected = 0;                                                                /* number of frame_expected inbound */
	nbuffered = 0;                                                                     /* initially no packets are buffered */
	event = frame_arrival;
	while(true) 
	{
		if(event!=timeout && event!=cksum_err)
		{
			int code = recv(Connection, MOTD, sizeof(MOTD), NULL); 
			if(code!=-1) event=frame_arrival;
			
		}

		switch(event) {
			case network_layer_ready:                                                  /* the network layer has a packet to send */
			/* Accept, save, and transmit a new frame. */
				from_network_layer(&buffer[next_frame_to_send]);                       /* fetch new packet */
				nbuffered = nbuffered + 1;                                             /* expand the sender’s window */
				if(hold_ack_flag != 1) send_data(next_frame_to_send, frame_expected, buffer);                 /* transmit the frame */
				cout << MOTD[0] << "\n";
				inc(next_frame_to_send);                                                /* advance sender’s upper window edge */
				event = frame_arrival;
				break;

			case frame_arrival:                                                        /* a data or control frame has arrived */
				from_physical_layer(&r);                                               /* get incoming frame from_physical_layer */
				to_network_layer(&r.info);                                         /* pass packet to_network_layer */
				if (hold_ack_timer >= (TIME_OUT_TIME - 2))
				event = timeout;
				break;

			case cksum_err: break;                                                     /* just ignore bad frames */

			case timeout:                                                              /* trouble; retransmit all outstanding frames */
				hold_ack_timer = 0;
				hold_ack_flag = 0;
				no_of_msgs_in_buffer = 0;
				event = frame_arrival;
				go_back_flag = 1;
		}
		if(nbuffered < MAX_SEQ)
			enable_network_layer();
		else
			disable_network_layer();
	}
}


int main()
{
	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //If WSAStartup returns anything other than 0, then that means an error has occured in the WinSock Startup.
	{
		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}
	
	SOCKADDR_IN addr; //Address to be binded to our Connection socket
	int sizeofaddr = sizeof(addr); //Need sizeofaddr for the connect function
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Address = localhost (this pc)
	addr.sin_port = htons(1111); //Port = 1111
	addr.sin_family = AF_INET; //IPv4 Socket

	Connection = socket(AF_INET, SOCK_STREAM, NULL); //Set Connection socket
	if (connect(Connection, (SOCKADDR*)&addr, sizeofaddr) != 0) //If we are unable to connect...
	{
		MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
		return 0; //Failed to Connect
	}
	std::cout << "Connected!" << std::endl;
	protocol5(3);
		Sleep(10);

	
}

