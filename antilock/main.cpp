#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#include <windows.h> // SendInput required WINVER to be set
#include <shellapi.h> // For system tray
#include <tchar.h> // For _T() and friends
#include <iostream>

HMENU hPopMenu;
bool bDone;
HICON hIcon;
HWND hWnd;
const TCHAR szClassName[] = _T("domainantilock");
const UINT uTimerDelay=5*1000;
DWORD IdleTime()
{
	LASTINPUTINFO lii;
	lii.cbSize=sizeof(LASTINPUTINFO);
	if (GetLastInputInfo(&lii))
	{
		return GetTickCount()-lii.dwTime;
	}
	return 0;
}

void FakeInput()
{
	INPUT input;
	input.type=INPUT_MOUSE;
	ZeroMemory(&input.mi,sizeof(MOUSEINPUT));
	input.mi.dwFlags=MOUSEEVENTF_MOVE;
	SendInput(1,&input,sizeof(INPUT));
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_USER+1:
			switch(LOWORD(lParam)) 
			{  
				case WM_RBUTTONDOWN:
				{
					POINT clickPoint;
					GetCursorPos(&clickPoint);
					
					SetForegroundWindow(hWnd);
					TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,clickPoint.x, clickPoint.y,0,hWnd,NULL);
					break;
				}
			}
			return TRUE;
		case WM_TIMER:
			std::cout << "Timer fired." << std::endl;
			FakeInput();
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case 0:
					MessageBox(hWnd,_T("Domain Anti-Lock by Jacob McIntosh"),_T("About Domain Anti-Lock"),MB_OK);
					break;
				case 1:
					bDone=true;
					break;
			}
			return 0;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	hIcon=NULL;
	hWnd=NULL;
	hPopMenu=NULL;
	bDone=false;

	NOTIFYICONDATA nidApp;
	try
	{
		TCHAR szIconFile[MAX_PATH];
		DWORD uLen=GetModuleFileName(hInstance,szIconFile,sizeof(szIconFile));
		if (!uLen) 
			throw std::runtime_error("Couldn't retrieve application pathname.");
		_tcscpy(szIconFile+uLen-3,_T("ico"));
		// Register the window class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= NULL;
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= szClassName;
		wcex.hIconSm		= NULL;
		if (!RegisterClassEx(&wcex))
			throw std::runtime_error("Couldn't register window class.");

		// Create the popup menu
		hPopMenu = CreatePopupMenu();
		if (!hPopMenu)
			throw std::runtime_error("Couldn't create tray popup menu.");
		InsertMenu(hPopMenu,-1,MF_BYPOSITION|MF_STRING,0,_T("&About"));
		InsertMenu(hPopMenu,-1,MF_BYPOSITION|MF_STRING,1,_T("E&xit"));

		// Load the tray icon
		hIcon=(HICON)LoadImage(NULL,szIconFile,IMAGE_ICON,0,0,LR_LOADFROMFILE); 
		if (!hIcon)
			throw std::runtime_error("Couldn't load tray icon.");
		
		// Create the message only window
		hWnd=CreateWindowEx(NULL,szClassName, _T(""), NULL, 0, 0, 0, 0, HWND_MESSAGE,NULL,hInstance,NULL);
		if (!hWnd)
			throw std::runtime_error("Couldn't create message only window.");

		// Set the fake input timer 
		if (!SetTimer(hWnd,0,uTimerDelay,NULL))
			throw std::runtime_error("Couldn't create timer to fake input.");

		// Create the tray icon
		nidApp.cbSize = sizeof(NOTIFYICONDATA); 
		nidApp.hWnd = (HWND) hWnd;
		nidApp.uID = 0; 
		nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; 
		nidApp.hIcon = hIcon; 
		nidApp.uCallbackMessage = WM_USER+1; 
		_tcscpy(nidApp.szTip,_T("Domain Anti-Lock"));
		if (!Shell_NotifyIcon(NIM_ADD, &nidApp))
		{
			throw std::runtime_error("Couldn't create tray icon.");
		}

		MSG msg;
		while (!bDone && GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} catch (std::exception&e) { MessageBoxA(NULL,e.what(),"Domain Anti-Lock Error",MB_OK); }
	catch (...) { }
	// Cleanup
	KillTimer(hWnd,0);
	if (hWnd) DestroyWindow(hWnd);
	if (hPopMenu) DestroyMenu(hPopMenu);
	if (hIcon) DestroyIcon(hIcon);
	if (bDone) Shell_NotifyIcon(NIM_DELETE,&nidApp);
	UnregisterClass(szClassName,hInstance);
	
	return bDone;
}