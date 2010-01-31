#include <cstdarg>
#include <cstdio>

#include "device.h"

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
    const uint8_t num = lstrlen((LPCTSTR)c) * sizeof(TCHAR);
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

HID::win32::device_type::device_type(const TCHAR* p, const HIDD_ATTRIBUTES& attributes) :
	    HID::device_type(stringFromTCHAR(p), attributes.ProductID, attributes.VendorID,
	    attributes.VersionNumber), _tpath(p), handle(INVALID_HANDLE_VALUE),
	    _preparsedData(NULL), _capabilities(NULL), _bufferFeatureReport(NULL),
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

    handle = CreateFile(_tpath.c_str(), m, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if( INVALID_HANDLE_VALUE == handle )
	return false;

    return true;
}

bool HID::win32::device_type::read(buffer_type& buffer)
{
    if( INVALID_HANDLE_VALUE == handle )
	return false;

    DWORD num = 0;
    const size_t length = capabilities()->InputReportByteLength;
    uint8_t* b = bufferInputReport();

    if( !ReadFile(handle, b, length, &num, NULL) )
	return false;

    buffer.reserve(num);
    buffer.assign(b, b+num);
    return true;
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

    return WriteFile(handle, b, capabilities()->OutputReportByteLength, &num, NULL);
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
    return HidD_SetFeature(handle, b, report.size()+1);
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

    return HidD_SetOutputReport(handle, b, length);
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
