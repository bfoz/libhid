#include <cstdarg>
#include <cstdio>

#include "common.h"
#include "device.h"
#include "button.h"
#include "value.h"
#include "collection.h"

#ifdef __GNUC__
extern "C"
{
// mingw doesn't define these (as of w32api-3.13)
WINHIDSDI BOOL WINAPI HidD_GetInputReport(HANDLE, void*, ULONG);
WINHIDSDI BOOL WINAPI HidD_SetOutputReport(HANDLE, void*, ULONG);
}
#endif

std::string stringFromTCHAR(const TCHAR* c)
{
    std::string s;
    const uint8_t num = lstrlen((LPCTSTR)c);
    for(unsigned k = 0; k < num; ++c, ++k)
	s.push_back(wctob(*c));
    return s;
}

uint8_t* HID::win32::device_type::bufferFeatureReport()
{
    if( !_bufferFeatureReport && capabilities() )
	_bufferFeatureReport = new uint8_t[capabilities()->FeatureReportByteLength];
    return _bufferFeatureReport;
}

uint8_t* HID::win32::device_type::bufferInputReport()
{
    if( !_bufferInputReport && capabilities() )
	_bufferInputReport = new uint8_t[capabilities()->InputReportByteLength];
    return _bufferInputReport;
}

uint8_t* HID::win32::device_type::bufferOutputReport()
{
    if( !_bufferOutputReport && capabilities() )
	_bufferOutputReport = new uint8_t[capabilities()->OutputReportByteLength];
    return _bufferOutputReport;
}

const HIDD_ATTRIBUTES& HID::win32::device_type::attributes()
{
    bool opened = false;

    if( sizeof(_attributes) == _attributes.Size )
	return _attributes;

    // Try to open the device if it isn't already open
    if( (INVALID_HANDLE_VALUE == handle) && !(opened = open(ReadMode)) )
	return _attributes;

    _attributes.Size = sizeof(_attributes);
    if( !HidD_GetAttributes(handle, &_attributes) )
	memset(&_attributes, 0, sizeof(_attributes));

    if( opened )	// Don't leave the device open if it wasn't open before
	close();

    return _attributes;
}

HIDP_CAPS* HID::win32::device_type::capabilities()
{
    if( !_capabilities )
    {
	_capabilities = new HIDP_CAPS;
	if( !preparsedData() || (HidP_GetCaps(preparsedData(), _capabilities) != HIDP_STATUS_SUCCESS) )
	    _capabilities = NULL;
    }
    return _capabilities;
}

PHIDP_PREPARSED_DATA HID::win32::device_type::preparsedData()
{
    bool opened = false;

    if( _preparsedData )
	return _preparsedData;

    // Try to open the device if it isn't already open
    if( (INVALID_HANDLE_VALUE == handle) && !(opened = open(ReadMode)) )
	return NULL;

    if( !HidD_GetPreparsedData(handle, &_preparsedData) )
	_preparsedData = NULL;

    if( opened )	// Don't leave the device open if it wasn't open before
	close();

    return _preparsedData;
}

HID::win32::device_type::device_type(const TCHAR* p) :
	    HID::device_type(stringFromTCHAR(p)), _tpath(p),
	    handle(INVALID_HANDLE_VALUE), _preparsedData(NULL),
	    _capabilities(NULL), _bufferFeatureReport(NULL),
	    _bufferInputReport(NULL), _bufferOutputReport(NULL)
{
    _attributes.Size = 0;
}

HID::win32::device_type::device_type(const TCHAR* p, const HIDD_ATTRIBUTES& _attr) :
	    HID::device_type(stringFromTCHAR(p)), _tpath(p), handle(INVALID_HANDLE_VALUE),
	    _attributes(_attr), _preparsedData(NULL), _capabilities(NULL), _bufferFeatureReport(NULL),
	    _bufferInputReport(NULL), _bufferOutputReport(NULL) {}

void HID::win32::device_type::close()
{
    if( INVALID_HANDLE_VALUE == handle )
	return;
    CloseHandle(handle);
    handle = INVALID_HANDLE_VALUE;
}

