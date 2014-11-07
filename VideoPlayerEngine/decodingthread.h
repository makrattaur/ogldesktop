#ifndef H_DECODINGTHREAD
#define H_DECODINGTHREAD


extern "C"
{
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <queue>
#include <memory>
#include <atomic>
#include <SFML/System/Thread.hpp>
#include <threadsafequeue.h>
#include <unordered_map>

#include "types.h"
#include "audiothread.h"
#include "packetsink.h"

class DecodingThread
{
public:
	DecodingThread();
	bool Init(AVFormatContext *formatCtx);
	void ReadPacketsStep();
	void DeInit();
	bool IsAtEndOfMedia();
	void AddSink(unsigned int streamIndex, PacketSink &sink);
	void Seek(int64_t target);
private:
	template<class T>
	void ApplyOnAllSinks(T action);

	template<class T>
	void ApplyOnAllSinksBreakable(T action);

	template<class T>
	bool AnySink(T func);

	template<class T>
	bool AllSinks(T func);

	bool ArePacketsNeeded();
	void ThreadProc();
	void SendNullPacket(PacketSink &sink, int streamIndex);

	AVFormatContext *m_formatCtx;

	std::unordered_map<unsigned int, PacketSink*> m_sinks;

	AVFrame *m_decodedFrame;
	AVPacket m_demuxerPacket;
	std::atomic<bool> m_endOfMedia;

	std::atomic<bool> m_quit;
	std::unique_ptr<sf::Thread> m_thread;

	bool m_signaledEndOfMedia;
	std::atomic<bool> m_seeking;
	std::atomic<bool> m_seekRequest;
	std::atomic<int64_t> m_seekRequestTarget;
	bool m_looping;
};


#endif

