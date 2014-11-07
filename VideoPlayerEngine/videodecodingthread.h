#ifndef H_VIDEODECODINGTHREAD
#define H_VIDEODECODINGTHREAD

#include "packetsink.h"
#include "types.h"
#include <threadsafequeue.h>
#include "objectpool.h"

class IVideoQueue;


class VideoDecodingThread : public PacketSink
{
public:
	void SetVideoFrameQueue(IVideoQueue &sink);
	void SetObjectPool(ObjectPool<QueuedVideoFrame> &pool);
protected:
	virtual bool Init(int streamIndex);
	virtual bool IsDecodedQueueFull();
	virtual int DecodeProc(AVPacket &packet, bool &hasFrame);

	virtual void OnMediaSeeking();
	virtual void OnClearSeek();
	virtual bool IsSeekDone();
private:
	void Transform10b16b(std::vector<uint8_t> &plane, int planeStride, int width, int height);
	AVCodecContext *m_videoCodecCtx;
	unsigned int m_videoStreamIndex;

	int m_frameWidth;
	int m_frameHeight;
	int m_sampledChromaWidth;
	int m_sampledChromaHeight;

	IVideoQueue *m_videoFrameQueue;
	bool m_useYCbCrPath;

	ObjectPool<QueuedVideoFrame> *m_objectPool;
};


#endif

