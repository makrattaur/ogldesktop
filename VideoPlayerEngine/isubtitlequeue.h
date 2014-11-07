#ifndef H_ISUBTITLEQUEUE
#define H_ISUBTITLEQUEUE

#include <memory>
#include "types.h"

class ISubtitleQueue
{
public:
	virtual void QueueSubtitle(std::unique_ptr<QueuedSubtitle> frame) = 0;
	virtual void OnSeek() = 0;
protected:
	~ISubtitleQueue() { };
};

#endif
