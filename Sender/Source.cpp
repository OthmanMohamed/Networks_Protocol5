/*
Author: Pindrought
Date: 11/13/2015
This is the solution for the server that you should have at the end of tutorial 1.
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include "protocol.h"

#define MAX_SEQ 7
#define TIME_OUT_TIME 4
#include <iostream>
typedef enum {frame_arrival, cksum_err, timeout, network_layer_ready} event_type;
#include "protocol.h"
using namespace std;
event_type event;
char MOTD[256] = ""; //Create buffer with message of the day
char MOTD_R[256] = "x"; //receiver buffer
SOCKET newConnection; //Socket to hold the client's 


static bool between(seq_nr a, seq_nr b, seq_nr c)
{
	/* Return true if a <= b < c circularly; false otherwise. */
	if(((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
		return(true);
	else
		return(false);
}

void enable_network_layer(void)
{
	//write code here
}


void from_physical_layer(frame * r)
{
	int code = recv(newConnection, MOTD_R, sizeof(MOTD_R), NULL); 
	for (int i = 0; i < 8; i++)
	{
		if (network_layer_buffer_char[i] == MOTD_R[0]) (*r).seq = i;
	}
	//if(code!=-1) event=frame_arrival;
}
void to_network_layer(packet * p)
{
	//write code here
}
void stop_timer(seq_nr k)
{
//
}
void to_physical_layer(frame * s)
{
	//
}
void init_timer()
{
	for (int j = 0; j<8; j++)
	{
		physical_layer_buffer_flag[j]=0;
		physical_layer_buffer_timer[j] = 0;
	}
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

static void send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[],int x)
{
	/* Construct and send a data frame. */
	frame s;                                                                           /* scratch variable */
	s.info = buffer[frame_nr];                                                         /* insert packet into frame */
	s.seq = frame_nr;                                                                  /* insert sequence number into frame */
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);                                /* piggyback ack */
	to_physical_layer(&s);                                                             /* transmit the frame */
	cout<<"sent: "<<network_layer_buffer_char[frame_nr];
	

	if(x==1)MOTD[0]=network_layer_buffer_char[frame_nr];
	else if (x == 2) {	MOTD[0] = ' '; cout << " with error";}
	cout << "\n";
	start_timer(frame_nr);                                                             /* start the timer running */
	int msg = send(newConnection, MOTD, sizeof(MOTD), NULL); //Send MOTD buffer
}
void from_network_layer(packet * p)
{
	static unsigned int i =0;
	*p = network_layer_buffer[i];
	i = (i+1) %8;
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
	event=network_layer_ready;
	cout << "enter 1 for successful sending, 2 for error \n";
	while(true) 
	{
		if(event==network_layer_ready)
		{  
			cin>>x;
			event = network_layer_ready;
		}
		

		switch(event) {
			case network_layer_ready:                                                  /* the network layer has a packet to send */
			/* Accept, save, and transmit a new frame. */
				from_network_layer(&buffer[next_frame_to_send]);                       /* fetch new packet */
				nbuffered = nbuffered + 1;                                             /* expand the sender’s window */
				send_data(next_frame_to_send, frame_expected, buffer,x);                 /* transmit the frame */
				/*if(x!=2)*/ inc(next_frame_to_send);                                               /* advance sender’s upper window edge */
				if(event!=timeout)event =frame_arrival;
				break;
			case frame_arrival:                                                        /* a data or control frame has arrived */
				from_physical_layer(&r);                                               /* get incoming frame from_physical_layer */
				if(MOTD_R[0]!='x')
				{
					cout << "acknowledged " << MOTD_R[0] << "\n";
					MOTD_R[0]='x';
					if (r.seq == ack_expected  && 	physical_layer_buffer_flag[frame_expected] == 1) {
						/* Frames are accepted only in order. */
						to_network_layer(&r.info);                                         /* pass packet to_network_layer */
						physical_layer_buffer_flag[frame_expected] = 0;
						inc(frame_expected);                                               /* advance lower edge of receiver’s window */
						inc(ack_expected);
					}
					r.ack = r.seq;
					while (between(ack_expected, r.ack, next_frame_to_send)) {
						//* Handle piggybacked ack. */
						nbuffered = nbuffered - 1;                                         /* one frame fewer buffered */
						//stop_timer(ack_expected);                                          /* frame_arrived intact; stop_timer */
						inc(ack_expected);                                                 /* contract sender’s window */
					}
				}
				event=network_layer_ready;				
				break;
			case cksum_err: break;                                                     /* just ignore bad frames */
			case timeout:                                                              /* trouble; retransmit all outstanding frames */
				cout<<network_layer_buffer_char[ack_expected]<<" In Time Out"<<"\n";
				next_frame_to_send = ack_expected;                                     /* start retransmitting here */
				for (i = 0; i <= TIME_OUT_TIME; i++) {
					nbuffered = nbuffered + 1;
					cout << "re";
					send_data(next_frame_to_send, frame_expected, buffer,1);             /* resend frame */
					inc(next_frame_to_send);                                           /* prepare to send the next one */
					from_physical_layer(&r);                                               /* get incoming frame from_physical_layer */
					if (MOTD_R[0] != 'x')
					{
						if (r.seq == frame_expected  && 	physical_layer_buffer_flag[frame_expected] == 1) {
							/* Frames are accepted only in order. */
							cout << "acknowledged: " << MOTD_R[0] << "\n";
							MOTD_R[0] = 'x';
							to_network_layer(&r.info);                                         /* pass packet to_network_layer */
							physical_layer_buffer_flag[frame_expected] = 0;
							inc(frame_expected);                                               /* advance lower edge of receiver’s window */
							inc(ack_expected);
						}
						while (between(ack_expected, r.ack, next_frame_to_send)) {
							//* Handle piggybacked ack. */
							nbuffered = nbuffered - 1;                                         /* one frame fewer buffered */
							stop_timer(ack_expected);                                          /* frame_arrived intact; stop_timer */
							inc(ack_expected);                                                 /* contract sender’s window */
						}
					}
				}
				event = network_layer_ready;
		}
		if(nbuffered < MAX_SEQ)
			enable_network_layer();
		else
			disable_network_layer();
	}
}



int main()
{

	//WinSock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //If WSAStartup returns anything other than 0, then that means an error has occured in the WinSock Startup.
	{
		MessageBoxA(NULL, "WinSock startup failed", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	
	SOCKADDR_IN addr; //Address that we will bind our listening socket to
	int addrlen = sizeof(addr); //length of the address (required for accept call)
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Broadcast locally
	addr.sin_port = htons(1111); //Port
	addr.sin_family = AF_INET; //IPv4 Socket

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //Create socket to listen for new connections
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); //Bind the address to the socket
	listen(sListen, SOMAXCONN); //Places sListen socket in a state in which it is listening for an incoming connection. Note:SOMAXCONN = Socket Oustanding Max Connections

	newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen); //Accept a new connection
	if (newConnection == 0) //If accepting the client connection failed
	{
		std::cout << "Failed to accept the client's connection." << std::endl;
	}
	else //If client connection properly accepted
	{
		std::cout << "Client Connected!" << std::endl;
	}
	protocol5(3);

	system("pause");
	return 0;
}