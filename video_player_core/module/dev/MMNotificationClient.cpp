#ifdef WIN32
#include "MMNotificationClient.h"
#include "core_dev_waveout.h"

void OutputDebugPrintf(const char * strOutputString, ...)
{
    return;
#define PUT_PUT_DEBUG_BUF_LEN   1024
    char strBuffer[PUT_PUT_DEBUG_BUF_LEN] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);  //_vsnprintf_s  _vsnprintf
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugStringA(strBuffer);  //OutputDebugString    // OutputDebugStringW
}

void WOutputDebugPrintf(const wchar_t * strOutputString, ...)
{
    return;
#define PUT_PUT_DEBUG_BUF_LEN   1024
	wchar_t strBuffer[PUT_PUT_DEBUG_BUF_LEN] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnwprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);  //_vsnprintf_s  _vsnprintf
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugStringW(strBuffer);  //OutputDebugString    // OutputDebugStringW
}

CMMNotificationClient::CMMNotificationClient(core_dev_waveout* dev) :
_cRef(1),
_pEnumerator(nullptr),
_dev(dev)

{
    ::CoInitialize(nullptr);
	HRESULT hr = S_OK;

	hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr,
		CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void**>(&_pEnumerator));

	if (hr == S_OK)
	{
            cout << "接口创建成功" << endl;
	}
	else
	{
            cout << "接口创建失败" << endl;
	}

	// 注册事件
    hr = _pEnumerator->RegisterEndpointNotificationCallback(this);
	if (hr == S_OK)
	{
		cout << "注册成功" << endl;
	}
	else
	{
		cout << "注册失败" << endl;
	}
}

CMMNotificationClient::~CMMNotificationClient()
{
	SAFE_RELEASE(_pEnumerator)
		::CoUninitialize();
}

// IUnknown methods -- AddRef, Release, and QueryInterface

ULONG STDMETHODCALLTYPE CMMNotificationClient::AddRef()
{
    return static_cast<ULONG>(InterlockedIncrement(&_cRef));
}

ULONG STDMETHODCALLTYPE CMMNotificationClient::Release()
{
    ULONG ulRef = static_cast<ULONG>(InterlockedDecrement(&_cRef));
	if (0 == ulRef)
	{
		delete this;
	}
	return ulRef;
}

HRESULT STDMETHODCALLTYPE CMMNotificationClient::QueryInterface(
	REFIID riid, VOID **ppvInterface)
{
	if (IID_IUnknown == riid)
	{
		AddRef();
        *ppvInterface = this;
	}
	else if (__uuidof(IMMNotificationClient) == riid)
	{
		AddRef();
        *ppvInterface = this;
	}
	else
	{
        *ppvInterface = nullptr;
		return E_NOINTERFACE;
	}
	return S_OK;
}

// Callback methods for device-event notifications.

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDefaultDeviceChanged(
	EDataFlow flow, ERole role,
	LPCWSTR pwstrDeviceId)
{
    if(role != eConsole)
        return S_OK;

        const char  *pszFlow = "?????";
        const char  *pszRole = "?????";

	_PrintDeviceName(pwstrDeviceId);

	switch (flow)
	{
	case eRender:
		pszFlow = "eRender";
        _dev->defaultChanged();
		break;
	case eCapture:
		pszFlow = "eCapture";
		break;
    case eAll:
        _dev->defaultChanged();
        break;
	}

	switch (role)
	{
	case eConsole:
		pszRole = "eConsole";
		break;
	case eMultimedia:
		pszRole = "eMultimedia";
		break;
	case eCommunications:
		pszRole = "eCommunications";
		break;
	}

	OutputDebugPrintf("  -->New default device: flow = %s, role = %s\n",
		pszFlow, pszRole);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
	_PrintDeviceName(pwstrDeviceId);

	printf("  -->Added device\n");
	return S_OK;
};

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
	_PrintDeviceName(pwstrDeviceId);

	printf("  -->Removed device\n");
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceStateChanged(
	LPCWSTR pwstrDeviceId,
	DWORD dwNewState)
{
        const char  *pszState = "?????";

	_PrintDeviceName(pwstrDeviceId);

	switch (dwNewState)
	{
	case DEVICE_STATE_ACTIVE:
		pszState = "ACTIVE";
		break;
	case DEVICE_STATE_DISABLED:
		pszState = "DISABLED";
		break;
	case DEVICE_STATE_NOTPRESENT:
		pszState = "NOTPRESENT";
		break;
	case DEVICE_STATE_UNPLUGGED:
		pszState = "UNPLUGGED";
		break;
	}

	OutputDebugPrintf("  -->New device state is DEVICE_STATE_%s (0x%8.8x)\n",
		pszState, dwNewState);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnPropertyValueChanged(
	LPCWSTR pwstrDeviceId,
	const PROPERTYKEY key)
{
	_PrintDeviceName(pwstrDeviceId);

	printf("  -->Changed device property "
		"{%8.8x-%4.4x-%4.4x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x}#%d\n",
		key.fmtid.Data1, key.fmtid.Data2, key.fmtid.Data3,
		key.fmtid.Data4[0], key.fmtid.Data4[1],
		key.fmtid.Data4[2], key.fmtid.Data4[3],
		key.fmtid.Data4[4], key.fmtid.Data4[5],
		key.fmtid.Data4[6], key.fmtid.Data4[7],
		key.pid);
	return S_OK;
}

DEFINE_PROPERTYKEY(PKEY_Device_FriendlyName, 0xa45c254e, 0xdf1c, 0x4efd,
	0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 14);

// Given an endpoint ID string, print the friendly device name.
HRESULT CMMNotificationClient::_PrintDeviceName(LPCWSTR pwstrId)
{
	HRESULT hr = S_OK;
    IMMDevice *pDevice = nullptr;
    IPropertyStore *pProps = nullptr;
	PROPVARIANT varString;

    CoInitialize(nullptr);
	PropVariantInit(&varString);

    if (_pEnumerator == nullptr)
	{
		// Get enumerator for audio endpoint devices.
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
            nullptr, CLSCTX_INPROC_SERVER,
			__uuidof(IMMDeviceEnumerator),
			(void**)&_pEnumerator);
	}
	if (hr == S_OK)
	{
		hr = _pEnumerator->GetDevice(pwstrId, &pDevice);
	}
	if (hr == S_OK)
	{
		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
	}
	if (hr == S_OK)
	{
		// Get the endpoint device's friendly-name property.
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varString);
	}
	WOutputDebugPrintf(L"----------------------\nDevice name: \"%s\"\n"
		L"  Endpoint ID string: \"%s\"\n",
		(hr == S_OK) ? varString.pwszVal : L"null device",
        (pwstrId != nullptr) ? pwstrId : L"null ID");

	PropVariantClear(&varString);

	SAFE_RELEASE(pProps)
    SAFE_RELEASE(pDevice)
    CoUninitialize();
	return hr;
}
#endif
