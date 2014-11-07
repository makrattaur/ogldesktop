#ifndef G_H_OBJECTPOOL
#define G_H_OBJECTPOOL

#include <memory>
#include <queue>
#include <mutex>
//#include <iostream>


template<class T>
class ObjectPool
{
public:
	ObjectPool();
	ObjectPool(size_t initialCapacity, bool expandable);
	T *GetInstance();
	void ReturnInstance(T *instance);
private:
	void Init(size_t initialCapacity, bool expandable);
	T *AddObject(bool addToFreeList);
	T *AddObjectNoLock(bool addToFreeList);

	std::mutex m_mutex;
	std::queue<std::unique_ptr<T>> m_objects;
	std::queue<T *> m_freeObjects;
	bool m_expandable;
};

template<class T>
ObjectPool<T>::ObjectPool()
{
	Init(10, true);
}

template<class T>
ObjectPool<T>::ObjectPool(size_t initialCapacity, bool expandable)
{
	Init(initialCapacity, expandable);
}

template<class T>
T *ObjectPool<T>::GetInstance()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if(m_freeObjects.empty())
	{
		return m_expandable ? AddObjectNoLock(false) : NULL;
	}
	else
	{
		T *obj = m_freeObjects.front();
		m_freeObjects.pop();

		return obj;
	}
}

template<class T>
void ObjectPool<T>::ReturnInstance(T *instance)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_freeObjects.push(instance);
}

template<class T>
void ObjectPool<T>::Init(size_t initialCapacity, bool expandable)
{
	m_expandable = expandable;

	for(size_t i = 0; i < initialCapacity; i++)
	{
		AddObjectNoLock(true);
	}
}

template<class T>
T *ObjectPool<T>::AddObject(bool addToFreeList)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	return AddObjectNoLock(addToFreeList);
}

template<class T>
T *ObjectPool<T>::AddObjectNoLock(bool addToFreeList)
{
	T *obj = new T();
	m_objects.push(std::unique_ptr<T>(obj));
	if(addToFreeList)
	{
		m_freeObjects.push(obj);
	}
//	else
//	{
//		std::cout << "expanded list of type " << typeid(T).name() << ", new cap " << m_objects.size() << std::endl;
//	}

	return obj;
}

#endif
