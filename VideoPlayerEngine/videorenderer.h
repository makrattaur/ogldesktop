#ifndef H_VIDEORENDERER
#define H_VIDEORENDERER

extern "C"
{
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#define NOMINMAX
#include <Windows.h>
#include <GL/glew.h>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <memory>
#include <atomic>
#include <threadsafequeue.h>
#include <circularhistory.h>
#include "objectpool.h"

#include "types.h"
#include "iclocksource.h"
#include "ivideoqueue.h"

class IEngineContext;


class VideoRenderer : public IClockSource, public IVideoQueue
{
public:
	VideoRenderer();
	bool Init(AVFormatContext *formatCtx, unsigned int streamIndex);
	void DeInit();
	void SetMasterClock(IClockSource &source);
	void Draw();
	void Update(float deltaTime);
	virtual double GetPosition();
	int64_t GetCurrentPTS();
	virtual void QueueFrame(QueuedVideoFrame *frame);
	virtual size_t FrameQueueSize();
	virtual void SetSeek();
	virtual void ClearSeek();
	virtual bool AllFramesDisplayed();
	int GetFrameWidth();
	int GetFrameHeight();
	void DumpNextFrame();
	bool GetUnderflowState();
	void SetUseScalableYCbCrPath(bool state);
	void SetEngineContext(IEngineContext *engineContext);
	void SetObjectPool(ObjectPool<QueuedVideoFrame> &pool);
private:
	friend class VideoRendererStats;
	void InitShader();
	void BindTexturesToUnits();

	bool InitVideo();
	void InitVideoFrame(AVCodecContext *videoCodecCtx);
	void UpdateVideoFrameYCbCr(QueuedVideoFrame *frame);
	void UpdateVideoFrameRGBA(QueuedVideoFrame *frame);
	void UpdateVideoFrame(QueuedVideoFrame *frame);
	void DrawTexturedQuad();
	void DrawTexturedQuad2();
	void CheckFrameRefresh(float deltaTime);

	void CreatePlaneTexture(GLuint &texture, int width, int height);
	void CreateRGBATexture(GLuint &texture, int width, int height);

	void DumpFrame(QueuedVideoFrame *frame);

	int m_frameWidth;
	int m_frameHeight;

	float m_displayFrameWidth;
	float m_displayFrameHeight;

	int m_sampledChromaWidth;
	int m_sampledChromaHeight;
	GLuint m_textureY;
	GLuint m_textureCb;
	GLuint m_textureCr;
	GLuint m_textureRGBA;

	sf::Shader m_shader;
	//GLhandleARB m_program;

	float m_frameTimer;
	float m_frameInterval;
	float m_nextFrameInterval;

	int m_catchupFrames;
	int m_repeatFrames;
	int m_longFrames;
	int m_noSyncFrames;
	AVPictureType m_currentFrameType;
	int m_frameCounter;
	int m_excessiveDesync;

	uint64_t m_currentVideoPTS;
	double m_currentVideoPos;

	ThreadSafeQueue<QueuedVideoFrame *> m_videoFrameQueue;

	double m_currentSyncDiff;
	float m_syncRefreshTimer;

	AVFormatContext *m_formatCtx;

	unsigned int m_videoStreamIndex;
	AVCodecContext *m_videoCodecCtx;

	bool m_dumpFrame;
	IClockSource *m_masterClockSource;

	bool m_frameUnderflowing;

	bool m_useYCbCrPath;
	bool m_useScalableYCbCrPath;
	bool m_initialized;

	sf::RenderTexture m_renderTargetYCbCr;
	IEngineContext *m_engineContext;

	ObjectPool<QueuedVideoFrame> *m_objectPool;

	std::atomic<bool> m_seeking;
	std::atomic<bool> m_allFramesDisplayed;
};

#endif
