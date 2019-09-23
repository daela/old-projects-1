#ifndef NXINPUTHANDLER_HPP_INCLUDED
#define NXINPUTHANDLER_HPP_INCLUDED
#include "nxCommon.hpp"
#include "nxRawInputDevice.hpp"
#include "nxUtility.hpp"
#include "nxInputHandler.hpp"
#include <boost/pool/singleton_pool.hpp>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <boost/scoped_array.hpp>
#include <dinput.h>
#include <XInput.h>

#define NX_GENERIC_INPUT (WM_USER+1000)
/*class CSync
{
public:
  CSync() { InitializeCriticalSection(&m_CriticalSection); }
  ~CSync() { DeleteCriticalSection(&m_CriticalSection); }
  void Acquire() { EnterCriticalSection(&m_CriticalSection); }
  void Release() { LeaveCriticalSection(&m_CriticalSection); }


private:
  CRITICAL_SECTION m_CriticalSection;              // Synchronization object
};

class CLockGuard
{
public:
  CLockGuard(CSync &refSync) : m_refSync(refSync) { Lock(); }
  ~CLockGuard() { Unlock(); }


private:
  CSync &m_refSync;                     // Synchronization object

  CLockGuard(const CLockGuard &refcSource);
  CLockGuard &operator=(const CLockGuard &refcSource);
  void Lock() { m_refSync.Acquire(); }
  void Unlock() { m_refSync.Release(); }
};*/




class nxInputInfo
{
    public:
    inline nxInputInfo(const std::string&strtext="",bool isaxis=false)
    : strInputText(strtext), bIsAxis(isaxis)
    { }
    std::string strInputText;
    bool bIsAxis;
    // TODO: other variables for keeping up with the command.
};
class nxInputData
{
    public:

        nxInputType dType;
        nxDeviceId uDeviceID;
        nxInputId uInputID;
        short sValue;
        inline nxInputData* Set(const nxInputType&type,const nxDeviceId&deviceid,const nxInputId&inputid,const short&value)
        {
            dType = type;
            uDeviceID = deviceid;
            uInputID = inputid;
            sValue=value;
            return this;
        }
        friend std::ostream&operator<<(std::ostream&os,const nxInputData&rhs);
};

class nxInputHandler
{
    private:
    // Class to keep up with the input state of a XInput device
    class XInputData
    {
        public:
            // Providing indexes for each of the buttons
            enum XInputButton { XI_UP=0, XI_DOWN, XI_LEFT, XI_RIGHT, XI_START, XI_BACK, XI_LTHUMB, XI_RTHUMB, XI_LSHOULDER, XI_RSHOULDER, XI_A, XI_B, XI_X, XI_Y, XI_NUMBUTTONS };
            enum XInputAxis { XI_AXIS_LX=0, XI_AXIS_LY, XI_AXIS_RX, XI_AXIS_RY, XI_AXIS_LTRIGGER, XI_AXIS_RTRIGGER, XI_NUMAXIS };
            // All the controller's button/axis values
            short bButtons[XI_NUMBUTTONS];
            short bAxis[XI_NUMAXIS];
            // Clear the object during construction
            inline XInputData()
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
    // Class to use to interface with an XInput device
    class XInputDevice
    {
        protected:
            unsigned int uDeviceID;
            DWORD dwPacket;
        public:

            inline XInputDevice(unsigned int id) : uDeviceID(id), dwPacket(0)
            { }

            // State storage, current and previous
            XInputData objData;
            XInputData objPrev;

            // Returns true if the state was retrieved
            bool GetState(void);
    };
    // Data sender class, used for making event transmissions simpler
    class DataSender
    {
        nxInputHandler&objHandler;
        nxInputType dType;
        nxDeviceId uDeviceId;

        public:
            inline DataSender(const nxInputType&type,const nxDeviceId&deviceid)
            : objHandler(*nxInputHandler::GetInstance()), dType(type), uDeviceId(deviceid)
            { }
            // Sends the data, note if uDevId is non-zero, the supplied ID is used instead of the one defined during construction
            void SendData(const unsigned short&uButtonId,const bool&IsAxis,const short&sValue,const nxDeviceId uDevId=0);
    };


    // Private variables, handles to the internal event window, and the internal raw input window
    HWND hMsgWnd;
    HWND hRawWnd;

    WNDPROC HandlerProc;
    // Keeps up with the running state

    bool bHandlerRunning;
    bool bRawInputRunning;
    bool bXInputRunning;

    // Using the singleton pool instead of the object pool so that it's thread safe
    typedef boost::singleton_pool<nxInputData, sizeof(nxInputData)> DataPool;

    // Map of guid to device id (must convert DInput's GUID structure to string)
    std::map<std::string,nxDeviceId> mpGUIDToDevice;

    // Map of raw handles to device id
    std::map<HANDLE,nxDeviceId> mpRawToDevice;

    // Map of DirectInput devices to their device interfaces
    std::map<LPDIRECTINPUTDEVICE8,nxDeviceId> mpDIDevice;

