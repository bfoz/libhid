#ifndef	HID_MACOS_UTIL_H
#define	HID_MACOS_UTIL_H

#include "common.h"

namespace HID
{
    namespace macos
    {
	bool getProperty(IOHIDDeviceRef device, CFStringRef key, long* value);
    }
}


#endif	// HID_MACOS_UTIL_H
