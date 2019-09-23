#include "nxInputHandler.hpp"


const WORD nxInputHandler::XInputData::xiButtonMask[nxInputHandler::XInputData::XI_NUMBUTTONS] = {
    XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT,XINPUT_GAMEPAD_START,XINPUT_GAMEPAD_BACK,
    XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB, XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
    XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y };

const unsigned char nxInputHandler::XINPUT_MAX_DEVICES = 4;
std::auto_ptr<nxInputHandler> nxInputHandler::pHandlerInstance(NULL);

std::ostream&operator<<(std::ostream&os,const nxInputData&rhs)
{
    switch (rhs.dType)
    {
        case NX_KEYBOARD: os << "Keyboard"; break;
        case NX_MOUSE: os << "Mouse"; break;
        case NX_XINPUT: os << "XInput"; break;
        case NX_DINPUT: os << "DInput"; break;
        default:
        os << "Unknown";
    }
    os << ": " << (unsigned short)rhs.uDeviceID << " " << rhs.uInputID << " " << rhs.sValue;
    return os;
}
void nxInputHandler::Create(WNDPROC proc)
{
    // Create the message window to handle the input events
    //hMsgWnd=nxCreateMessageWindow(proc);
    HandlerProc=proc;
    //hRawWnd=nxCreateMessageWindow(RawInputProc);

    uNextDevId = XINPUT_MAX_DEVICES+1; //0 is generic

    // Create the xinput ids 1-4
    for (nxDeviceId i = 1;i <= XINPUT_MAX_DEVICES;++i)
    {
        std::stringstream ss;
        ss << "XINPUT_" << i;
        mpGUIDToDevice[ss.str()] = i;
    }

    // Enumerate the raw input devices on the system
    std::vector<nxRawInputDevice> vcDevices;
    nxRawInputDevice::Enumerate(vcDevices);

    // Create device ids for RawInput
    for (std::vector<nxRawInputDevice>::iterator it=vcDevices.begin();it!=vcDevices.end();++it)
    {
        // See if the GUID already has a device id
        std::map<std::string,nxDeviceId>::iterator rit = mpGUIDToDevice.find(it->GetGUID());
        nxDeviceId uDevId = 0;
        if (rit == mpGUIDToDevice.end())
        {
            // If it doesn't, create one and add the mapping.
            uDevId = CreateDeviceId();
            mpGUIDToDevice[it->GetGUID()] = uDevId;
        }
        else // If it does, retrieve it.
            uDevId = rit->second;
        // Map the handle to the device id
        mpRawToDevice[it->GetDeviceHandle()] = uDevId;
    }
    // Create the DirectInput interface
    if (DirectInput8Create(nxInstanceHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8, ( VOID** )&pDI, NULL) != DI_OK)
        pDI = NULL;
    else
    {
        // Enumerate the devices
        pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,EnumJoysticksCallback,NULL, DIEDFL_ATTACHEDONLY );
    }
    // TODO: Here is where we read in the device aliases, and then the configuration files
}
std::string nxInputHandler::GUIDToString(const GUID&guid)
{
    std::stringstream ss;
    ss << '{';
    ss << std::hex << std::uppercase << std::setw(sizeof(guid.Data1)*2) << std::setfill('0') << (unsigned long)guid.Data1 << '-';
    ss << std::hex << std::uppercase << std::setw(sizeof(guid.Data2)*2) << std::setfill('0') << (unsigned long)guid.Data2 << '-';
    ss << std::hex << std::uppercase << std::setw(sizeof(guid.Data3)*2) << std::setfill('0') << (unsigned long)guid.Data3 << '-';
    for (unsigned char i=0;i<sizeof(guid.Data4);++i)
    {
        if (i==2)
            ss << '-';
        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (unsigned long)guid.Data4[i];
    }
    ss << '}';
    return ss.str();
}
BOOL CALLBACK nxInputHandler::EnumJoysticksCallback( const DIDEVICEINSTANCE* pInstance,VOID* ptData )
{
    // Skip XInput devices
    if (IsXInputDevice(pInstance->guidProduct))
        return DIENUM_CONTINUE;

    nxInputHandler&objHandler = *nxInputHandler::GetInstance();

    LPDIRECTINPUTDEVICE8 ptDevice;

    //  Attempt to create a device interface for the joystick
    if (objHandler.pDI->CreateDevice( pInstance->guidInstance, &ptDevice, NULL ) == DI_OK)
    {
        // Create the interface to device id mapping
        nxDeviceId uDevId = objHandler.CreateDeviceId();
        objHandler.mpDIDevice[ptDevice] = uDevId;
        // Map the GUID to the device id
        objHandler.mpGUIDToDevice[GUIDToString(pInstance->guidInstance)] = uDevId;
    }
    return DIENUM_CONTINUE;
}


