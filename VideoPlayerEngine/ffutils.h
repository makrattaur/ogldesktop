#ifndef H_FFUTILS
#define H_FFUTILS

extern "C"
{
#include <libavutil/pixfmt.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


namespace ffutil
{
	bool OpenFirstMatchingCodecContext(AVFormatContext *formatCtx, AVMediaType type, unsigned int &streamIndex);
	bool GetFirstMatchingStream(AVFormatContext *formatCtx, AVMediaType type, unsigned int &streamIndex);
	const char *PixelFormatToString(AVPixelFormat fmt);
	const char *SampleFormatToString(AVSampleFormat fmt);
	const char *CodecIdToString(AVCodecID id);
	const char *ChannelLayoutToString(uint64_t channelLayout);
}


#endif

