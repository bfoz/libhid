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

long HID::macos::getProductID( IOHIDDeviceRef device )
{
    long result = 0;
    HID::macos::getProperty(device, CFSTR( kIOHIDProductIDKey ), &result );
    return result;
}

long HID::macos::getVendorID( IOHIDDeviceRef device )
{
    long result = 0;
    HID::macos::getProperty(device, CFSTR( kIOHIDVendorIDKey ), &result );
    return result;
}

long HID::macos::getVersionNumber( IOHIDDeviceRef device )
{
    long result = 0;
    HID::macos::getProperty(device, CFSTR( kIOHIDVersionNumberKey ), &result );
    return result;
}