bool nxInputHandler::IsXInputDevice(const GUID&guidProduct)
{
    bool bResult = false;
    if (memcmp(guidProduct.Data4,"\0\0PIDVID",8) == 0)
    {
        // Create the Vendor ID and Product ID strings
        std::string strVID, strPID;
        // Arbitrary blocks to scope the stringstream
        {
            std::stringstream ss;
            ss << "VID_" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << (guidProduct.Data1 & 0xFFFF);
            strVID = ss.str();
        }
        {
            std::stringstream ss;
            ss << "PID_" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << ((guidProduct.Data1>>16) & 0xFFFF);
            strPID = ss.str();
        }


        // Open the registry key so we can enumerate the HID devices listed in the registry
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Enum\\HID",0,KEY_ENUMERATE_SUB_KEYS,&hKey) != ERROR_SUCCESS)
            return false;

        // Maximum registry key length is 255 characters
        char ptKeyBuf[255];
        // Initialize the size of the buffer
        DWORD dwSize = sizeof(ptKeyBuf);
        // Loop through the keys
        for (DWORD dwIndex = 0;RegEnumKeyEx(hKey,dwIndex,ptKeyBuf,&dwSize,NULL,NULL,NULL,NULL) == ERROR_SUCCESS;++dwIndex,dwSize=sizeof(ptKeyBuf))
        {
            // Get uppercase string version of the key name
            std::string strKeyBuf;
            strKeyBuf.resize(dwSize);
            strKeyBuf[dwSize] = '\0';
            while (dwSize--)
                strKeyBuf[dwSize] = std::toupper(ptKeyBuf[dwSize]);

            // Check for a match
            if ((strKeyBuf.find(strVID) != std::string::npos) && // Vendor ID matches
                (strKeyBuf.find(strPID) != std::string::npos) && // Product ID matches
                (strKeyBuf.find("IG_") != std::string::npos)) // Is an XInput device
            {
                bResult = true;
                break;
            }
        }
        // Clean up the key handle
        RegCloseKey(hKey);
    }
    return bResult;
}

