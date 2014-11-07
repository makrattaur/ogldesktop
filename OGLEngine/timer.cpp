#include "timer.h"

Timer::Timer()
	: m_started(false)
{
	QueryPerformanceFrequency(&m_frequency);
}

void Timer::Start()
{
	if(m_started)
	{
		return;
	}

	m_started = true;
	QueryPerformanceCounter(&m_reference);
}

double Timer::Stop()
{
	if(!m_started)
	{
		return 0.0;
	}

	m_started = false;
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	return ComputeDifference(currentTime);
}

double Timer::Restart()
{
	if(!m_started)
	{
		return 0.0;
	}

	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	double diff = ComputeDifference(currentTime);

	m_reference = currentTime;

	return diff;
}

double Timer::ComputeDifference(LARGE_INTEGER &currentTime)
{
	return (double)(currentTime.QuadPart - m_reference.QuadPart) / (double)m_frequency.QuadPart;
}

