#include "injector.h"

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <autohandle.h>
#include <autohandle_win32.h>


InjectConfig::InjectConfig()
	: silent(false),
	use64Bit(false),
	useDebug(false),
	dllPath(NULL),
	functionName(NULL),
	dataSize(0),
	data(NULL)
{
}

bool InjectProcessCommon(HANDLE process, InjectConfig &ic);

int GetProgramBitness()
{
#if _WIN32 || _WIN64
#if _WIN64
	return 64;
#else
	return 32;
#endif
#else
#error Cannot determine program bitness !
#endif
}

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

bool InjectProcess64(HANDLE process, const char *dllPath, const char *functionName, bool silent)
{
	InjectConfig ic;

	ic.silent = silent;
	ic.useDebug = false;
	ic.dllPath = dllPath;
	ic.functionName = functionName;

	return InjectProcess64(process, ic);
}

bool InjectProcess32(HANDLE process, const char *dllPath, const char *functionName, bool silent)
{
	InjectConfig ic;

	ic.silent = silent;
	ic.useDebug = false;
	ic.dllPath = dllPath;
	ic.functionName = functionName;

	return InjectProcess32(process, ic);
}

bool InjectProcess64(HANDLE process, InjectConfig &ic)
{
	if(GetProcessBitness(process) != 64)
	{
		return false;
	}

	ic.use64Bit = true;

	return InjectProcessCommon(process, ic);
}

bool InjectProcess32(HANDLE process, InjectConfig &ic)
{
	if(GetProcessBitness(process) != 32)
	{
		return false;
	}

	ic.use64Bit = false;

	return InjectProcessCommon(process, ic);
}

struct FunctionPointers
{
	FARPROC loadLibraryA;
	FARPROC getProcAddress;
	FARPROC exitProcess;
	FARPROC exitThread;
	FARPROC freeLibrary;
	FARPROC freeLibraryAndExitThread;
	FARPROC loadLibraryExA;
};

struct PatchData
{
	size_t codeEnd;
	size_t user32LibNameOffset;
	size_t messageBoxANameOffset;
	size_t dllPathOffset;
	size_t functionNameOffset;
	size_t messageBoxTitleOffset;
	size_t noDLLMessageOffset;
	size_t noFunctionMessageOffset;
	size_t dataOffset;
};

struct InjectData
{
	InjectConfig ic;
	FunctionPointers fps;
	PatchData pd;
};

typedef std::map<int, DWORD> ReferenceMap;

void PushBackBuffer(std::vector<char> &v, const char *buffer, size_t count);
void PushBackString(std::vector<char> &v, const std::string &str);
void PushBackStringNullTerminate(std::vector<char> &v, const std::string &str);
std::string GetNoDLLErrorMessage(const char *dllPath);
std::string GetNoFunctionErrorMessage(const char *functionName);
void BuildFunctionPointers(FunctionPointers &fps);
void PickInjectedCode(std::vector<char> &injectedBuffer, InjectConfig &ic);
void BuildInjectedBuffer(std::vector<char> &injectedBuffer, InjectData &id);
bool WriteDWORD(std::vector<char> &buffer, size_t offset, DWORD value);
bool ReadDWORD(std::vector<char> &buffer, size_t offset, DWORD &value);
void BuildLookupTable(ReferenceMap &map, ULONG codeBase, InjectData &id);
void PatchCode(std::vector<char> &buffer, ULONG codeBase, InjectData &id);
void WriteCodeToProcess(std::vector<char> &buffer, HANDLE process, LPVOID injectedProcessMemory);

bool InjectProcessCommon(HANDLE process, InjectConfig &ic)
{
	InjectData id;
	id.ic = ic;

	BuildFunctionPointers(id.fps);

	std::vector<char> injectedBuffer;
	BuildInjectedBuffer(injectedBuffer, id);

	LPVOID injectedProcessMemory = VirtualAllocEx(process, 0, injectedBuffer.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	ULONG codeBase = PtrToUlong(injectedProcessMemory);
	PatchCode(injectedBuffer, codeBase, id);
	WriteCodeToProcess(injectedBuffer, process, injectedProcessMemory);

	//unsigned char *buffer = (unsigned char *)injectedBuffer.data();

	DWORD threadId;
	GenericAutoHandle thread(CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)(injectedProcessMemory), 0, 0, &threadId));
	if(thread.IsInvalid())
	{
		DWORD err = GetLastError();
		std::cout << "Cannot create remote thread ! (" << err << ")" << std::endl;

		return false;
	}

	WaitForSingleObject(thread, INFINITE);

	DWORD exitCode;
	GetExitCodeThread(thread, &exitCode);

	VirtualFreeEx(process, injectedProcessMemory, 0, MEM_RELEASE);

	return exitCode == 0;
}

