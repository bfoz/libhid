#ifndef	HID_ENUMERATOR_H
#define	HID_ENUMERATOR_H

#include <list>

#include "device.h"

namespace HID
{
    class filter_type;

    typedef std::list<device_type*> device_list;

    class enumerator_type
    {
    public:
	typedef void (*callback_type)(enumerator_type*, device_type*, void* context);

	enumerator_type(filter_type* f=0) : _filter(f), _matchCallback(NULL), _matchContext(NULL) {}
	virtual ~enumerator_type()
	{
	    device_list::iterator i = _devices.begin();
	    for(; i != _devices.end(); ++i)
		delete *i;
	}

	void filter(filter_type* f) { _filter = f;	}
	filter_type* filter() const { return _filter;   }

	virtual bool start() = 0;
	virtual void stop() = 0;

	callback_type setMatchCallback(callback_type callback, void* context=NULL)
	{
	    callback_type temp = callback;
	    _matchCallback = callback;
	    _matchContext = context;
	    return temp;
	}

	callback_type setRemovalCallback(callback_type callback, void* context=NULL)
	{
	    callback_type temp = callback;
	    _removalCallback = callback;
	    _removalContext = context;
	    return temp;
	}

    protected:
	device_list	_devices;
	filter_type*	_filter;
	callback_type   _matchCallback;
	void*		_matchContext;
	callback_type   _removalCallback;
	void*		_removalContext;
    };
}


#endif	// MACOS_ENUMERATOR_H
