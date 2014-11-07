#ifndef H_VIDEORENDERERSTATS
#define H_VIDEORENDERERSTATS

extern "C"
{
#include <libavutil/avutil.h>
}


class VideoRenderer;

class VideoRendererStats
{
public:
	VideoRendererStats(VideoRenderer &renderer);
	const int &m_catchupFrames;
	const int &m_repeatFrames;
	const int &m_longFrames;
	const int &m_noSyncFrames;
	const AVPictureType &m_currentFrameType;
	const int &m_frameCounter;
	const int &m_excessiveDesync;
};

#endif