#include "private/code.h"

void PushBackBuffer(std::vector<char> &v, const char *buffer, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		v.push_back(buffer[i]);
	}
}

void PushBackString(std::vector<char> &v, const std::string &str)
{
	//for (size_t i = 0; i < str.length(); i++)
	//{
	//	v.push_back(str[i]);
	//}

	v.insert(v.end(), str.begin(), str.end());
}

void PushBackStringNullTerminate(std::vector<char> &v, const std::string &str)
{
	PushBackString(v, str);
	v.push_back('\0');
}

std::string messageBoxTitle("Error");
std::string user32LibName("user32.dll");
std::string messageBoxAName("MessageBoxA");

std::string GetNoDLLErrorMessage(const char *dllPath)
{
	std::ostringstream ss;

	ss << "Failed to load the DLL " << dllPath << ".";

	return ss.str();
}

std::string GetNoFunctionErrorMessage(const char *functionName)
{
	std::ostringstream ss;

	ss << "Failed to get the function " << functionName << ".";

	return ss.str();
}

void BuildFunctionPointers(FunctionPointers &fps)
{
	ModuleAutoHandle kernel32(LoadLibrary("kernel32.dll"));

	// Assume that the pointers can be safely truncated to 32 bits on 64-bit Windows.

	fps.loadLibraryA = GetProcAddress(kernel32, "LoadLibraryA");
	fps.getProcAddress = GetProcAddress(kernel32, "GetProcAddress");
	fps.exitProcess = GetProcAddress(kernel32, "ExitProcess");
	fps.exitThread = GetProcAddress(kernel32, "ExitThread");
	fps.freeLibrary = GetProcAddress(kernel32, "FreeLibrary");
	fps.freeLibraryAndExitThread = GetProcAddress(kernel32, "FreeLibraryAndExitThread");
	fps.loadLibraryExA = GetProcAddress(kernel32, "LoadLibraryExA");
}

void PickInjectedCode(std::vector<char> &injectedBuffer, InjectConfig &ic)
{
	const unsigned char *injectedCode;
	int injectedCodeSize;

	if(ic.use64Bit)
	{
		if(ic.useDebug)
		{
			injectedCode = InjectedCodeX64Debug;
			injectedCodeSize = InjectedCodeX64DebugSize;
		}
		else
		{
			injectedCode = InjectedCodeX64;
			injectedCodeSize = InjectedCodeX64Size;
		}
	}
	else
	{
		if(ic.useDebug)
		{
			injectedCode = InjectedCodeX86Debug;
			injectedCodeSize = InjectedCodeX86DebugSize;
		}
		else
		{
			injectedCode = InjectedCodeX86;
			injectedCodeSize = InjectedCodeX86Size;
		}
	}

	PushBackBuffer(injectedBuffer, (char *)injectedCode, injectedCodeSize);
}

void BuildInjectedBuffer(std::vector<char> &injectedBuffer, InjectData &id)
{
	PickInjectedCode(injectedBuffer, id.ic);
	injectedBuffer.push_back('\xCC');
	injectedBuffer.push_back('\xCC');
	injectedBuffer.push_back('\xCC');
	injectedBuffer.push_back('\xCC');

	PatchData &pd = id.pd;
	size_t codeEnd = pd.codeEnd = injectedBuffer.size();

	pd.user32LibNameOffset = injectedBuffer.size() - codeEnd;
	PushBackStringNullTerminate(injectedBuffer, user32LibName);

	pd.messageBoxANameOffset = injectedBuffer.size() - codeEnd;
	PushBackStringNullTerminate(injectedBuffer, messageBoxAName);

	pd.dllPathOffset = injectedBuffer.size() - codeEnd;
	PushBackStringNullTerminate(injectedBuffer, id.ic.dllPath);

	pd.functionNameOffset = injectedBuffer.size() - codeEnd;
	PushBackStringNullTerminate(injectedBuffer, id.ic.functionName);

	pd.messageBoxTitleOffset = injectedBuffer.size() - codeEnd;
	PushBackStringNullTerminate(injectedBuffer, messageBoxTitle);

	pd.noDLLMessageOffset = injectedBuffer.size() - codeEnd;
	std::string noDLLMessage = GetNoDLLErrorMessage(id.ic.dllPath);
	PushBackStringNullTerminate(injectedBuffer, noDLLMessage);

	pd.noFunctionMessageOffset = injectedBuffer.size() - codeEnd;
	std::string noFunctionMessage = GetNoFunctionErrorMessage(id.ic.functionName);
	PushBackStringNullTerminate(injectedBuffer, noFunctionMessage);

	if(id.ic.dataSize != 0)
	{
		pd.dataOffset = injectedBuffer.size() - codeEnd;
		PushBackBuffer(injectedBuffer, (const char *)id.ic.data, id.ic.dataSize);
	}
	else
	{
		pd.dataOffset = 0;
	}
}

