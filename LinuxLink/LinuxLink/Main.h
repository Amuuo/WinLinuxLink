#include<Windows.h>
#include<Windowsx.h>
#include<tchar.h>
#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<WinSock.h>
#include<string>
#include<iostream>
#include<conio.h>
#include<string>
#include<fstream>
#include"SocketStruct.h"
#pragma comment(lib, "Ws2_32.lib")





#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC  ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT) 0x02)
#endif

#define MOUSEMOVE  0x10000000
#define RMB_UP     0x20000000
#define RMB_DOWN   0x30000000
#define LMB_UP     0x40000000
#define LMB_DOWN   0x50000000
#define WHEELUP    0x60000000
#define WHEELDOWN  0x70000000
#define KEYUP      0x80000000
#define KEYDOWN    0x90000000




//unsigned int port{5600};

static TCHAR  szWindowClass[] = _T("win32app");
static TCHAR  szTitle[]       = _T("LinuxLink");

SocketStruct keyboardStruct{"keyboardStruct", 5600};
SocketStruct mouseStruct   {"mouseStruct",    5601};
SocketStruct signalStruct  {"signalStruct",   5602};

WSADATA    winSock;  
FILE*      out;
RAWMOUSE   mouse;
HINSTANCE  hInst;





void initializeWinsock() 
{	
	if (WSAStartup(MAKEWORD(2, 2), &winSock) != 0)
  {
		printf("\nFailed to initialize");
  }
  printf("\n>> Initialized Winsock...");
}




void sendKeyToLinux(int16_t msg) 
{
  send(keyboardStruct.sock, (char*)&msg, 4, 0);  
}


void sendMouseToLinux(int8_t* msg) 
{
  send(mouseStruct.sock, (char*)msg, 4, 0);
}


void printDebug(std::string type, uint16_t msg_) 
{
  fprintf(out, "\n%s: %x", type.c_str(), msg_);
}





void initializeConnection(HWND hwnd) 
{

  static const char* tmp_message = "\n\n\n       Run WinLink on Linux";
  static PAINTSTRUCT ps;
  static HBRUSH      hBrush;
  static HRGN        bgRgn;
  static RECT        clientRect;
  static HDC         hdc;
  
  ShowWindow  (hwnd, SW_SHOW);	    
      
  initializeWinsock();	
  mouseStruct.setupAndConnectSocket();

  Sleep(1000);
	keyboardStruct.setupAndConnectSocket();
  Sleep(1000);
  signalStruct.setupAndConnectSocket();
  Sleep(1000);


  GetClientRect(hwnd, &clientRect);
  hdc    = BeginPaint(hwnd, &ps);
  bgRgn  = CreateRectRgnIndirect(&clientRect);
  hBrush = CreateSolidBrush(RGB(75, 53, 118));
  FillRgn(hdc, bgRgn, hBrush);
  DrawText(hdc, tmp_message, strlen(tmp_message), &clientRect, 0);
  EndPaint(hwnd, &ps);


  Beep(523, 75);
  Beep(987, 75);
  Beep(783, 250);
  Beep(1567, 750);
  

  MoveWindow  (hwnd, 3000, 500, 500, 500, true);
  ShowWindow  (hwnd, SW_MAXIMIZE);
	UpdateWindow(hwnd);      
}
