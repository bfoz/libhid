/*  USB HID interface
*/

#ifndef	HID_H
#define	HID_H

#include <list>

#include "device.h"
#include "filter.h"

namespace HID
{
    typedef std::list<device_type*> device_list;

    device_list find(filter_type* f=NULL);
}

#endif	// HID_H