bool WriteDWORD(std::vector<char> &buffer, size_t offset, DWORD value)
{
	if(offset + 4 > buffer.size())
	{
		return false;
	}

	for(unsigned int i = 0; i < 4; i++)
	{
		buffer[offset + i] = (value >> (i * 8)) & 0xFF;
	}

	return true;
}

bool ReadDWORD(std::vector<char> &buffer, size_t offset, DWORD &value)
{
	if(offset + 4 > buffer.size())
	{
		return false;
	}

	value = 0;
	for(unsigned int i = 0; i < 4; i++)
	{
		value |= ((unsigned char)buffer[offset + i]) << ((i) * 8);
	}

	return true;
}

#define ABSOLUTE_STRING(field) (DWORD)(codeBase + pd.codeEnd + field)
#define TRUNCATE_POINTER(ptr) (DWORD)(ptr)

void BuildLookupTable(ReferenceMap &map, ULONG codeBase, InjectData &id)
{
	PatchData &pd = id.pd;
	FunctionPointers &fps = id.fps;

	map.insert(std::make_pair(1, ABSOLUTE_STRING(pd.user32LibNameOffset)));
	map.insert(std::make_pair(2, TRUNCATE_POINTER(fps.loadLibraryA)));
	map.insert(std::make_pair(3, ABSOLUTE_STRING(pd.messageBoxANameOffset)));
	map.insert(std::make_pair(4, TRUNCATE_POINTER(fps.getProcAddress)));
	map.insert(std::make_pair(5, ABSOLUTE_STRING(pd.dllPathOffset)));
	map.insert(std::make_pair(6, ABSOLUTE_STRING(pd.messageBoxTitleOffset)));
	map.insert(std::make_pair(7, ABSOLUTE_STRING(pd.noDLLMessageOffset)));
	map.insert(std::make_pair(8, TRUNCATE_POINTER(fps.exitProcess)));
	map.insert(std::make_pair(9, ABSOLUTE_STRING(pd.functionNameOffset)));
	map.insert(std::make_pair(10, ABSOLUTE_STRING(pd.noFunctionMessageOffset)));
	map.insert(std::make_pair(11, TRUNCATE_POINTER(fps.exitThread)));
	map.insert(std::make_pair(12, TRUNCATE_POINTER(fps.freeLibraryAndExitThread)));
	map.insert(std::make_pair(13, TRUNCATE_POINTER(fps.freeLibrary)));
	map.insert(std::make_pair(14, (DWORD)(id.ic.silent ? 1 : 0)));
	map.insert(std::make_pair(15, TRUNCATE_POINTER(fps.loadLibraryExA)));
	map.insert(std::make_pair(16, (pd.dataOffset != 0) ? ABSOLUTE_STRING(pd.dataOffset) : 0));
	map.insert(std::make_pair(17, (DWORD)id.ic.dataSize));
}

#undef TRUNCATE_POINTER
#undef ABSOLUTE_STRING

void PatchCode(std::vector<char> &buffer, ULONG codeBase, InjectData &id)
{
	ReferenceMap map;

	BuildLookupTable(map, codeBase, id);

	DWORD value;
	for(size_t i = 0; i < id.pd.codeEnd - 3; i++)
	{
		if(!ReadDWORD(buffer, i, value))
			break;

		if((value & 0xFFFF00FF) == 0xFEDA00EC)
		{
			int index = (value & 0xFF00) >> 8;
			WriteDWORD(buffer, i, map[index]);
			i += 4;
		}
	}
}


void WriteCodeToProcess(std::vector<char> &buffer, HANDLE process, LPVOID injectedProcessMemory)
{
	DWORD oldProtect;	
	SIZE_T bytesRet;

	// Allow write to block
	VirtualProtectEx(process, injectedProcessMemory, buffer.size(), PAGE_EXECUTE_READWRITE, &oldProtect);

	// Write the injected code
	WriteProcessMemory(process, injectedProcessMemory, buffer.data(), buffer.size(), &bytesRet);

	// Restore page protection
	VirtualProtectEx(process, injectedProcessMemory, buffer.size(), oldProtect, &oldProtect);

	// Make sure our changes are written right away
	FlushInstructionCache(process, injectedProcessMemory, buffer.size());
}

