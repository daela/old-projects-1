#include "nxInputManager.hpp"

#if defined(OS_WINDOWS_)

#include "nxLog.hpp"

const short nxInputManager::INPUT_MIN=-32767;
const short nxInputManager::INPUT_MAX=32767;
void nxInputManager::Prepare(bool bRawInput,bool bDInput,bool bXInput)
{
    PostEvent(NX_INPUT_ENUMERATE,0,0);
    /// Create XINPUT devices
    if (bXInput)
    {
        for (DWORD i = 0;i < nxXInputDevice::XI_MAXDEVICES;++i)
        {
            nxDeviceId uDevId = CreateDeviceId();
            std::stringstream ss;
            ss << "XINPUT_" << (i+1);
            // Map the handle/index to the device id
            mpHandleId[i] = uDevId;
            // Map the "GUID" to the device id
            SetGUIDMapping(uDevId,ss.str());
        }
    }
    /// Create RAWINPUT devices
    if (bRawInput)
    {
        // Enumerate the raw input devices on the system
        std::vector<nxRawInputDevice> vcDevices;
        nxRawInputDevice::Enumerate(vcDevices);
        for (std::vector<nxRawInputDevice>::iterator it=vcDevices.begin();it!=vcDevices.end();++it)
        {
            // See if the GUID already has a device id
            nxDeviceId uDevId;
            const nxDeviceId*ptDevId=srDeviceGUID.LookupRight(it->GetGUID());
            // If the guid already has a device id, get it
            if (ptDevId)
            {
                uDevId = *ptDevId;
            }
            else // Otherwise, make one.
            {
                // If it doesn't, create one and add the mapping.
                uDevId = CreateDeviceId();
                SetGUIDMapping(uDevId,it->GetGUID());
            }

            // Map the handle to the device id
            mpHandleId[it->GetDeviceHandle()] = uDevId;
        }
    }
    /// Create DIRECTINPUT devices
    if (bDInput)
    {
        // Try to create a directinput interface if it doesn't exist already

        if (DirectInput8Create(nxInstanceHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8, ( VOID** )&pDI, NULL) != DI_OK)
            pDI = NULL;
        else
        {
            // Enumerate the devices
            pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,EnumJoysticksCallback,NULL, DIEDFL_ATTACHEDONLY );
        }
    }
    PostEvent(NX_INPUT_READY,0,0);
}
void nxInputManager::Cleanup(void)
{
    uNextDevId = 0;
    srDeviceGUID.Clear();
    srInputLabel.Clear();
    mpHandleId.clear();

    if (pDI)
    {
        for (std::set<LPDIRECTINPUTDEVICE8>::iterator it=stDInputDevices.begin();it!=stDInputDevices.end();++it)
            (*it)->Release();
        pDI->Release();
        pDI = NULL;
    }
    stDInputDevices.clear();
}

std::string nxInputManager::GUIDToString(const GUID&guid)
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

BOOL CALLBACK nxInputManager::EnumJoysticksCallback( const DIDEVICEINSTANCE* pInstance,VOID* ptData )
{
    // Skip XInput devices
    if (nxXInputDevice::IsXInputDevice(pInstance->guidProduct))
        return DIENUM_CONTINUE;

    nxInputManager&objManager = nxInputManager::instance();

    LPDIRECTINPUTDEVICE8 ptDevice;
    //  Attempt to create a device interface for the joystick
    if (objManager.pDI->CreateDevice( pInstance->guidInstance, &ptDevice, NULL ) == DI_OK)
    {
        /*TODO: Set the range for axis and buttons
        DIPROPRANGE diPropRange;
        diPropRange.diph.dwSize = sizeof(DIPROPRANGE);
        diPropRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);*/


        // Set the device parameters
        if( ptDevice->SetDataFormat( &c_dfDIJoystick ) != DI_OK ||
            ptDevice->SetCooperativeLevel(NULL,DISCL_BACKGROUND|DISCL_NONEXCLUSIVE) != DI_OK ||
            ptDevice->EnumObjects(EnumObjectsCallback,reinterpret_cast<VOID*>(ptDevice),DIDFT_AXIS) != DI_OK)
        {
            ptDevice->Release();
        }
        else
        {
            // Create the interface to device id mapping
            nxDeviceId uDevId = objManager.CreateDeviceId();
            // Add the handle to device id mapping
            objManager.mpHandleId[ptDevice] = uDevId;
            // Add the device to the list of DInput devices (used for polling)
            objManager.stDInputDevices.insert(ptDevice);
            // Map the GUID to the device id
            objManager.SetGUIDMapping(uDevId,GUIDToString(pInstance->guidInstance));
        }
    }
    return DIENUM_CONTINUE;
}

