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
	const uint16_t _productID;
	const uint16_t _vendorID;
	const uint16_t _versionNumber;

    public:
	device_type(const std::string& path, uint16_t pid, uint16_t vid, uint16_t vn) : _path(path), _productID(pid), _vendorID(vid), _versionNumber(vn) {}
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
	uint16_t productID() const { return _productID;	}
	uint16_t vendorID()  const { return _vendorID;	}
	uint16_t versionNumber() const { return _versionNumber;	}
    };
}

#endif	// HID_DEVICE_H
