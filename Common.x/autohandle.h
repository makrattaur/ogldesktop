#ifndef H_AUTOHANDLE
#define H_AUTOHANDLE

#include <Windows.h>

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue = 0>
class AutoHandle
{
public:
	AutoHandle();
	AutoHandle(T handle);
	// transfer handle ownership with copy ctor.
	AutoHandle(AutoHandle<T, CloseFunction, InvalidValue> &other);
	~AutoHandle();

	void Close();
	T Disown();
	void Reset(T newHandle);

	bool IsValid() const;
	bool IsInvalid() const;

	operator T() const;
	// transfer handle ownership with assignement.
	AutoHandle<T, CloseFunction, InvalidValue>& operator=(AutoHandle<T, CloseFunction, InvalidValue> &rhs);
private:
	void Dispose();
	T m_handle;
};

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
AutoHandle<T, CloseFunction, InvalidValue>::AutoHandle() : m_handle(InvalidValue)
{
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
AutoHandle<T, CloseFunction, InvalidValue>::AutoHandle(T handle) : m_handle(handle)
{
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
AutoHandle<T, CloseFunction, InvalidValue>::AutoHandle(AutoHandle<T, CloseFunction, InvalidValue> &other) : m_handle(other.m_handle)
{
	other.m_handle = InvalidValue;
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
AutoHandle<T, CloseFunction, InvalidValue>::~AutoHandle()
{
	Dispose();
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
void AutoHandle<T, CloseFunction, InvalidValue>::Close()
{
	Dispose();
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
T AutoHandle<T, CloseFunction, InvalidValue>::Disown()
{
	T copy = m_handle;
	m_handle = InvalidValue;

	return copy;
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
void AutoHandle<T, CloseFunction, InvalidValue>::Reset(T newHandle)
{
	T copy = m_handle;
	m_handle = newHandle;

	if(copy != InvalidValue)
	{
		CloseFunction(copy);
	}
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
bool AutoHandle<T, CloseFunction, InvalidValue>::IsValid() const
{
	return !(m_handle == InvalidValue);
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
bool AutoHandle<T, CloseFunction, InvalidValue>::IsInvalid() const
{
	return !IsValid();
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
AutoHandle<T, CloseFunction, InvalidValue>::operator T() const
{
	return m_handle;
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
AutoHandle<T, CloseFunction, InvalidValue>& AutoHandle<T, CloseFunction, InvalidValue>::operator=(AutoHandle<T, CloseFunction, InvalidValue> &rhs)
{
	if(this == &rhs) // self-assignement.
		return *this;

	m_handle = rhs.m_handle;
	rhs.m_handle = InvalidValue;

	return *this;
}

template<class T, BOOL (WINAPI * CloseFunction)(T), T InvalidValue>
void AutoHandle<T, CloseFunction, InvalidValue>::Dispose()
{
	if(IsValid())
	{
		CloseFunction(m_handle);
		m_handle = InvalidValue;
	}
}


typedef AutoHandle<HANDLE, CloseHandle, NULL> GenericAutoHandle;


#endif

