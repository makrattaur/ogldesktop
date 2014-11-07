#include "audiodecodingthread.h"
#include "ffutils.h"
#include <iostream>


bool AudioDecodingThread::Init(int streamIndex)
{
	m_audioCodecCtx = m_formatCtx->streams[streamIndex]->codec;

	bool delayCap = (m_audioCodecCtx->codec->capabilities & CODEC_CAP_DELAY) == CODEC_CAP_DELAY;
	bool subframeCap = (m_audioCodecCtx->codec->capabilities & CODEC_CAP_SUBFRAMES) == CODEC_CAP_SUBFRAMES;

	return true;
}

bool AudioDecodingThread::IsDecodedQueueFull()
{
	return m_audioThread->GetSampleQueueSize() >= 2;
}

void AudioDecodingThread::SetAudioThread(AudioThread &audioThread)
{
	m_audioThread = &audioThread;
}

int AudioDecodingThread::DecodeProc(AVPacket &packet, bool &hasFrame)
{
	int iHasFrame;
	int aret = avcodec_decode_audio4(m_audioCodecCtx, m_decodedFrame, &iHasFrame, &packet);
			
	bool complete = aret == packet.size;
	if(!complete)
	{
		std::cerr << "Audio codec did not process whole packet !" << std::endl;
	}

	hasFrame = iHasFrame != 0;
	if(hasFrame)
	{
		m_audioThread->ResampleAndQueueSample(m_decodedFrame, packet.pts);
	}

	return aret;
}

