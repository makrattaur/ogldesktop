#ifndef H_IVIDEOQUEUE
#define H_IVIDEOQUEUE

#include <memory>
#include "types.h"

class IVideoQueue
{
public:
	virtual void QueueFrame(QueuedVideoFrame *frame) = 0;
	virtual size_t FrameQueueSize() = 0;
	virtual void SetSeek() = 0;
	virtual void ClearSeek() = 0;
	virtual bool AllFramesDisplayed() = 0;
protected:
	~IVideoQueue() { };
};

#endif
