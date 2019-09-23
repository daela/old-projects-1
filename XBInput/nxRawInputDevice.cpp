
#include "nxRawInputDevice.hpp"

// Need to perfect the structure before making this happen

bool nxRawInputDevice::GetInformation(void)
{
    HKEY hKey = NULL;
    try
    {
        /// Retrieve the encoded device name
        UINT uSize = 0;
        // Retrieve the number of characters needed to store the device name (including the null)
        // Calling the unicode version, that way we get the number of characters properly.
        // All the characters used, in practice, are representable in ANSI.
        if (GetRawInputDeviceInfoW(hDevice,RIDI_DEVICENAME,NULL,&uSize) < 0)
            THROWERROR("Couldn't retrieve length of device name string.");

        // Allocate the space
        strDeviceName.resize(uSize-1);
        // Retrieve the device name.  Example: "\\??\\ACPI#PNP0303#3&13c0b0c5&0#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}"
        if (GetRawInputDeviceInfoA(hDevice,RIDI_DEVICENAME,const_cast<char*>(strDeviceName.c_str()),&uSize) < 0)
            THROWERROR("Couldn't retrieve the encoded device name.");

        /// Multiply Occurring Error.  Placing here so we can include the device name.
        const std::string strFormatError = std::string("Device name is encoded in an unsupported format. \"") + strDeviceName + std::string("\"");
        // Validate the device name is nonempty.
        if (strDeviceName.empty())
                THROWERROR(strFormatError);

        /// Split the device name into parts
        // Delimiter offsets
        std::string::size_type uCurrent, uNext;
        // First # position
        uNext = strDeviceName.find_first_of('#');
        // Position of the trailing slash of the beginning "\\??\\" part.
        uCurrent = strDeviceName.find_last_of('\\',uNext);

        /// Loop through the fields
        std::string strField;
        for (UINT uField = 0;uField < 4;++uField)
        {
            // Validate field offsets
            if (uCurrent == std::string::npos)
                THROWERROR(strFormatError);
            // Pull out the field data
            if (uNext != std::string::npos)
                strField = strDeviceName.substr(uCurrent+1,uNext-uCurrent-1);
            else
                strField = strDeviceName.substr(uCurrent+1);

            // Ensure some value was retrieved
            if (strField.empty())
                THROWERROR(strFormatError);

            /// Set the appropriate field
            switch (uField)
            {
                case 0:	strClassCode = strField; break;
                case 1:	strSubClassCode = strField; break;
                case 2:	strProtocolCode = strField; break;
                case 3:	strGUID = strField; break;
                default:
                    THROWERROR("Unhandled token index, notify the developer of his silly mistake.");
            }
            // Advance to the next token.
            uCurrent = uNext;
            if (uCurrent != std::string::npos)
                uNext = strDeviceName.find_first_of('#',uCurrent+1);
            else
                uNext = std::string::npos;
        }

        /// Retrieve the device description and class from the registry
        DWORD dwKeyType;
        DWORD dwKeySize;
        std::string strRegKey = std::string("SYSTEM\\CurrentControlSet\\Enum\\")+strClassCode+"\\"+strSubClassCode+"\\"+strProtocolCode;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,strRegKey.c_str(),0,KEY_QUERY_VALUE,&hKey) != ERROR_SUCCESS)
        {
            hKey = NULL;
            THROWERROR(std::string("Couldn't open the device's registry key - ") + strRegKey);
        }
        // Retrieve data type and size (including null)
        if (RegQueryValueEx(hKey,"DeviceDesc",NULL,&dwKeyType,NULL,&dwKeySize) != ERROR_SUCCESS)
            THROWERROR(std::string("Couldn't retrieve size of \"DeviceDesc\" value for registry key - ") + strRegKey);
        // Size the buffer accordingly
        strDeviceDesc.resize(dwKeySize-1);
        // Added the const_cast for clarity, though it isn't necessary.
        if (RegQueryValueEx(hKey,"DeviceDesc",NULL,&dwKeyType,(BYTE*)const_cast<char*>(strDeviceDesc.c_str()),&dwKeySize) != ERROR_SUCCESS)
            THROWERROR(std::string("Couldn't retrieve value of \"DeviceDesc\" for registry key - ") + strRegKey);

        // Retrieve data type and size (including null)
        if (RegQueryValueEx(hKey,"Class",NULL,&dwKeyType,NULL,&dwKeySize) != ERROR_SUCCESS)
            THROWERROR(std::string("Couldn't retrieve size of \"Class\" value for registry key - ") + strRegKey);
        // Size the buffer accordingly
        strDeviceClass.resize(dwKeySize-1);
        // Added the const_cast for clarity, though it isn't necessary.
        if (RegQueryValueEx(hKey,"Class",NULL,&dwKeyType,(BYTE*)const_cast<char*>(strDeviceClass.c_str()),&dwKeySize) != ERROR_SUCCESS)
            THROWERROR(std::string("Couldn't retrieve value of \"Class\" for registry key - ") + strRegKey);

        /// Use registry information to verify our keyboards/mice are in fact keyboards and mice.
        // This will also turn the terminal services "system" devices to HID
        if (stricmp(strClassCode.c_str(),"root")==0)
            dwType = RIM_TYPEHID;
        else if (IsKeyboard())
        {
            if (stricmp(strDeviceClass.c_str(),"keyboard")!=0)
                dwType = RIM_TYPEHID;
        }
        else if (IsMouse())
        {
            if (stricmp(strDeviceClass.c_str(),"mouse")!=0)
                dwType = RIM_TYPEHID;
        }
        else //HID
        {
            if (stricmp(strDeviceClass.c_str(),"keyboard")==0)
                dwType = RIM_TYPEKEYBOARD;
            else if (stricmp(strDeviceClass.c_str(),"mouse")==0)
                dwType = RIM_TYPEMOUSE;
        }

        // Make sure this isn't vmware's keyboard driver (rather than a physical keyboard)
        if (IsKeyboard())
        {
            HKEY hKeyVM = NULL;
            // If this fails, then something is amiss (there should be a control key), but, it isn't really a bail-out situation
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,(strRegKey+"\\Control").c_str(),0,KEY_QUERY_VALUE,&hKeyVM) == ERROR_SUCCESS)
            {
                // Retrieve data type and size (including null)
                if (RegQueryValueEx(hKeyVM,"ActiveService",NULL,&dwKeyType,NULL,&dwKeySize) == ERROR_SUCCESS)
                {
                    std::string strActiveService;
                    // Size the buffer accordingly
                    strActiveService.resize(dwKeySize-1);
                    // Added the const_cast for clarity, though it isn't necessary.
                    if (RegQueryValueEx(hKeyVM,"ActiveService",NULL,&dwKeyType,(BYTE*)const_cast<char*>(strActiveService.c_str()),&dwKeySize) == ERROR_SUCCESS)
                    {
                        if (stricmp(strActiveService.c_str(),"vmkbd") == 0)
                        {
                            // It's the vmware keyboard, not a physical keyboard.
                            dwType = RIM_TYPEHID;
                        }
                    }
                }
                RegCloseKey(hKeyVM);
            }
        }

        // Clean up
        RegCloseKey(hKey);
        hKey=NULL;

        // Return false if it isn't a supported keyboard and mouse
        if (!IsMouse() && !IsKeyboard())
            return false;

    } catch (const std::exception&e)
    {
        DebugMessage(e.what());
        Clear();
        if (hKey)
            RegCloseKey(hKey);
        return false;
    }
    return true;
}

