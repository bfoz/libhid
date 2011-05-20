---
layout: documentation
title: Examples
---

# Enumerating Devices
_libhid_ provides for both synchronous and asynchronous device discovery. To use
the synchronous interface, call `HID::find()` with an optional filter set. The
call will return a list of device objects that match the filters. If no filters
were given, all available devices are returned.

The asynchronous interface is built around an enumerator class. An instance of
the enumerator class can be used to discover connected devices and receive
notifications when devices are attached or removed. If provided with a set of
filters the enumerator will only generate notifications for devices that match
the filters.

## Finding Devices

{% highlight cpp %}
#include <hid.h>;

HID::filter::And filters;
filters.push_back(new VendorID(VENDOR_ID));
filters.push_back(new ProductID(PRODUCT_ID));

// Get a list of available devices matching both PRODUCT_ID and VENDOR_ID
HID::device_list devices = HID::find(&filters);

// Display each device's product string on the console
HID::device_list::iterator i = devices.begin();
for(; i != devices.end(); ++i)
    printf("Found device %s\n", (*i)->product().c_str());
{% endhighlight %}

## The Enumerator

{% highlight cpp %}
#include <hid.h>;

void deviceAttach(HID::enumerator_type* enumerator, HID::device_type* device, void* context)
{
    printf("Found device %s\n", device->product().c_str());
}

void deviceRemoval(HID::enumerator_type* enumerator, HID::device_type* device, void* context)
{
    printf("Lost device %s\n", device->product().c_str());
}

// Create a simple filter object
HID::filter::And filters;
filters.push_back(new VendorID(VENDOR_ID));
filters.push_back(new ProductID(PRODUCT_ID));

// Create an enumerator and register callbacks
HID::enumerator_type enumerator(&filters);
enumerator.setMatchCallback(deviceAttach);
enumerator.setRemovalCallback(deviceRemoval);

if( enumerator->start() )
{
    printf("started the enumerator\n");

    // Start an event loop and block (only needed for console apps)
    HID::run();
}

{% endhighlight %}

**Note** that on _win32_, if the device isn't already open, calling `device->product()`
will automatically open the device read-only, get the product string, and then
close the device. If the device was already open it will remain open after the
call to `device->product()`.

# Opening Devices

Once a particular device has been found (either using `HID::find()` or a
`HID::enumerator_type`), the resulting device object can be opened by calling
its `open()` method. By default, the device is opened in read-only mode. To open
the device in write-only mode, pass `HID::WriteMode` to 'open()'.

{% highlight cpp %}
#include <hid.h>;

// Get a list of all available devices
HID::device_list devices = HID::find();
if( devices.size() )
{
    // Take the first device on the list
    HID::device_type* device = devices.front();

    // Open the device for writing
    device->open(HID::WriteMode);

    // Send the output report contained in buffer
    HID::buffer_type buffer;
    device->output(REPORT_ID, buffer);

    // Close the device, just for fun
    device->close();

    // Let's open it again, but for reading this time
    device->open(HID::ReadMode);

    // Get an input report from the device and store it in buffer
    buffer = device->input(REPORT_ID);

    // Close it for good this time
    device->close();
}
{% endhighlight %}

Of course, a device can be opened for both reading and writing by passing
`(HID::ReadMode | HID::WriteMode)` to `open()`...

{% highlight cpp %}
#include <hid.h>;

// Get a list of all available devices
HID::device_list devices = HID::find();
if( devices.size() )
{
    // Take the first device on the list
    HID::device_type* device = devices.front();

    // Open the device for reading and writing
    device->open(HID::ReadMode | HID::WriteMode);

    // Send the output report contained in buffer
    HID::buffer_type buffer;
    device->output(REPORT_ID, buffer);

    // Get an input report from the device and store it in buffer
    buffer = device->input(REPORT_ID);

    // We're finished with the device, so close it
    device->close();
}
{% endhighlight %}
