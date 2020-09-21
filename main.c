#include "main.h"

unsigned char dropped[MAX_PATH];
char szClassName[ ] = "Windows_Pxe_Boot";

void CenterOnScreen(HWND hnd)
{
     RECT rcClient, rcDesktop;
     SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
     GetWindowRect(hnd, &rcClient);
     int nX=((rcDesktop.right - rcDesktop.left) / 2) -((rcClient.right - rcClient.left) / 2);
     int nY=((rcDesktop.bottom - rcDesktop.top) / 2) -((rcClient.bottom - rcClient.top) / 2);
     SetWindowPos(hnd, NULL, nX, nY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
//	 SetWindowPos(hnd,HWND_TOPMOST,0,0,0,0,SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);                    
}


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            hWnd = hwnd;
            InitCommonControls();
            CenterOnScreen(hwnd);
            Is_On = LoadBitmap(ins,MAKEINTRESOURCE(2000));
            Is_Off = LoadBitmap(ins,MAKEINTRESOURCE(2001));
           
            HWND s1;
            s1 = CreateWindow("BUTTON", "DHCP", WS_CHILD | WS_VISIBLE| BS_GROUPBOX| BS_CENTER ,4,2, 45,161, hwnd,0, ins, NULL);                
            SNDMSG(s1, WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
            //ang1
            ang1 = CreateWindow("STATIC","",WS_VISIBLE|WS_CHILD|SS_BITMAP,15,123,20,33,hwnd,(HMENU)0,ins,NULL);
            MAKE_OFF(ang1);
             //ang2
            ang2 = CreateWindow("STATIC","",WS_VISIBLE|WS_CHILD|SS_BITMAP,15,88,20,33,hwnd,(HMENU)0,ins,NULL);
            MAKE_OFF(ang2);
            //ang3
            ang3 = CreateWindow("STATIC","",WS_VISIBLE|WS_CHILD|SS_BITMAP,15,53,20,33,hwnd,(HMENU)0,ins,NULL);
            MAKE_OFF(ang3);
            //ang4
            ang4 = CreateWindow("STATIC","",WS_VISIBLE|WS_CHILD|SS_BITMAP,15,18,20,33,hwnd,(HMENU)0,ins,NULL);
            MAKE_OFF(ang4);
            
            s1 = CreateWindow("BUTTON", "TFTP", WS_CHILD | WS_VISIBLE| BS_GROUPBOX| BS_CENTER ,54,2, 281,161, hwnd,0, ins, NULL);                
            SNDMSG(s1, WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
            
            InitializeCriticalSection(&cs);
            sss = CreateWindow("EDIT", "Drag-Drop Here BootLoader File to Start", WS_CHILD | WS_VISIBLE  | ES_AUTOVSCROLL| ES_AUTOHSCROLL | ES_MULTILINE |ES_WANTRETURN,
            60,17, 268, 140, hwnd,0, ins, NULL);                
            SNDMSG(sss, WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
            SendMessage(sss,EM_SETLIMITTEXT,(WPARAM)0xFFFFFFFF,(LPARAM)0);  
            
            if(WSAStartup(MAKEWORD(2,0),&wsaData) == SOCKET_ERROR)
            {
                MessageBox(hwnd, "Cant't init WSAStartup","StartUp Error",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
                break;
            }
            DragAcceptFiles(hwnd,1);
            
        }
        break;
        case WM_DROPFILES:
		{				
		    ZeroMemory(dropped,MAX_PATH);
			HDROP hDrop=(HDROP)wParam;
			DragQueryFile(hDrop,0,dropped,MAX_PATH);
			DragFinish(hDrop);
		    
            WIN32_FIND_DATA dataFile; 
            HANDLE hFile;
            hFile = FindFirstFile(dropped,&dataFile);
            if(hFile == INVALID_HANDLE_VALUE)
            {
                 MessageBox(hwnd, "Failed to open Draged File Doesn't exist",dropped,MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
                 break;
            } 
            FindClose(hFile);
            if(dataFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                    MessageBox(hwnd, "BootLoader must be a File not a Folder","BootLoader",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
                    break;
            }
            unsigned char *p;
            p=strrchr(dropped,'\\');
            if(!p) p=strrchr(dropped,'/');
            if(!p)
            {
                    MessageBox(hwnd, "strrchr[drop] Failedr","BootLoader",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
                    break;
            }
            p++;
            sprintf(Path,"%s\0",p);
            SetWindowText(sss,Path);
            DragAcceptFiles(hwnd,0);
            CreateThread(0,0,(LPTHREAD_START_ROUTINE)StartLoop,0,0,0); 
        }
        break;
        case WM_DESTROY:
        {
            WSACleanup();
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
        }
        break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain (HINSTANCE _h_, HINSTANCE _p_, LPSTR _l_, int _f_)
{
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;
    ins = _h_;
    wincl.hInstance = _h_;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS; 
    wincl.cbSize = sizeof (WNDCLASSEX);

    wincl.hIcon = LoadIcon (ins,MAKEINTRESOURCE(200));
    wincl.hIconSm = LoadIcon (ins,MAKEINTRESOURCE(200));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL; 
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH)CreateSolidBrush(0xF0F0F0); //COLOR_BACKGROUND+1;

    if (!RegisterClassEx (&wincl))
        return 0;
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "PXE - NetWork Boot",       /* Title Text */
          WS_OVERLAPPEDWINDOW,// WS_SYSMENU|WS_MINIMIZEBOX, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           347,                 /* The programs width */
           200,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           ins,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );
    ShowWindow (hwnd, _f_);

    while(GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
 return messages.wParam;
}


void StartLoop()
{
    unsigned char temp[MAX_PATH];
	struct hostent *host;   
    while(1)
    {
        _sleep(500);
        memset(temp, 0, MAX_PATH);   
        if(gethostname(temp, MAX_PATH) != 0)
              continue;
        if((host=gethostbyname(temp)) == NULL)
              continue;
        sprintf(temp,"%s\0",inet_ntoa(*(struct in_addr*)host->h_addr));
        if(strstr(temp,"127.0.0.1"))
              continue;
        break;
    }
    SetWindowText(hWnd,temp);
    sscanf( temp, "%d.%d.%d.%d", &c[0], &c[1], &c[2], &c[3]);
    CreateThread(0,0,(LPTHREAD_START_ROUTINE)StartDHCP,0,0,0);
    _sleep(1000);
    CreateThread(0,0,(LPTHREAD_START_ROUTINE)StartTFTP,0,0,0); 
}

