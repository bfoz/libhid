#ifndef	MACOS_ENUMERATOR_H
#define	MACOS_ENUMERATOR_H

#include <enumerator.h>

#include "common.h"

namespace HID
{
    namespace macos
    {
	class enumerator_type;
    }
}

class HID::macos::enumerator_type : public HID::enumerator_type
{
    IOHIDManagerRef manager;

public:
    enumerator_type(filter_type* f=0) : HID::enumerator_type(f), manager(NULL) {}
    virtual ~enumerator_type()
    {
	stop();
    }

    virtual bool start();
    virtual void stop();

    // Entry points for the device matching and removal callbacks
    void matched(IOReturn, void*, IOHIDDeviceRef);
    void removed(IOReturn, void*, IOHIDDeviceRef);
};

#endif	// MACOS_ENUMERATOR_H