bool nxRawInputDevice::Enumerate(std::vector<nxRawInputDevice>&vcRawDevices)
{
    nxRawInputDevice objDevice;
    UINT uNumDev;
    vcRawDevices.clear();
    try
    {
        if (GetRawInputDeviceList(NULL,&uNumDev,sizeof(RAWINPUTDEVICELIST)) != 0)
            THROWERROR("Error retrieving number of raw input devices.");
        std::vector<RAWINPUTDEVICELIST> rdl(uNumDev);
        if (uNumDev && (GetRawInputDeviceList(&rdl[0],&uNumDev,sizeof(RAWINPUTDEVICELIST)) == (UINT)-1))
            THROWERROR("Error retrieving list of raw input devices.");

        for (std::vector<RAWINPUTDEVICELIST>::iterator it=rdl.begin();it!=rdl.end();++it)
        {

            objDevice.hDevice = it->hDevice;
            objDevice.dwType = it->dwType;


            if (objDevice.GetInformation())
				vcRawDevices.push_back(objDevice);

            objDevice.Clear();
        }

    } catch (const std::exception&e)
    {
        DebugMessage(e.what());
        return false;
    }
    return true;

}




bool nxRawInputDevice::RegisterKeyboardHandler(HWND hEvent)
{
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_INPUTSINK | RIDEV_APPKEYS | RIDEV_NOLEGACY;
    rid.hwndTarget = hEvent;
    return (RegisterRawInputDevices(&rid,1,sizeof(RAWINPUTDEVICE)));
}
bool nxRawInputDevice::RemoveKeyboardHandler(HWND hEvent)
{
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_REMOVE;
    rid.hwndTarget = hEvent;
    return (RegisterRawInputDevices(&rid,1,sizeof(RAWINPUTDEVICE)));
}
bool nxRawInputDevice::RegisterMouseHandler(HWND hEvent,bool bCaptureMouse)
{
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY | (bCaptureMouse?RIDEV_CAPTUREMOUSE:0);
    rid.hwndTarget = hEvent;
    return (RegisterRawInputDevices(&rid,1,sizeof(RAWINPUTDEVICE)));
}
bool nxRawInputDevice::RemoveMouseHandler(HWND hEvent)
{
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_REMOVE;
    rid.hwndTarget = hEvent;
    return (RegisterRawInputDevices(&rid,1,sizeof(RAWINPUTDEVICE)));
}
