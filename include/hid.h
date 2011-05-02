/*  USB HID interface
*/

#ifndef	HID_H
#define	HID_H

#include "enumerator.h"
#include "device.h"
#include "filter.h"

namespace HID
{
    enumerator_type* enumerator(filter_type* f=NULL);
    device_list find(filter_type* f=NULL);
}

#endif	// HID_H
