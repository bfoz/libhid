/* Interface class for USB HID devices
*/

#ifndef	HID_DEVICE_H
#define	HID_DEVICE_H

#include <string>
#include <vector>

namespace HID
{
    typedef enum { ReadMode=1, WriteMode=2 } OpenMode;
    typedef std::vector<uint8_t> buffer_type;

    class device_type
    {
	const std::string _path;

    public:
	device_type(const std::string& path) : _path(path) {}
	virtual ~device_type() {}

	virtual void close()=0;
	virtual bool open(OpenMode)=0;
	virtual bool read(buffer_type&)=0;
	virtual bool write(const buffer_type&)=0;

	// Reports
	virtual bool feature(unsigned reportID, buffer_type&)=0;
	virtual buffer_type feature(unsigned reportID)=0;
	virtual buffer_type input(unsigned reportID)=0;
	virtual bool output(unsigned reportID, buffer_type&)=0;

	// Getters
	const std::string& path() const { return _path;	}
	virtual uint16_t productID()=0;
	virtual uint16_t usage()=0;
	virtual uint16_t usagePage()=0;
	virtual uint16_t vendorID()=0;
	virtual uint16_t versionNumber()=0;
    };
}

#endif	// HID_DEVICE_H