    // Maps an input ID value to it's associated information/bindings
    std::map<nxInputId,nxInputInfo> mpInputInfo;

    // Maps an input's textual identifier to an input id (as we figure them out)
    std::map<std::string,nxInputId> mpTextToInputId;

    // Device ID counter
    nxDeviceId uNextDevId;

    // DI8 Interface
    LPDIRECTINPUT8 pDI;

    // Pointer to the singleton instance
    static std::auto_ptr<nxInputHandler> pHandlerInstance;

    // Internal callbacks
    static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pInstance,VOID* ptHandler );
    static LRESULT CALLBACK RawInputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Adds the specified information for the provided input
    inline nxInputInfo* AddInputInfo(const nxInputId&id,const std::string&strText,const bool&isaxis=false)
    {
        std::map<nxInputId,nxInputInfo>::iterator it = mpInputInfo.insert(std::pair<nxInputId,nxInputInfo>(id,nxInputInfo(strText,isaxis))).first;
        return &(it->second);
    }

    // Returns the next available device id for use
    inline nxDeviceId CreateDeviceId(void)
    {
        return (uNextDevId++);
    }

    // Constructor - it's private, for singleton purposes
    inline nxInputHandler() { bHandlerRunning=bRawInputRunning=bXInputRunning=false; hRawWnd=hMsgWnd=NULL;};

    // Initialization routine for the object
    void Create(WNDPROC proc);

    // Retrieves the mapping from a RawInput HANDLE to the device id
    inline nxDeviceId GetRawDeviceId(const HANDLE&hRaw)
    {
        std::map<HANDLE,nxDeviceId>::iterator it = mpRawToDevice.find(hRaw);
        if (it == mpRawToDevice.end())
        {
            mpRawToDevice[hRaw] = 0;
            return 0;
        }
        return it->second;
    }

    public:
        // Maximum number of XInput devices
        static const nxDeviceId XINPUT_MAX_DEVICES;

        // "Constructor" for the singleton object
        inline static nxInputHandler*CreateInstance(WNDPROC proc)
        {
            if (!GetInstance())
            {
                pHandlerInstance.reset(new nxInputHandler());
                pHandlerInstance->Create(proc);
            }
            return GetInstance();
        }
        // Retrieves the singleton instance
        inline static nxInputHandler*GetInstance(void)
        {
            return pHandlerInstance.get();
        }
        // Converts a directinput GUID to a string
        static std::string GUIDToString(const GUID&guid);

        // Determines if a device is an XInput defines based upon the product GUID provided by DirectInput.
        // This method uses the registry, not WMI.
        static bool IsXInputDevice(const GUID&guidProduct);

        // Creates an input id based upon a few characteristics.  For keyboard, extended differentiates between scan code and vkey defined keys.
        // For mouse/joysticks, it differentiates between button and axis.
        inline static nxInputId MakeInputId(const nxInputType&dType,const unsigned short&uButtonID,const nxExtendedId&uExtended=0)
        {
            return (static_cast<nxInputId>(dType) << 24u) | ((static_cast<nxInputId>(uExtended) & 0xFFu)<<16u) | (static_cast<nxInputId>(uButtonID) & 0xFFFFu);
        }
        // Retrieves the information associated with a given input.  If the input reaches the event handler, this will definitely not be NULL.
        // Internally, however, it may be NULL (at which point the information is determined and added).
        inline nxInputInfo*GetInputInfo(const nxInputId&uInputId)
        {
            std::map<nxInputId,nxInputInfo>::iterator it=mpInputInfo.find(uInputId);
            if (it == mpInputInfo.end())
                return NULL;
            return &(it->second);
        }

        // Function to be called by whatever thread you want to poll XInput from
        bool XInputLoop(DWORD dwMilliseconds=5);
        bool RawInputLoop(bool bCaptureMouse=false);
        bool HandlerLoop();
        //void DirectInputLoop();

        inline void Stop()
        {
            bRawInputRunning=bXInputRunning=bHandlerRunning=false;
        }
        inline ~nxInputHandler()
        {
            Stop();
        }

        // Allocate data from the singleton memory pool (thread safe) to be used for message passing
        inline nxInputData* AllocData(void)
        {
            return reinterpret_cast<nxInputData*>(DataPool::malloc());
        }
        // Free data from the singleton pool
        inline void FreeData(nxInputData*ptData)
        {
            DataPool::free(ptData);
        }
        // By passing each parameter instead of an object, the object doesn't have to be created, and then recreated when dynamically allocated
        // to be sent to the message procedure.  I could also alleviate this problem by letting the user pass a pointer to the structure,
        // but then I can't necessarily assume it was allocated by new, and I need to delete all incoming data received in the message
        // procedure.  So, this somewhat cumbersome method is what I decided on, seeing as this function will be called a _lot_
        inline void SendData(nxInputData*ptData)
        {
            if (bHandlerRunning)
                PostMessage(hMsgWnd,NX_GENERIC_INPUT,reinterpret_cast<WPARAM>(ptData),0);
            else
                FreeData(ptData);
        }
};



#endif // NXINPUTHANDLER_HPP_INCLUDED
