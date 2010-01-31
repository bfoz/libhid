#include "util.h"

bool HID::macos::getProperty(IOHIDDeviceRef device, CFStringRef key, long* value)
{
    if( device && (IOHIDDeviceGetTypeID() == CFGetTypeID(device)) )
    {
	CFTypeRef property = IOHIDDeviceGetProperty(device, key);

	// If the result is a number, get its value
	if( property && (CFNumberGetTypeID() == CFGetTypeID(property)) )
	    return CFNumberGetValue((CFNumberRef)property, kCFNumberSInt32Type, value);
    }
    return false;
}
