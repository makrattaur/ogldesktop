#ifndef H_THREADSAFEQUEUE
#define H_THREADSAFEQUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

template<class T>
class ThreadSafeQueue
{
public:
	T& dequeue();
	bool tryDequeue(T& elem);
	T& peek();
	T dequeueValue();
	void enqueue(const T &elem);
	size_t size();
	bool empty();
	T& front();
	void pop();
private:
	std::queue<T> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_emptyQueueCondition;
};

template<class T>
T& ThreadSafeQueue<T>::dequeue()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	while(m_queue.empty())
		m_emptyQueueCondition.wait(lock);

	T& elem = m_queue.front();
	m_queue.pop();
	return elem;
}

template<class T>
bool ThreadSafeQueue<T>::tryDequeue(T& elem)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if(m_queue.empty())
		return false;

	elem = m_queue.front();
	m_queue.pop();

	return true;
}

template<class T>
T& ThreadSafeQueue<T>::peek()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	while(m_queue.empty())
		m_emptyQueueCondition.wait(lock);

	T& elem = m_queue.front();
	return elem;
}

template<class T>
T ThreadSafeQueue<T>::dequeueValue()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	while(m_queue.empty())
		m_emptyQueueCondition.wait(lock);

	T elem = m_queue.front();
	m_queue.pop();
	return elem;
}

template<class T>
void ThreadSafeQueue<T>::enqueue(const T &elem)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_queue.push(elem);
	lock.unlock();
	m_emptyQueueCondition.notify_one();
}

template<class T>
size_t ThreadSafeQueue<T>::size()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return m_queue.size();
}

template<class T>
bool ThreadSafeQueue<T>::empty()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return m_queue.empty();
}

template<class T>
T& ThreadSafeQueue<T>::front()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	return m_queue.front();
}

template<class T>
void ThreadSafeQueue<T>::pop()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_queue.pop();
}


#endif

