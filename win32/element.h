#ifndef	HID_WIN32_ELEMENT_H
#define	HID_WIN32_ELEMENT_H

#include <element.h>

namespace HID
{
    namespace win32
    {
	struct element_type;
    }
}

struct HID::win32::element_type : public HID::element_type
{
    element_type(device_type* device) : HID::element_type(device) {};
    virtual ~element_type() {};

    virtual size_t parentCollectionIndex() const = 0;
};

#endif	// HID_WIN32_ELEMENT_H
