#include <hid.h>

#include "enumerator.h"
#include "device.h"

namespace HID
{
    enumerator_type* enumerator(filter_type* f)
    {
	return new macos::enumerator_type(f);
    }

    device_list find(filter_type* f)
    {
	device_list devices;

	IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if( CFGetTypeID(manager) != IOHIDManagerGetTypeID() )
	    return device_list();

	// Setting the filter to NULL causes all devices to be returned
	IOHIDManagerSetDeviceMatching(manager, NULL);

	// Open the HID Manager
	if( IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess )
	    return device_list();

	CFSetRef deviceCFSetRef = IOHIDManagerCopyDevices(manager);
	if( !deviceCFSetRef )
	    return device_list();

	CFIndex deviceIndex, deviceCount = CFSetGetCount(deviceCFSetRef);

	IOHIDDeviceRef* tIOHIDDeviceRefs = (IOHIDDeviceRef*)malloc( sizeof(IOHIDDeviceRef)*deviceCount );
	if( !tIOHIDDeviceRefs )
	    return device_list();

	CFSetGetValues(deviceCFSetRef, (const void**)tIOHIDDeviceRefs);

	for( deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++ )
	{
	    const IOHIDDeviceRef d = tIOHIDDeviceRefs[deviceIndex];
	    device_type* device = new macos::device_type(d);
	    if( !f || f->accept(*device) )
		devices.push_back(device);
	    else
		delete device;
	}

	if( manager )
	{
	    IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
	    CFRelease(manager);
	}

	return devices;
    }

    // Block until the run loop finishes
    void run()
    {
	CFRunLoopRun();
    }
}
