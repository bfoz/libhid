/*  USB HID interface
*/

#ifndef	HID_H
#define	HID_H

#include <list>

#include "device.h"

namespace HID
{
    typedef std::list<device_type*> device_list;
}

#endif	// HID_H
