#include "videodecodingthread.h"
#include "ffutils.h"
#include <iostream>
extern "C"
{
#include <libavutil/imgutils.h>
}
#include "ivideoqueue.h"


bool VideoDecodingThread::Init(int streamIndex)
{
	m_videoStreamIndex = streamIndex;
	m_videoCodecCtx = m_formatCtx->streams[m_videoStreamIndex]->codec;

	if(m_videoCodecCtx->pix_fmt != PIX_FMT_YUV420P &&
		m_videoCodecCtx->pix_fmt != PIX_FMT_BGRA)
	{
		return false;
	}

	m_useYCbCrPath = m_videoCodecCtx->pix_fmt == PIX_FMT_YUV420P;

	bool delayCap = (m_videoCodecCtx->codec->capabilities & CODEC_CAP_DELAY) == CODEC_CAP_DELAY;
	bool subframeCap = (m_videoCodecCtx->codec->capabilities & CODEC_CAP_SUBFRAMES) == CODEC_CAP_SUBFRAMES;

	m_frameWidth = m_videoCodecCtx->width;
	m_frameHeight = m_videoCodecCtx->height;
	m_sampledChromaWidth = m_frameWidth / 2;
	m_sampledChromaHeight = m_frameHeight / 2;

	return true;
}

bool VideoDecodingThread::IsDecodedQueueFull()
{
	return m_videoFrameQueue->FrameQueueSize() >= 2;
}

void VideoDecodingThread::SetVideoFrameQueue(IVideoQueue &sink)
{
	m_videoFrameQueue = &sink;
}

void VideoDecodingThread::SetObjectPool(ObjectPool<QueuedVideoFrame> &pool)
{
	m_objectPool = &pool;
}

int VideoDecodingThread::DecodeProc(AVPacket &packet, bool &hasFrame)
{
	int iHasFrame;
	int vret = avcodec_decode_video2(m_videoCodecCtx, m_decodedFrame, &iHasFrame, &packet);

	bool complete = vret == packet.size;
	if(!complete)
	{
		//std::cerr << "Video codec did not process whole packet !" << std::endl;
	}

	hasFrame = iHasFrame != 0;
	if(hasFrame)
	{
		//std::cout << "Take" << std::endl;
		auto queuedFrame = m_objectPool->GetInstance();
		queuedFrame->Type = m_decodedFrame->pict_type;
		//queuedFrame->QP = m_decodedFrame->quality / (float)FF_QP2LAMBDA;
		queuedFrame->PTS = av_frame_get_best_effort_timestamp(m_decodedFrame);
		queuedFrame->Duration = av_frame_get_pkt_duration(m_decodedFrame);

		if(m_useYCbCrPath)
		{
			if(!queuedFrame->DataAllocated)
			{
				queuedFrame->PlaneYStride = m_decodedFrame->linesize[0];
				queuedFrame->PlaneCbStride = m_decodedFrame->linesize[1];
				queuedFrame->PlaneCrStride = m_decodedFrame->linesize[2];

				queuedFrame->PlaneY.resize(queuedFrame->PlaneYStride * m_frameHeight);
				queuedFrame->PlaneCb.resize(queuedFrame->PlaneCbStride * m_sampledChromaHeight);
				queuedFrame->PlaneCr.resize(queuedFrame->PlaneCrStride * m_sampledChromaHeight);

				queuedFrame->DataAllocated = true;
			}

			if(queuedFrame->PlaneYStride != m_decodedFrame->linesize[0] ||
				queuedFrame->PlaneCbStride != m_decodedFrame->linesize[1] ||
				queuedFrame->PlaneCrStride != m_decodedFrame->linesize[2]
			)
			{
				__debugbreak();
			}

			memcpy(queuedFrame->PlaneY.data(), m_decodedFrame->data[0], queuedFrame->PlaneY.size());
			memcpy(queuedFrame->PlaneCb.data(), m_decodedFrame->data[1], queuedFrame->PlaneCb.size());
			memcpy(queuedFrame->PlaneCr.data(), m_decodedFrame->data[2], queuedFrame->PlaneCr.size());
		}
		else
		{
			if(!queuedFrame->DataAllocated)
			{
				queuedFrame->RGBAStride = m_decodedFrame->linesize[0];
				queuedFrame->RGBABytes.resize(queuedFrame->RGBAStride * m_frameHeight);

				queuedFrame->DataAllocated = true;
			}

			if(queuedFrame->RGBAStride != m_decodedFrame->linesize[0])
			{
				__debugbreak();
			}


			memcpy(queuedFrame->RGBABytes.data(), m_decodedFrame->data[0], queuedFrame->RGBABytes.size());
		}

		m_videoFrameQueue->QueueFrame(queuedFrame);
	}

	return vret;
}

void VideoDecodingThread::OnMediaSeeking()
{
	m_videoFrameQueue->SetSeek();
}

void VideoDecodingThread::OnClearSeek()
{
	m_videoFrameQueue->ClearSeek();
}

bool VideoDecodingThread::IsSeekDone()
{
	return m_videoFrameQueue->AllFramesDisplayed();
}

void VideoDecodingThread::Transform10b16b(std::vector<uint8_t> &plane, int planeStride, int width, int height)
{
	for(int y = 0; y < height; y++)
	{
		uint16_t *line = (uint16_t *)(((char *)plane.data()) + planeStride * y);
		for(int x = 0; x < width; x++)
		{
			line[x] = (uint16_t)((double)line[x] * 65535.0 / 1023.0);
		}
	}
}

