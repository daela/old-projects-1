//NOTE: We're linking a directx lib, keep up with where you did this.

#include "nxCommon.hpp"
#include "nxUtility.hpp"
#include "nxRawInputDevice.hpp"
#include "nxThread.hpp"
//#include "nxGenericInput.hpp"
#include "nxInputHandler.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <map>
#include <XInput.h> // XInput API
#include <DInput.h>


/*
    A
    MOUSE_0 MOUSE_AXIS_0
    JOY_0 JOY_AXIS_0
    XB_0 XB_AXIS_0

*/

/*
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static nxInputHandler*ptHandler=NULL;
    switch(msg)
    {
        //static inline nxInputId nxMakeInputId(const nxInputType&dType,const unsigned short&uButtonID,const nxExtendedId&uExtended=0)
        case nxInputHandler::NX_SET_HANDLER:
        {
            ptHandler = reinterpret_cast<nxInputHandler*>(wParam);
            return 0;
        }

        case nxInputHandler::NX_GENERIC_INPUT:
        {
            // This will handle the deletion.
            nxInputData*ptInput = reinterpret_cast<nxInputData*>(wParam);
            nxInputHandler*ptHandler = reinterpret_cast<nxInputHandler*>(lParam);

            if (ptInput && ptHandler)
            {
                std::cout << "Got " << ptInput->uInputID << std::endl;
                //DebugMessage("Got generic!");
                ptHandler->FreeMessage(ptInput);
            }

            return 0;
        }
        case WM_INPUT:
        {
			RAWINPUT*priData=NULL;
			try {
				// In case we somehow magically have a null handle, we shouldn't do anything special.
				if (!lParam) break;
				UINT uSize=sizeof(RAWINPUTHEADER);
				HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lParam);

				if (GetRawInputData(hRawInput,RID_INPUT,NULL,&uSize,sizeof(RAWINPUTHEADER)) != 0)
					THROWERROR("Couldn't retrieve RAWINPUT data size for WM_INPUT message.");
				priData = (RAWINPUT*)new unsigned char[uSize];

				if (GetRawInputData(hRawInput,RID_INPUT,priData,&uSize,sizeof(RAWINPUTHEADER)) == (UINT)-1)
					THROWERROR("Couldn't retrieve RAWINPUT data for WM_INPUT message.");

				// Retrieve the device ID for the keyboard.  Will be nonzero if it's a physical keyboard.
				// If it's simulated input, it will be zero.  Same for random HID devices (and multimedia keys).
				ULONG uDeviceID = reinterpret_cast<ULONG>(priData->header.hDevice);

                switch (priData->header.dwType)
                {
                    // Keyboard input handling
                    case RIM_TYPEKEYBOARD:
                    {
                        // Get a reference to the keyboard object
                        RAWKEYBOARD&riKB = priData->data.keyboard;

                        // We don't want to do anything in the case of an overrun code.
                        if (riKB.VKey != KEYBOARD_OVERRUN_MAKE_CODE)
                        {
                            std::string strKeyText = "";

                            // The pause key, at least on my keyboard, sends an 0xFF, and then get's interpreted as LCTRL and then PAUSE
                            // or something similar.  So, I just handle the pause key as a special case, and ignore the 0xFF.
                            if (riKB.VKey == VK_PAUSE)
                            {
                                strKeyText = "Pause";
                            }
                            // Numlock, at least for me, isn't setting it's extended bit properly, and thus is being labeled as the "Pause" key.. so we set it.
                            else if (riKB.VKey == VK_NUMLOCK)
                            {
                                // Set the extended bit so it is identified properly.
                                riKB.Flags |= RI_KEY_E0;
                            }
                            // If we haven't gotten the key name already with a special case
                            if (strKeyText.empty())
                            {
                                // Size the string to a size that will definitely be large enough
                                strKeyText.resize(255);
                                // Resize the string to fit the results of getting the key's name text.  Returns 0 on error, so no problems.
                                strKeyText.resize(GetKeyNameText(riKB.MakeCode<<16 | (riKB.Flags&RI_KEY_E0?0x1<<24:0),const_cast<char*>(strKeyText.c_str()),255));
                            }

                            // If we haven't gotten a name for the key yet, then we need to construct one based on the flags and scan code.
                            // This ensures that we at least support every key in some way.
                            if (strKeyText.empty())
                            {
                                std::stringstream ss;
                                USHORT uVal = 0;
                                // Using the extended bits, create a value from 0-3 to reflect their values
                                char chExtended = "0123"[(riKB.Flags&RI_KEY_E0?0x1:0x0) | (riKB.Flags&RI_KEY_E1?0x2:0x0)];
                                // If there's a scan code, we prefer to use that
                                if (riKB.MakeCode)
                                {
                                    // Prefix
                                    ss << "KSC_";
                                    uVal = riKB.MakeCode;
                                }
                                // Otherwise, we'll use the vkey
                                else if (riKB.VKey)
                                {
                                    // Prefix
                                    ss << "KVK_";
                                    uVal = riKB.VKey;
                                }
                                // If either a scan code or a vkey were available, then we're ready to create the name.
                                // This should always happen, but if it doesnt.. I guess I won't be supporting that key, now will I?
                                if (uVal)
                                {
                                    // Name will be <prefix><extended bit value><scancode or vkey in hex>
                                    ss << chExtended << std::hex << std::uppercase << std::setfill('0') << std::setw(sizeof(USHORT)*2) << uVal;
                                    strKeyText = ss.str();
                                }
                            }
                            // If we got a textual identifier for the key
                            if (!strKeyText.empty())
                            {
                                // Make the key identifier uppercase
                                for (std::string::iterator it=strKeyText.begin();it!=strKeyText.end();++it)
                                {
                                    if (*it == ' ') *it = '_';
                                    else *it = std::toupper(*it);
                                }

                                // Dispatch the event for the keypress
                                //nxInputId = nxGenericInput::nxMakeInputId(NX_KEYBOARD,riKB.MakeCode,0);
                                //lpHandler
                                printf("%s == %u %u %u %u\n",strKeyText.c_str(),(UINT)priData->header.hDevice,priData->data.keyboard.MakeCode,priData->data.keyboard.Flags,priData->data.keyboard.VKey);
                            }
                        }
                        break;
                    }
                    // Mouse input handling (multiple dispatch points)
                    case RIM_TYPEMOUSE:
                    {
                        // Get a reference to the mouse object
                        RAWMOUSE&riMouse = priData->data.mouse;

                        // Check if the mouse has moved relatively, we are not supporting absolute mice (like touch screens)
                        if (riMouse.usFlags & MOUSE_MOVE_ABSOLUTE)
                        {
                            // TODO: Convert it to relative positioning based on the old coordinate
                            printf("Moved mouse absolute: (%ld,%ld)\n",riMouse.lLastX,riMouse.lLastY);
                        }
                        else
                        {
                            // Must be relative movement.
                            if (riMouse.lLastX || riMouse.lLastY)
                                printf("Moved mouse relative: (%ld,%ld)\n",riMouse.lLastX,riMouse.lLastY);

                        }
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN)
                        {
                            printf("Mouse 1 down.\n");
                        }
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)
                        {
                            printf("Mouse 1 up.\n");
                        }
                        // goes up to mouse5
                        if (riMouse.usButtonFlags & RI_MOUSE_WHEEL)
                        {
                            printf("Mouse wheel: %d\n",(SHORT)riMouse.usButtonData);
                        }

                        break;
                    }
                }

			} catch (std::exception&e)
			{
                DebugMessage(e.what());
			}
			// Clean up the RAWINPUT structure
			if (priData) delete[] priData;
			// We don't need to call DefWindowProc if it's a WM_INPUT message.
			return 0;
        }
    }
    // If we somehow got a message other than WM_INPUT, we'll pass it on.
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
*/




