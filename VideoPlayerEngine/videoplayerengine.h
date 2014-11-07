#ifndef H_VIDEOPLAYERENGINE
#define H_VIDEOPLAYERENGINE

#include <engine.h>

extern "C"
{
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#define NOMINMAX
#include <Windows.h>
#include <GL/glew.h>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/System/Thread.hpp>

#include "autocomptr.h"
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <memory>
#include <atomic>
#include <autohandle.h>
#include <vector>
#include <threadsafequeue.h>
#include <fstream>
#include "textrenderer.h"
#include "types.h"
#include <circularhistory.h>
#include "objectpool.h"

#include "audiothread.h"
#include "decodingthread.h"
#include "audiodecodingthread.h"
#include "videodecodingthread.h"
#include "videorenderer.h"
#include "iclocksource.h"
#include "subtitlerenderer.h"
#include "subtitledecodingthread.h"


class VideoPlayerEngine : public Engine
{
public:
	VideoPlayerEngine();
	virtual bool Init();
	virtual void Update(float deltaTime);
	virtual void Draw();
	virtual void DeInit();

	virtual int PreferredWidth();
	virtual int PreferredHeight();
	virtual const char *PreferredTitle();
protected:
	virtual void OnKeyDown(KeyboardKey key);
	virtual void OnMouseButtonDown(MouseButton button, int x, int y);
private:
	class MasterClock : public IClockSource
	{
	public:
		MasterClock(VideoPlayerEngine &parent);
		virtual double GetPosition();
	private:
		VideoPlayerEngine &m_parent;
	};

	void InitOpenGL();

	bool InitFFmpeg();
	void OpenAvailableStreams();
	void InitVideo();
	void InitAudio();
	void InitSubtitle();
	bool InitDemuxer();
	void DrawTexturedQuad();
	void DeInitFFmpeg();

	void DrawPlaybackStats();
	void InitMediaInfo();
	void DrawMediaInfo();

	static void Color4ubPM(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

	double Average(CircularHistory<double> &history);

	void DrawMarker(float mPos, float yPos, float markerSize = 10.0f);

	AVFormatContext *m_formatCtx;
	MasterClock m_masterClock;
	DecodingThread m_decodingThread;

	VideoDecodingThread m_videoSink;
	AudioDecodingThread m_audioSink;
	SubtitleDecodingThread m_subtitleSink;

	bool m_useSubtitle;
	bool m_useAudio;

	bool m_audioStreamAvailable;
	unsigned int m_audioStreamIndex;
	AVCodecContext *m_audioCodecCtx;
	AudioThread m_audioThread;
	bool m_audioStarted;

	bool m_videoStreamAvailable;
	unsigned int m_videoStreamIndex;
	AVCodecContext *m_videoCodecCtx;
	VideoRenderer m_videoRenderer;
	std::unique_ptr<VideoRendererStats> m_videoRendererStats;

	sf::Font m_font;
	TextRenderer m_textRenderer;
	float m_currentDeltaTime;

	int m_currentVideoQueueSize;
	int m_currentAudioQueueSize;

	double m_monotonicCounter;
	double m_currentDevicePos;
	double m_currentVideoPos;
	double m_currentAudioPos;

	std::string m_mediaInfoVideo;
	std::string m_mediaInfoAudio;
	std::string m_mediaInfoContainer;

	CircularHistory<double> m_diffDeviceHistory;
	CircularHistory<double> m_diffAudioHistory;

	double m_currentMasterClockPos;
	CircularHistory<double> m_diffClockHistory;

	bool m_wireframe;

	double m_currentSyncDiff;
	float m_syncRefreshTimer;

	bool m_subtitleStreamAvailable;
	unsigned int m_subtitleStreamIndex;
	AVCodecContext *m_subtitleCodecCtx;
	SubtitleRenderer m_subtitleRenderer;

	bool m_useScale;
	bool m_autoScale;
	bool m_autoScaleMode;
	bool m_autoScaleSmall;

	float m_xScalingFactor;
	float m_yScalingFactor;

	ObjectPool<QueuedSample> m_audioPool;
	ObjectPool<QueuedVideoFrame> m_videoPool;
};


#endif

