#ifndef	HID_MACOS_DEVICE_H
#define	HID_MACOS_DEVICE_H

#include <string>

#include <device.h>

#include "common.h"

namespace HID
{
    namespace macos
    {
	class device_type;
    }
}

class HID::macos::device_type : public HID::device_type
{
    IOHIDDeviceRef  handle;
    size_t _lengthFeatureBuffer;
    size_t _lengthInputBuffer;
    size_t _lengthOutputBuffer;

    uint8_t*	_bufferFeature;
    uint8_t*	_bufferInput;
    uint8_t*	_bufferOutput;    

public:
    device_type(IOHIDDeviceRef d, uint16_t pid, uint16_t vid, uint16_t vn) : HID::device_type("", pid, vid, vn), handle(d), _bufferFeature(0), _bufferInput(0), _bufferOutput(0) {}
    virtual ~device_type()
    {
	close();
    }

    virtual void close();
    virtual bool open(OpenMode);
    virtual bool read(buffer_type&);
    virtual bool write(const buffer_type&);

    // Reports
    virtual bool feature(unsigned reportID, buffer_type&);
    virtual buffer_type feature(unsigned reportID);
    virtual buffer_type input(unsigned reportID);
    virtual bool output(unsigned reportID, buffer_type&);
};

#endif	// HID_MACOS_DEVICE_H
