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

#define MOUSEMOVE  0x10
#define RMB_UP     0x20
#define RMB_DOWN   0x30
#define LMB_UP     0x40
#define LMB_DOWN   0x50
#define WHEELUP    0x60
#define WHEELDOWN  0x70
#define KEYUP      0x80
#define KEYDOWN    0x90

typedef int SOCKET;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct sockaddr sockaddr;


pthread_t signalThread;
pthread_t keyThread;
pthread_t mouseThread;
pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;

SOCKET keyboardClientSocket;
SOCKET signalClientSocket;
SOCKET mouseClientSocket;
SOCKET keyboardListenSock;
SOCKET signalListenSock;
SOCKET mouseListenSock;

sockaddr_in mouseAddr;
sockaddr_in keyboardAddr;
sockaddr_in signalAddr;

int fd;


typedef struct {

  SOCKET*      listenSock;
  SOCKET*      clientSock;
  sockaddr_in* sockAddr;
  int          port;
  char*        socketName;

}  argStruct;


const char* localIPAddress = "192.168.1.6";


void print_args(argStruct* args) {

  pthread_mutex_lock(&mutx);

  printf("\n\n%10s : %d, %d", "listenSock", args->listenSock, *args->listenSock);
  printf("\n%10s : %d, %d", "clientSock", args->clientSock, *args->clientSock);
  printf("\n%10s : %d, %d", "sockAddr", args->sockAddr, *args->sockAddr);
  printf("\n%10s : %d", "port", args->port);
  printf("\n%10s : %s\n", "name", args->socketName);

  pthread_mutex_unlock(&mutx);
}



