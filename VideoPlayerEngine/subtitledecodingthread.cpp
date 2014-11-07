#include "subtitledecodingthread.h"
#include "ffutils.h"
#include "isubtitlequeue.h"
#include <iostream>


bool SubtitleDecodingThread::Init(int streamIndex)
{
	m_subtitleCodecCtx = m_formatCtx->streams[streamIndex]->codec;
	ResetSubtitile(&m_subtitle);

	return true;
}

bool SubtitleDecodingThread::IsDecodedQueueFull()
{
//	return m_subtitleQueue->SubtitleQueueSize() >= 1;
	return true;
}

void SubtitleDecodingThread::SetSubtitleQueue(ISubtitleQueue &queue)
{
	m_subtitleQueue = &queue;
}

int SubtitleDecodingThread::DecodeProc(AVPacket &packet, bool &hasFrame)
{
	int iHasFrame;
	int sret = avcodec_decode_subtitle2(m_subtitleCodecCtx, &m_subtitle, &iHasFrame, &packet);
			
	bool complete = sret == packet.size;
	if(!complete)
	{
		std::cerr << "Subtitle codec did not process whole packet !" << std::endl;
	}

	hasFrame = iHasFrame != 0;
	if(hasFrame)
	{
		if(m_subtitle.format == 1 &&
			m_subtitle.num_rects == 1)
		{
			auto queuedSubtitle = std::unique_ptr<QueuedSubtitle>(new QueuedSubtitle());
			queuedSubtitle->PTS = m_subtitle.pts;
			queuedSubtitle->Delay = m_subtitle.start_display_time;
			queuedSubtitle->Duration = m_subtitle.end_display_time - m_subtitle.start_display_time;
			queuedSubtitle->String = std::string(m_subtitle.rects[0]->ass);

			m_subtitleQueue->QueueSubtitle(std::move(queuedSubtitle));
		}

		avsubtitle_free(&m_subtitle);
	}

	return sret;
}

bool SubtitleDecodingThread::UseUnbufferedDecoding()
{
	return true;
}

void SubtitleDecodingThread::OnMediaSeeking()
{
	m_subtitleQueue->OnSeek();
}

void SubtitleDecodingThread::ResetSubtitile(AVSubtitle *sub)
{
	memset(sub, 0, sizeof(*sub));
	sub->pts = AV_NOPTS_VALUE;
}
