#include <iostream>
#include <string>
#include <sstream>

#include <autohandle.h>
#include <autohandletypes.h>


typedef BOOL (WINAPI *PFN_ISWOW64PROCESS)(HANDLE hProcess, PBOOL Wow64Process);

int GetProcessBitness(HANDLE process)
{
	SYSTEM_INFO systemInfo;
	GetNativeSystemInfo(&systemInfo);
	
	if(systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) // 32-bit Windows do not have WOW64.
	{
		return 32;
	}
	else if(systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
	{
		ModuleAutoHandle kernel32(LoadLibrary("kernel32.dll"));

		if(kernel32.IsInvalid())
			return -1; // !?!?

		PFN_ISWOW64PROCESS isWow64Process = (PFN_ISWOW64PROCESS)GetProcAddress(kernel32, "IsWow64Process");
	
		if(isWow64Process == NULL) // No WOW64 layer present, presume 32 bit windows.
		{
			return 32;
		}

		BOOL state;
		if(isWow64Process(process, &state) == 0)
		{
			return -2;
		}

		return state == TRUE ? 32 : 64;
	}
	else
	{
		return -3; // !?!?
	}
}

int main(int argc, char *argv[])
{
	{
		std::ostringstream ss;
		ss << "Current process bitness: " << GetProcessBitness(GetCurrentProcess());
		std::cout << ss.str() << std::endl;
		MessageBox(0, ss.str().c_str(), "Information", MB_ICONINFORMATION);
	}

	{
		SYSTEM_INFO systemInfo;
		GetNativeSystemInfo(&systemInfo);

		std::ostringstream ss;
		ss << "Processor bitness: " << (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "PROCESSOR_ARCHITECTURE_AMD64" : "PROCESSOR_ARCHITECTURE_INTEL");
		std::cout << ss.str() << std::endl;
		MessageBox(0, ss.str().c_str(), "Information", MB_ICONINFORMATION);
	}

	return 0;
}

