#ifndef CIRCULARHISTORY_H_G__
#define CIRCULARHISTORY_H_G__

#include <cassert>


template<class T>
class CircularHistory
{
public:
	CircularHistory(int capacity);
	~CircularHistory();
	CircularHistory(const CircularHistory<T> &other);
	void Add(const T &element);
	T &Get(int index) const;
	int Count() const;
	int Capacity() const;
	void Clear();
private:
	int WrapIndex(int index) const;
	int m_capacity;
	int m_count;
	int m_pos;
	T *m_data;
};

template<class T>
CircularHistory<T>::CircularHistory(int capacity) : m_capacity(capacity), m_count(0), m_pos(0)
{
	m_data = new T[m_capacity];
}

template<class T>
CircularHistory<T>::~CircularHistory()
{
	delete[] m_data;
}

template<class T>
CircularHistory<T>::CircularHistory(const CircularHistory<T> &other) : m_capacity(other.m_capacity), m_count(other.m_count), m_pos(other.m_pos)
{
	m_data = new T[m_capacity];

	for(int i = 0; i < m_capacity; i++)
	{
		m_data[i] = other.m_data[i];
	}
}

template<class T>
void CircularHistory<T>::Add(const T &element)
{
	if(m_count < m_capacity)
	{
		m_data[m_count++] = element;
	}
	else
	{
		m_data[m_pos] = element;
		m_pos = WrapIndex(m_pos + 1);
	}
}

template<class T>
T &CircularHistory<T>::Get(int index) const
{
	assert(index < m_count);
	return m_data[WrapIndex(m_pos + index)];
}

template<class T>
int CircularHistory<T>::Count() const
{
	return m_count;
}

template<class T>
int CircularHistory<T>::Capacity() const
{
	return m_capacity;
}

template<class T>
int CircularHistory<T>::WrapIndex(int index) const
{
	return index % m_count;
}

template<class T>
void CircularHistory<T>::Clear()
{
	m_count = 0;
	m_pos = 0;
}


#endif

