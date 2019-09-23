#ifndef NXGENERICINPUT_HPP_INCLUDED
#define NXGENERICINPUT_HPP_INCLUDED

#include "nxCommon.hpp"
#include <map>
#include <string>
#include <sstream>
#include <dinput.h>
class nxGenericInput
{
    public:
        static const unsigned char XINPUT_MAX_DEVICES;
        class nxInputInfo
        {
            public:
            nxInputInfo(const std::string&strtext="",bool isaxis=false);
            std::string strInputText;
            bool bIsAxis;
            // TODO: other variables for keeping up with the command.
        };

        nxGenericInput();

        // Extended will allow you to, for example, differentiate between a scan code and a vkey.

        // If keyboard, uExtended differentiates scancodes from vkeys
        // If mouse, uExtended differentiates buttons from axis
        // If joystick, uExtended differentiates buttons from axis
        // This will connect to a structure that has the string for the data, as well as the events associated
        // event 0 is the "generic" event, that isn't device id specific.
        static inline nxInputId nxMakeInputId(const nxInputType&dType,const unsigned short&uButtonID,const nxExtendedId&uExtended=0)
        {
            return (static_cast<nxInputId>(dType) << 24u) | ((static_cast<nxInputId>(uExtended) & 0xFFu)<<16u) | (static_cast<nxInputId>(uButtonID) & 0xFFFFu);
        }

        inline nxInputInfo* GetInputInfo(const nxInputId&id)
        {
            std::map<nxInputId,nxInputInfo>::iterator it = mpInputIdToData.find(id);
            if (it != mpInputIdToData.end())
                return &(it->second);
            return NULL;
        }
        void AddInputInfo(const nxInputId&id,const std::string&strText,const bool&isaxis);

        // Returns false if the item was not already present, meaning you need to update the device information
        inline bool GetDeviceId(const HANDLE&hRaw,nxDeviceId&uDevId)
        {
            std::map<HANDLE,nxDeviceId>::iterator it=mpRawHandleToDeviceId.find(hRaw);
            if (it == mpRawHandleToDeviceId.end())
            {
                uDevId = (mpRawHandleToDeviceId[hRaw] = CreateDeviceId());
                return false;
            }
            uDevId = it->second;
            return true;
        }
        inline nxDeviceId GetDeviceId(const LPDIRECTINPUTDEVICE&lpDevice,nxDeviceId&uDevId)
        {
            std::map<LPDIRECTINPUTDEVICE,nxDeviceId>::iterator it=mpDInputToDeviceId.find(lpDevice);
            if (it == mpDInputToDeviceId.end())
            {
                uDevId = (mpDInputToDeviceId[lpDevice] = CreateDeviceId());
                return false;
            }
            uDevId = it->second;
            return true;
        }

        // MYCONTROLLER = XI_0,  MYKEYBOARD = {12312414151412}
        // This maps the RHS to the appropriate device ID
        inline void MapGUIDToDeviceId(const std::string&strGUID,const nxDeviceId&uDevId);
    protected:

        inline nxDeviceId CreateDeviceId(void)
        {
            return (uNextDevId++);
        }


        nxDeviceId uNextDevId;
        // Maps an input ID value to it's associated information/bindings
        std::map<nxInputId,nxInputInfo> mpInputIdToData;
        // Maps an input's textual identifier to an input id (as we figure them out)
        std::map<std::string,nxInputId> mpTextToInputId;
        // TODO: a map of string to associated bindings for prior to determining the associated input id, will be moved after.
        //       has to do with configuration file loading.

        // keyboard/mouse = handle or guid
        // xbox = #
        // di = guid



        // A map of the raw input handles to their device id
        std::map<HANDLE,nxDeviceId> mpRawHandleToDeviceId;
        // A map of the directinput device to it's device id
        std::map<LPDIRECTINPUTDEVICE,nxDeviceId> mpDInputToDeviceId;
        // XInput can only have 4 devices, so we'll use device id 1-4 for xinput devices, that way they map the same

        // Mapping of device names to IDs.  These are only for user-defined aliases.
        std::map<std::string,nxDeviceId> mpGUIDToDeviceId;

        // Map of aliases to instance GUIDs.  XInput devices are XI_1 through XI_4.
        // TODO: Make a tool to create a configuration file to generate this information.
        //std::map<std::string,std::string> mpAliasToGUID;

};


#endif // NXGENERICINPUT_HPP_INCLUDED
