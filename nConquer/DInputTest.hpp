#include <windows.h>
//#include <d3d.h>
#include <dinput.h>

/*
HRESULT WINAPI (*orgDirectInput8Create)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter);
HRESULT WINAPI mchDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	// Let the world know we're working.
	MessageBeep(MB_ICONINFORMATION);

	HRESULT hr = orgDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	if(SUCCEEDED(hr))
		*ppvOut = new MyDirectInput8(reinterpret_cast<IDirectInput8*>(*ppvOut),KeyHandler);

	return hr;
}
*/
typedef void (*pfnKeyHandler)(char*);
class KeyboardDevice : public IDirectInputDevice8
{
	IDirectInputDevice8* m_device;
	pfnKeyHandler ptKeyHandler;
public:
	KeyboardDevice(IDirectInputDevice8* device,pfnKeyHandler handler) : m_device(device), ptKeyHandler(handler)//, m_serverPipe(FULL_PIPE_NAME)
	{
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj)
	{
		return m_device->QueryInterface(riid, ppvObj);
	}

	STDMETHOD_(ULONG,AddRef)()
	{
		return m_device->AddRef();
	}

	STDMETHOD_(ULONG,Release)()
	{
		ULONG count = m_device->Release();
		if(0 == count)
			delete this;

		return count;
	}

	/*** IDirectInputDevice8 methods ***/
	STDMETHOD(GetCapabilities)(LPDIDEVCAPS devCaps)
	{
		return m_device->GetCapabilities(devCaps);
	}

	STDMETHOD(EnumObjects)(LPDIENUMDEVICEOBJECTSCALLBACK callback, LPVOID ref, DWORD flags)
	{
		return m_device->EnumObjects(callback, ref, flags);
	}

	STDMETHOD(GetProperty)(REFGUID rguid, LPDIPROPHEADER ph)
	{
		return m_device->GetProperty(rguid, ph);
	}

	STDMETHOD(SetProperty)(REFGUID rguid, LPCDIPROPHEADER ph)
	{
		return m_device->SetProperty(rguid, ph);

		/*
		You can detect immediate/buffered modes as such:

		HRESULT hr = m_device->SetProperty(rguid, ph);

		if(SUCCEEDED(hr) && rguid == DIPROP_BUFFERSIZE)
		{
			DWORD data = *reinterpret_cast<const DWORD*>(ph + 1);
			m_is_immediate = (data == 0);
		}

		return hr;
		*/
	}

	STDMETHOD(Acquire)()
	{
		return m_device->Acquire();
	}

	STDMETHOD(Unacquire)()
	{
		return m_device->Unacquire();
	}

	STDMETHOD(GetDeviceState)(DWORD size, LPVOID data)
	{
		HRESULT hr = m_device->GetDeviceState(size, data);

		if(SUCCEEDED(hr) && size == sizeof(BYTE) * 256)
		{
			if (ptKeyHandler)
				ptKeyHandler(static_cast<char*>(data));

		}

		return hr;
	}

	STDMETHOD(GetDeviceData)(DWORD size, LPDIDEVICEOBJECTDATA data, LPDWORD numElements, DWORD flags)
	{
		return m_device->GetDeviceData(size, data, numElements, flags);
	}

	STDMETHOD(SetDataFormat)(LPCDIDATAFORMAT df)
	{
		return m_device->SetDataFormat(df);
	}

	STDMETHOD(SetEventNotification)(HANDLE event)
	{
		return m_device->SetEventNotification(event);
	}

	STDMETHOD(SetCooperativeLevel)(HWND window, DWORD level)
	{
		return m_device->SetCooperativeLevel(window, level);
	}

	STDMETHOD(GetObjectInfo)(LPDIDEVICEOBJECTINSTANCE object, DWORD objId, DWORD objHow)
	{
		return m_device->GetObjectInfo(object, objId, objHow);
	}

	STDMETHOD(GetDeviceInfo)(LPDIDEVICEINSTANCE di)
	{
		return m_device->GetDeviceInfo(di);
	}

	STDMETHOD(RunControlPanel)(HWND owner, DWORD flags)
	{
		return m_device->RunControlPanel(owner, flags);
	}

	STDMETHOD(Initialize)(HINSTANCE instance, DWORD version, REFGUID rguid)
	{
		return m_device->Initialize(instance, version, rguid);
	}

	STDMETHOD(CreateEffect)(REFGUID rguid, LPCDIEFFECT effect_params, LPDIRECTINPUTEFFECT* effect, LPUNKNOWN unknown)
	{
		return m_device->CreateEffect(rguid, effect_params, effect, unknown);
	}

    STDMETHOD(EnumEffects)(LPDIENUMEFFECTSCALLBACK callback, LPVOID ref, DWORD type)
	{
		return m_device->EnumEffects(callback, ref, type);
	}

