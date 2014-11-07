#ifndef G_H_UTILS
#define G_H_UTILS

#include <Windows.h>

#include <ostream>
#include <string>

namespace util
{
	std::string getWorkingDirectory();
	std::string getExeDirectory();
	bool FindDesktopWindows(HWND &worker, HWND &defView, HWND &listView);
	extern std::ostream dcout;
}

namespace util
{
namespace iomanip
{
	class hexfmt
	{
	public:
		hexfmt(int digitCount);
		std::ostream &operator()(std::ostream &stream) const;
	private:
		int m_digitCount;
	};
}
}

std::ostream &operator<<(std::ostream &stream, util::iomanip::hexfmt fmt);


#endif

