///NOTE: Keep in mind SDL as a potential option for porting this to linux

#include "nxCommon.hpp"
#include "nxUtility.hpp"
#include "nxRawInputDevice.hpp"
#include "nxThread.hpp"
#include "nxLexer.hpp"
#include "nxInputManager.hpp"
#include "nxIteratedFile.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <map>
#include <XInput.h> // XInput API
#include <DInput.h>
#include <XIMCore.h>
#include <list>
#include <stack>

nxRecursiveMutex csXIState;
XIMXbox360Input ximInput;
bool bExitProgram = false;
bool bChangeMode = false;
bool bBrandy = true;
bool bAimMode = false;
LRESULT CALLBACK InputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch(msg)
    {
        case NX_INPUT_ENUMERATE:
        {
            std::cout << "Starting device enumeration..." << std::endl;
            return 0;
        }
        case NX_INPUT_DEVICE:
        {
            nxInputId uInputId;
            nxDeviceId uDeviceId;
            nxInputManager::SplitMessage(wParam,uInputId,uDeviceId);
            std::cout << "Found Device: " << (unsigned int)uDeviceId << " == " << *nxInputManager::instance().srDeviceGUID.LookupLeft(uDeviceId) << std::endl;
            return 0;
        }
        case NX_INPUT_READY:
        {
            std::cout << "Finished enumerating devices." << std::endl;
            return 0;
        }
        case NX_INPUT_LABEL:
        {
            nxInputId uInputId;
            nxDeviceId uDeviceId;
            nxInputManager::SplitMessage(wParam,uInputId,uDeviceId);
            //std::cout << "Got label for " << uInputId << " as " << *nxInputManager::instance().srInputLabel.LookupLeft(uInputId) << std::endl;
            return 0;
        }

        // You press RB, send LT
        // You press Start, send RB
        // You press LT, send Start
        case NX_INPUT_RECEIVED:
        {
            nxInputId uInputId;
            nxDeviceId uDeviceId;
            // Separate the input id and the device id
            nxInputManager::SplitMessage(wParam,uInputId,uDeviceId);

            nxInputManager&objManager=nxInputManager::instance();

            const std::string*ptInputText = objManager.srInputLabel.LookupLeft(uInputId);
            if (!ptInputText) return 0;

            short sValue = static_cast<short>(lParam);
            if (uDeviceId == 1)
            {
                if (sValue < XIMStickMinimum)
                    sValue = XIMStickMinimum;
                if (sValue > XIMStickMaximum)
                    sValue = XIMStickMaximum;

                //nxScopedGuard objGuard(csXIState);
                nxInputId uButtonId = uInputId & 0xFFFFu;
                XIMDIGITAL digValue;
                    if (sValue)
                        digValue = XIMButtonPressed;
                    else digValue = XIMButtonReleased;
                XIMANALOG axisValue = static_cast<XIMANALOG>(sValue);
                    if (axisValue < XIMStickMinimum)
                        axisValue = XIMStickMinimum;
                    if (axisValue > XIMStickMaximum)
                        axisValue = XIMStickMaximum;
                if (!nxInputManager::IsInputAxis(uInputId))
                {
                    nxScopedRecursiveLock objGuard(csXIState);
                    switch (uButtonId)
                    {
                        case 0: // Up
                            ximInput.Up = digValue; break;
                        case 1: // Down
                            ximInput.Down = digValue; break;
                        case 2: // Left
                            ximInput.Left = digValue; break;
                        case 3: // Right
                            ximInput.Right = digValue; break;
                        case 4: // Start
							ximInput.Start = digValue;
                            break;
                        case 5: // Back
                            ximInput.Back = digValue; break;
                        case 6: // Left Thumb
                            ximInput.LeftStick = digValue; break;
                        case 7: // Right Thumb
                            ximInput.RightStick = digValue; break;
                        case 8: // Left Bumper
                            ximInput.LeftBumper = digValue;
                            break;
                        case 9: // Right Bumper
                            //ximInput.RightBumper = digValue; break;
							// remap
							ximInput.LeftTrigger = (digValue == XIMButtonPressed?XIMTriggerMost:XIMTriggerRest);
							break;
                        case 10: // A
                            ximInput.A = digValue; break;
                        case 11: // B
							ximInput.B = digValue; break;
                            break;
                        case 12: // X
                            ximInput.X = digValue; break;
                        case 13: // Y
							// REMAP
                            ximInput.RightBumper = digValue;
                            break;

                    }
                }
                else
                {
                    // Axis
                    nxScopedRecursiveLock objGuard(csXIState);
                    switch (uButtonId)
                    {
                        case 0: // LX
                            ximInput.LeftStickX = axisValue; break;
                        case 1: // LY
                            ximInput.LeftStickY = axisValue; break;
                        case 2: // RX
                            ximInput.RightStickX = axisValue; break;
                        case 3: // RY
                            ximInput.RightStickY = axisValue; break;
                        case 4: // LT
                            // REMAP
							ximInput.Y = (axisValue > 100?XIMButtonPressed:XIMButtonReleased);
                            break;
                        case 5: // RT
                            ximInput.RightTrigger = axisValue; break;
                    }
                }


            }
/*            if (uDeviceId > 1 && uDeviceId <= 4)
                std::cout << (unsigned int)uDeviceId << '\t' << uInputId << '\t' << sValue << '\t' << *ptInputText << std::endl;*/
            if (*ptInputText == "ESC")
                 bExitProgram = true;

            const std::string*ptGUID = objManager.srDeviceGUID.LookupLeft(uDeviceId);
            std::string strGUID = (ptGUID?*ptGUID:"None");
            //std::cout << (unsigned int)uDeviceId << '\t' << strGUID <<  '\t' <<  uInputId << '\t' << sValue << '\t' << *ptInputText << std::endl;
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

class nxParseError
{
	public:
		nxParseError(const unsigned long&token,const unsigned long &expected)
		: uToken(token), uExpectedToken(expected)
		{ }
		unsigned long uToken;
		unsigned long uExpectedToken;
};

class nxToken
{
    public:
        std::string strValue;
        unsigned long uTokenId;
        std::list<nxToken> lstChildren;

        nxToken(const std::string&value="",const unsigned long&tokenid=nxLexer::LEX_NONE)
        : strValue(value), uTokenId(tokenid)
        { }

        inline const unsigned long&GetId(void) const { return uTokenId; }
        void AddChild(const nxToken&objToken)
        {
            lstChildren.push_back(objToken);
        }
        void Clear(void)
        {
            lstChildren.clear();
            strValue = "";
            uTokenId=nxLexer::LEX_NONE;
        }
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    nxInstanceHandle(hInstance);

	ZeroMemory(&ximInput,sizeof(XIMXbox360Input));

    nxInputManager&objManager=nxInputManager::instance();
    objManager.Start(InputProc,true,true,true);


    std::cout << "Calling XIMConnect()" << std::endl;
    XIMSTATUS stat = XIMConnect();
    std::cout << "Call completed.  Press escape to exit." << std::endl;
    if (stat != XIMSTATUS_OK)
    {
        std::cout << "XIM connection failed " << stat << std::endl;
        return 0;
    }
    XIMSetMode(XIMModeNone);
    bExitProgram = false;
	XIMXbox360Input input;
    while(!bExitProgram)
    {
            // Set all 'input' fields based on 360 controller state
		// Grab the current data from the input thread
        {
        	nxScopedRecursiveLock objGuard(csXIState);
            input = ximInput;
        }
		// According to the ACTUAL buttons pressed on the fake joystick make a way to press guide
        if (input.Start == XIMButtonPressed && input.Back == XIMButtonPressed)
        {
            input.Start = XIMButtonReleased;
            input.Back = XIMButtonReleased;
            input.Guide = XIMButtonPressed;
        }
        else
        {
            input.Guide = XIMButtonReleased;
        }
        XIMSendXbox360Input(&input, 16); // Wait 16ms (60 updates per sec)

    }
    objManager.Stop();
    XIMDisconnect();
    return 0;
}