    STDMETHOD(GetEffectInfo)(LPDIEFFECTINFO effect_info, REFGUID rguid)
	{
		return m_device->GetEffectInfo(effect_info, rguid);
	}

    STDMETHOD(GetForceFeedbackState)(LPDWORD state)
	{
		return m_device->GetForceFeedbackState(state);
	}

    STDMETHOD(SendForceFeedbackCommand)(DWORD flags)
	{
		return m_device->SendForceFeedbackCommand(flags);
	}

    STDMETHOD(EnumCreatedEffectObjects)(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK callback, LPVOID ref, DWORD flags)
	{
		return m_device->EnumCreatedEffectObjects(callback, ref, flags);
	}

    STDMETHOD(Escape)(LPDIEFFESCAPE escape)
	{
		return m_device->Escape(escape);
	}

    STDMETHOD(Poll)()
	{
		return m_device->Poll();
	}

	STDMETHOD(SendDeviceData)(DWORD size, LPCDIDEVICEOBJECTDATA data, LPDWORD num_elements, DWORD flags)
	{
		return m_device->SendDeviceData(size, data, num_elements, flags);
	}

	STDMETHOD(EnumEffectsInFile)(LPCTSTR file_name, LPDIENUMEFFECTSINFILECALLBACK callback, LPVOID ref, DWORD flags)
	{
		return m_device->EnumEffectsInFile(file_name, callback, ref, flags);
	}

    STDMETHOD(WriteEffectToFile)(LPCTSTR file_name, DWORD num_entries, LPDIFILEEFFECT effects, DWORD flags)
	{
		return m_device->WriteEffectToFile(file_name, num_entries, effects, flags);
	}

    STDMETHOD(BuildActionMap)(LPDIACTIONFORMAT format, LPCTSTR username, DWORD flags)
	{
		return m_device->BuildActionMap(format, username, flags);
	}

    STDMETHOD(SetActionMap)(LPDIACTIONFORMAT format, LPCTSTR username, DWORD flags)
	{
		return m_device->SetActionMap(format, username, flags);
	}

    STDMETHOD(GetImageInfo)(LPDIDEVICEIMAGEINFOHEADER image_header)
	{
		return m_device->GetImageInfo(image_header);
	}
};
class MyDirectInput8 : public IDirectInput8
{
	IDirectInput8* m_di;
	pfnKeyHandler ptKeyHandler;
public:
	MyDirectInput8(IDirectInput8* di,pfnKeyHandler handler) : m_di(di), ptKeyHandler(handler)
	{
	}

	/*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj)
	{
		return m_di->QueryInterface(riid, ppvObj);
	}

    ULONG STDMETHODCALLTYPE AddRef()
	{
		return m_di->AddRef();
	}

    ULONG STDMETHODCALLTYPE Release()
	{
		ULONG count = m_di->Release();
		if(0 == count)
			delete this;

		return count;
	}

    /*** IDirectInput8 methods ***/
    STDMETHOD(CreateDevice)(REFGUID rguid, IDirectInputDevice8** device, LPUNKNOWN unknown)
	{
		HRESULT hr = m_di->CreateDevice(rguid, device, unknown);
		if(SUCCEEDED(hr) && rguid == GUID_SysKeyboard)
		{
			 //Return our own keyboard device that checks for injected keypresses
			*device = new KeyboardDevice(*device,ptKeyHandler);
		}

		return hr;
	}

    STDMETHOD(EnumDevices)(DWORD devType,LPDIENUMDEVICESCALLBACK callback, LPVOID ref, DWORD flags)
	{
		return m_di->EnumDevices(devType, callback, ref, flags);
	}

    STDMETHOD(GetDeviceStatus)(REFGUID rguid)
	{
		return m_di->GetDeviceStatus(rguid);
	}

    STDMETHOD(RunControlPanel)(HWND owner, DWORD flags)
	{
		return m_di->RunControlPanel(owner, flags);
	}

    STDMETHOD(Initialize)(HINSTANCE instance, DWORD version)
	{
		return m_di->Initialize(instance, version);
	}

    STDMETHOD(FindDevice)(REFGUID rguid, LPCTSTR name, LPGUID guidInstance)
	{
		return m_di->FindDevice(rguid, name, guidInstance);
	}

    STDMETHOD(EnumDevicesBySemantics)(LPCTSTR username, LPDIACTIONFORMAT action,
		LPDIENUMDEVICESBYSEMANTICSCB callback, LPVOID ref, DWORD flags)
	{
		return m_di->EnumDevicesBySemantics(username, action, callback, ref, flags);
	}

    STDMETHOD(ConfigureDevices)(LPDICONFIGUREDEVICESCALLBACK callback, LPDICONFIGUREDEVICESPARAMS params,
		DWORD flags, LPVOID ref)
	{
		return m_di->ConfigureDevices(callback, params, flags, ref);
	}

};