BOOL CALLBACK nxInputManager::EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext )
{
    //MAKEWORD(low,high)

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    LPDIRECTINPUTDEVICE8 ptDevice = reinterpret_cast<LPDIRECTINPUTDEVICE8>(pContext);
    if( ptDevice && pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg;
        diprg.diph.dwSize = sizeof( DIPROPRANGE );
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin = INPUT_MIN;
        diprg.lMax = INPUT_MAX;
        // Set the range for the axis
        if( ptDevice->SetProperty( DIPROP_RANGE, &diprg.diph ) != DI_OK )
            return DIENUM_STOP;

    }
    return DIENUM_CONTINUE;
}
LRESULT CALLBACK nxInputManager::RawInputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch(msg)
    {
        case WM_INPUT:
        {
            nxInputManager&objManager = nxInputManager::instance();
			unsigned char*ptData = NULL;
			RAWINPUT*priData=NULL;
			try {
				// In case we somehow magically have a null handle, we shouldn't do anything special.
				if (!lParam) break;
				UINT uSize=sizeof(RAWINPUTHEADER);
				HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lParam);

				if (GetRawInputData(hRawInput,RID_INPUT,NULL,&uSize,sizeof(RAWINPUTHEADER)) != 0)
					nxThrow("Couldn't retrieve RAWINPUT data size for WM_INPUT message.");
                ptData = new unsigned char[uSize];
                priData = reinterpret_cast<RAWINPUT*>(ptData);


				if (GetRawInputData(hRawInput,RID_INPUT,priData,&uSize,sizeof(RAWINPUTHEADER)) == (UINT)-1)
					nxThrow("Couldn't retrieve RAWINPUT data for WM_INPUT message.");

				// Retrieve the device ID for the keyboard.  Will be nonzero if it's a physical keyboard.
				// If it's simulated input, it will be zero.  Same for random HID devices (and multimedia keys).


				nxDeviceId uDeviceId = objManager.GetInputHandleId(priData->header.hDevice);

                switch (priData->header.dwType)
                {
                    // Keyboard input handling
                    case RIM_TYPEKEYBOARD:
                    {
                        // Get a reference to the keyboard object
                        RAWKEYBOARD&riKB = priData->data.keyboard;

                        // We don't want to do anything in the case of an overrun code, or if it's not a keydown or keyup message
                        if (riKB.VKey != KEYBOARD_OVERRUN_MAKE_CODE && (riKB.Message == WM_KEYDOWN || riKB.Message == WM_KEYUP))
                        {
                            // If there's no make code specified, we'll be looking at the vkey
                            bool isVKey = (riKB.MakeCode?false:true);

                            // The pause key, at least on my keyboard, sends an 0xFF, and then get's interpreted as LCTRL and then PAUSE
                            // or something similar.  So, I just handle the pause key as a special case, and ignore the 0xFF.
                            // There's an additional error where the numlock key is identified as the pause key (imagine that...)
                            // So, in these cases I definitely wants to categorize the key by its VKey.
                            switch (riKB.VKey)
                            {
                                case VK_PAUSE:
                                case VK_NUMLOCK:
                                    isVKey = true;
                            }

                            // Create an input id for this information
                            nxInputId uInputId = MakeInputId(NX_KEYBOARD,(isVKey?(unsigned short)riKB.VKey:(unsigned short)riKB.MakeCode),false,(isVKey?0x1u:0x0u) | (riKB.Flags&RI_KEY_E0?0x2u:0x0u) | (riKB.Flags&RI_KEY_E1?0x4u:0x0u));

                            // If we haven't gotten the key name at some time prior
                            //nxInputInfo*pInputInfo=objManager.GetInputInfo(uInputId);
                            bool bIsText = (objManager.srInputLabel.LookupLeft(uInputId)?true:false);
                            if (!bIsText)
                            {
                                std::string strKeyText = "";
                                // If it's one of our special cases, set the text
                                if (isVKey)
                                {
                                    switch (riKB.VKey)
                                    {
                                        case VK_PAUSE: strKeyText = "PAUSE"; break;
                                        case VK_NUMLOCK: strKeyText = "NUM_LOCK"; break;
                                    }

                                }
                                // if it wasn't one of our special cases
                                if (strKeyText.empty())
                                {
                                    // If the key isn't defined by it's VKey, we try to get the key name from the key's scan code and extended bit
                                    if (!isVKey)
                                    {
                                        // Size the string to a size that will definitely be large enough
                                        strKeyText.resize(255);
                                        // Resize the string to fit the results of getting the key's name text.  Returns 0 on error, so no problems.
                                        strKeyText.resize(GetKeyNameText(riKB.MakeCode<<16 | (riKB.Flags&RI_KEY_E0?0x1<<24:0),const_cast<char*>(strKeyText.c_str()),255));
                                    }
                                    // If we haven't gotten a name for the key yet, then we need to construct one based on the flags and the scan code or vkey
                                    // This ensures that we at least support every key in some way.
                                    if (strKeyText.empty())
                                    {
                                        std::stringstream ss;
                                        USHORT uVal = 0;
                                        // Using the extended bits, create a value from 0-3 to reflect their values
                                        char chExtended = "0123"[(riKB.Flags&RI_KEY_E0?0x1:0x0) | (riKB.Flags&RI_KEY_E1?0x2:0x0)];

                                        // If the key is defined by the scan code, make a name using it.
                                        if (!isVKey)
                                        {
                                            // Prefix
                                            ss << "KSC_";
                                            uVal = riKB.MakeCode;
                                        }
                                        // Otherwise, we'll use the vkey if the vkey exists
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
                                    if (strKeyText==";")
                                        strKeyText = "SEMICOLON";
                                    else if (strKeyText==":")
                                        strKeyText = "COLON";
                                    else if (strKeyText==",")
                                        strKeyText = "COMMA";
                                    else if (strKeyText=="'")
                                        strKeyText = "QUOTE";
                                    else if (strKeyText == "/")
                                        strKeyText = "SLASH";
                                    else if (strKeyText == "\\")
                                        strKeyText = "BACKSLASH";
                                    else if (strKeyText == ".")
                                        strKeyText = "PERIOD";
                                    else if (strKeyText == "=")
                                        strKeyText = "EQUALS";
                                    else if (strKeyText == "-")
                                        strKeyText = "MINUS";
                                    else if (strKeyText == "`")
                                        strKeyText = "ACUTE";
                                    else if (strKeyText == "[")
                                        strKeyText = "LBRACK";
                                    else if (strKeyText == "]")
                                        strKeyText = "RBRACK";
                                    objManager.AddInputLabel(uInputId,strKeyText);
                                    bIsText = true;
                                }
                            }

                            if (bIsText)
                                objManager.PostEvent(NX_INPUT_RECEIVED,MakeMessage(uInputId,uDeviceId),(priData->data.keyboard.Flags&RI_KEY_BREAK?0:INPUT_MAX));
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
                            objSender.SendData(0,false,INPUT_MAX);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)
                            objSender.SendData(0,false,0);

                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN)
                            objSender.SendData(1,false,INPUT_MAX);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP)
                            objSender.SendData(1,false,0);

                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN)
                            objSender.SendData(2,false,INPUT_MAX);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP)
                            objSender.SendData(2,false,0);

                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
                            objSender.SendData(3,false,INPUT_MAX);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
                            objSender.SendData(3,false,0);

                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
                            objSender.SendData(4,false,INPUT_MAX);
                        if (riMouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
                            objSender.SendData(4,false,0);
                        break;
                    }
                }

			} catch (std::exception&e)
			{
                nxLog << e.what() << std::endl;
			}
            delete[] ptData;
			// We don't need to call DefWindowProc if it's a WM_INPUT message.
			return 0;
        }
    }
    // If we somehow got a message other than WM_INPUT, we'll pass it on.
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void nxInputManager::DataSender::SendData(const unsigned short&uButtonId,const bool&IsAxis,const short&sValue,const nxDeviceId&uDevId,const nxExtendedId&uExtended)
{
    nxInputId uInputId = MakeInputId(dType,uButtonId,IsAxis,uExtended);
    if (!objManager.srInputLabel.LookupLeft(uInputId))
    {
        nxScopedRecursiveLock objLock(objManager.csDataSenderSendData);
        // Be sure it wasn't added while waiting for the lock
        if (!objManager.srInputLabel.LookupLeft(uInputId))
        {
            std::stringstream ss;
            switch (dType)
            {
                case NX_MOUSE:
                    ss << "MOUSE_"; break;
                case NX_XINPUT:
                    ss << "XI_"; break;
                case NX_DINPUT:
                    ss << "DI_";
                    if (uExtended)
                        ss << "POV_";
                    break;
                default:
                    break;
            }
            if (IsAxis)
                ss << "AXIS_";

            ss << uButtonId;
            objManager.AddInputLabel(uInputId,ss.str());
        }
    }
    objManager.PostEvent(NX_INPUT_RECEIVED,MakeMessage(uInputId,(uDevId?uDevId:uDeviceId)),sValue);
    //objHandler.SendData(objHandler.AllocData()->Set(dType,(uDevId?uDevId:uDeviceId),uInputId,sValue));
}

