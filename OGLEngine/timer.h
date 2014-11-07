#ifndef G_H_TIMER
#define G_H_TIMER

#include "enginecommon.h"
#include <Windows.h>


// Times are in seconds.
class ENGINE_API Timer
{
public:
	Timer();
	void Start();
	double Stop();
	double Restart();
private:
	double ComputeDifference(LARGE_INTEGER &currentTime);

	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_reference;
	bool m_started;
};

#endif

