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

HID::device_list devices = HID:find(&filters);
if( devices.size() )
{
    HID::device_type& device = devices.front();
    device.open();
    HID::buffer_type buffer;
    device.output(REPORT_ID, buffer);
}
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

# Opening Devices
