#include "scopedcominitialisation.h"
#include <Windows.h>


ScopedComInitialisation::ScopedComInitialisation(ApartementType type)
{
	DWORD flags = 0;

	switch (type)
	{
		case ScopedComInitialisation::MTA:
		{
			flags = COINIT_MULTITHREADED;

			break;
		}
		case ScopedComInitialisation::STA:
		{
			flags = COINIT_APARTMENTTHREADED;

			break;
		}
		default:
		{
			flags = COINIT_MULTITHREADED;

			break;
		}
	}

	HRESULT hr = CoInitializeEx(NULL, flags);
	m_initialised = hr == S_OK;
}

ScopedComInitialisation::~ScopedComInitialisation()
{
	CoUninitialize();
}

bool ScopedComInitialisation::IsInitialised()
{
	return m_initialised;
}

