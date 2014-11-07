#ifndef H_PACKETSINK
#define H_PACKETSINK

extern "C"
{
//#include <libavutil/frame.h>
//#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libswscale/swscale.h>
//#include <libswresample/swresample.h>
//#include <libavutil/opt.h>
}
#include <threadsafequeue.h>
#include <SFML/System/Thread.hpp>
#include <atomic>
#include <memory>


class PacketSink
{
public:
	PacketSink();
	bool InitSink(AVFormatContext *formatCtx, unsigned int streamIndex);
	void QueuePacket(AVPacket &packet);
	void Quit();
	bool IsSinkFull();
	int GetByteQueueSize();

	void SignalEndOfMedia();
	bool AreFramesFlushed();

	void OnSeek();
	bool SeekDone();
	void ClearSeek();
protected:
	bool ArePacketsInQueue();
	bool DequeuePacket(AVPacket &packet);
	bool ShouldQuit();
	virtual bool Init(int streamIndex) = 0;
	virtual int DecodeProc(AVPacket &packet, bool &hasFrame) = 0;
	virtual bool IsDecodedQueueFull() = 0;
	virtual bool UseUnbufferedDecoding();
	virtual void OnMediaSeeking();
	virtual void OnClearSeek();
	virtual bool IsSeekDone();

	AVFormatContext *m_formatCtx;
	AVFrame *m_decodedFrame;
	unsigned int m_streamIndex;
private:
	void ThreadProc();
	int DecodeStep(AVPacket &packet, bool &hasFrame);

	AVPacket m_demuxerPacket;
	ThreadSafeQueue<AVPacket> m_packetQueue;
	std::atomic<bool> m_quitThread;
	std::unique_ptr<sf::Thread> m_thread;
	std::atomic<int> m_byteSize;

	std::atomic<bool> m_endOfMedia;
	std::atomic<bool> m_flushedFrames;
	AVPacket m_nullPacket;

	AVCodecContext *m_codecCtx;
	std::atomic<bool> m_seekRequest;
	std::atomic<bool> m_flushedPackets;
};


#endif

