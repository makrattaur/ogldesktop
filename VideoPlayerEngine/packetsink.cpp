#include "packetsink.h"
#include <Windows.h>
#include <iostream>


PacketSink::PacketSink() : m_quitThread(false), m_formatCtx(NULL), m_byteSize(0),
	m_endOfMedia(false), m_flushedFrames(false),
	m_streamIndex(0)
{
}

bool PacketSink::InitSink(AVFormatContext *formatCtx, unsigned int streamIndex)
{
	m_formatCtx = formatCtx;
	m_streamIndex = streamIndex;

	m_decodedFrame = av_frame_alloc();
	if(m_decodedFrame == NULL)
	{
		return false;
	}

	if(!Init(streamIndex))
	{
		return false;
	}

	av_init_packet(&m_nullPacket);
	m_nullPacket.data = NULL;
	m_nullPacket.size = 0;
	m_nullPacket.stream_index = streamIndex;
	m_codecCtx = formatCtx->streams[streamIndex]->codec;

	m_thread.reset(new sf::Thread(&PacketSink::ThreadProc, this));
	m_thread->launch();

	return true;
}

void PacketSink::QueuePacket(AVPacket &packet)
{
	AVPacket copy = packet;

	av_dup_packet(&copy);
	m_packetQueue.enqueue(copy);

	m_byteSize += copy.size;
}

void PacketSink::Quit()
{
	m_quitThread = true;
	m_thread->wait();
}

bool PacketSink::IsSinkFull()
{
	return !UseUnbufferedDecoding() ? (m_packetQueue.size() >= 3 || IsDecodedQueueFull()) : true;
}

int PacketSink::GetByteQueueSize()
{
	return m_byteSize;
}

void PacketSink::SignalEndOfMedia()
{
	m_endOfMedia = true;
}

bool PacketSink::AreFramesFlushed()
{
	return m_flushedFrames;
}

void PacketSink::OnSeek()
{
	m_endOfMedia = false;
	m_flushedFrames = false;

	m_flushedPackets = false;
	m_seekRequest = true;
}

bool PacketSink::SeekDone()
{
	return m_flushedPackets && IsSeekDone();
}

void PacketSink::ClearSeek()
{
	m_seekRequest = false;
	m_flushedPackets = false;
}

bool PacketSink::ArePacketsInQueue()
{
	return !m_packetQueue.empty();
}

bool PacketSink::DequeuePacket(AVPacket &packet)
{
	bool ret = m_packetQueue.tryDequeue(packet);

	m_byteSize -= packet.size;

	return ret;
}

bool PacketSink::ShouldQuit()
{
	return m_quitThread;
}

bool PacketSink::UseUnbufferedDecoding()
{
	return false;
}

void PacketSink::OnMediaSeeking()
{
}

void PacketSink::OnClearSeek()
{
}

bool PacketSink::IsSeekDone()
{
	return true;
}

void PacketSink::ThreadProc()
{
	while(!ShouldQuit())
	{
		while(!ShouldQuit() &&
			(!ArePacketsInQueue() || (IsDecodedQueueFull() && !UseUnbufferedDecoding())) &&
			!(m_endOfMedia && !m_flushedFrames) &&
			!(m_seekRequest && !m_flushedPackets)
		)
		{
			Sleep(10);
		}

		if(ShouldQuit())
		{
			break;
		}

		if(m_endOfMedia && !m_flushedFrames)
		{
			if(ArePacketsInQueue())
			{
				while(DequeuePacket(m_demuxerPacket))
				{
					bool hasFrame;
					int ret = DecodeStep(m_demuxerPacket, hasFrame);
				}
			}

			bool hasFrame = true;
			while(hasFrame)
			{
				int ret = DecodeStep(m_nullPacket, hasFrame);
			}

			m_flushedFrames = true;
			continue;
		}

		if(m_seekRequest && !m_flushedPackets)
		{
			OnMediaSeeking();

			if(ArePacketsInQueue())
			{
				while(DequeuePacket(m_demuxerPacket))
				{
					av_free_packet(&m_demuxerPacket);
				}
			}

			avcodec_flush_buffers(m_codecCtx);

			m_flushedPackets = true;
			continue;
		}

		if(!ArePacketsInQueue())
		{
			continue;
		}

		if(IsDecodedQueueFull() && !UseUnbufferedDecoding())
		{
			continue;
		}

		//std::cout << "Dequeue" << std::endl;
		DequeuePacket(m_demuxerPacket);
		bool hasFrame;
		int ret = DecodeStep(m_demuxerPacket, hasFrame);
	}
}

int PacketSink::DecodeStep(AVPacket &packet, bool &hasFrame)
{
	int ret = DecodeProc(packet, hasFrame);
	if(ret >= 0)
	{
		bool complete = ret == m_demuxerPacket.size;
		if(!complete)
		{
			//std::cerr << "Codec did not process whole packet !" << std::endl;
		}
	}
	else
	{
		std::cerr << "Error while decoding !" << std::endl;
	}
	av_free_packet(&packet);

	return ret;
}

