#include <map>

#include "common.h"
#include <dbt.h>

#include <filter.h>

#include "device.h"
#include "enumerator.h"

#define	WINDOW_CLASS	L"HID::enumerator"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

typedef	std::map<HWND, HID::win32::enumerator_type*>	enumerator_map;
static enumerator_map hwnd2enumerators;

static WNDCLASSEX windowClass =
{
    sizeof(WNDCLASSEX), 0, WindowProc, 0, 0, NULL, 0, 0, 0, NULL, WINDOW_CLASS, 0,
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if( (WM_DEVICECHANGE == message) && ((void*)lParam) )
    {
	DEV_BROADCAST_DEVICEINTERFACE *const d = (DEV_BROADCAST_DEVICEINTERFACE*)lParam;
	switch( wParam )
	{
	    case DBT_DEVICEARRIVAL:
	    {
		enumerator_map::const_iterator i = hwnd2enumerators.find(hwnd);
		if( i != hwnd2enumerators.end() )
		    i->second->matched(*d);
		break;
	    }
	    case DBT_DEVICEREMOVECOMPLETE:
	    {
		enumerator_map::const_iterator i = hwnd2enumerators.find(hwnd);
		if( i != hwnd2enumerators.end() )
		    i->second->removed(*d);
		break;
	    }
	}
    }
    else
	return DefWindowProc(hwnd, message, wParam, lParam);

    return true;
}


bool HID::win32::enumerator_type::start()
{
    // Get an instance handle, if needed
    if( !windowClass.hInstance )
    {
	// Bail out on all errors
	if( !(windowClass.hInstance = GetModuleHandle(NULL)) )
	    return false;

	// Register the window class
	if( !RegisterClassEx(&windowClass) )
	    return false;
    }

    // Create an invisible window to receive events
    hwnd = CreateWindow(WINDOW_CLASS, NULL, 0, CW_USEDEFAULT, 0, 0, 0, NULL,
			NULL, windowClass.hInstance, NULL);
    if( !hwnd )
	return false;

    GUID    guid;
    HidD_GetHidGuid(&guid);	// Get a GUID for the HID device class

    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = {0};
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = guid;

    notificationHandle = RegisterDeviceNotification(hwnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
    if( !notificationHandle )
	return false;

    // Save the hwnd for later lookup
    hwnd2enumerators[hwnd] = this;

    return true;
}

void HID::win32::enumerator_type::stop()
{
    if( windowClass.hInstance && notificationHandle )
    {
        UnregisterDeviceNotification(notificationHandle);
	notificationHandle = NULL;
    }
}

/* Add the new device to the device list and notify the registered callback, but
    only if it matches the filter set. Accept all devices if there's no filer set.  */
void HID::win32::enumerator_type::matched(DEV_BROADCAST_DEVICEINTERFACE& d)
{
    device_type* device = new win32::device_type(d.dbcc_name);
    if( !_filter || _filter->accept(*device) )
    {
	// Add the device to the device list
	_devices.push_back(device);

	// Notify the callee
	if( _matchCallback )
	    _matchCallback(this, device, _matchContext);
    }
}

/* Notify the registered callback that a device has been removed, but only if
    the device is on the enumerator's device list. Otherwise, it's probably a
    device that the callee doesn't care about. The device_type object is
    guaranteed to exist until the callback returns	*/
void HID::win32::enumerator_type::removed(DEV_BROADCAST_DEVICEINTERFACE& d)
{
    // Make a temporary device object
    device_type* device = new win32::device_type(d.dbcc_name);

    // Find the device in the list
    device_list::iterator i = _devices.begin();
    for(; i != _devices.end(); ++i)
    {
	if( (*i)->path() == device->path() )
	{
	    // Call the callback
	    if( _removalCallback )
		_removalCallback(this, *i, _removalContext);

	    // Destroy the device object and remove it from the device list
	    HID::device_type* tmp = *i;
	    _devices.remove(*i);
	    delete tmp;

	    break;
	}
    }

    delete device;
}
