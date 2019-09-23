#ifndef NXUTILITY_HPP_INCLUDED
#define NXUTILITY_HPP_INCLUDED

#include "nxCommon.hpp"

HWND nxCreateMessageWindow(WNDPROC lpfnWndProc);
bool nxFilterKeyRepeat(bool bEnabled);
bool nxEnableWindowsKey(bool enabled);
#endif // NXUTILITY_HPP_INCLUDED