DWORD WINAPI nxInputManager::MessageThread(LPVOID lpParameter)
{
    nxInputManager&objManager=nxInputManager::instance();
    objManager.MessageLoop((WNDPROC)lpParameter);
    return 0;
}
void nxInputManager::MessageLoop(WNDPROC lpfnProc)
{
    MSG Msg;
    hManager=nxCreateMessageWindow(lpfnProc);
    while(isRunning && (GetMessage(&Msg, hManager, 0, 0) > 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    DestroyWindow(hManager);
    hManager=NULL;
}

DWORD WINAPI nxInputManager::RawInputThread(LPVOID lpParameter)
{
    nxInputManager&objManager=nxInputManager::instance();
    objManager.RawInputLoop((bool)lpParameter);
    return 0;
}
void nxInputManager::RawInputLoop(bool bCaptureMouse)
{
    MSG Msg;
    hRawInput=nxCreateMessageWindow(RawInputProc);
    nxRawInputDevice::RegisterKeyboardHandler(hRawInput);
    nxRawInputDevice::RegisterMouseHandler(hRawInput,bCaptureMouse);
    while(isRunning && (GetMessage(&Msg, hRawInput, 0, 0) > 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    nxRawInputDevice::RemoveKeyboardHandler(hRawInput);
    nxRawInputDevice::RemoveMouseHandler(hRawInput);
    DestroyWindow(hRawInput);
    hRawInput=NULL;
}

DWORD WINAPI nxInputManager::XInputThread(LPVOID lpParameter)
{
    nxInputManager&objManager=nxInputManager::instance();
    objManager.XInputLoop(reinterpret_cast<DWORD>(lpParameter));
    return 0;
}
void nxInputManager::XInputLoop(DWORD dwMilliseconds)
{
    std::vector<nxXInputDevice> vcDevices;
    std::vector<nxDeviceId> vcDeviceIds;
    DataSender objSender(NX_XINPUT,0);
    for (DWORD i=0;i<nxXInputDevice::XI_MAXDEVICES;++i)
    {
        vcDevices.push_back(nxXInputDevice(i));
        vcDeviceIds.push_back(GetInputHandleId(i));
    }

    while (isRunning)
    {
        for (DWORD i=0;i<nxXInputDevice::XI_MAXDEVICES;++i)
        {
            nxXInputDevice &objDevice = vcDevices[i];
            nxDeviceId&uDeviceId = vcDeviceIds[i];
            if (objDevice.GetState())
            {
                nxXInputData &objData=objDevice.objData;
                nxXInputData &objPrev=objDevice.objPrev;
                for (unsigned char j=0;j<nxXInputData::XI_NUMBUTTONS;++j)
                {
                    if (objData.bButtons[j] != objPrev.bButtons[j])
                        objSender.SendData(j,false,(objData.bButtons[j]?INPUT_MAX:0),uDeviceId);
                }
                for (unsigned char j=0;j<nxXInputData::XI_NUMAXIS;++j)
                {
                    if (objData.bAxis[j] != objPrev.bAxis[j])
                        objSender.SendData(j,true,objData.bAxis[j],uDeviceId);
                }
            }
        }
        Sleep(dwMilliseconds);
    }
}

DWORD WINAPI nxInputManager::DInputThread(LPVOID lpParameter)
{
    nxInputManager&objManager=nxInputManager::instance();
    objManager.DInputLoop(reinterpret_cast<DWORD>(lpParameter));
    return 0;
}
#define IS_POV_CENTERED(pov) ((BOOL)(LOWORD(pov) == 0xFFFF))
void nxInputManager::DInputLoop(DWORD dwMilliseconds)
{
    std::map<LPDIRECTINPUTDEVICE8,DIJOYSTATE> mpStates;

    DIJOYSTATE idleState;
    ZeroMemory(&idleState,sizeof(DIJOYSTATE));

    DataSender objSender(NX_DINPUT,0);
    while (isRunning)
    {
        for (std::set<LPDIRECTINPUTDEVICE8>::iterator it = stDInputDevices.begin();it!=stDInputDevices.end();++it)
        {
            LPDIRECTINPUTDEVICE8 ptDevice = *it;
            nxDeviceId uDeviceId = GetInputHandleId(ptDevice);
            HRESULT hr = ptDevice->Poll();
            if (FAILED(hr))
            {
                // Acquire and try again
                ptDevice->Acquire();
                hr = ptDevice->Poll();
            }
            // If it failed again, proceed to the next device
            if (FAILED(hr))
                continue;

            // Process the data
            DIJOYSTATE newState;
            if (FAILED( hr = ptDevice->GetDeviceState( sizeof( DIJOYSTATE ), &newState ) ))
                continue;

            // Get the old state, creating it if it doesn't exist.
            DIJOYSTATE&oldState = mpStates.insert(std::pair<LPDIRECTINPUTDEVICE8,DIJOYSTATE>(ptDevice,idleState)).first->second;

            // Find the differences

            // Axis
            if (oldState.lX != newState.lX)
                objSender.SendData(0,true,newState.lX,uDeviceId);
            if (oldState.lY != newState.lY)
                objSender.SendData(1,true,newState.lY,uDeviceId);
            if (oldState.lZ != newState.lZ)
                objSender.SendData(2,true,newState.lZ,uDeviceId);

            if (oldState.lRx != newState.lRx)
                objSender.SendData(3,true,newState.lRx,uDeviceId);
            if (oldState.lRy != newState.lRy)
                objSender.SendData(4,true,newState.lRy,uDeviceId);
            if (oldState.lRz != newState.lRz)
                objSender.SendData(5,true,newState.lRz,uDeviceId);

            if (oldState.rglSlider[0] != newState.rglSlider[0])
                objSender.SendData(6,true,newState.rglSlider[0],uDeviceId);
            if (oldState.rglSlider[1] != newState.rglSlider[1])
                objSender.SendData(7,true,newState.rglSlider[1],uDeviceId);

            // POV
            for (unsigned char i=0;i<4;++i)
            {
                // Change the value to be in tenths of a degree instead of hundredths
                if (!IS_POV_CENTERED(newState.rgdwPOV[i]))
                    newState.rgdwPOV[i] /= 10;

                if (oldState.rgdwPOV[i] != newState.rgdwPOV[i])
                    // The value is in tenths of a degree clockwise from north. -1 = center, 0 = top, 900 = right 1800 = bottom 2700 = left
                    objSender.SendData(i,false,newState.rgdwPOV[i],uDeviceId,1); //extended of 1 means POV
            }
            // Buttons
            for (int i=0;i<32;++i)
            {
                if ((oldState.rgbButtons[i] & 0x80) != (newState.rgbButtons[i] & 0x80))
                        objSender.SendData(i,false,(newState.rgbButtons[i] & 0x80?INPUT_MAX:0),uDeviceId);
            }
            oldState = newState;
        }
        Sleep(dwMilliseconds);
    }
}

bool nxInputManager::Start(WNDPROC lpfnMessageProc,bool bRawInput,bool bDInput,bool bXInput,DWORD dwMilliseconds)
{
    nxScopedRecursiveLock objGuard(csThreadManagement);
    // If it's already running, or no input interfaces are chosen (meaning, there will be no messages/devices to handle).. return false
    if (isRunning || (!bRawInput && !bDInput && !bXInput))
        return false;
    isRunning=true;
    vcThreads.push_back(nxCreateThread(MessageThread,(LPVOID)lpfnMessageProc));
    // Wait for the input manager to be created
    while (!hManager)
        Sleep(1);
    // Enumerate and obtain information for all devices
    Prepare(bRawInput,bDInput,bXInput);
    if (bRawInput)
        vcThreads.push_back(nxCreateThread(RawInputThread,(LPVOID)false));
    if (bDInput)
        vcThreads.push_back(nxCreateThread(DInputThread,(LPVOID)dwMilliseconds));
    if (bXInput)
        vcThreads.push_back(nxCreateThread(XInputThread,(LPVOID)dwMilliseconds));
    return true;
}

void nxInputManager::Stop(void)
{
    nxScopedRecursiveLock objGuard(csThreadManagement);
    if (!isRunning) return;
    // Set the stop flag
    isRunning=false;
    // Send a message to wake up the raw input thread and the message thread, both of which are blocking until a message arrives
    // This allows them to see the stop flag has been set.
    if (hRawInput)
        PostMessage(hRawInput,NX_INPUT_AWAKEN,0,0);
    if (hManager)
        PostMessage(hManager,NX_INPUT_AWAKEN,0,0);
    // Wait for the threads to close as a result of the stop flag being set.
    if (!vcThreads.empty())
        WaitForMultipleObjects(vcThreads.size(),&vcThreads[0],TRUE,INFINITE);
    // Close the thread handles, as the threads have already terminated.
    for (std::vector<HANDLE>::iterator it=vcThreads.begin();it!=vcThreads.end();++it)
        CloseHandle(*it);
    // Clear the thread list
    vcThreads.clear();
    // Release any devices/cleanup.
    Cleanup();

}

#endif