// XBOX_WHATEVERFLAG ==> ID ==> STRING
class nxXInputThread : public nxThreadProc
{
    virtual DWORD WINAPI ThreadProc(void)
    {
        nxInputHandler::GetInstance()->XInputLoop();
        return 0;
    }
};
class nxHandlerThread : public nxThreadProc
{
    virtual DWORD WINAPI ThreadProc(void)
    {
        nxInputHandler::GetInstance()->HandlerLoop();
        return 0;
    }
};
class nxRawInputThread : public nxThreadProc
{
    virtual DWORD WINAPI ThreadProc(void)
    {
        nxInputHandler::GetInstance()->RawInputLoop();
        return 0;
    }
};

LRESULT CALLBACK InputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch(msg)
    {
        case NX_GENERIC_INPUT:
        {
            // This will handle the deletion.
            nxInputData*ptInput = reinterpret_cast<nxInputData*>(wParam);
            nxInputHandler&objHandler = *nxInputHandler::GetInstance();

            if (ptInput)
            {
                std::cout << *ptInput << " " << objHandler.GetInputInfo(ptInput->uInputID)->strInputText << std::endl;
                //DebugMessage("Got generic!");
                objHandler.FreeData(ptInput);
            }

            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
// TODO: Add mouse support, add xinput support, add directinput support, remove nxGenericInput from project
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{

    XIMConnect();
    XIMSetMode(XIMModeAutoAnalogDisconnect);
    while (1)
    {
        Sleep(1000);
    }
    nxInstanceHandle(hInstance);
    /*HWND hwnd=nxCreateMessageWindow(WndProc);
    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 3: The Message Loop
    std::vector<nxRawInputDevice> vcDevices;
    nxRawInputDevice::Enumerate(vcDevices);

    for (std::vector<nxRawInputDevice>::iterator it=vcDevices.begin();it!=vcDevices.end();++it)
    {
        std::cout << "Raw device: " << (unsigned long)it->GetDeviceHandle() << " - " << it->GetGUID() << " - " << it->GetDeviceName() << std::endl;
    }
    std::cout << vcDevices.size() << std::endl;

	if (!nxRawInputDevice::RegisterKeyboardHandler(hwnd))
        DebugMessage("NOT GETTING KB MESSAGES!");
	if (!nxRawInputDevice::RegisterMouseHandler(hwnd))
        DebugMessage("NOT GETTING MOUSE MESSAGES!");*/

//    if (!XBRawInputDevice::SetFilterKeyNoRepeat())
//        DebugMessage("failboat repeat");


    nxInputHandler&objHandler = *nxInputHandler::CreateInstance(InputProc);
    nxXInputThread objXIThread;
    nxHandlerThread objHandlerThread;
    nxRawInputThread objRawInputThread;
    nxCreateThread(objXIThread);
    nxCreateThread(objHandlerThread);
    nxCreateThread(objRawInputThread);

    MSG Msg;
    while(1)
        Sleep(1000);
    return Msg.wParam;
}
