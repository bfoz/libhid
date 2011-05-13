#ifndef	WIN32_ENUMERATOR_H
#define	WIN32_ENUMERATOR_H

#include <enumerator.h>

#include "common.h"
#include <dbt.h>

namespace HID
{
    namespace win32
    {
	class enumerator_type;
    }
}

class HID::win32::enumerator_type : public HID::enumerator_type
{
    HDEVNOTIFY	notificationHandle;
    HWND	hwnd;

public:
    enumerator_type(filter_type* f=0) : HID::enumerator_type(f), notificationHandle(NULL), hwnd(NULL) {}
    virtual ~enumerator_type()
    {
	stop();
    }

    virtual bool start();
    virtual void stop();

    // Entry points for the device matching and removal callbacks
    void matched(DEV_BROADCAST_DEVICEINTERFACE&);
    void removed(DEV_BROADCAST_DEVICEINTERFACE&);
};

#endif	// WIN32_ENUMERATOR_H