void createSocket(argStruct* args)
{
  if ((*args->listenSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n>> Count not create socket : \n");
    pthread_exit(1);
  }
  printf("\n>> Socket created");
  return;
}


void setupProtocols(argStruct* args)
{
  memset(args->sockAddr, 0, sizeof(sockaddr_in));
  args->sockAddr->sin_addr.s_addr = inet_addr(localIPAddress);
  args->sockAddr->sin_family = AF_INET;
  args->sockAddr->sin_port = htons(args->port);
  printf("\n>> Protocols created.");
}


void bindToSocket(argStruct* args)
{
  if ((bind(*args->listenSock, (sockaddr*)args->sockAddr, sizeof(sockaddr))) < 0)
  {
		printf("\nBind failed with error code: %d", errno);
    pthread_exit(1);
  }
  printf("\nBound to socket %d", *args->clientSock);

}


void listenForConnections(argStruct* args)
{
	if((listen(*args->listenSock, 1)) < 0)
  {
    printf("\n>> Failed to listen. Exiting...");
    pthread_exit(1);
  }
	printf("\n>> Waiting for incoming connections...");
}


void acceptConnection(argStruct* args)
{
	static int c = sizeof(sockaddr_in);
	*args->clientSock = accept(*args->listenSock, (sockaddr*)args->sockAddr, &c);
  printf("\nClient accepted: clientSock: %d", *args->listenSock);
}


void* setupSocketConnection(void* argsPtr) {

  argStruct* args = (argStruct*)argsPtr;

  printf("\n>> Setting up %s on port %i", args->socketName, args->port);

  setupProtocols(args);
  createSocket(args);
  print_args(args);

  bindToSocket(args);
  listenForConnections(args);
  acceptConnection(args);

  print_args(args);

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
  memset(&event, 0, eventSize);

}



void* receivingSignal()
{

  printf("\n\n>> Receiving signal");
  uint8_t signalCode;

  while(1)
  {
    recv(signalClientSocket, (uint8_t*)&signalCode, 4, 0);
    if(signalCode == SIGINT)
    {
      close(keyboardClientSocket);
      close(signalClientSocket);
      close(mouseClientSocket);
      exit(1);
    }
  }
}






void* receivingMouse()
{
  int8_t mouseData[4];

  printf("\n\n>> Receiving mouse data: hex: %x x: %d, y: %d", 
         *mouseData,
         mouseData[0],
         mouseData[1]);

  while(1)
  {

    recv(mouseClientSocket, (int8_t*)mouseData, 4, 0);
    /*printf("\n\n>> Receiving mouse data: \nhex: %x \nx: %d, \ny: %d",
    mouseData,
    mouseData[0],
    mouseData[1]);*/
    printf("\n\nmouseData[0]: %d\nmouseData[1]: %d\nmouseData[2]: %d\nmouseData[3]: %d",
           mouseData[0],
           mouseData[1],
           mouseData[2],
           mouseData[3]);

    printf("\nmouseData[0] & 0xf0: %d", mouseData[0] & 0xf0);

    switch(mouseData[0] & 0xf0)
    {
    case MOUSEMOVE:
      emit(EV_KEY, REL_X, mouseData[2]&0x01);
      emit(EV_KEY, REL_Y, mouseData[3]&0x01);
      break;

    case RMB_UP:
      emit(EV_KEY, BTN_RIGHT, 0);
      break;

    case RMB_DOWN:
      emit(EV_KEY, BTN_RIGHT, 1);
      break;

    case LMB_UP:
      emit(EV_KEY, BTN_LEFT, 0);
      break;

    case LMB_DOWN:
      emit(EV_KEY, BTN_LEFT, 1);
      break;

    case WHEELUP:
      emit(EV_REL, REL_WHEEL, 2);
      break;

    case WHEELDOWN:
      emit(EV_REL, REL_WHEEL, -2);
      break;

    default:
      break;
    }
    memset(mouseData, 0, 4);
    //emit(EV_SYN, SYN_REPORT, 0);
  }
}






void* receivingKey()
{
  uint16_t keyData;

  printf("\n\n>> Receiving key: %4x = %c", keyData, (char)keyData&0x00ff);

  while(1)
  {
    //memset(keyData, 0, 2);
    recv(keyboardClientSocket, (uint16_t*)&keyData, 2, 0);
    printf("\n\n>> Receiving key: %4x = %c", keyData, (char)keyData&0x00ff);
    switch(keyData&0xf000)
    {
    case 0x8000:
      emit(EV_KEY, keyData & 0x00ff, 1);
      break;

    case 0x4000:
      emit(EV_KEY, keyData & 0x00ff, 0);
      break;

    default:
      break;
    }

  //emit(EV_SYN, SYN_REPORT, 0);
  }
}




void setupKeyboardDriver()
{
  int i;
  struct uinput_setup keyboardSetup;

  for(i = 1; i < 120; ++i)
  {
    ioctl(fd, UI_SET_KEYBIT, i);
  }

  ioctl(fd, UI_SET_EVBIT, EV_KEY);
  strcpy(keyboardSetup.name, "WindowsKeyboard");
  ioctl(fd, UI_DEV_SETUP, &keyboardSetup);
  ioctl(fd, UI_DEV_CREATE);

  printf("\n>> Keyboard driver created.");

  sleep(1);
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

  sleep(1);
}



void setupDrivers()
{
  fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  setupKeyboardDriver();
  setupMouseDriver();
}















int main(int argc, char* argv[])
{
  system("clear");

  setupDrivers();

  argStruct keyboardArgs = (argStruct){&keyboardListenSock,
                                      &keyboardClientSocket,
                                      &keyboardAddr,
                                      5600,
                                      "keyboardSock"};

  argStruct mouseArgs= (argStruct){&mouseListenSock,
                                   &mouseClientSocket,
                                   &mouseAddr,
                                   5601,
                                   "mouseSock"};

  argStruct signalArgs = (argStruct){&signalListenSock,
                                     &signalClientSocket,
                                     &signalAddr,
                                     5602,
                                     "signalSock"};


  print_args(&keyboardArgs);
  print_args(&mouseArgs);
  print_args(&signalArgs);

  pthread_create (&keyThread,    NULL, &setupSocketConnection, &keyboardArgs);
  pthread_create (&mouseThread,  NULL, &setupSocketConnection, &mouseArgs);
  pthread_create (&signalThread, NULL, &setupSocketConnection, &signalArgs);

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
