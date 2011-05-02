#include <filter.h>

#include "device.h"
#include "enumerator.h"

// Forward the device matching event to the enumerator object that asked for it
static void device_matched(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{
    if( context && device )
	static_cast<HID::macos::enumerator_type*>(context)->matched(result, sender, device);
}

// Forward the device removal event to the enumerator object that asked for it
static void device_removed(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{
    if( context && device )
	static_cast<HID::macos::enumerator_type*>(context)->removed(result, sender, device);
}

bool HID::macos::enumerator_type::start()
{
    if( !manager )
	manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    if( CFGetTypeID(manager) != IOHIDManagerGetTypeID() )
    {
	CFRelease(manager);
	manager = NULL;
	return false;
    }

    // Set the filter to NULL to cause all devices to be returned
    IOHIDManagerSetDeviceMatching(manager, NULL);

    // Register a device matching callback
    IOHIDManagerRegisterDeviceMatchingCallback(manager, device_matched, this);

    // Register a device removal callback
    IOHIDManagerRegisterDeviceRemovalCallback(manager, device_removed, this);

    // Put the HID manager on the default run loop
    IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    // Open the HID Manager
    return IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone) == kIOReturnSuccess;
}

void HID::macos::enumerator_type::stop()
{
    if( !manager )
	return;

    // Unregister the callbacks
    IOHIDManagerRegisterDeviceMatchingCallback(manager, NULL, this);
    IOHIDManagerRegisterDeviceRemovalCallback(manager, NULL, this);

    // Unschedule the HID manager
    IOHIDManagerUnscheduleFromRunLoop(manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    // Close and release the HID manager
    IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
    CFRelease(manager);
}

/* Add the new device to the device list and notify the registered callback, but
    only if it matches the filter set. Accept all devices if there's no filer set.  */
void HID::macos::enumerator_type::matched(IOReturn result, void* sender, IOHIDDeviceRef deviceref)
{
    if( result && !sender )    // Ignore all errors
	return;

    device_type* device = new macos::device_type(deviceref);
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
void HID::macos::enumerator_type::removed(IOReturn result, void* sender, IOHIDDeviceRef deviceref)
{
    if( result && !sender )    // Ignore all errors
	return;

    // Find the device in the list
    device_list::iterator i = _devices.begin();
    for(; i != _devices.end(); ++i)
    {
	if( static_cast<macos::device_type*>(*i)->device() == deviceref )
	{
	    // Call the callback
	    if( _removalCallback )
		_removalCallback(this, *i, _removalContext);

	    // Destroy the device object and remove it from the device list
	    _devices.remove(*i);
	    delete *i;

	    break;
	}
    }
}