LRESULT CALLBACK nxInputHandler::RawInputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch(msg)
    {
        /*case nxInputHandler::NX_GENERIC_INPUT:
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
        }*/
        case WM_INPUT:
        {
            nxInputHandler&objHandler = *nxInputHandler::GetInstance();
			boost::scoped_array<unsigned char>uData(NULL);
			RAWINPUT*priData=NULL;
			try {
				// In case we somehow magically have a null handle, we shouldn't do anything special.
				if (!lParam) break;
				UINT uSize=sizeof(RAWINPUTHEADER);
				HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lParam);

				if (GetRawInputData(hRawInput,RID_INPUT,NULL,&uSize,sizeof(RAWINPUTHEADER)) != 0)
					THROWERROR("Couldn't retrieve RAWINPUT data size for WM_INPUT message.");
                uData.reset(new unsigned char[uSize]);
                priData = reinterpret_cast<RAWINPUT*>(uData.get());


				if (GetRawInputData(hRawInput,RID_INPUT,priData,&uSize,sizeof(RAWINPUTHEADER)) == (UINT)-1)
					THROWERROR("Couldn't retrieve RAWINPUT data for WM_INPUT message.");

				// Retrieve the device ID for the keyboard.  Will be nonzero if it's a physical keyboard.
				// If it's simulated input, it will be zero.  Same for random HID devices (and multimedia keys).


				nxDeviceId uDeviceId = objHandler.GetRawDeviceId(priData->header.hDevice);

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
                            bool isVKey = false;

                            // The pause key, at least on my keyboard, sends an 0xFF, and then get's interpreted as LCTRL and then PAUSE
                            // or something similar.  So, I just handle the pause key as a special case, and ignore the 0xFF.
                            switch (riKB.VKey)
                            {
                                case VK_PAUSE:
                                case VK_NUMLOCK:
                                    isVKey = true;
                            }

                            // Create an input id for this information
                            nxInputId uInputId = MakeInputId(NX_KEYBOARD,(isVKey?(unsigned short)riKB.VKey:(unsigned short)riKB.MakeCode),(isVKey?1:0));

                            // If we haven't gotten the key name already with a special case
                            nxInputInfo*pInputInfo=objHandler.GetInputInfo(uInputId);
                            if (!pInputInfo)
                            {
                                std::string strKeyText = "";
                                if (isVKey)
                                {
                                    switch (riKB.VKey)
                                    {
                                        case VK_PAUSE: strKeyText = "PAUSE"; break;
                                        case VK_NUMLOCK: strKeyText = "NUM_LOCK"; break;
                                    }
                                }
                                else
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
                                    // Add the input information
                                    pInputInfo=objHandler.AddInputInfo(uInputId,strKeyText);
                                }
                            }

                            if (pInputInfo)
                            {
                                objHandler.SendData(objHandler.AllocData()->Set(NX_KEYBOARD,uDeviceId,uInputId,(priData->data.keyboard.Flags&0x1?false:true)));
                                //printf("%s == %u %u %u %u\n",strKeyText.c_str(),(UINT)priData->header.hDevice,priData->data.keyboard.MakeCode,priData->data.keyboard.Flags,priData->data.keyboard.VKey);
                            }
                        }
                        break;
                    }
                    // Mouse input handling (multiple dispatch points)
                    case RIM_TYPEMOUSE:
                    {
                        // Get a reference to the mouse object
                        RAWMOUSE&riMouse = priData->data.mouse;

                        DataSender objSender(NX_MOUSE,uDeviceId);

                        // Check if the mouse has moved relatively, we are not supporting absolute mice (like touch screens)
                        if (riMouse.usFlags & MOUSE_MOVE_ABSOLUTE)
                        {
                            // TODO: Convert it to relative positioning based on the old coordinate
                            //printf("Moved mouse absolute: (%ld,%ld)\n",riMouse.lLastX,riMouse.lLastY);
                        }
                        else
                        {
                            // Must be relative movement.
                            if (riMouse.lLastX)
                                objSender.SendData(0,true,riMouse.lLastX);
                            if (riMouse.lLastY)
                                objSender.SendData(1,true,riMouse.lLastY);
                        }
                        // Wheel will be axis 2
                        if (riMouse.usButtonFlags & RI_MOUSE_WHEEL)
                            objSender.SendData(2,true,(SHORT)riMouse.usButtonData);


                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN)
                            objSender.SendData(0,false,1);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)
                            objSender.SendData(0,false,0);

                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN)
                            objSender.SendData(1,false,1);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP)
                            objSender.SendData(1,false,0);

                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN)
                            objSender.SendData(2,false,1);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP)
                            objSender.SendData(2,false,0);

                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
                            objSender.SendData(3,false,1);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
                            objSender.SendData(3,false,0);

                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
                            objSender.SendData(4,false,1);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
                            objSender.SendData(4,false,0);
                        break;
                    }
                }

			} catch (std::exception&e)
			{
                DebugMessage(e.what());
			}

			// We don't need to call DefWindowProc if it's a WM_INPUT message.
			return 0;
        }
    }
    // If we somehow got a message other than WM_INPUT, we'll pass it on.
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void nxInputHandler::DataSender::SendData(const unsigned short&uButtonId,const bool&IsAxis,const short&sValue,const nxDeviceId uDevId)
{
    nxInputId uInputId = MakeInputId(dType,uButtonId,(IsAxis?1:0));
    if (!objHandler.GetInputInfo(uInputId))
    {
        std::stringstream ss;
        switch (dType)
        {
            case NX_MOUSE:
                ss << "MOUSE_"; break;
            case NX_XINPUT:
                ss << "XI_"; break;
            case NX_DINPUT:
                ss << "DI_"; break;
            default:
                break;
        }
        if (IsAxis)
            ss << "AXIS_";
        ss << uButtonId;
        objHandler.AddInputInfo(uInputId,ss.str(),IsAxis);
    }
    objHandler.SendData(objHandler.AllocData()->Set(dType,(uDevId?uDevId:uDeviceId),uInputId,sValue));
}

