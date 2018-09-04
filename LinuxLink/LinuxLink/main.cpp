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
using namespace std;
#pragma comment(lib, "Ws2_32.lib")

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC  ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT) 0x02)
#endif

#define MOUSEMOVE  0x1000
#define RMBDOWN    0x2001
#define RMBUP      0x2000
#define KEYUP      0x3000
#define KEYDOWN    0x4000
#define LMBDOWN    0x5000
#define LMBUP      0x5001
#define WHEELUP    0x7000
#define WHEELDOWN  0x8000




//unsigned int port{5600};

struct socket_struct
{ 
  socket_struct(){}
  socket_struct(const char* n, unsigned int p) : name{n}, port{p} {}
  
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





static TCHAR  szWindowClass[] = _T("win32app");
static TCHAR  szTitle[]       = _T("LinuxLink");

socket_struct keyboardStruct{"keyboardStruct", 5600};
socket_struct mouseStruct   {"mouseStruct",    5601};
socket_struct signalStruct  {"signalStruct",   5602};

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




void sendKeyToLinux(uint16_t msg) 
{
  send(keyboardStruct.sock, (char*)msg, 2, 0);  
}





void printDebug(std::string type, uint16_t msg_) 
{
  fprintf(out, "\n%s: %x", type.c_str(), msg_);
}





void initializeConnection(HWND hwnd) {

  static const char* tmp_message = "\n\n\n       Run WinLink on Linux";
  static PAINTSTRUCT ps;
  static HBRUSH      hBrush;
  static HRGN        bgRgn;
  static RECT        clientRect;
  static HDC         hdc;
  
  ShowWindow  (hwnd, SW_SHOW);	    
      
  initializeWinsock();	
  mouseStruct.setupAndConnectSocket();
	keyboardStruct.setupAndConnectSocket();
  signalStruct.setupAndConnectSocket();


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




LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

  static BYTE lpb[40];
  static RAWINPUT* raw = (RAWINPUT*)lpb;
  static UINT dwSize = 40;
  static int8_t mouseRel[2];

  
  
  switch (Msg) {			

    
    case WM_CREATE:
      fopen_s(&out, "data.txt", "w");
      initializeConnection(hwnd);
		  break;
    
    
    case WM_INPUT:
      GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
      
      if (raw->header.dwType == RIM_TYPEMOUSE) {        
        mouseRel[0] = raw->data.mouse.lLastX;
        mouseRel[1] = raw->data.mouse.lLastY;        
      }
      send(mouseStruct.sock, (char*)mouseRel, 2, 0);
      break;

    
    case WM_MOUSEWHEEL: 
      
      if (HIWORD(wParam) == 0x88) {
        sendKeyToLinux(WHEELDOWN);
        printDebug("wheeldown", WHEELDOWN);
      } else { 
        sendKeyToLinux(WHEELUP); 
        printDebug("wheeldown", WHEELUP);
      } 
      break;

    
    case WM_LBUTTONDOWN: 
      sendKeyToLinux (LMBDOWN); 
      printDebug("lmbDown", LMBDOWN); 
      break;
    
    
    case WM_LBUTTONUP:   
      sendKeyToLinux (LMBUP);   
      printDebug("lmbUP", LMBUP); 
      break;
    
    
    case WM_RBUTTONDOWN: 
      sendKeyToLinux (RMBDOWN); 
      printDebug("rmbDOWN", RMBDOWN); 
      break;
    
    
    case WM_RBUTTONUP:   
      sendKeyToLinux (RMBUP);   
      printDebug("rmbUP", RMBUP); 
      break;
    
    
    case WM_KEYDOWN:     
      sendKeyToLinux ((HIWORD(lParam))+KEYDOWN); 
      printDebug("keydown", (HIWORD(lParam))+KEYDOWN); 
      break;
    
    
    case WM_KEYUP:       
      sendKeyToLinux (HIWORD(lParam)+KEYUP);         
      printDebug("keyup",(HIWORD(lParam))+KEYUP);
      break;
    
    
    case WM_SYSKEYDOWN:  
      sendKeyToLinux ((HIWORD(lParam))+KEYDOWN); 
      printDebug("keydown", (HIWORD(lParam))+KEYDOWN); 
      break;
    
    
    case WM_SYSKEYUP:    
      sendKeyToLinux (HIWORD(lParam)+KEYUP);         
      printDebug("keyup", (HIWORD(lParam))+KEYUP); 
      break;

    
    case WM_DESTROY:     
      send(signalStruct.sock,(char*)"exit",4,0); 
      fclose(out);
      exit(1);
      break;
    
    
    case WM_CLOSE:       
      send(signalStruct.sock, (char*)"exit", 4, 0); 
      fclose(out);
		  exit(1);
      break;
    
    
    default:             
      return DefWindowProc(hwnd, Msg, wParam, lParam);
	}	

  return 0;
}



int CALLBACK WinMain(HINSTANCE hInstance, 
                     HINSTANCE hPrevInstance, 
                     LPSTR lpCmdLine, 
                     int nCmdShow) {
	                                            
	WNDCLASSEX wc;

	// SET UP WINDOW CLASS
	wc.cbSize         = sizeof(WNDCLASSEX);
	wc.style          = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc    = WndProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;
	wc.hInstance      = hInstance;
	wc.hIcon          = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName   = NULL;
	wc.lpszClassName  = szWindowClass;
	wc.hIconSm        = LoadIcon(wc.hInstance, IDI_APPLICATION);

	// REGISTER WINDOW CLASS
	if (!RegisterClassEx(&wc)) {
		MessageBox(
			NULL, 
			_T("Call to RegisterClassEx failed!"),
			_T("Window Desktop Guided Tour"), 
			MB_OK);

		return 1;
	}
	
	hInst = hInstance;
	
	// CREATE WINDOW
	HWND hwnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_POPUP,
		500, 500,
		400, 150,
		NULL,
		NULL,
		hInstance,
		NULL
	);
  
  RAWINPUTDEVICE rid[1];
  rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
  rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
  rid[0].dwFlags = RIDEV_INPUTSINK;
  rid[0].hwndTarget = hwnd;
  RegisterRawInputDevices(rid, 1, sizeof(rid[0]));

  if (!hwnd) {
		MessageBox(
			NULL,
			_T("Call to CreateWindow failed!"),
			_T("Windows Desktop Guided Tour"),
			MB_OK);

		return 1;
	}

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}

	return (int)msg.wParam;

}













