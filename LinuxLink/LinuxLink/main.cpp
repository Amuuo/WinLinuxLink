#include"Main.h"




LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) 
{

  static BYTE lpb[40];
  static RAWINPUT* raw = (RAWINPUT*)lpb;
  static UINT dwSize = 40;
  static int8_t mouseRel[2];

  
  
  switch (Msg)
  {			
  
    case WM_CREATE:
      fopen_s(&out, "data.txt", "w");
      initializeConnection(hwnd);
		  break;
    
    
    case WM_INPUT:
      GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
      
      if (raw->header.dwType == RIM_TYPEMOUSE) 
      {        
        mouseRel[0] = raw->data.mouse.lLastX;
        mouseRel[1] = raw->data.mouse.lLastY;        
      }
      send(mouseStruct.sock, (char*)mouseRel, 2, 0);
      break;

    
    case WM_MOUSEWHEEL: 
      
      if (HIWORD(wParam) == 0x88) 
      {
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
                     int nCmdShow) 
{
	                                            
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
	if (!RegisterClassEx(&wc)) 
  {
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

  if (!hwnd) 
  {
		MessageBox(
			NULL,
			_T("Call to CreateWindow failed!"),
			_T("Windows Desktop Guided Tour"),
			MB_OK);

		return 1;
	}

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) 
  {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}

	return (int)msg.wParam;

}













