#include "nxGenericInput.hpp"
const unsigned char nxGenericInput::XINPUT_MAX_DEVICES = 4;


nxGenericInput::nxInputInfo::nxInputInfo(const std::string&strtext,bool isaxis)
: strInputText(strtext), bIsAxis(isaxis)
{ }

nxGenericInput::nxGenericInput()
{
    uNextDevId = XINPUT_MAX_DEVICES;
    // Map XI_1 to 0, XI_2 to 1, ...
    for (nxDeviceId i = 0;i < XINPUT_MAX_DEVICES;++i)
    {
        std::stringstream ss;
        ss << "XI_" << (i+1);
        MapGUIDToDeviceId(ss.str(),i);
    }
}

        // Extended will allow you to, for example, differentiate between a scan code and a vkey.

        // If keyboard, uExtended differentiates scancodes from vkeys
        // If mouse, uExtended differentiates buttons from axis
        // If joystick, uExtended differentiates buttons from axis
        // This will connect to a structure that has the string for the data, as well as the events associated
        // event 0 is the "generic" event, that isn't device id specific.

void nxGenericInput::AddInputInfo(const nxInputId&id,const std::string&strText,const bool&isaxis)
{
    mpTextToInputId[strText] = id;
    mpInputIdToData[id] = nxInputInfo(strText,isaxis);
    //TODO: check for binds with that text
    //TODO: creation of device IDs, and forwarding of all events... refining of this input model.
}


// MYCONTROLLER = XI_0,  MYKEYBOARD = {12312414151412}
// This maps the RHS to the appropriate device ID
void nxGenericInput::MapGUIDToDeviceId(const std::string&strGUID,const nxDeviceId&uDevId)
{
    mpGUIDToDeviceId[strGUID] = uDevId;
    //TODO: Check aliases for entries for this GUID, swap them over to the device id
}

