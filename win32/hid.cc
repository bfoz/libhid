#include <cstdarg>
#include <hid.h>

#include "auto_free.h"
#include "enumerator.h"
#include "device.h"

inline GUID getGUID()
{
    GUID    guid;
    HidD_GetHidGuid(&guid);	// Get a GUID for the HID device class
    return guid;
}

namespace HID
{
    enumerator_type* enumerator(filter_type* f)
    {
	return new win32::enumerator_type(f);
    }
}

HID::device_list HID::find(filter_type* f)
{
    GUID    guid = getGUID();	// Get a GUID for the HID device class
    device_list devices;

    // Get a pointer to the device information set
    HANDLE info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if( INVALID_HANDLE_VALUE == info )
	return devices;

    unsigned i = 0;
    while(1)
    {
	SP_DEVICE_INTERFACE_DATA devInterface = { cbSize : sizeof(SP_DEVICE_INTERFACE_DATA) };
	if( !SetupDiEnumDeviceInterfaces(info, NULL, &guid, i, &devInterface) )
	    break;
	++i;

	// Get the required buffer size for the interface's details
	DWORD size;
	if( !SetupDiGetDeviceInterfaceDetail(info, &devInterface, NULL, 0, &size, NULL) && (122 != GetLastError()))
	    break;

	// Now actually get the detail structure
	//  NOTE: The detail structure contains only the device's path
	auto_free<SP_DEVICE_INTERFACE_DETAIL_DATA> detail((SP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(size));
	detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	if( !SetupDiGetDeviceInterfaceDetail(info, &devInterface, detail.get(), size, NULL, NULL) )
	    break;

	device_type* device = new win32::device_type(detail->DevicePath);
	if( !f || f->accept(*device) )
	    devices.push_back(device);
	else
	    delete device;
    }
    SetupDiDestroyDeviceInfoList(info);
    return devices;
}

// Get and dispatch messages until GetMessage() says otherwise
void HID::run()
{
    MSG message;
    while( GetMessage(&message, NULL, 0, 0) > 0 )
	DispatchMessage(&message);	// Default processing
}
