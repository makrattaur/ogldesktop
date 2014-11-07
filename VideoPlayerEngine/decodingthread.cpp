#include "decodingthread.h"

#include "ffutils.h"
#include <iostream>
#include <limits.h>


DecodingThread::DecodingThread()
	: m_endOfMedia(false), m_quit(false), m_signaledEndOfMedia(false),
	m_seeking(false), m_looping(true), m_seekRequest(false)
{
}

bool DecodingThread::Init(AVFormatContext *formatCtx)
{
	m_formatCtx = formatCtx;

	m_decodedFrame = av_frame_alloc();
	if(m_decodedFrame == NULL)
	{
		return false;
	}

	av_init_packet(&m_demuxerPacket);
	m_demuxerPacket.data = NULL;
	m_demuxerPacket.size = 0;

	m_thread.reset(new sf::Thread(&DecodingThread::ThreadProc, this));
	m_thread->launch();

	return true;
}

void DecodingThread::ReadPacketsStep()
{
	if(m_endOfMedia)
	{
		return;
	}

	int ret = av_read_frame(m_formatCtx, &m_demuxerPacket);

	if(ret >= 0)
	{
		if(m_sinks.count(m_demuxerPacket.stream_index) != 0)
		{
			m_sinks[m_demuxerPacket.stream_index]->QueuePacket(m_demuxerPacket);
		}
		else
		{
			av_free_packet(&m_demuxerPacket);
		}
	}
	else
	{
		m_endOfMedia = true;

#if 0
		// flush any remaining frames.
		SendNullPacket(*m_videoSink, m_videoStreamIndex);
		SendNullPacket(*m_audioSink, m_audioStreamIndex);
#endif
#if 0
		SendNullPacket(*m_videoSink, m_videoStreamIndex);
		SendNullPacket(*m_videoSink, m_videoStreamIndex);
		SendNullPacket(*m_videoSink, m_videoStreamIndex);
		SendNullPacket(*m_videoSink, m_videoStreamIndex);
		SendNullPacket(*m_audioSink, m_audioStreamIndex);
		SendNullPacket(*m_audioSink, m_audioStreamIndex);
		SendNullPacket(*m_audioSink, m_audioStreamIndex);
		SendNullPacket(*m_audioSink, m_audioStreamIndex);
#endif
	}
}

void DecodingThread::DeInit()
{
	m_quit = true;
	m_thread->wait();
	av_frame_free(&m_decodedFrame);
}

bool DecodingThread::IsAtEndOfMedia()
{
	return m_endOfMedia && AllSinks([](PacketSink *sink)
	{
		return sink->AreFramesFlushed();
	});
//	return m_endOfMedia;
}

void DecodingThread::AddSink(unsigned int streamIndex, PacketSink &sink)
{
	m_sinks.insert(std::make_pair(streamIndex, &sink));
}

void DecodingThread::Seek(int64_t target)
{
	if(!m_seeking)
	{
		m_seekRequestTarget = target;
		m_seekRequest = true;
	}
}

template<class T>
void DecodingThread::ApplyOnAllSinks(T action)
{
	for (auto it = m_sinks.begin(); it != m_sinks.end(); it++)
	{
		action(it->second);
	}
}

template<class T>
void DecodingThread::ApplyOnAllSinksBreakable(T action)
{
	for (auto it = m_sinks.begin(); it != m_sinks.end(); it++)
	{
		bool exit = action(it->second);
		if(!exit)
		{
			break;
		}
	}
}


template<class T>
bool DecodingThread::AnySink(T func)
{
	bool ret = false;

	ApplyOnAllSinksBreakable([&](PacketSink *sink)
	{
		bool lret = func(sink);
		if(lret)
		{
			ret = true;
			return false;
		}

		return true;
	});

	return ret;
}

template<class T>
bool DecodingThread::AllSinks(T func)
{
	bool ret = true;

	ApplyOnAllSinksBreakable([&](PacketSink *sink)
	{
		bool lret = func(sink);
		if(!lret)
		{
			ret = false;
			return false;
		}

		return true;
	});

	return ret;
}


bool DecodingThread::ArePacketsNeeded()
{
	return AnySink([](PacketSink *sink)
	{
		return !sink->IsSinkFull();
	});
}

void DecodingThread::ThreadProc()
{
	while(!m_quit)
	{
		if(ArePacketsNeeded() && !m_endOfMedia && !m_seeking)
		{
			//std::cout << "Read" << std::endl;
			ReadPacketsStep();
		}
#if 0
		else if(ArePacketsNeeded() && m_endOfMedia)
		{
			// flush any remaining frames.
			SendNullPacket(*m_videoSink, m_videoStreamIndex);
			SendNullPacket(*m_audioSink, m_audioStreamIndex);

			Sleep(10);
		}
#endif
#if 1
		else if(m_endOfMedia && !m_signaledEndOfMedia)
		{
			ApplyOnAllSinks([](PacketSink *sink)
			{
				sink->SignalEndOfMedia();
			});
			m_signaledEndOfMedia = true;
		}
#endif
		else if(m_seekRequest && !m_seeking)
		{
			//av_seek_frame(m_formatCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
			//av_seek_frame(m_formatCtx, -1, 0, 0);

			uint64_t targetTS = m_seekRequestTarget;
			int ret = avformat_seek_file(m_formatCtx, -1, _I64_MIN, targetTS, _I64_MAX, AVSEEK_FLAG_FRAME);
			//av_seek_frame(m_formatCtx, -1, m_seekRequestTarget, AVSEEK_FLAG_BACKWARD);

			ApplyOnAllSinks([](PacketSink *sink)
			{
				sink->OnSeek();
			});

			m_seeking = true;
			m_seekRequest = false;
		}
		else if(m_looping && m_signaledEndOfMedia && IsAtEndOfMedia())
		{
			Seek(0);
			//std::cout << "Looping" << std::endl;
		}
		else if(m_seeking && AllSinks([](PacketSink *sink)
			{
				return sink->SeekDone();
			})
		)
		{
			m_seeking = false;

			m_endOfMedia = false;
			m_signaledEndOfMedia = false;

			ApplyOnAllSinks([](PacketSink *sink)
			{
				sink->ClearSeek();
			});
			//std::cout << "Looped" << std::endl;
		}
		else
		{
			Sleep(10);
		}
	}
}

void DecodingThread::SendNullPacket(PacketSink &sink, int streamIndex)
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;
	packet.stream_index = streamIndex;

	sink.QueuePacket(packet);
}

