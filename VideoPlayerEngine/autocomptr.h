#ifndef G_H_AUTOCOMPTR
#define G_H_AUTOCOMPTR


template<class T>
class AutoComPtr
{
public:
	AutoComPtr();
	AutoComPtr(T *comPtr);
	AutoComPtr(AutoComPtr<T> &other);
	~AutoComPtr();

	void Release(); // COM release
	T *Disown(); // std::<X>_ptr::release
	void Reset(T *newComPtr);

	bool IsValid() const;
	bool IsInvalid() const;

	operator T*() const;
	T *operator->() const;
	AutoComPtr<T> &operator=(AutoComPtr<T> &rhs);
private:
	void Dispose();
	T *m_comPtr;
};

template<class T>
AutoComPtr<T>::AutoComPtr() : m_comPtr(nullptr)
{
}

template<class T>
AutoComPtr<T>::AutoComPtr(T *comPtr) : m_comPtr(comPtr)
{
}

template<class T>
AutoComPtr<T>::AutoComPtr(AutoComPtr<T> &other) : m_comPtr(other.comPtr)
{
	other.m_comPtr = nullptr;
}

template<class T>
AutoComPtr<T>::~AutoComPtr()
{
	Dispose();
}

template<class T>
void AutoComPtr<T>::Release()
{
	Dispose();
}

template<class T>
T *AutoComPtr<T>::Disown()
{
	T *comPtr = m_comPtr;
	m_comPtr = nullptr;

	return comPtr;
}

template<class T>
void AutoComPtr<T>::Reset(T *newComPtr)
{
	T *oldComPtr = m_comPtr;
	m_comPtr = newComPtr;

	if(oldComPtr != nullptr)
	{
		oldComPtr->Release();
	}
}

template<class T>
bool AutoComPtr<T>::IsValid() const
{
	return m_comPtr != nullptr;
}

template<class T>
bool AutoComPtr<T>::IsInvalid() const
{
	return !IsValid();
}

template<class T>
AutoComPtr<T>::operator T *() const
{
	return m_comPtr;
}

template<class T>
T* AutoComPtr<T>::operator ->() const
{
	return m_comPtr;
}

template<class T>
AutoComPtr<T> &AutoComPtr<T>::operator=(AutoComPtr<T> &rhs)
{
	if(this == &rhs)
	{
		return *this;
	}

	m_comPtr = rhs.m_comPtr;
	rhs.m_comPtr = nullptr;

	return *this;
}

template<class T>
void AutoComPtr<T>::Dispose()
{
	if(IsValid())
	{
		m_comPtr->Release();
		m_comPtr = nullptr;
	}
}


#endif

