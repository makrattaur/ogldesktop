#ifndef H_SUBTITLEDECODINGTHREAD
#define H_SUBTITLEDECODINGTHREAD

#include "packetsink.h"

class ISubtitleQueue;


class SubtitleDecodingThread : public PacketSink
{
public:
	void SetSubtitleQueue(ISubtitleQueue &queue);
protected:
	virtual bool Init(int streamIndex);
	virtual bool IsDecodedQueueFull();
	virtual int DecodeProc(AVPacket &packet, bool &hasFrame);
	virtual bool UseUnbufferedDecoding();
	virtual void OnMediaSeeking();
private:
	static void ResetSubtitile(AVSubtitle *sub);

	AVCodecContext *m_subtitleCodecCtx;
	unsigned int m_subtitleStreamIndex;

	ISubtitleQueue *m_subtitleQueue;
	AVSubtitle m_subtitle;
};

#endif
