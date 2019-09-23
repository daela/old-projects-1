#include "nxUtility.hpp"


namespace
{
    LRESULT CALLBACK nxEmptyWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

HWND nxCreateMessageWindow(WNDPROC lpfnWndProc)
{
    // The class registration will fail if it's occurred more than once, but we need to run this once per thread.. so this is the simplest way.
    const char szClassName[] = "nxMessageWindow";
    WNDCLASSEX wc;
    wc.hInstance     = nxInstanceHandle();
    wc.lpszClassName = szClassName;
    wc.lpfnWndProc   = nxEmptyWindowProc;
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.hIconSm       = NULL;
    RegisterClassEx(&wc);

    HWND hwnd=CreateWindowEx(0,szClassName,NULL,0,0,0,0,0,HWND_MESSAGE,NULL,nxInstanceHandle(),NULL);
    if (hwnd)
    {
        if (!SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)lpfnWndProc))
        {
            DestroyWindow(hwnd);
            hwnd = NULL;
        }
    }
    return hwnd;
}

bool nxFilterKeyRepeat(bool bEnabled)
{
    static FILTERKEYS objOriginalSettings;
    static bool bState = false;
    if (bState == bEnabled) return true;
    if (!bState && bEnabled)
    {
        // Retrieve current settings
        objOriginalSettings.cbSize = sizeof(FILTERKEYS);
        if (!SystemParametersInfo(SPI_GETFILTERKEYS,sizeof(FILTERKEYS),&objOriginalSettings,0))
            return false;

        // Enable the filter
        FILTERKEYS objFilter;
        objFilter.cbSize = sizeof(FILTERKEYS);
        objFilter.dwFlags = FKF_AVAILABLE | FKF_FILTERKEYSON;
        objFilter.iWaitMSec = 0;
        objFilter.iDelayMSec = 0;
        objFilter.iRepeatMSec = 0;
        objFilter.iBounceMSec = 0;
        if (!SystemParametersInfo(SPI_SETFILTERKEYS,sizeof(FILTERKEYS),&objFilter,0))
            return false;

        // Update state
        bState = true;
    }
    else
    {
        // Restore original settings
        SystemParametersInfo(SPI_SETFILTERKEYS,sizeof(FILTERKEYS),&objOriginalSettings,0);

        // Update state
        bState = false;
    }
    return true;
}

namespace
{
    LRESULT CALLBACK nxWinKeyDisableHookProc(int code, WPARAM wparam, LPARAM lparam)
    {
      PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT)lparam;

      switch(wparam)
      {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
          if(key->vkCode==VK_LWIN || key->vkCode==VK_RWIN)
            return 1;
      }

      return CallNextHookEx(NULL, code, wparam, lparam);
    }
}
bool nxEnableWindowsKey(bool enabled)
{
	static HHOOK hHook = NULL;
	static bool bEnabled = true;
	if (bEnabled && !enabled)
	{
		hHook=SetWindowsHookEx(WH_KEYBOARD_LL,nxWinKeyDisableHookProc,nxInstanceHandle(),0);
		if (!hHook)
			return false;
		bEnabled = false;
	}
	else if (!bEnabled && enabled)
	{
		if (!UnhookWindowsHookEx(hHook))
			return false;
		bEnabled = true;
		hHook = NULL;
	}
	// if the above conditions didn't match, we're already in the right state
	return true;
}
