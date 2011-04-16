#include <limits>

#include "element.h"

#define	kIOHIDElementReportedMaxKey	"ElementReportedMax"
#define	kIOHIDElementReportedMinKey	"ElementReportedMin"


#pragma mark -
#pragma mark Private getters

bool HID::macos::element_type::property(CFStringRef key, int32_t* value)
{
    CFTypeRef ref = IOHIDElementGetProperty(_element, key);
    if( ref && (CFNumberGetTypeID() == CFGetTypeID(ref)) )
	return CFNumberGetValue((CFNumberRef)ref, kCFNumberSInt32Type, &value);
    return false;
}

/* This is a specialized getter for doubles because a double can be NaN.
    Therefore there's no need to have a seperate return value to indiciate
    success or failure.	*/
double HID::macos::element_type::property(CFStringRef key) const
{
    double value = std::numeric_limits<double>::quiet_NaN();
    CFTypeRef ref = IOHIDElementGetProperty(_element, key);

    if( ref && (CFNumberGetTypeID() == CFGetTypeID(ref)) )
	CFNumberGetValue((CFNumberRef)ref, kCFNumberDoubleType, &value);

    return value;
}

#pragma mark -
#pragma mark Private setters

bool HID::macos::element_type::property(CFStringRef key, int32_t value)
{
    CFNumberRef number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
    if( number )
    {
	IOHIDElementSetProperty(_element, key, number);
	CFRelease(number);
	return true;
    }
    return false;
}

bool HID::macos::element_type::property(CFStringRef key, double value)
{
    CFNumberRef number = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &value);
    if( number )
    {
	IOHIDElementSetProperty(_element, key, number);
	CFRelease(number);
	return true;
    }
    return false;
}

#pragma mark -
#pragma mark Element value

double HID::macos::element_type::calibratedValue()
{
    if( !_value )
	IOHIDDeviceGetValue(IOHIDElementGetDevice(_element), _element, &_value);
    return IOHIDValueGetScaledValue(_value, kIOHIDValueScaleTypeCalibrated);
}

int32_t	HID::macos::element_type::integerValue()
{
    IOHIDValueRef   _value1=NULL;
    if( !_value1 )
	IOHIDDeviceGetValue(IOHIDElementGetDevice(_element), _element, &_value1);
    return IOHIDValueGetIntegerValue(_value1);
}

double HID::macos::element_type::physicalValue()
{
    if( !_value )
	IOHIDDeviceGetValue(IOHIDElementGetDevice(_element), _element, &_value);
    return IOHIDValueGetScaledValue(_value, kIOHIDValueScaleTypePhysical);
}

size_t HID::macos::element_type::valueLength()
{
    return IOHIDValueGetLength(_value);
}

#pragma mark -
#pragma mark Getters

std::string& HID::macos::element_type::name()
{
    if( !(_name.size() && _element) )
    {
	CFStringRef n = IOHIDElementGetName(_element);
	if( n )
	{
	    const char* s = CFStringGetCStringPtr(n, kCFStringEncodingASCII);
	    if( s )
		_name.assign(CFStringGetCStringPtr(n, kCFStringEncodingASCII));
	    else
	    {
		const size_t size = CFStringGetLength(n)+1;
		s = new char[size];
		CFStringGetCString(n, (char*)s, size, kCFStringEncodingASCII);
		_name.assign(s);
		delete[] s;
	    }
	}
	else
	    _name.assign(typeName());
    }
    return _name;
}

const char* HID::macos::element_type::typeName() const
{
    switch( IOHIDElementGetType(_element) )
    {
	case kIOHIDElementTypeInput_Misc:	return "Input(Miscellaneous)";
	case kIOHIDElementTypeInput_Button:	return "Input(Button)";
	case kIOHIDElementTypeInput_Axis:	return "Input(Axis)";
	case kIOHIDElementTypeInput_ScanCodes:	return "Input(ScanCodes)";
	case kIOHIDElementTypeOutput:		return "Output";
	case kIOHIDElementTypeFeature:		return "Feature";
	case kIOHIDElementTypeCollection:
	    switch( IOHIDElementGetCollectionType(_element) )
	    {
		case kIOHIDElementCollectionTypePhysical:	return "Collection(Physical)";
		case kIOHIDElementCollectionTypeApplication:	return "Collection(Application)";
		case kIOHIDElementCollectionTypeLogical:	return "Collection(Logical)";
		case kIOHIDElementCollectionTypeReport:		return "Collection(Report)";
		case kIOHIDElementCollectionTypeNamedArray:	return "Collection(Named Array)";
		case kIOHIDElementCollectionTypeUsageSwitch:	return "Collection(Usage Switch)";
		case kIOHIDElementCollectionTypeUsageModifier:	return "Collection(Usage Modifier)";
		default:					return "Collection";
	    }
    }
    return "Unknown";
}

HID::element_type::range_type HID::macos::element_type::logicalRange() const
{
    return std::make_pair(IOHIDElementGetLogicalMin(_element), IOHIDElementGetLogicalMax(_element));
}

HID::element_type::range_type HID::macos::element_type::physicalRange() const
{
    return std::make_pair(IOHIDElementGetPhysicalMin(_element), IOHIDElementGetPhysicalMax(_element));
}

HID::element_type::range_type HID::macos::element_type::reportedRange()
{
    range_type range;

    if( !property(CFSTR(kIOHIDElementReportedMinKey), (int32_t*)&range.first) )
	if( !property(CFSTR(kIOHIDElementMaxKey), (int32_t*)&range.first) )
	    range.first = std::numeric_limits<range_type::first_type>::max();

    if( !property(CFSTR(kIOHIDElementReportedMaxKey), (int32_t*)&range.second) )
	if( !property(CFSTR(kIOHIDElementMinKey), (int32_t*)&range.second) )
	    range.second = std::numeric_limits<range_type::first_type>::min();

    return range;
}

#pragma mark -
#pragma mark Setters

void HID::macos::element_type::calibration(double min, double max)
{
    property(CFSTR(kIOHIDElementCalibrationMinKey), min);
    property(CFSTR(kIOHIDElementCalibrationMaxKey), max);
}

void HID::macos::element_type::deadzone(double min, double max)
{
    property(CFSTR(kIOHIDElementCalibrationSaturationMinKey), min);
    property(CFSTR(kIOHIDElementCalibrationSaturationMaxKey), max);
}

void HID::macos::element_type::granularity(double _granularity)
{
    property(CFSTR(kIOHIDElementCalibrationGranularityKey), _granularity);
}

void HID::macos::element_type::saturation(double min, double max)
{
    property(CFSTR(kIOHIDElementCalibrationDeadZoneMinKey), min);
    property(CFSTR(kIOHIDElementCalibrationDeadZoneMaxKey), max);
}
