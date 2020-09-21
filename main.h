#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <winsock.h>
#include <commctrl.h>

HINSTANCE ins;
HWND hWnd;

CRITICAL_SECTION cs;
WSADATA wsaData;

HWND sss;

unsigned char Path[MAX_PATH];
int c[4];

HBITMAP Is_On, Is_Off;
HWND ang1, ang2, ang3, ang4;

#define MAKE_ON(_x_)      SendMessage(_x_, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(HANDLE)Is_On);
#define MAKE_OFF(_x_)     SendMessage(_x_, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(HANDLE)Is_Off);
 

void StartLoop();
void StartDHCP();
void StartTFTP();
