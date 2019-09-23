#ifndef NXRAWINPUTDEVICE_HPP_INCLUDED
#define NXRAWINPUTDEVICE_HPP_INCLUDED

#include <string>
#include <vector>
#include "nxCommon.hpp"
class nxRawInputDevice
{
	protected:
		HANDLE hDevice;
		DWORD dwType;

		// The following fields are populated by GetInformation()
		std::string strDeviceName;
		std::string strDeviceClass; // From the registry, this tells you without a doubt if the device is actually a keyboard
		std::string strDeviceDesc; // Friendly device description
		std::string strClassCode; // If class is case insensitively "root", it's for terminal services/remote desktop.  Ignore it.
		std::string strSubClassCode;
		std::string strProtocolCode;
		std::string strGUID; // Important for remembering devices.  Globally Unique IDentifier.

        inline void Clear(void)
		{
			hDevice = NULL;
			dwType = RIM_TYPEHID;
			strDeviceName = strDeviceClass = strDeviceDesc = strClassCode = strSubClassCode = strProtocolCode = strGUID = "";
		}
        bool GetInformation(void);
    public:
        inline nxRawInputDevice()
		{
		    Clear();
		}
        inline bool IsMouse(void) { return (dwType==RIM_TYPEMOUSE); }
        inline bool IsKeyboard(void) { return (dwType==RIM_TYPEKEYBOARD); }

        inline const std::string & GetDeviceName(void) const { return strDeviceName; }
		inline const std::string & GetDeviceClass(void) const { return strDeviceClass; }
		inline const std::string & GetDeviceDesc(void) const { return strDeviceDesc; }
		inline const std::string & GetClassCode(void) const { return strClassCode; }
		inline const std::string & GetSubClassCode(void) const { return strSubClassCode; }
		inline const std::string & GetProtocolCode(void) const { return strProtocolCode; }
		inline const std::string & GetGUID(void) const { return strGUID; }
		inline const HANDLE & GetDeviceHandle(void) const { return hDevice; }

        static bool Enumerate(std::vector<nxRawInputDevice>&vcRawDevices);
        static bool RegisterKeyboardHandler(HWND hEvent);
        static bool RemoveKeyboardHandler(HWND hEvent);
        static bool RegisterMouseHandler(HWND hEvent,bool bCaptureMouse = false);
        static bool RemoveMouseHandler(HWND hEvent);
};

#endif // NXRAWINPUTDEVICE_HPP_INCLUDED
