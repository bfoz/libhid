# Introduction

libhid is a cross-platform C++ library for interacting with [USB Human Interface Devices](http://en.wikipedia.org/wiki/USB_human_interface_device_class).

# Installation

## git submodule

To use libhid as a submodule in your project's git repository:

```
git submodule add git://github.com/bfoz/libhid.git *path*
```

where *path* is the path to put the submodule (relative to your repository root). Then follow the instructions below for linking the library into your project.

# Linking

## Qt / qmake

Simply include `libhid.pri` in your project's `.pro` file. qmake will take care of the rest.

Alternatively, just add

```
SUBDIRS += libhid
```

to your project's `.pro` file.

# Example

Here is a simple example that finds a device by it's Product ID and Vendor ID, opens the device, and then sends a simple output report.

```cpp
	#include <hid.h>

	HID::filter::And filters;
	filters.push_back(new VendorID(VENDOR_ID));
	filters.push_back(new ProductID(PRODUCT_ID));

	HID::device_list devices = HID::find(&filters);
	if( devices.size() )
	{
	    HID::device_type& device = devices.front();
	    HID::buffer_type buffer;
	    device.output(REPORT_ID, buffer);
	}
```

Devices can also be found using an enumerator object. The enumerator generates notifications whenever a device is added or removed and matches an optional filter (the same filter objects used by `find()`). Here's a quick example…

```cpp
	#include <hid.h>

	// Called when a device is added that matches the enumerator's filter
	void matched(HID::enumerator_type* enumerator, HID::device_type* device, void* context)
	{}

	// Called when a matching device is removed
	void removed(HID::enumerator_type* enumerator, HID::device_type* device, void* context)
	{}

	// Create a USB HID enumerator to watch for device events and dispatch them accordingly
	HID::enumerator_type* enumerator = HID::enumerator();
	enumerator->setMatchCallback(matched, &my_context);
	enumerator->setRemovalCallback(removed, &my_context);    
```
