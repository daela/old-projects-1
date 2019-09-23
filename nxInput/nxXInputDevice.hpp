#ifndef NXXINPUTDEVICE_HPP_INCLUDED
#define NXXINPUTDEVICE_HPP_INCLUDED

#include "nxCommon.hpp"

#if defined(OS_WINDOWS_)

#include <XInput.h>
#include <sstream>
#include <iomanip>

class nxXInputData
{
    public:
        // Providing indexes for each of the buttons
        enum XInputButton { XI_UP=0, XI_DOWN, XI_LEFT, XI_RIGHT, XI_START, XI_BACK, XI_LTHUMB, XI_RTHUMB, XI_LSHOULDER, XI_RSHOULDER, XI_A, XI_B, XI_X, XI_Y, XI_NUMBUTTONS };
        enum XInputAxis { XI_AXIS_LX=0, XI_AXIS_LY, XI_AXIS_RX, XI_AXIS_RY, XI_AXIS_LTRIGGER, XI_AXIS_RTRIGGER, XI_NUMAXIS };

        // All the controller's button/axis values
        short bButtons[XI_NUMBUTTONS];
        short bAxis[XI_NUMAXIS];

        // Clear the object during construction
        inline nxXInputData()
        {
            for(unsigned char i=0;i<XI_NUMBUTTONS;++i)
                bButtons[i] = 0;
            for(unsigned char i=0;i<XI_NUMAXIS;++i)
                bAxis[i] = 0;
        }
        // Set the data based on the XINPUT_GAMEPAD structure
        void Set(const XINPUT_GAMEPAD&objPad);
    private:
        // Providing the mask for each entry in the XInputButton enum
        static const WORD xiButtonMask[XI_NUMBUTTONS];
};

class nxXInputDevice
{
    protected:
        nxDeviceId uDeviceID;
        DWORD dwPacket;
    public:
        static bool IsXInputDevice(const GUID&guidProduct);
        enum { XI_MAXDEVICES=4 };
        inline nxXInputDevice(nxDeviceId id) : uDeviceID(id), dwPacket(0)
        { }

        // State storage, current and previous
        nxXInputData objData;
        nxXInputData objPrev;

        // Returns true if the state was retrieved
        bool GetState(void);
};


#endif

#endif // NXXINPUTDEVICE_HPP_INCLUDED
