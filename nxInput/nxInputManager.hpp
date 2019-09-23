#ifndef NXINPUTMANAGER_HPP_INCLUDED
#define NXINPUTMANAGER_HPP_INCLUDED

#include "nxCommon.hpp"

#if defined(OS_WINDOWS_)

#include "nxXInputDevice.hpp"
#include "nxRawInputDevice.hpp"
#include "nxUtility.hpp"
#include "nxThread.hpp"
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <iomanip>
#include <DInput.h>
#include <set>

// Base offset for messages
#define NX_INPUT_BASE           (WM_USER+1234)
// Input event, keyup/keydown/axis movement/button press/...
#define NX_INPUT_RECEIVED       (NX_INPUT_BASE+1)
// Tells that device enumeration is fixing to begin.
#define NX_INPUT_ENUMERATE      (NX_INPUT_BASE+2)
// Tells that an individual device has been added.
#define NX_INPUT_DEVICE         (NX_INPUT_BASE+3)
// An initialization message, signifies that all devices have been enumerated and the threads are ready.
#define NX_INPUT_READY          (NX_INPUT_BASE+4)
// Tells when a new textual label has been determined for an input event.
#define NX_INPUT_LABEL          (NX_INPUT_BASE+5)
// Tells the raw input thread to wake up to check for the exit condition
#define NX_INPUT_AWAKEN         (NX_INPUT_BASE+6)



class nxInputManager : public nxSingleton<nxInputManager>
{
    friend class nxSingleton<nxInputManager>;
    private:
        class InputHandle
        {
            private:
                enum { NX_RAWINPUT=0, NX_DINPUT, NX_XINPUT } eType;
                union
                {
                    HANDLE hRI;
                    LPDIRECTINPUTDEVICE8 lpDI;
                    DWORD uXI;
                } Data;
            public:
                inline bool operator<(const InputHandle&rhs) const
                {
                    if (eType<rhs.eType) return true;
                    if (rhs.eType<eType) return false;
                    switch (eType)
                    {
                        case NX_RAWINPUT:
                            return (Data.hRI < rhs.Data.hRI);
                        case NX_DINPUT:
                            return (Data.lpDI<rhs.Data.lpDI);
                        case NX_XINPUT:
                            return (Data.uXI<rhs.Data.uXI);
                    }
                    return false;
                }
                inline InputHandle(const HANDLE&hRawInput)
                {
                    eType=NX_RAWINPUT;
                    Data.hRI=hRawInput;
                }
                inline InputHandle(const LPDIRECTINPUTDEVICE8&lpDInput)
                {
                    eType=NX_DINPUT;
                    Data.lpDI=lpDInput;
                }
                inline InputHandle(const DWORD&uXInput)
                {
                    eType=NX_XINPUT;
                    Data.uXI=uXInput;
                }
        };
        class DataSender
        {
            nxInputManager&objManager;
            nxInputType dType;
            nxDeviceId uDeviceId;

            public:
                inline DataSender(const nxInputType&type,const nxDeviceId&deviceid)
                : objManager(nxInputManager::instance()), dType(type), uDeviceId(deviceid)
                { }
                // Sends the data, note if uDevId is non-zero, the supplied ID is used instead of the one defined during construction
                void SendData(const unsigned short&uButtonId,const bool&IsAxis,const short&sValue,const nxDeviceId&uDevId=0,const nxExtendedId&uExtended=0);
        };


        /// Thread Safety
        // Makes sure adding input information is thread safe
        nxRecursiveMutex csDataSenderSendData;
        // Make Start() and Stop() thread safe
        nxRecursiveMutex csThreadManagement;

        /// Message Window Handles
        // Handle to the message only window for the input manager, and the raw input window
        HWND hManager;
        HWND hRawInput;

        /// Internal values
        // Device ID counter
        nxDeviceId uNextDevId;

        // DI8 Interface
        LPDIRECTINPUT8 pDI;

        // Internal mappings, maps input handles to device ids
        std::map<InputHandle,nxDeviceId> mpHandleId;
        // A list of the directinput devices (used for polling)
        std::set<LPDIRECTINPUTDEVICE8> stDInputDevices;


        /// Thread Creation/Destruction
        // Boolean used to tell worker threads whether to continue or to stop
        bool isRunning;
        // List of thread handles if Start() has been called.
        std::vector<HANDLE> vcThreads;

        inline nxDeviceId CreateDeviceId(void)
        {
            return (++uNextDevId);
        }

        /// Constructor
        inline nxInputManager(void)
        : hManager(NULL), hRawInput(NULL), uNextDevId(0), pDI(NULL), isRunning(false)
        {
        }
        inline ~nxInputManager(void)
        {
            if (isRunning)
                Stop();
        }

