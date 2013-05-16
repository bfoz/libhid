#ifndef	HID_FILTER_H
#define	HID_FILTER_H

#include "device.h"

namespace HID
{
    namespace filter
    {
	class And;
	class Not;
	class Or;

	class productID;
	class usage;
	class usagePage;
	class vendorID;

	class gamepad;		// Game Pad usage in Generic Desktop page
	class joystick;		// Joystick usage in Generic Desktop page
    }
    class filter_type;
    class filter_set;
}

class HID::filter_type
{
public:
    virtual ~filter_type() {}
    virtual bool accept(device_type&) = 0;
};

class HID::filter_set : public filter_type
{
protected:
    typedef std::vector<filter_type*> child_list_type;

    child_list_type children;

public:
    virtual ~filter_set()
    {
	child_list_type::iterator i = children.begin();
	for(; i != children.end(); ++i)
	    delete *i;
    }
    void push_back(filter_type* f)  { children.push_back(f); }
};

class HID::filter::And : public filter_set
{
    bool accept(device_type& device)
    {
	child_list_type::iterator i = children.begin();
	for(; i != children.end(); ++i)
	{
	    if( !((*i)->accept(device)) )
		return false;
	}
	return true;
    }
};

class HID::filter::Not : public filter_type
{
    filter_type* child;

public:
    Not(filter_type* f) : child(f) {}
    virtual ~Not() { delete child; }
    bool accept(device_type& device)
    {
	return child && !(child->accept(device));
    }
};

class HID::filter::Or : public filter_set
{
    bool accept(device_type& device)
    {
	child_list_type::iterator i = children.begin();
	for(; i != children.end(); ++i)
	{
	    if( (*i)->accept(device) )
		return true;
	}
	return false;
    }
};

class HID::filter::productID : public filter_type
{
    unsigned pid;

public:
    productID(unsigned p) : pid(p) {}
    bool accept(device_type& device)
    {
	return pid == device.productID();
    }
};

class HID::filter::usage : public filter_type
{
    unsigned _usage;

public:
    usage(unsigned u) : _usage(u) {}
    bool accept(device_type& device)
    {
	return _usage == device.usage();
    }
};

class HID::filter::usagePage : public filter_type
{
    unsigned _usagePage;

public:
    usagePage(unsigned u) : _usagePage(u) {}
    bool accept(device_type& device)
    {
	return _usagePage == device.usagePage();
    }
};

class HID::filter::vendorID : public filter_type
{
    unsigned vid;

public:
    vendorID(unsigned v) : vid(v) {}
    bool accept(device_type& device)
    {
	return vid == device.vendorID();
    }
};

// Filter for Game Pad usage in the Generic Desktop usage page
class HID::filter::gamepad : public filter_type
{
public:
    bool accept(device_type& device)
    {
	return (1 == device.usagePage()) && (5 == device.usage());
    }
};

// Filter for Joystick usage in the Generic Desktop usage page
class HID::filter::joystick : public filter_type
{
public:
    bool accept(device_type& device)
    {
	return (1 == device.usagePage()) && (4 == device.usage());
    }
};

#endif	// HID_FILTER_H
