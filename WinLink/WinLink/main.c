#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/uinput.h>
#include <sys/time.h>
#include <ctype.h>

#define MOUSEMOVE  0x1000
#define RMB        0x2000
#define KEYUP      0x3000
#define KEYDOWN    0x4000
#define LMB        0x5000
#define WHEELUP    0x7000
#define WHEELDOWN  0x8000

typedef int SOCKET;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct sockaddr sockaddr;


pthread_t signalThread;
pthread_t keyThread;
pthread_t mouseThread;
pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;

SOCKET keyboardSocket;
SOCKET signalSocket;
SOCKET mouseSocket;
SOCKET keyboardListenSock;
SOCKET signalListenSock;
SOCKET mouseListenSock;

sockaddr_in mouseAddr;
sockaddr_in keyboardAddr;
sockaddr_in signalAddr;

int PORT{5600};
int fd;


const char* localIPAddress = "192.168.1.6";
 


void* setupSocketConnection(SOCKET listenSock, 
                            SOCKET sock, 
                            sockaddr_in sockAddr,
                            int port, 
                            char* socketName) {
  createSocket(sock);
  setupProtocols(&sockAddr, port);
  connectSocket(sock, sockAddr, socketName);
  bindToSocket(sock, sockAddr);
  listenForConnections(sock);
  acceptConnection(listenSock, sock, sockAddr);
}


void createSocket(SOCKET sock) 
{
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
  {
    printf("\n>> Count not create socket : \n");
    exit(1);
  }
  return;
}


void setupProtocols(sockaddr_in *addr, int port) 
{
  memset(addr, 0, sizeof(addr));
  addr->sin_addr.s_addr = inet_addr(localIPAddress);
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  printf("\n>> Protocols created.");
}


void connectSocket(SOCKET sock, sockaddr_in sockAddr, char* socketName) 
{
  if((connect(sock, (sockaddr*)&sockAddr, sizeof(sockaddr))) < 0)
  {
    printf("\n>> Could not connect with MAIN_SERVER, Error: %d", errno);
    printf("\nbefore connect in function");
    exit(1);
  }
  printf("\n>> %s connected", socketName);
}


void bindToSocket(SOCKET sock, sockaddr_in sockAddr) 
{
	if (bind(sock, (sockaddr*)&sockAddr, sizeof(sockAddr)) < 0)
		printf("\nBind failed with error code: %d", errno);
}


void listenForConnections(SOCKET sock) 
{
	if(listen(sock, 1) < 0)
  {
    printf("\n>> Failed to listen. Exiting...");
    exit(1);
  }
	printf("\n>> Waiting for incoming connections...");
}


void acceptConnection(SOCKET listenSock, SOCKET sock, sockaddr_in sockAddr) 
{
	static int c = sizeof(sockAddr);
	sock = accept(listenSock, (sockaddr*)&sockAddr, &c);
}






void* receivingSignal()
{   
  uint8_t signalCode;
    
  while(1)
  {
    recv(signalSocket, (uint8_t*)&signalCode, 1, 0);
    if(signalCode == SIGINT)
    {
      close(keyboardSocket);
      close(signalSocket);   
      close(mouseSocket);                     
      exit(1);
    }
  }
}






void* receivingMouse()
{

  int8_t mouseData[2];
  while(1)
  {
    recv(mouseSocket, (int8_t*)mouseData, 2, 0);
    case(mouseData&)
    emit(EV_KEY, REL_X, mouseData[0]);
    emit(EV_KEY, REL_Y, mouseData[1]);
    emit(EV_KEY, BTN_RIGHT, (data>>1) & 0x01);
    emit(EV_KEY, BTN_LEFT,  data & 0x01);      
    emit(EV_REL, REL_WHEEL,  2);               
    emit(EV_KEY, REL_WHEEL, -2);               
    
    emit(EV_SYN, SYN_REPORT, 0);
  }
}






void* receivingKey() 
{   
  uint16_t data;

  while(1) 
  {
    recv(keyboardSocket, (uint16_t*)&data, 2, 0);  
        
    switch(data&0x1000)
    {        
      case KEYUP:   emit(EV_KEY, data & 0xff, 1); break;
      case KEYDOWN: emit(EV_KEY, data & 0xff, 0); break;

    }
  } 
}





void emit(uint16_t type, uint16_t code, int32_t val)
{
    struct input_event event;
    static int eventSize = sizeof(event);
    
    event.type = type;
    event.code = code;
    event.value = val;
    event.time.tv_sec = 0;
    event.time.tv_usec = 0;

    pthread_mutex_lock(&mutx);
    write(fd, &event, eventSize);
    emit(EV_SYN, SYN_REPORT, 0);
    pthread_mutex_unlock(&mutx);
    //memset(&event, 0, eventSize);

}//================================================================





void setupDrivers() 
{
  fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  setupKeyboardDriver();
  setupMouseDriver();
}





void setupKeyboardDriver()
{    
    int i;
    struct uinput_setup keyboardSetup;   

    for(i = 1; i < 120; ++i) 
        ioctl(fd, UI_SET_KEYBIT, i);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    strcpy(keyboardSetup.name, "WindowsKeyboard");
    ioctl(fd, UI_DEV_SETUP, &keyboardSetup);
    ioctl(fd, UI_DEV_CREATE);
    

    printf("\n>> Keyboard driver created.");
        
}





void setupMouseDriver() 
{
  struct uinput_setup mouseSetup;
  
    
  ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
  ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
  ioctl(fd, UI_SET_EVBIT,  EV_REL);
  ioctl(fd, UI_SET_RELBIT, REL_X);
  ioctl(fd, UI_SET_RELBIT, REL_Y);
  ioctl(fd, UI_SET_RELBIT, REL_WHEEL);

  strcpy(mouseSetup.name, "WindowsMouse");
  ioctl(fd, UI_DEV_SETUP, &mouseSetup);
  ioctl(fd, UI_DEV_CREATE);

  printf("\n>> Mouse driver created.");
}



typedef struct {
  SOCKET listenSock;
  SOCKET sock;
  sockaddr_in sockAddr;
  int port;
  char* socketName;
}  argStruct;


int main(int argc, char* argv[]) 
{                                        
  
  system("clear");
  
  
  setupDrivers();

  argStruct keyboardArgs = (argStruct){keyboardListenSock, 
                                    keyboardSocket, 
                                    keyboardAddr, 
                                    5600, 
                                    "keyboardSock"};

  argStruct mouseArgs= (argStruct){mouseListenSock, 
                                   mouseSocket, 
                                   mouseAddr, 
                                   5601, 
                                   "mouseSock"};

  argStruct signalArgs = (argStruct){signalListenSock, 
                                     signalSocket, 
                                     signalAddr,
                                     5602, 
                                     "signalSock"};

  
  pthread_create (&keyThread,    NULL, &setupSocketConnection, (void*)&keyboardArgs);
  pthread_create (&mouseThread,  NULL, &setupSocketConnection, (void*)&mouseArgs);
  pthread_create (&signalThread, NULL, &setupSocketConnection, (void*)&signalArgs);
                             
  pthread_join (keyThread,    NULL);
  pthread_join (mouseThread,  NULL);
  pthread_join (signalThread, NULL);
  
  pthread_create (&keyThread,    NULL, &receivingKey,    NULL);
  pthread_create (&mouseThread,  NULL, &receivingMouse,  NULL);
  pthread_create (&signalThread, NULL, &receivingSignal, NULL);
  
  
  pthread_join (keyThread,    NULL);
  pthread_join (signalThread, NULL);
  pthread_join (mouseThread,  NULL);
    
  return 0;
}
