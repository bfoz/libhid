#include <cstdarg>
#include <cstdio>

#include "device.h"
#include "util.h"

void HID::macos::device_type::close()
{
    if( _bufferFeature )
    {
	delete[] _bufferFeature;
	_bufferFeature = NULL;
    }
    if( _bufferInput )
    {
	delete[] _bufferInput;
	_bufferInput = NULL;
    }
    if( _bufferOutput )
    {
	delete[] _bufferOutput;
	_bufferOutput = NULL;
    }
    IOHIDDeviceClose(handle, kIOHIDOptionsTypeNone);
}

bool HID::macos::device_type::open(OpenMode)
{
    if( _bufferFeature || _bufferInput || _bufferOutput )   // Already open?
	return true;

    if( IOHIDDeviceOpen(handle, kIOHIDOptionsTypeNone) == kIOReturnSuccess )
    {
	int32_t value;
	CFTypeRef property;

	_lengthFeatureBuffer = 0;
	_lengthInputBuffer = 0;
	_lengthOutputBuffer = 0;

	if((property = IOHIDDeviceGetProperty(handle, CFSTR(kIOHIDMaxFeatureReportSizeKey))) &&
	   (CFNumberGetTypeID() == CFGetTypeID(property)) &&
	    CFNumberGetValue((CFNumberRef)property, kCFNumberSInt32Type, &value) )
	{
	    _lengthFeatureBuffer = value;
	}
	if((property = IOHIDDeviceGetProperty(handle, CFSTR(kIOHIDMaxInputReportSizeKey))) &&
	   (CFNumberGetTypeID() == CFGetTypeID(property)) &&
	    CFNumberGetValue((CFNumberRef)property, kCFNumberSInt32Type, &value) )
	{
	    _lengthInputBuffer = value;
	}
	if((property = IOHIDDeviceGetProperty(handle, CFSTR(kIOHIDMaxOutputReportSizeKey))) &&
	   (CFNumberGetTypeID() == CFGetTypeID(property)) &&
	    CFNumberGetValue((CFNumberRef)property, kCFNumberSInt32Type, &value) )
	{
	    _lengthOutputBuffer = value;
	}

	// Allocate report buffers
	if( _lengthFeatureBuffer )
	    _bufferFeature = new uint8_t[_lengthFeatureBuffer];
	if( _lengthInputBuffer )
	    _bufferInput = new uint8_t[_lengthInputBuffer];
	if( _lengthOutputBuffer )
	    _bufferOutput = new uint8_t[_lengthOutputBuffer];

	// Success?
	if( _bufferFeature || _bufferInput || _bufferOutput )
	    return true;

	// Failure - a HID with no reports is no HID at all
	close();
    }
    return false;
}

bool HID::macos::device_type::read(buffer_type&)
{
    return false;
}

bool HID::macos::device_type::write(const buffer_type&)
{
    return false;
}

// *** Reports ***

bool HID::macos::device_type::feature(unsigned reportID, buffer_type& report)
{
    if( !_bufferFeature || (report.size() > _lengthFeatureBuffer) )
	return false;
    std::copy(report.begin(), report.end(), _bufferFeature);
    return IOHIDDeviceSetReport(handle, kIOHIDReportTypeFeature, reportID, _bufferFeature, _lengthFeatureBuffer) == kIOReturnSuccess;
}

HID::buffer_type HID::macos::device_type::feature(unsigned reportID)
{
    CFIndex length = _lengthFeatureBuffer;

    if( _bufferFeature && (IOHIDDeviceGetReport(handle, kIOHIDReportTypeFeature, reportID, _bufferFeature, &length) == kIOReturnSuccess) )
	return buffer_type(_bufferFeature+1, _bufferFeature+length);
    return buffer_type();
}

HID::buffer_type HID::macos::device_type::input(unsigned reportID)
{
    CFIndex length = _lengthInputBuffer;

    if( _bufferInput && (IOHIDDeviceGetReport(handle, kIOHIDReportTypeInput, reportID, _bufferInput, &length) == kIOReturnSuccess) )
	return buffer_type(_bufferInput+1, _bufferInput+length);
    return buffer_type();
}

bool HID::macos::device_type::output(unsigned reportID, buffer_type& report)
{
    if( !_bufferOutput || (report.size() > _lengthOutputBuffer) )
	return false;
    std::copy(report.begin(), report.end(), _bufferOutput);
    return IOHIDDeviceSetReport(handle, kIOHIDReportTypeOutput, reportID, _bufferOutput, _lengthOutputBuffer) == kIOReturnSuccess;
}

uint16_t HID::macos::device_type::usage()
{
    long value = 0;
    if( !getProperty(handle, CFSTR(kIOHIDDeviceUsageKey), &value) || !value )
	getProperty(handle, CFSTR(kIOHIDPrimaryUsageKey), &value);
    return value;
}

uint16_t HID::macos::device_type::usagePage()
{
    long value = 0;
    if( !getProperty(handle, CFSTR(kIOHIDDeviceUsagePageKey), &value) || !value )
	getProperty(handle, CFSTR(kIOHIDPrimaryUsagePageKey), &value);
    return value;
}
