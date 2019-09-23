#include "nxUtility.hpp"
#include <sstream>
#include <iomanip>

const nxEndian::EndianUnion nxEndian::objEndianUnion = { 0x1u };
const boost::uint8_t & nxEndian::Native = objEndianUnion.uMem;
const boost::uint8_t nxEndian::Little = 0x1u;
const boost::uint8_t nxEndian::Big = 0x0u;

#if defined(OS_WINDOWS_)
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

std::string nxVKeyToString(const USHORT&uVK)
{
    switch (uVK)
    {
        //case VK_LBUTTON: return "VK_LBUTTON";
        case VK_LBUTTON: return "VK_LBUTTON";
        case VK_RBUTTON: return "VK_RBUTTON";
        case VK_CANCEL: return "VK_CANCEL";
        case VK_MBUTTON: return "VK_MBUTTON";
        #if (_WIN32_WINNT >= 0x0500)
        case VK_XBUTTON1: return "VK_XBUTTON1";
        case VK_XBUTTON2: return "VK_XBUTTON2";
        #endif
        case VK_BACK: return "VK_BACK";
        case VK_TAB: return "VK_TAB";
        case VK_CLEAR: return "VK_CLEAR";
        case VK_RETURN: return "VK_RETURN";
        case VK_SHIFT: return "VK_SHIFT";
        case VK_CONTROL: return "VK_CONTROL";
        case VK_MENU: return "VK_MENU";
        case VK_PAUSE: return "VK_PAUSE";
        case VK_CAPITAL: return "VK_CAPITAL";

        case VK_KANA: // VK_HANGEUL | VK_HANGUL
            return "VK_KANA|VK_HANGEUL|VK_HANGUL";

        case VK_JUNJA: return "VK_JUNJA";
        case VK_FINAL: return "VK_FINAL";

        case VK_HANJA: // VK_KANJI
            return "VK_HANJA|VK_KANJI";

        case VK_ESCAPE: return "VK_ESCAPE";
        case VK_CONVERT: return "VK_CONVERT";
        case VK_NONCONVERT: return "VK_NONCONVERT";
        case VK_ACCEPT: return "VK_ACCEPT";
        case VK_MODECHANGE: return "VK_MODECHANGE";
        case VK_SPACE: return "VK_SPACE";
        case VK_PRIOR: return "VK_PRIOR";
        case VK_NEXT: return "VK_NEXT";
        case VK_END: return "VK_END";
        case VK_HOME: return "VK_HOME";
        case VK_LEFT: return "VK_LEFT";
        case VK_UP: return "VK_UP";
        case VK_RIGHT: return "VK_RIGHT";
        case VK_DOWN: return "VK_DOWN";
        case VK_SELECT: return "VK_SELECT";
        case VK_PRINT: return "VK_PRINT";
        case VK_EXECUTE: return "VK_EXECUTE";
        case VK_SNAPSHOT: return "VK_SNAPSHOT";
        case VK_INSERT: return "VK_INSERT";
        case VK_DELETE: return "VK_DELETE";
        case VK_HELP: return "VK_HELP";
        case VK_LWIN: return "VK_LWIN";
        case VK_RWIN: return "VK_RWIN";
        case VK_APPS: return "VK_APPS";
        case VK_SLEEP: return "VK_SLEEP";
        case VK_NUMPAD0: return "VK_NUMPAD0";
        case VK_NUMPAD1: return "VK_NUMPAD1";
        case VK_NUMPAD2: return "VK_NUMPAD2";
        case VK_NUMPAD3: return "VK_NUMPAD3";
        case VK_NUMPAD4: return "VK_NUMPAD4";
        case VK_NUMPAD5: return "VK_NUMPAD5";
        case VK_NUMPAD6: return "VK_NUMPAD6";
        case VK_NUMPAD7: return "VK_NUMPAD7";
        case VK_NUMPAD8: return "VK_NUMPAD8";
        case VK_NUMPAD9: return "VK_NUMPAD9";
        case VK_MULTIPLY: return "VK_MULTIPLY";
        case VK_ADD: return "VK_ADD";
        case VK_SEPARATOR: return "VK_SEPARATOR";
        case VK_SUBTRACT: return "VK_SUBTRACT";
        case VK_DECIMAL: return "VK_DECIMAL";
        case VK_DIVIDE: return "VK_DIVIDE";
        case VK_F1: return "VK_F1";
        case VK_F2: return "VK_F2";
        case VK_F3: return "VK_F3";
        case VK_F4: return "VK_F4";
        case VK_F5: return "VK_F5";
        case VK_F6: return "VK_F6";
        case VK_F7: return "VK_F7";
        case VK_F8: return "VK_F8";
        case VK_F9: return "VK_F9";
        case VK_F10: return "VK_F10";
        case VK_F11: return "VK_F11";
        case VK_F12: return "VK_F12";
        case VK_F13: return "VK_F13";
        case VK_F14: return "VK_F14";
        case VK_F15: return "VK_F15";
        case VK_F16: return "VK_F16";
        case VK_F17: return "VK_F17";
        case VK_F18: return "VK_F18";
        case VK_F19: return "VK_F19";
        case VK_F20: return "VK_F20";
        case VK_F21: return "VK_F21";
        case VK_F22: return "VK_F22";
        case VK_F23: return "VK_F23";
        case VK_F24: return "VK_F24";
        case VK_NUMLOCK: return "VK_NUMLOCK";
        case VK_SCROLL: return "VK_SCROLL";
        case VK_LSHIFT: return "VK_LSHIFT";
        case VK_RSHIFT: return "VK_RSHIFT";
        case VK_LCONTROL: return "VK_LCONTROL";
        case VK_RCONTROL: return "VK_RCONTROL";
        case VK_LMENU: return "VK_LMENU";
        case VK_RMENU: return "VK_RMENU";
        #if (_WIN32_WINNT >= 0x0500)
        case VK_BROWSER_BACK: return "VK_BROWSER_BACK";
        case VK_BROWSER_FORWARD: return "VK_BROWSER_FORWARD";
        case VK_BROWSER_REFRESH: return "VK_BROWSER_REFRESH";
        case VK_BROWSER_STOP: return "VK_BROWSER_STOP";
        case VK_BROWSER_SEARCH: return "VK_BROWSER_SEARCH";
        case VK_BROWSER_FAVORITES: return "VK_BROWSER_FAVORITES";
        case VK_BROWSER_HOME: return "VK_BROWSER_HOME";
        case VK_VOLUME_MUTE: return "VK_VOLUME_MUTE";
        case VK_VOLUME_DOWN: return "VK_VOLUME_DOWN";
        case VK_VOLUME_UP: return "VK_VOLUME_UP";
        case VK_MEDIA_NEXT_TRACK: return "VK_MEDIA_NEXT_TRACK";
        case VK_MEDIA_PREV_TRACK: return "VK_MEDIA_PREV_TRACK";
        case VK_MEDIA_STOP: return "VK_MEDIA_STOP";
        case VK_MEDIA_PLAY_PAUSE: return "VK_MEDIA_PLAY_PAUSE";
        case VK_LAUNCH_MAIL: return "VK_LAUNCH_MAIL";
        case VK_LAUNCH_MEDIA_SELECT: return "VK_LAUNCH_MEDIA_SELECT";
        case VK_LAUNCH_APP1: return "VK_LAUNCH_APP1";
        case VK_LAUNCH_APP2: return "VK_LAUNCH_APP2";
        #endif
        case VK_OEM_1: return "VK_OEM_1";
        #if (_WIN32_WINNT >= 0x0500)
        case VK_OEM_PLUS: return "VK_OEM_PLUS";
        case VK_OEM_COMMA: return "VK_OEM_COMMA";
        case VK_OEM_MINUS: return "VK_OEM_MINUS";
        case VK_OEM_PERIOD: return "VK_OEM_PERIOD";
        #endif
        case VK_OEM_2: return "VK_OEM_2";
        case VK_OEM_3: return "VK_OEM_3";
        case VK_OEM_4: return "VK_OEM_4";
        case VK_OEM_5: return "VK_OEM_5";
        case VK_OEM_6: return "VK_OEM_6";
        case VK_OEM_7: return "VK_OEM_7";
        case VK_OEM_8: return "VK_OEM_8";
        #if (_WIN32_WINNT >= 0x0500)
        case VK_OEM_102: return "VK_OEM_102";
        #endif
        case VK_PROCESSKEY: return "VK_PROCESSKEY";
        #if (_WIN32_WINNT >= 0x0500)
        case VK_PACKET: return "VK_PACKET";
        #endif
        case VK_ATTN: return "VK_ATTN";
        case VK_CRSEL: return "VK_CRSEL";
        case VK_EXSEL: return "VK_EXSEL";
        case VK_EREOF: return "VK_EREOF";
        case VK_PLAY: return "VK_PLAY";
        case VK_ZOOM: return "VK_ZOOM";
        case VK_NONAME: return "VK_NONAME";
        case VK_PA1: return "VK_PA1";
        case VK_OEM_CLEAR: return "VK_OEM_CLEAR";
        case KEYBOARD_OVERRUN_MAKE_CODE: return "KEYBOARD_OVERRUN_MAKE_CODE";
    }
    if (uVK >= 'A' && uVK <= 'Z')
        return std::string("VK_") +(char)uVK;
    std::stringstream ss;
    ss << "UNKNOWN(0x" << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << uVK << ")";
    return ss.str();
}
#endif
