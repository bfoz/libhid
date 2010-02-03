#ifndef	HID_MACOS_UTIL_H
#define	HID_MACOS_UTIL_H

#include "common.h"

namespace HID
{
    namespace macos
    {
	bool getProperty(IOHIDDeviceRef device, CFStringRef key, long* value);
	long getProductID(IOHIDDeviceRef);
	long getVendorID(IOHIDDeviceRef);
	long getVersionNumber(IOHIDDeviceRef);
    }
}


#endif	// HID_MACOS_UTIL_H
