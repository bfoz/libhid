#ifndef	AUTO_FREE_H
#define	AUTO_FREE_H

template<typename _type>
struct auto_free_ref
{
    _type* _ptr;

    explicit auto_free_ref(_type* p): _ptr(p) { }
};

template<typename _T>
class auto_free
{
    _T* _ptr;

public:
    typedef _T element_type;	// The pointed-to type

    explicit auto_free(element_type* __p = 0) throw() : _ptr(__p) { }
    auto_free(auto_free& __a) throw() : _ptr(__a.release()) { }

    template<typename _T1>
      auto_free(auto_free<_T1>& __a) throw() : _ptr(__a.release()) { }

    ~auto_free() { free(_ptr); }

    auto_free& operator=(auto_free& __a) throw()
    {
	reset(__a.release());
	return *this;
    }

    template<typename _T1>
    auto_free& operator=(auto_free<_T1>& __a) throw()
    {
	reset(__a.release());
	return *this;
    }

    element_type& operator*() const throw() { return *_ptr; }
    element_type* operator->() const throw() { return _ptr; }

    element_type* get() const throw() { return _ptr; }

    element_type* release() throw()
    {
	element_type* __tmp = _ptr;
	_ptr = 0;
	return __tmp;
    }

    void reset(element_type* __p = 0) throw()
    {
	if( __p != _ptr )
	{
	    free(_ptr);
	    _ptr = __p;
	}
    }

    auto_free(auto_free_ref<element_type> __ref) throw() : _ptr(__ref._ptr) {}

    auto_free& operator=(auto_free_ref<element_type> __ref) throw()
    {
	reset(__ref._ptr);
	return *this;
    }

    template<typename _T1>
    operator auto_free_ref<_T1>() throw() { return auto_free_ref<_T1>(release()); }

    template<typename _T1>
    operator auto_free<_T1>() throw() { return auto_free<_T1>(release()); }
};

#endif	// AUTO_FREE_H
