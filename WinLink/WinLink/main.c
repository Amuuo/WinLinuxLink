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
sockaddr_in keyboardAddr, mouseAddr, signalAddr;
int PORT;
int fd;


void* receivingMouse();
void  createSocket();
void  setupProtocols(sockaddr_in*, unsigned short);
void  connectSocket();
void  emit(uint16_t,uint16_t,int32_t);
void* receivingKey();
void* receivingSignal();
void* receivingMouse();
void  setupDriver();



//=================================================================
//                            M A I N
//=================================================================
int main(int argc, char* argv[]) 
{                                        
    system("clear");
    setupDriver();    
    createSocket();
    setupProtocols(&keyboardAddr, 5600);
    setupProtocols(&mouseAddr, 5601);
    setupProtocols(&signalAddr, 5508);
    connectSocket();
    pthread_create(&keyThread, NULL, &receivingKey, NULL);
    pthread_create(&mouseThread, NULL, &receivingMouse, NULL);
    pthread_create(&signalThread, NULL, &receivingSignal, NULL);
    pthread_join(keyThread, NULL);
    pthread_join(signalThread, NULL);
    pthread_join(mouseThread, NULL);
    
    return 0;
}//================================================================

void createSocket() 
{

    if ((keyboardSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\n>> Count not create socket : \n");
        exit(1);
    }
    if ((mouseSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\n>> Count not create socket : \n");
        exit(1);
    }
    if ((signalSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\n>> Count not create socket : \n");
        exit(1);
    }
    printf("\n>> Socket created.");

    return;
}
//================================================================

void setupProtocols(sockaddr_in *addr, unsigned short port) 
{
    memset(addr, 0, sizeof(addr));
    addr->sin_addr.s_addr = inet_addr("192.168.1.4");
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    printf("\n>> Protocols created.");
}
//================================================================

void connectSocket() 
{
    if((connect(mouseSocket, (sockaddr*)&mouseAddr, sizeof(sockaddr))) < 0)
    {
        printf("\n>> Could not connect with MAIN_SERVER, Error: %d", errno);
        printf("\nbefore connect in function");
        exit(1);
    }
    //usleep(500);
    printf("\n>> mouseSocket connected");
    if((connect(keyboardSocket, (sockaddr*)&keyboardAddr, sizeof(sockaddr))) < 0) 
    {
        printf("\n>> Could not connect with MAIN_SERVER, Error: %d", errno);
        printf("\nbefore connect in function");
        exit(1);
    }
    //usleep(500);
    printf("\n>> keyboardSocket connected");
    if((connect(signalSocket, (sockaddr*)&signalAddr, sizeof(sockaddr))) < 0) 
    {
        printf("\n>> Could not connect with MAIN_SERVER, Error: %d", errno);
        printf("\nbefore connect in function");
        exit(1);
    }

    printf("\n>> signalSocket connected");
    //printf("\n>> Socket connected\n\n");
    //printf("\n>> keyboardSocket: %d, signalSocket: %d", keyboardSocket, signalSocket);
}
//================================================================

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
//================================================================

void* receivingMouse()
{

    int8_t mouseData[2];
    while(1)
    {
        recv(mouseSocket, (int8_t*)mouseData, 2, 0);
        emit(EV_KEY, REL_X, mouseData[0]);
        emit(EV_KEY, REL_Y, mouseData[1]);
        emit(EV_SYN, SYN_REPORT, 0);
    }
}

void* receivingKey() 
{   
    uint16_t data;

	while(1) 
  {
        recv(keyboardSocket, (uint16_t*)&data, 2, 0);  
        //printf("\ndata: %x", data);
        
        switch(data&0x1000)
        {        
            case KEYUP:      emit(EV_KEY, data & 0xff, 1);         break;
            case KEYDOWN:    emit(EV_KEY, data & 0xff, 0);         break;
            case RMB:        emit(EV_KEY, BTN_RIGHT, (data>>1) & 0x01); break;
            case LMB:        emit(EV_KEY, BTN_LEFT,  data & 0x01); break;
            case WHEELUP:    emit(EV_REL, REL_WHEEL,  2);          break;
            case WHEELDOWN:  emit(EV_KEY, REL_WHEEL, -2);          break;
        }

        /*if(wInput.kData != 0) {
            if      (wInput.kData & 0x8000) emit(EV_KEY, wInput.kData & 0xff, 1);
            else if (wInput.kData & 0x4000) emit(EV_KEY, wInput.kData & 0xff, 0);
        }
        
        emit(EV_KEY, BTN_LEFT, wInput.mData[0] & 0x1);
		emit(EV_KEY, BTN_RIGHT, (wInput.mData[0] >> 1) & 0x1);
		emit(EV_REL, REL_X, wInput.mData[1]);
		emit(EV_REL, REL_Y, wInput.mData[2]);
        
        if      (wInput.mWheel == 0x88) emit(EV_REL, REL_WHEEL, -2);
        else if (wInput.mWheel == 0x78) emit(EV_REL, REL_WHEEL, 2);
        */
        

        //memset(&wInput, 0, sizeInput);
	} 
}//================================================================

void emit(uint16_t type, uint16_t code, int32_t val)
{
    pthread_mutex_lock(&mutx);
    struct input_event event;
    static int eventSize = sizeof(event);
    
    event.type = type;
    event.code = code;
    event.value = val;
    event.time.tv_sec = 0;
    event.time.tv_usec = 0;

    write(fd, &event, eventSize);
    emit(EV_SYN, SYN_REPORT, 0);
    pthread_mutex_unlock(&mutx);
    //memset(&event, 0, eventSize);

}//================================================================

void setupDriver()
{    
    int i;
    struct uinput_setup keyboardSetup;   
    struct uinput_setup mouseSetup;
    
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    for(i = 1; i < 120; ++i) 
        ioctl(fd, UI_SET_KEYBIT, i);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    strcpy(keyboardSetup.name, "WindowsKeyboard");
    ioctl(fd, UI_DEV_SETUP, &keyboardSetup);
    ioctl(fd, UI_DEV_CREATE);
    

    printf("\n>> Keyboard driver created.");
    
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);
    ioctl(fd, UI_SET_RELBIT, REL_WHEEL);

    strcpy(mouseSetup.name, "WindowsKeyboard");
    ioctl(fd, UI_DEV_SETUP, &mouseSetup);
    ioctl(fd, UI_DEV_CREATE);

    printf("\n>> Mouse driver created.");

}
