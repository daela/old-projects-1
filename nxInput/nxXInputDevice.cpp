#include "nxXInputDevice.hpp"

#if defined(OS_WINDOWS_)

const WORD nxXInputData::xiButtonMask[nxXInputData::XI_NUMBUTTONS] = {
    XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT,XINPUT_GAMEPAD_START,XINPUT_GAMEPAD_BACK,
    XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB, XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
    XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y };


void nxXInputData::Set(const XINPUT_GAMEPAD&objPad)
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

    // Map a value from the range [0x00,0xFF] into the range [0x00,0x7FFF]
    // Same result as the floating point operation:  (value/0xFF)*0x7FFF
    // ...except this rounds .5 and greater up to the next number, and is faster.
    //(static_cast<short>(ch)*0x101u)>>0x1u;
    bAxis[XI_AXIS_LTRIGGER] = (static_cast<short>(objPad.bLeftTrigger)*0x101u)>>0x1u;
    bAxis[XI_AXIS_RTRIGGER] = (static_cast<short>(objPad.bRightTrigger)*0x101u)>>0x1u;
}



bool nxXInputDevice::GetState(void)
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

bool nxXInputDevice::IsXInputDevice(const GUID&guidProduct)
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
#endif