bool HID::win32::device_type::open(OpenMode mode)
{
    long m = 0;

    if( INVALID_HANDLE_VALUE != handle )
	return true;

    if( mode & ReadMode )
	m |= GENERIC_READ;
    if( mode & WriteMode )
	m |= GENERIC_WRITE;

    overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    overlapped.Offset = 0;
    overlapped.OffsetHigh = 0;

    handle = CreateFile(_tpath.c_str(), m, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if( INVALID_HANDLE_VALUE == handle )
	return false;

    return true;
}

// Use overlapped IO here to prevent report functions from blocking while
//  waiting for interrupt reports. But read() is expected to block, so wrap the
//  overlapped IO in a while loop, and stay there until a report is received.
bool HID::win32::device_type::read(buffer_type& buffer)
{
    if( INVALID_HANDLE_VALUE == handle )
	return false;

    const size_t length = capabilities()->InputReportByteLength;
    uint8_t* b = bufferInputReport();

    DWORD num = 0;
    const size_t time = 100;
    bool run = true;
    while(run)
    {
	if( INVALID_HANDLE_VALUE == handle )
	    return false;
	ReadFile(handle, b, length, NULL, &overlapped);
	switch( WaitForSingleObject(overlapped.hEvent, time) )
	{
	    case WAIT_OBJECT_0:
		GetOverlappedResult(handle, &overlapped, &num, 0);
		run = false;
		break;
	    case WAIT_TIMEOUT:
		CancelIo(handle);
		break;
	    default:
		run = false;
		CancelIo(handle);
		break;
	}
    }

    if( num )
    {
	buffer.reserve(num);
	buffer.assign(b, b+num);
	return true;
    }
    else
	buffer.clear();

    return false;
}

bool HID::win32::device_type::write(const buffer_type& buffer)
{
    DWORD num;

    if( INVALID_HANDLE_VALUE == handle )
	return false;
    if( buffer.size() > capabilities()->OutputReportByteLength )
	return false;

    uint8_t* b = bufferInputReport();
    std::copy(buffer.begin(), buffer.end(), b);

    return WriteFile(handle, b, capabilities()->OutputReportByteLength, &num, NULL) != FALSE;
}

#if !defined(_MSC_VER)
#pragma mark -
#pragma mark Report elements
#endif

// Fetch the button capabilities and generate element_type objects for each
void HID::win32::device_type::button_elements(elements_type& _buttons)
{
    capabilities();	// Ensure that the HIDP_CAPS structure is populated

    // Get the array lengths from _capabilities
    USHORT numFeature = _capabilities->NumberFeatureButtonCaps;
    USHORT numInput = _capabilities->NumberInputButtonCaps;
    USHORT numOutput = _capabilities->NumberOutputButtonCaps;
    size_t numberOfCaps = numFeature + numInput + numOutput;

    if (numberOfCaps <= 0)
        return;

    std::vector<HIDP_BUTTON_CAPS> buffer(numberOfCaps);

    HidP_GetButtonCaps(HidP_Feature, &buffer[0], &numFeature, _preparsedData);

    HIDP_BUTTON_CAPS *const _inputButtons = &buffer[numFeature];
    HidP_GetButtonCaps(HidP_Input, _inputButtons, &numInput, _preparsedData);

    HIDP_BUTTON_CAPS *const _outputButtons = &_inputButtons[numInput];
    HidP_GetButtonCaps(HidP_Output, _outputButtons, &numOutput, _preparsedData);

    // Update the array length in case HidP_GetButtonCaps() returned different lengths
    numberOfCaps = numFeature + numInput + numOutput;

    // Store the structures
    for(size_t i = 0; i < numberOfCaps; ++i)
    {
	// If the item is a range, generate elements for each member
	if( buffer[i].IsRange )
	{
	    unsigned k = 0;
	    for(unsigned j=buffer[i].Range.DataIndexMin; j < buffer[i].Range.DataIndexMax; ++j, ++k)
	    {
		element_type *const element = new button_type(buffer[i], k, this);
		if( element )
		    _buttons.push_back(element);
	    }
	}
	else
	{
	    element_type *const element = new button_type(buffer[i], 0, this);
	    if( element )
		_buttons.push_back(element);
	}
    }
}

// Fetch the value capabilities and generate element_type objects for each
void HID::win32::device_type::value_elements(elements_type& _values)
{
    capabilities();	// Ensure that the HIDP_CAPS structure is populated

    // Get the array lengths from _capabilities
    USHORT numFeature = _capabilities->NumberFeatureValueCaps;
    USHORT numInput = _capabilities->NumberInputValueCaps;
    USHORT numOutput = _capabilities->NumberOutputValueCaps;
    size_t numberOfCaps = numFeature + numInput + numOutput;

    if (numberOfCaps <= 0)
	return;

    std::vector<HIDP_VALUE_CAPS> buffer(numberOfCaps);

    if (numFeature > 0) {
	HidP_GetValueCaps(HidP_Feature, &buffer[0], &numFeature, _preparsedData);
    }

    if (numInput > 0) {
	HidP_GetValueCaps(HidP_Input, &buffer[numFeature], &numInput, _preparsedData);
    }

    if (numOutput > 0) {
	HidP_GetValueCaps(HidP_Output, &buffer[numFeature + numInput], &numOutput, _preparsedData);
    }

    // Update the array length in case HidP_GetButtonCaps() returned different lengths
    numberOfCaps = numFeature + numInput + numOutput;

    // Store the structures
    for(size_t i = 0; i < numberOfCaps; ++i)
    {
	// If the item is a range, generate elements for each member
	if( buffer[i].IsRange )
	{
	    for(unsigned j=buffer[i].Range.DataIndexMin; j < buffer[i].Range.DataIndexMax; ++j)
	    {
		element_type *const element = new HID::win32::value_type(buffer[i], j, this);
		if( element )
		    _values.push_back(element);
	    }
	}
	else
	{
	    element_type *const element = new win32::value_type(buffer[i], 0, this);
	    if( element )
		_values.push_back(element);
	}
    }
}

HID::elements_type& HID::win32::device_type::elements()
{
    if( _elements.size() )
	return _elements;

    // Ensure that the preparsed data and HIDP_CAPS structures are populated
    if( !capabilities() )
	return _elements;

    // A temporary container for holding elements that haven't been reparented yet
    elements_type temp;

    // Get the collection list
    /*	NOTE: The reparenting loop at the bottom of this function requires that
      		the link collections be at the beginning of the temp container
      		and in the same order returned by HidP_GetLinkCollectionNodes()	*/
    unsigned long length = _capabilities->NumberLinkCollectionNodes;
    std::vector<HIDP_LINK_COLLECTION_NODE> nodes(length);
    HidP_GetLinkCollectionNodes(&nodes[0], &length, _preparsedData);
    for(size_t i=0; i < length; ++i)
    {
	element_type *const collection = new collection_type(nodes[i], this);

	// Everything goes into the temporary container
	temp.push_back(collection);
    }
    if (length > 0)
    {
	/* The top-level collections were divided into different physical device object (PDO) in Windows
	   (see: http://msdn.microsoft.com/en-us/library/windows/hardware/ff543475(v=vs.85).aspx). 
	   Therefore, there is only one top-level collection, others are sub-collections. */
	_elements.push_back(temp[0]);
	link_subcollections(temp, nodes);
    }

    // Get the button elements
    button_elements(temp);

    // Get the value elements
    value_elements(temp);

    // Walk the temporary container and reparent anything that has a parent (skip collections)
    elements_type::iterator it = temp.begin() + length;
    for(; it != temp.end(); ++it) {
	const unsigned parent = static_cast<win32::element_type*>(*it)->parentCollectionIndex();
	if (parent < length)
	    temp[parent]->children().push_back(*it);
    }

    return _elements;
}

// *** Reports ***

bool HID::win32::device_type::feature(unsigned reportID, buffer_type& report)
{
    size_t length = capabilities()->FeatureReportByteLength;
    if( report.size() > (length-1) )
	return false;

    uint8_t* b = bufferFeatureReport();

    HidP_InitializeReportForID(HidP_Feature, reportID, preparsedData(), (char*)b, length);
    std::copy(report.begin(), report.end(), b+1);
    return HidD_SetFeature(handle, b, report.size()+1) != FALSE;
}

HID::buffer_type HID::win32::device_type::feature(unsigned reportID)
{
    size_t length = capabilities()->FeatureReportByteLength;
    uint8_t* b = bufferFeatureReport();

    HidP_InitializeReportForID(HidP_Feature, reportID, preparsedData(), (char*)b, length);
    if( HidD_GetFeature(handle, b, length) )
	return buffer_type(b+1, b+length);
    return buffer_type();
}

HID::buffer_type HID::win32::device_type::input(unsigned reportID)
{
    size_t length = capabilities()->InputReportByteLength;
    uint8_t* b = bufferInputReport();

    HidP_InitializeReportForID(HidP_Input, reportID, preparsedData(), (char*)b, length);
    if( HidD_GetInputReport(handle, b, length) )
	return buffer_type(b+1, b+length);
    return buffer_type();
}

bool HID::win32::device_type::output(unsigned reportID, buffer_type& report)
{
    size_t length = capabilities()->OutputReportByteLength;
    if( report.size() > (length-1) )
	return false;

    uint8_t* b = bufferOutputReport();

    HidP_InitializeReportForID(HidP_Output, reportID, preparsedData(), (char*)b, length);
    std::copy(report.begin(), report.end(), b+1);

    return HidD_SetOutputReport(handle, b, length) != FALSE;
}

const std::string& HID::win32::device_type::manufacturer()
{
    if( _manufacturer.size()==0 )
    {
	bool opened = false;

	// If the handle is invalid, try opening the device in read mode
	if( INVALID_HANDLE_VALUE == handle )
	    if( !(opened = open(ReadMode)) )
		return _manufacturer;

	char* s = new char[256];
	HidD_GetManufacturerString(handle, s, 256);
	_manufacturer = stringFromTCHAR((const TCHAR*)s);
	delete[] s;

	if( opened )	// Don't leave the device open if it wasn't open before
	    close();
    }

    return _manufacturer;
}

const std::string& HID::win32::device_type::product()
{
    if( _product.size()==0 )
    {
	bool opened = false;

	// If the handle is invalid, try opening the device in read mode
	if( INVALID_HANDLE_VALUE == handle )
	    if( !(opened = open(ReadMode)) )
		return _product;

	char* s = new char[256];
	HidD_GetProductString(handle, s, 256);
	_product = stringFromTCHAR((const TCHAR*)s);
	delete[] s;

	if( opened )	// Don't leave the device open if it wasn't open before
	    close();
    }

    return _product;
}

uint16_t HID::win32::device_type::usage()
{
    if( !capabilities() )
	return 0;
    return capabilities()->Usage;
}

uint16_t HID::win32::device_type::usagePage()
{
    if( !capabilities() )
	return 0;
    return capabilities()->UsagePage;
}

void HID::win32::device_type::link_subcollections(elements_type& elements, std::vector<HIDP_LINK_COLLECTION_NODE>& nodes, size_t index)
{
    /* refer: http://msdn.microsoft.com/en-us/library/windows/hardware/ff542355(v=vs.85).aspx#ddk_link_collection_array_kg. */
    const size_t firstChild = nodes[index].FirstChild;
    if (firstChild <= 0)
	return;

    if (nodes[firstChild].FirstChild > 0)
	link_subcollections(elements, nodes, firstChild);
    elements[index]->children().push_back(elements[firstChild]);
    for (HIDP_LINK_COLLECTION_NODE* node = &nodes[nodes[index].FirstChild];
	node->NextSibling > 0;
	node = &nodes[node->NextSibling])
    {
	const size_t nextSibling = node->NextSibling;
	if (nodes[nextSibling].FirstChild > 0)
	    link_subcollections(elements, nodes, nextSibling);
	elements[index]->children().push_back(elements[nextSibling]);
    }
}
