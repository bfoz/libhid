#include <cstdarg>
#include <cstdio>

#include "device.h"
#include "util.h"

/* An IN report callback that stops its run loop when called. This is purely for
    emulating blocking behavior in the device_type::read() method.  */
static void input_oneshot(void*		    context,
			  IOReturn	    result,
			  void*		    deviceRef,
			  IOHIDReportType   type,
			  uint32_t	    reportID,
			  uint8_t*	    report,
			  CFIndex	    length)
{
    HID::buffer_type *const buffer = static_cast<HID::buffer_type*>(context);

    /* If the report is valid, copy it into the caller's buffer
	The Report ID is prepended to the buffer so the caller can identify the
	report	*/
    if( buffer )
    {
	buffer->clear();	// Return an empty buffer on error
	if( !result && report && deviceRef )
	{
	    buffer->reserve(length+1);
	    buffer->push_back(reportID);
	    buffer->insert(buffer->end(), report, report+length);
	}
    }

    CFRunLoopStop(CFRunLoopGetCurrent());
}


void HID::macos::device_type::close()
{
    // Close the IOHIDDevice if it was open
    if( _bufferFeature || _bufferInput || _bufferOutput )
	IOHIDDeviceClose(handle, kIOHIDOptionsTypeNone);

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

// Block while waiting for an IN interrupt report
bool HID::macos::device_type::read(buffer_type& buffer)
{
    // Register a callback
    IOHIDDeviceRegisterInputReportCallback(handle, _bufferInput, _lengthInputBuffer, input_oneshot, &buffer);

    // Schedule the device on the current run loop in case it isn't already scheduled
    IOHIDDeviceScheduleWithRunLoop(handle, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    // Trap in the run loop until a report is received
    CFRunLoopRun();

    // The run loop has returned, so unschedule the device
    IOHIDDeviceUnscheduleFromRunLoop(handle, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    if( buffer.size() )
	return true;
    return false;
}

bool HID::macos::device_type::write(const buffer_type&)
{
    return false;
}

#pragma mark -
#pragma mark Report elements

HID::elements_type& HID::macos::device_type::elements()
{
    if( _elements.size() )
	return _elements;

    /* Put all of the elements into a temporary container and also put the
	top-level elements into the _elements container. Then walk the temporary
	container and reparent anything that isn't a top-level element. The end
	result should be the proper hierarchy rooted in the _elements container.
    */
    CFArrayRef a = IOHIDDeviceCopyMatchingElements(handle, NULL, kIOHIDOptionsTypeNone);
    if( a )
    {
	const size_t length = CFArrayGetCount(a);
	elements_type	temp;
	for(size_t i = 0; i < length; ++i)
	{
	    IOHIDElementRef ref = (IOHIDElementRef)CFArrayGetValueAtIndex(a, i);
	    if( ref )
	    {
		element_type *const element = new element_type(ref, this);
		if( element )
		{
		    temp.push_back(element);

		    IOHIDElementRef parent = element->parentIOHIDElement();
		    if( !parent )    // Top-level item?
			_elements.push_back(element);
		}
	    }
	}

	// Reparent anything with a parent
	elements_type::iterator i = temp.begin();
	for(; i != temp.end(); ++i)
	{
	    IOHIDElementRef parent = ((macos::element_type*)*i)->parentIOHIDElement();
	    if( parent )
	    {
		// Find the parent
		elements_type::iterator j = temp.begin();
		for(; j != temp.end(); ++j)
		{
		    if( ((macos::element_type*)*j)->element() == parent )
		    {
			((macos::element_type*)*j)->children().push_back(*i);
			break;
		    }
		}
	    }
	}
    }
    return _elements;
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

const std::string& HID::macos::device_type::manufacturer()
{
    if( (_manufacturer.size()==0) && handle )
    {
	CFStringRef n = (CFStringRef)IOHIDDeviceGetProperty(handle, CFSTR(kIOHIDManufacturerKey));
	if( n )
	{
	    const char* s = CFStringGetCStringPtr(n, kCFStringEncodingASCII);
	    if( s )
		_manufacturer.assign(s);
	    else
	    {
		const size_t size = CFStringGetLength(n)+1;
		s = new char[size];
		CFStringGetCString(n, (char*)s, size, kCFStringEncodingASCII);
		_manufacturer.assign(s);
		delete[] s;
	    }
	}
    }

    return _manufacturer;
}

const std::string& HID::macos::device_type::product()
{
    if( (_product.size()==0) && handle )
    {
	CFStringRef n = (CFStringRef)IOHIDDeviceGetProperty(handle, CFSTR(kIOHIDProductKey));
	if( n )
	{
	    const char* s = CFStringGetCStringPtr(n, kCFStringEncodingASCII);
	    if( s )
		_product.assign(s);
	    else
	    {
		const size_t size = CFStringGetLength(n)+1;
		s = new char[size];
		CFStringGetCString(n, (char*)s, size, kCFStringEncodingASCII);
		_product.assign(s);
		delete[] s;
	    }
	}
    }

    return _product;
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
