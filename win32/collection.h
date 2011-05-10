#ifndef	HID_WIN32_COLLECTION_H
#define	HID_WIN32_COLLECTION_H

#include "common.h"
#include "element.h"

namespace HID
{
    namespace win32
    {
	class collection_type;
    }
}

class HID::win32::collection_type : public HID::win32::element_type
{
    HIDP_LINK_COLLECTION_NODE _collection;

public:
    collection_type(HIDP_LINK_COLLECTION_NODE& c, device_type* device) : HID::win32::element_type(device), _collection(c)
    {
	_name = "Collection";
	_children.reserve(_collection.NumberOfChildren);
    }
    virtual ~collection_type() {};

    // Properties
    virtual bool hasNullState() const { return false; }
    virtual bool hasPreferredState() const { return false; }
    virtual bool isArray() const { return false; }
    virtual bool isCollection()	const { return true; }
    virtual bool isNonLinear() const { return false; }
    virtual bool isRelative() const { return false; }
    virtual bool isVirtual() const { return false; }
    virtual bool isWrapping() const { return false; }

    // Element value
    virtual int32_t	integerValue()	{ return 0;	}

    // Getters
    virtual double granularity() const { return 0; }
    virtual std::string& name()	{ return _name;	}
    virtual range_type logicalRange() const { return std::make_pair(0, 0);  }
    virtual range_type physicalRange() const{ return std::make_pair(0, 0);  }
    virtual uint32_t reportID() const { return 0; }
    virtual uint32_t reportSize() const { return 0; }
    virtual uint32_t reportCount() const { return 0; }
    virtual const char*	typeName() const    { return "Collection";	}
    virtual uint32_t unit() const { return 0; }
    virtual uint32_t unitExponent() const { return 0; }
    virtual uint32_t usage() const { return _collection.LinkUsage; }
    virtual uint32_t usagePage()const { return _collection.LinkUsagePage; }

    // Setters
    virtual void calibration(double min, double max){}
    virtual void deadzone(double min, double max){}
    virtual void granularity(double _granularity){}
    virtual void saturation(double min, double max){}

    // Win32 specific
    virtual size_t parentCollectionIndex() const { return _collection.Parent; }
};

#endif	// HID_WIN32_COLLECTION_H