        /// Internal Functions
        // Erases old device records/directinput objects, re-enumerates the devices, and creates device ids.
        void Prepare(bool bRawInput,bool bDInput,bool bXInput);
        void Cleanup(void);

        inline bool AddInputLabel(const nxInputId&uInputId,const std::string&strLabel)
        {
            bool bRet = srInputLabel.Insert(uInputId,strLabel);
            if (bRet)
                PostEvent(NX_INPUT_LABEL,uInputId,0);
            return bRet;
        }
        inline nxDeviceId GetInputHandleId(const InputHandle&objHandle)
        {
            std::map<InputHandle,nxDeviceId>::iterator it = mpHandleId.find(objHandle);
            if (it != mpHandleId.end())
                return it->second;
            return 0;
        }
        static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pInstance,VOID* ptData);
        static BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext );
        static LRESULT CALLBACK RawInputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


        // 1 byte = device id
        // 4 bits = type of input
        // 1 bit = isaxis
        // 3 bits = extra button id (vkey or scan).. for keyboard, it's isVkey, e0, e1
        // 2 bytes = button id
        // 0x00FFFFFFu == without the device id
        inline static nxInputId MakeInputId(const nxInputType&dType,const unsigned short&uButtonID,const bool&isAxis=false,const nxExtendedId&uExtended=0)
        {
            // Input ID is _THREE_ bytes.. the upper byte is reserved for passing a device id in messages.
            return ((static_cast<nxInputId>(dType)&0xFu) << 20u) | (static_cast<nxInputId>(isAxis?1:0) << 19u) | ((static_cast<nxInputId>(uExtended)&0x7u)<<16) | (static_cast<nxInputId>(uButtonID) & 0xFFFFu);
        }

        inline static nxInputMessage MakeMessage(const nxInputId&uInputId,const nxDeviceId&uDeviceId)
        {
            return ((static_cast<nxInputMessage>(uDeviceId) & 0xFF)<<24u)  | (static_cast<nxInputMessage>(uInputId)&0x00FFFFFFu);
        }

        inline void PostEvent(const UINT&uEvent,const nxInputMessage&uMessage,const short&uValue)
        {
            if (hManager)
                PostMessage(hManager,uEvent,static_cast<WPARAM>(uMessage),static_cast<LPARAM>(uValue));
        }

        /// Thread Functions
        static DWORD WINAPI XInputThread(LPVOID lpParameter);
        static DWORD WINAPI DInputThread(LPVOID lpParameter);
        static DWORD WINAPI RawInputThread(LPVOID lpParameter);
        static DWORD WINAPI MessageThread(LPVOID lpParameter);
        void XInputLoop(DWORD dwMilliseconds=8);
        void DInputLoop(DWORD dwMilliseconds=8);
        void RawInputLoop(bool bCaptureMouse=false);
        void MessageLoop(WNDPROC lpfnProc);


        void SetGUIDMapping(const nxDeviceId&uDeviceId,const std::string&strGUID)
        {
            srDeviceGUID.Insert(uDeviceId,strGUID);
            PostEvent(NX_INPUT_DEVICE,MakeMessage(0,uDeviceId),0);
        }
    public:
        /// External interface
        static const short INPUT_MIN;
        static const short INPUT_MAX;
        // Converts GUID structure to a string
        static std::string GUIDToString(const GUID&guid);

        // External mappings, maps input to the textual label for that input
        nxSafeRelation<nxInputId,std::string> srInputLabel;
        // Maps a device id to it's GUID
        nxSafeRelation<nxDeviceId,std::string> srDeviceGUID;

        // Calling this function will start enough threads to handle the input, polling DInput and XInput with dwMilliseconds for the delay.
        bool Start(WNDPROC lpfnMessageProc,bool bRawInput,bool bDInput,bool bXInput,DWORD dwMilliseconds=8);
        // Will stop any threads that may be running already, will not return until threads have ceased.  Will also perform cleanup.
        void Stop(void);

        inline static bool IsInputAxis(const nxInputId&uInputId)
        {
            return ((uInputId&(0x1 << 19u))?true:false);
        }

        inline static void SplitMessage(const nxInputMessage&uMessage,nxInputId&uInputId,nxDeviceId&uDeviceId)
        {
            uInputId = static_cast<nxInputId>(uMessage&0x00FFFFFFu);
            uDeviceId = static_cast<nxDeviceId>((uMessage&0xFF000000u)>>24u);
        }

};

#endif

#endif // NXINPUTMANAGER_HPP_INCLUDED
