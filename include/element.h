/*  USB Hid Report Element abstraction
*/

#ifndef	HID_ELEMENT_H
#define	HID_ELEMENT_H

#include <string>
#include <vector>

// workaround for no 'stdint.h' support before msvc 10
#if defined(_MSC_VER) && _MSC_VER < 1600
#   include "stdint.h"
#else
#   include <stdint.h>
#endif

#if defined(_MSC_VER)
    typedef int ssize_t;
#endif

namespace HID
{
    class device_type;
    class element_type;
    typedef std::vector<element_type*>	elements_type;
}

class HID::element_type
{
protected:
    device_type*    _device;
    elements_type   _children;

    std::string	    _name;

public:
    // Use a std::pair to describe a range of signed values: first=min, second=max
    typedef	std::pair<ssize_t,ssize_t>	range_type;

    element_type(device_type* device) : _device(device) {};
    virtual ~element_type() {};

    elements_type&  children()	{ return _children; }
    device_type* device() const { return _device; }

    // Properties
    virtual bool hasNullState() const=0;
    virtual bool hasPreferredState() const=0;
    virtual bool isArray() const=0;
    virtual bool isCollection()	const=0;
    virtual bool isNonLinear() const=0;
    virtual bool isRelative() const=0;
    virtual bool isVirtual() const=0;
    virtual bool isWrapping() const=0;

    // Element value
    virtual int32_t	integerValue()=0;

    // Getters
    virtual std::string& name()=0;
    virtual range_type logicalRange() const=0;
    virtual range_type physicalRange() const=0;
    virtual range_type range() const { return logicalRange(); }
    virtual uint32_t reportID() const=0;
    virtual uint32_t reportSize() const=0;
    virtual uint32_t reportCount() const=0;
    virtual const char*	typeName() const=0;
    virtual uint32_t unit() const=0;
    virtual uint32_t unitExponent() const=0;
    virtual uint32_t usage() const=0;
    virtual uint32_t usagePage()const=0;
};

#endif	// HID_ELEMENT_H
