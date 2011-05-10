#ifndef	HID_WIN32_BUTTON_H
#define	HID_WIN32_BUTTON_H

#include "common.h"
#include "element.h"

namespace HID
{
    namespace win32
    {
	class button_type;
    }
}

class HID::win32::button_type : public HID::win32::element_type
{
    HIDP_BUTTON_CAPS _button;

public:
    button_type(HIDP_BUTTON_CAPS& b, unsigned index, device_type* device) : HID::win32::element_type(device), _button(b)
    {
	_name = "Button";
	// If the button caps structure is actually a collapsed range of elements
	//	use the index value to expand the local copy of the structure
	if( _button.IsRange )
	{
	    _button.IsRange = false;
	    _button.NotRange.Usage = b.Range.UsageMin + index;
	    _button.NotRange.DataIndex = b.Range.DataIndexMin + index;
	}
    }
    virtual ~button_type() {};

    // Properties
    virtual bool hasNullState() const { return false; }
    virtual bool hasPreferredState() const { return false; }
    virtual bool isArray() const { return false; }
    virtual bool isCollection()	const { return false; }
    virtual bool isNonLinear() const { return false; }
    virtual bool isRelative() const { return false; }
    virtual bool isVirtual() const { return false; }
    virtual bool isWrapping() const { return false; }

    // Element value
    double	calibratedValue()   	{ return 0;	}
    virtual int32_t	integerValue()	{ return 0;	}
    double	physicalValue()   	{ return 0;	}
    size_t	valueLength()   	{ return 0;	}

    // Getters
    virtual double granularity() const { return 0; }
    virtual std::string& name()	{ return _name;	}
    virtual range_type logicalRange() const { return std::make_pair(0, 1);  }
    virtual range_type physicalRange() const{ return std::make_pair(0, 1);  }
    virtual range_type reportedRange()	    { return std::make_pair(0, 1);  }
    virtual uint32_t reportID() const { return _button.ReportID; }
    virtual uint32_t reportSize() const { return 0; }
    virtual uint32_t reportCount() const { return 0; }
    virtual const char*	typeName() const	{ return "Button";	}
    virtual uint32_t unit() const { return 0; }
    virtual uint32_t unitExponent() const { return 0; }
    virtual uint32_t usage() const { return _button.NotRange.Usage; }
    virtual uint32_t usagePage()const { return _button.UsagePage; }

    // Setters
    virtual void calibration(double min, double max){}
    virtual void deadzone(double min, double max){}
    virtual void granularity(double _granularity){}
    virtual void saturation(double min, double max){}

    // Win32 specific
    virtual size_t parentCollectionIndex() const { return _button.LinkCollection; }
};

#endif	// HID_WIN32_BUTTON_H
