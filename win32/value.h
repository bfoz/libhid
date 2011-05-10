#ifndef	HID_WIN32_VALUE_H
#define	HID_WIN32_VALUE_H

#include "common.h"
#include "element.h"

namespace HID
{
    namespace win32
    {
	class value_type;
    }
}

class HID::win32::value_type : public HID::win32::element_type
{
    HIDP_VALUE_CAPS _value;

public:
    value_type(HIDP_VALUE_CAPS& v, unsigned index, device_type* device) : HID::win32::element_type(device), _value(v)
    {
	_name = "Value";
	// If the value caps structure is actually a collapsed range of elements
	//	use the index value to expand the local copy of the structure
	if( _value.IsRange )
	{
	    _value.IsRange = false;
	    _value.NotRange.Usage = v.Range.UsageMin + index;
	    _value.NotRange.DataIndex = v.Range.DataIndexMin + index;
	}
    }
    virtual ~value_type() {};

    // Properties
    virtual bool hasNullState() const { return _value.HasNull; }
    virtual bool hasPreferredState() const { return false; }
    virtual bool isArray() const { return false; }
    virtual bool isCollection()	const { return false; }
    virtual bool isNonLinear() const { return false; }
    virtual bool isRelative() const { return !_value.IsAbsolute; }
    virtual bool isVirtual() const { return false; }
    virtual bool isWrapping() const { return false; }

    // Element value
    double	calibratedValue()	{ return 0; }
    virtual int32_t	integerValue()	{ return 0; }
    double	physicalValue()		{ return 0; }
    size_t	valueLength()		{ return 0; }

    // Getters
    virtual double granularity() const { return 0; }
    virtual std::string& name()	{ return _name;	}
    virtual range_type logicalRange() const 	{ return std::make_pair(_value.LogicalMin, _value.LogicalMax);	}
    virtual range_type physicalRange() const	{ return std::make_pair(_value.PhysicalMin, _value.PhysicalMax);}
    virtual range_type reportedRange()		{ return std::make_pair(_value.LogicalMin, _value.LogicalMax);	}
    virtual uint32_t reportID() const	{ return _value.ReportID;   }
    virtual uint32_t reportSize() const { return 0;	}
    virtual uint32_t reportCount() const{ return 0;	}
    virtual const char*	typeName() const{ return "Value";	}
    virtual uint32_t unit() const	{ return _value.Units;	}
    virtual uint32_t unitExponent() const   { return _value.UnitsExp;	}
    virtual uint32_t usage() const	{ return _value.NotRange.Usage;	}
    virtual uint32_t usagePage() const	{ return _value.UsagePage;	}

    // Setters
    virtual void calibration(double min, double max){}
    virtual void deadzone(double min, double max){}
    virtual void granularity(double _granularity){}
    virtual void saturation(double min, double max){}

    // Win32 specific
    virtual size_t parentCollectionIndex() const { return _value.LinkCollection; }
};

#endif	// HID_WIN32_VALUE_H
