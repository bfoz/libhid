---
layout: documentation
title: Examples
---

# Finding Devices

{% highlight cpp %}
#include <hid.h>;

HID::filter:And filters;
filters.push_back(new VendorID(VENDOR_ID));
filters.push_back(new ProductID(PRODUCT_ID));

HID::device_list devices = HID:find(&filters);
if( devices.size() )
{
    HID::device_type& device = devices.front();
    HID::buffer_type buffer;
    device.output(REPORT_ID, buffer);
}
{% endhighlight %}

# Opening Devices
