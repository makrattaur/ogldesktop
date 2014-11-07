#ifndef H_AUDIODECODINGTHREAD
#define H_AUDIODECODINGTHREAD

#include "objectpool.h"
#include "packetsink.h"
#include "audiothread.h"


class AudioDecodingThread : public PacketSink
{
public:
	void SetAudioThread(AudioThread &audioThread);
protected:
	virtual bool Init(int streamIndex);
	virtual bool IsDecodedQueueFull();
	virtual int DecodeProc(AVPacket &packet, bool &hasFrame);
private:
	AVCodecContext *m_audioCodecCtx;

	AudioThread *m_audioThread;
};


#endif

