#include "utils.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <memory>
#include <iomanip>


std::string util::getWorkingDirectory()
{
	size_t count = GetCurrentDirectory(0, NULL);
	std::vector<char> buffer(count);
	GetCurrentDirectoryA(count, buffer.data());

	return std::string(buffer.data(), count - 1);
}

std::string util::getExeDirectory()
{
	std::vector<char> buffer(1024);
	DWORD size = GetModuleFileName(NULL, buffer.data(), buffer.size());

	if(buffer[size - 1] == '\0')
		size--;

	std::string pathToExe(buffer.data(), size);

	return pathToExe.substr(0, pathToExe.find_last_of('\\') + 1);
}

bool util::FindDesktopWindows(HWND &worker, HWND &defView, HWND &listView)
{
	HWND desktop = GetDesktopWindow();

	HWND previousWorker = NULL;
	worker = NULL;
	defView = NULL;
	listView = NULL;

	while(true)
	{
		worker = FindWindowEx(desktop, previousWorker, "WorkerW", "");
		if(worker == NULL)
		{
			break;
		}

		defView = FindWindowEx(worker, NULL, "SHELLDLL_DefView", "");

		if(defView == NULL)
		{
			previousWorker = worker;
		}
		else
		{
			listView = FindWindowEx(defView, NULL, "SysListView32", "FolderView");
			break;
		}
	}

	if(worker == NULL || defView == NULL || listView == NULL)
	{
		return false;
	}

	return true;
}

class DebugOutputBuffer : public std::stringbuf
{
public:
	virtual int sync()
	{
		OutputDebugString(str().c_str());
		str("");

		return 0;
	}
};

std::unique_ptr<DebugOutputBuffer> debugOutputBuffer(new DebugOutputBuffer());
std::ostream util::dcout(debugOutputBuffer.get());

util::iomanip::hexfmt::hexfmt(int digitCount)
	: m_digitCount(digitCount)
{
}

std::ostream &util::iomanip::hexfmt::operator()(std::ostream &stream) const
{
	stream << "0x" << std::hex << std::setfill('0') << std::setw(m_digitCount);

	return stream;
}

std::ostream &operator<<(std::ostream &stream, util::iomanip::hexfmt fmt)
{
	return fmt(stream);
}

