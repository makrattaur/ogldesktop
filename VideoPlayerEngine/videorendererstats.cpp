#include "videorendererstats.h"
#include "videorenderer.h"


VideoRendererStats::VideoRendererStats(VideoRenderer &renderer)
	: m_catchupFrames(renderer.m_catchupFrames),
	m_repeatFrames(renderer.m_repeatFrames),
	m_longFrames(renderer.m_longFrames),
	m_noSyncFrames(renderer.m_noSyncFrames),
	m_currentFrameType(renderer.m_currentFrameType),
	m_frameCounter(renderer.m_frameCounter),
	m_excessiveDesync(renderer.m_excessiveDesync)
{
}