void nxInputHandler::XInputData::Set(const XINPUT_GAMEPAD&objPad)
{
    const WORD &wButtons = objPad.wButtons;
    for (unsigned char i=0;i<XI_NUMBUTTONS;++i)
    {
        bButtons[i] = (wButtons&xiButtonMask[i]?1:0);
    }
    bAxis[XI_AXIS_LX] = objPad.sThumbLX;
    bAxis[XI_AXIS_LY] = objPad.sThumbLY;
    bAxis[XI_AXIS_RX] = objPad.sThumbRX;
    bAxis[XI_AXIS_RY] = objPad.sThumbRY;

    //scales from [0,0xFF to [0,0x7FFF]  (aka, 0 to max of signed short)
    bAxis[XI_AXIS_LTRIGGER] = (static_cast<short>(objPad.bLeftTrigger)*0x101)>>1;
    bAxis[XI_AXIS_RTRIGGER] = (static_cast<short>(objPad.bRightTrigger)*0x101)>>1;
}

bool nxInputHandler::XInputDevice::GetState(void)
{
    XINPUT_STATE objState;
    XINPUT_GAMEPAD &objPad = objState.Gamepad;
    if (XInputGetState(uDeviceID,&objState) != ERROR_SUCCESS)
        return false;
    if (objState.dwPacketNumber == dwPacket)
        return false;
    objPrev = objData;
    objData.Set(objPad);
    dwPacket = objState.dwPacketNumber;
    return true;
}

bool nxInputHandler::XInputLoop(DWORD dwMilliseconds)
{
    if (bXInputRunning) return false;
    std::vector<XInputDevice> vcDevices;
    DataSender objSender(NX_XINPUT,0);
    for (unsigned char i=0;i<XINPUT_MAX_DEVICES;++i)
        vcDevices.push_back(XInputDevice(i));

    bXInputRunning=true;
    while (bXInputRunning)
    {
        for (unsigned char i=0;i<XINPUT_MAX_DEVICES;++i)
        {
            XInputDevice &objDevice = vcDevices[i];
            if (objDevice.GetState())
            {
                XInputData &objData=objDevice.objData;
                XInputData &objPrev=objDevice.objPrev;
                for (unsigned char j=0;j<XInputData::XI_NUMBUTTONS;++j)
                {
                    if (objData.bButtons[j] != objPrev.bButtons[j])
                        objSender.SendData(j,false,objData.bButtons[j],i+1);
                }
                for (unsigned char j=0;j<XInputData::XI_NUMAXIS;++j)
                {
                    if (objData.bAxis[j] != objPrev.bAxis[j])
                        objSender.SendData(j,true,objData.bAxis[j],i+1);
                }
            }
        }
        Sleep(dwMilliseconds);
    }
    return true;
}
bool nxInputHandler::RawInputLoop(bool bCaptureMouse)
{
    MSG Msg;
    if (bRawInputRunning) return false;
    bRawInputRunning=true;
    hRawWnd=nxCreateMessageWindow(RawInputProc);
    nxRawInputDevice::RegisterKeyboardHandler(hRawWnd);
    nxRawInputDevice::RegisterMouseHandler(hRawWnd,bCaptureMouse);
    while(bRawInputRunning && (GetMessage(&Msg, hRawWnd, 0, 0) > 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    nxRawInputDevice::RemoveKeyboardHandler(hRawWnd);
    nxRawInputDevice::RemoveMouseHandler(hRawWnd);
    DestroyWindow(hRawWnd);
    hRawWnd=NULL;
    return true;

}
bool nxInputHandler::HandlerLoop()
{
    MSG Msg;
    if (bHandlerRunning) return false;
    bHandlerRunning=true;
    hMsgWnd=nxCreateMessageWindow(HandlerProc);
    while(bHandlerRunning && (GetMessage(&Msg, hMsgWnd, 0, 0) > 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    DestroyWindow(hMsgWnd);
    hMsgWnd=NULL;
    return true;
}
