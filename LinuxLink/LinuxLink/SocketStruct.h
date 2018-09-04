#pragma once
#include<Windows.h>
#include<iostream>

using namespace std;



class SocketStruct
{ 

public:

  SocketStruct()
  {
  }
  SocketStruct(const char* n, unsigned int p) : name{n}, port{p} 
  {
  }
  
  const char*   name;
  SOCKADDR_IN   addr{};
  SOCKET        sock{};
  int           port{};

  void setupAndConnectSocket() 
  {
    setupSocketProtocols();
    createSocket();
    connectSocket();
  }
  

  void setupSocketProtocols() 
  {      
	  memset(&addr, 0, sizeof(SOCKADDR_IN));
	  addr.sin_addr.s_addr  = inet_addr("192.168.1.4");
	  addr.sin_family       = AF_INET;
	  addr.sin_port         = htons(port);
    printf("\n>> Protocols established...");
  }
  
  
  void createSocket() 
  {
	  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
		  printf("\n>> Count not create socket : \n");
    }
    printf("\n>> Connected to Socket...");
  }


  void connectSocket() 
  {
    if ((connect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR))) < 0) 
    {
      printf("\n>> Could not connect socket");
    }
  }


};

