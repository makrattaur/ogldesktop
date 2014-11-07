#ifndef H_SCOPEDOGLCONTEXT
#define H_SCOPEDOGLCONTEXT

//#include <Windows.h>
//#include <utils_win32.h>
//#include <utils_io.h>


template<class TContext>
class ScopedOGLContext
{
public:
	ScopedOGLContext(TContext &context)
		: m_context(context)
	{
//		util::dcout << "Context " << util::HexPrint((void *)&m_context) << " current for thread " << GetCurrentThreadId() << std::endl;
		m_context.setActive(true);
	}
	~ScopedOGLContext()
	{
		m_context.setActive(false);
//		util::dcout << "Context " << util::HexPrint((void *)&m_context) << " not current for thread " << GetCurrentThreadId() << std::endl;
	}
private:
	TContext &m_context;
};


#endif
