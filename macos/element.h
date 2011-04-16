#ifndef	HID_MACOS_ELEMENT_H
#define	HID_MACOS_ELEMENT_H

#include <element.h>

#include "common.h"

namespace HID
{
    namespace macos
    {
	class element_type;
    }
}

class HID::macos::element_type : public HID::element_type
{
    IOHIDElementRef _element;
    IOHIDValueRef   _value;

    // Private getters
    bool property(CFStringRef key, int32_t* value);
    double property(CFStringRef key) const;	// Specialized because a double can be NaN

    // Private setters
    bool property(CFStringRef key, int32_t value);
    bool property(CFStringRef key, double value);

public:
    element_type(IOHIDElementRef e, device_type* device) : HID::element_type(device), _element(e), _value(NULL) {};
    virtual ~element_type() {};

    IOHIDElementRef element() const { return _element;	}
    IOHIDElementRef parentIOHIDElement() const	{ return IOHIDElementGetParent(_element);   }

    // Properties
    virtual bool hasNullState() const { return IOHIDElementHasNullState(_element); }
    virtual bool hasPreferredState() const { return IOHIDElementHasPreferredState(_element); }
    virtual bool isArray() const { return IOHIDElementIsArray(_element); }
    virtual bool isCollection()	const { return kIOHIDElementTypeCollection == IOHIDElementGetType(_element); }
    virtual bool isNonLinear() const { return IOHIDElementIsNonLinear(_element); }
    virtual bool isRelative() const { return IOHIDElementIsRelative(_element); }
    virtual bool isVirtual() const { return IOHIDElementIsVirtual(_element); }
    virtual bool isWrapping() const { return IOHIDElementIsWrapping(_element); }

    // Element value
    double	calibratedValue();
    virtual int32_t	integerValue();
    double	physicalValue();
    size_t	valueLength();

    // Getters
    virtual double granularity() const { return property(CFSTR(kIOHIDElementCalibrationGranularityKey)); }
    virtual std::string& name();
    virtual range_type logicalRange() const;
    virtual range_type physicalRange() const;
    virtual range_type reportedRange();
    virtual uint32_t reportID() const { return IOHIDElementGetReportID(_element); }
    virtual uint32_t reportSize() const { return IOHIDElementGetReportSize(_element); }
    virtual uint32_t reportCount() const { return IOHIDElementGetReportCount(_element); }
    virtual const char*	typeName() const;
    virtual uint32_t unit() const { return IOHIDElementGetUnit(_element); }
    virtual uint32_t unitExponent() const { return IOHIDElementGetUnitExponent(_element); }
    virtual uint32_t usage() const { return IOHIDElementGetUsage(_element); }
    virtual uint32_t usagePage()const { return IOHIDElementGetUsagePage(_element); }

    // Setters
    virtual void calibration(double min, double max);
    virtual void deadzone(double min, double max);
    virtual void granularity(double _granularity);
    virtual void saturation(double min, double max);
};

#endif	// HID_MACOS_ELEMENT_H
